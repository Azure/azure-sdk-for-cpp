// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/job_engine.hpp"

#include <algorithm>
#include <vector>

#include <azure/core/azure_assert.hpp>

#include "azure/storage/blobs/blob_options.hpp"
#include "azure/storage/datamovement/tasks/download_blob_to_file_task.hpp"
#include "azure/storage/datamovement/tasks/upload_blob_from_file_task.hpp"
#include "azure/storage/datamovement/utilities.hpp"

namespace Azure { namespace Storage { namespace _detail {

  namespace {
    constexpr size_t g_UploadBlockSize = 8 * 1024 * 1024;
    constexpr size_t g_DownloadBlockSize = 8 * 1024 * 1024;
    constexpr size_t g_NumSubtasksPerPart = 50000;
    constexpr size_t g_MaxTasksGenerated = 1000000;
    constexpr int32_t g_BlobListPageSize = 250;

    std::string GetParentDir(const std::string& blobPath)
    {
      auto pos = blobPath.find_last_of("/\\");
      if (std::string::npos != pos)
      {
        return blobPath.substr(0, pos);
      }

      return std::string();
    }

    void CreateDirectoryIfNotExists(const std::string& dirPath)
    {
      if (!_internal::IsDirectory(dirPath))
      {
        const auto parent = GetParentDir(dirPath);
        if (!parent.empty())
        {
          CreateDirectoryIfNotExists(parent);
        }

        _internal::CreateDirectory(dirPath);
      }
    }
  } // namespace

  void JobPlan::GeneratePartImpl(const PartGeneratorModel& gen)
  {
    std::vector<TaskModel> newTasks;
    size_t numNewSubtasks = 0;
    size_t totalNumNewSubtasks = 0;
    auto flushNewTasks = [&]() {
      if (newTasks.empty())
      {
        return;
      }
      const uint32_t partId = ++m_maxPartId;
      JobPlan::CreateJobPart(partId, m_jobPlanDir, newTasks);
      m_jobParts[partId] = nullptr;
      newTasks.clear();
      numNewSubtasks = 0;
    };
    auto taskGenerated = [&](TaskModel&& newTask) {
      AZURE_ASSERT(newTask.NumSubtasks > 0);
      numNewSubtasks += newTask.NumSubtasks;
      totalNumNewSubtasks += newTask.NumSubtasks;
      newTasks.push_back(std::move(newTask));
      if (numNewSubtasks >= g_NumSubtasksPerPart)
      {
        flushNewTasks();
      }
    };
    std::vector<PartGeneratorModel> partGens;

    if (m_model.Source.m_type == _internal::TransferEnd::EndType::LocalFile)
    {
      AZURE_ASSERT(gen.Source.empty());
      AZURE_ASSERT(gen.Destination.empty());
      AZURE_ASSERT(gen.ContinuationToken.empty());
      const std::string filePath = _internal::PathFromUrl(m_model.Source.m_url);
      int64_t fileSize = _internal::GetFileSize(filePath);

      TaskModel task;
      task.ObjectSize = fileSize;
      task.ChunkSize = g_UploadBlockSize;
      task.NumSubtasks
          = static_cast<int32_t>((fileSize + g_UploadBlockSize - 1) / g_UploadBlockSize);
      task.NumSubtasks = std::max(task.NumSubtasks, 1);
      taskGenerated(std::move(task));
    }
    else if (m_model.Source.m_type == _internal::TransferEnd::EndType::LocalDirectory)
    {
      AZURE_ASSERT(gen.ContinuationToken.empty());
      const std::string jobRootPath = _internal::PathFromUrl(m_model.Source.m_url);

      partGens.push_back(gen);
      do
      {
        PartGeneratorModel currGen = std::move(partGens.back());
        partGens.pop_back();

        _internal::DirectoryIterator dirIterator(_internal::JoinPath(jobRootPath, currGen.Source));
        while (true)
        {
          auto entry = dirIterator.Next();
          if (entry.Name.empty())
          {
            break;
          }
          if (!entry.IsDirectory)
          {
            TaskModel task;
            task.Source = _internal::JoinPath(currGen.Source, entry.Name);
            task.Destination = _internal::JoinPath(currGen.Destination, entry.Name);
            task.ObjectSize = entry.Size;
            task.ChunkSize = g_UploadBlockSize;
            task.NumSubtasks
                = static_cast<int32_t>((entry.Size + g_UploadBlockSize - 1) / g_UploadBlockSize);
            task.NumSubtasks = std::max(task.NumSubtasks, 1);
            taskGenerated(std::move(task));
          }
          else
          {
            PartGeneratorModel newGen;
            newGen.Source = _internal::JoinPath(currGen.Source, entry.Name);
            newGen.Destination = _internal::JoinPath(currGen.Destination, entry.Name);
            partGens.push_back(std::move(newGen));
          }
        }

      } while (!partGens.empty() && totalNumNewSubtasks < g_MaxTasksGenerated);
    }
    else if (m_model.Source.m_type == _internal::TransferEnd::EndType::AzureBlob)
    {
      AZURE_ASSERT(gen.Source.empty());
      AZURE_ASSERT(gen.Destination.empty());
      AZURE_ASSERT(gen.ContinuationToken.empty());
      // TODO: It's not a good idea to invoke a network request when generating tasks, as this
      // function is executed in single thread so it's likely to become a bottleneck. Find some way
      // to optimize this. This also applies to downloading directory, downloading page blob etc.
      int64_t fileSize = m_model.Source.m_blobClient.Value().GetProperties().Value.BlobSize;

      TaskModel task;
      task.ObjectSize = fileSize;
      task.ChunkSize = g_DownloadBlockSize;
      task.NumSubtasks
          = static_cast<int32_t>((fileSize + g_UploadBlockSize - 1) / g_DownloadBlockSize);
      task.NumSubtasks = std::max(task.NumSubtasks, 1);
      taskGenerated(std::move(task));
    }
    else if (m_model.Source.m_type == _internal::TransferEnd::EndType::AzureBlobFolder)
    {
      const std::string rootDirectory = _internal::PathFromUrl(m_model.Destination.m_url);
      std::string currentDirectory = rootDirectory;

      partGens.push_back(gen);
      CreateDirectoryIfNotExists(currentDirectory);

      do
      {
        PartGeneratorModel currGen = std::move(partGens.back());
        partGens.pop_back();

        Blobs::ListBlobsOptions options;
        std::string prefix = m_model.Source.m_blobFolder.Value().m_folderPath;
        if (!prefix.empty() && (prefix.back() != '/'))
        {
          prefix.append("/");
        }

        options.Prefix = std::move(prefix);
        options.PageSizeHint = g_BlobListPageSize;
        options.ContinuationToken = std::move(currGen.ContinuationToken);

        std::vector<_internal::Task> subtasks;
        auto result = m_model.Source.m_blobFolder.Value().m_blobContainerClient.ListBlobs(options);
        for (auto blobItem : result.Blobs)
        {
          TaskModel task;
          const std::string blobName = blobItem.Name.substr(options.Prefix->length());
          std::string localFileName = blobName;

          std::string parentDir = _internal::JoinPath(rootDirectory, GetParentDir(localFileName));
          if (parentDir != currentDirectory)
          {
            CreateDirectoryIfNotExists(parentDir); // TODO: error handling
            currentDirectory = std::move(parentDir);
          }

          task.Source = _internal::JoinPath(currGen.Source, blobName);
          task.Destination = _internal::JoinPath(currGen.Destination, localFileName);
          task.ObjectSize = blobItem.BlobSize;
          task.ChunkSize = g_DownloadBlockSize;
          task.NumSubtasks = static_cast<int32_t>(
              (blobItem.BlobSize + g_DownloadBlockSize - 1) / g_DownloadBlockSize);
          task.NumSubtasks = std::max(task.NumSubtasks, 1);
          taskGenerated(std::move(task));
        }

        if (result.NextPageToken.HasValue())
        {
          currGen.ContinuationToken = std::move(result.NextPageToken.Value());
          partGens.push_back(std::move(currGen));
        }
      } while (!partGens.empty() && totalNumNewSubtasks < g_MaxTasksGenerated);
    }
    else
    {
      AZURE_NOT_IMPLEMENTED();
    }

    flushNewTasks();
    std::reverse(partGens.begin(), partGens.end());
    AppendPartGenerators(partGens);
  }

  std::vector<_internal::Task> JobPlan::HydrateTasks(
      std::shared_ptr<JobPart>& jobPart,
      const std::vector<TaskModel>& taskModels)
  {
    size_t bitmapOffset = 0;
    std::vector<_internal::Task> tasks;

    if ((m_model.Source.m_type == _internal::TransferEnd::EndType::LocalFile
         && m_model.Destination.m_type == _internal::TransferEnd::EndType::AzureBlob)
        || (m_model.Source.m_type == _internal::TransferEnd::EndType::LocalDirectory
            && m_model.Destination.m_type == _internal::TransferEnd::EndType::AzureBlobFolder))
    {
      for (auto& taskModel : taskModels)
      {
        std::string source
            = _internal::JoinPath(_internal::PathFromUrl(m_model.Source.m_url), taskModel.Source);
        auto destination = m_model.Destination.m_type == _internal::TransferEnd::EndType::AzureBlob
            ? m_model.Destination.m_blobClient.Value()
            : m_model.Destination.m_blobFolder.Value().GetBlobClient(taskModel.Destination);
        if (taskModel.NumSubtasks == 1)
        {
          auto task = m_rootTask->CreateTask<Blobs::_detail::UploadBlobFromFileTask>(
              _internal::TaskType::NetworkUpload, source, destination);
          task->MemoryCost = static_cast<size_t>(taskModel.ObjectSize);
          task->JournalContext = JournalContext{jobPart, bitmapOffset};
          tasks.emplace_back(std::move(task));
          ++bitmapOffset;
        }
        else if (taskModel.NumSubtasks > 1)
        {
          auto context = std::make_shared<Blobs::_detail::ReadFileRangeToMemoryTask::TaskContext>(
              source, destination);
          context->FileSize = taskModel.ObjectSize;
          context->NumBlocks = static_cast<int>(
              (taskModel.ObjectSize + taskModel.ChunkSize - 1) / taskModel.ChunkSize);
          context->NumStagedBlocks = 0;
          std::string doneBits;

          if (taskModel.ExtendedAttributes.count("_subtasks") != 0)
          {
            doneBits = taskModel.ExtendedAttributes.at("_subtasks");
          }
          for (int i = 0; i < context->NumBlocks; ++i)
          {
            if (!doneBits.empty() && doneBits[i] != '0')
            {
              context->NumStagedBlocks.fetch_add(1, std::memory_order_relaxed);
              ++bitmapOffset;
              continue;
            }
            auto task = m_rootTask->CreateTask<Blobs::_detail::ReadFileRangeToMemoryTask>(
                _internal::TaskType::DiskIO);
            task->Context = context;
            task->BlockId = i;
            task->Offset = i * taskModel.ChunkSize;
            task->Length = static_cast<size_t>(
                std::min<uint64_t>(context->FileSize - task->Offset, taskModel.ChunkSize));
            task->MemoryCost = task->Length;
            task->JournalContext = JournalContext{jobPart, bitmapOffset};
            tasks.emplace_back(std::move(task));
            ++bitmapOffset;
          }
        }
      }
    }
    else if (
        (m_model.Source.m_type == _internal::TransferEnd::EndType::AzureBlob
         && m_model.Destination.m_type == _internal::TransferEnd::EndType::LocalFile)
        || (m_model.Source.m_type == _internal::TransferEnd::EndType::AzureBlobFolder
            && m_model.Destination.m_type == _internal::TransferEnd::EndType::LocalDirectory))
    {
      for (auto& taskModel : taskModels)
      {
        auto source = m_model.Source.m_type == _internal::TransferEnd::EndType::AzureBlob
            ? m_model.Source.m_blobClient.Value()
            : m_model.Source.m_blobFolder.Value().GetBlobClient(taskModel.Source);
        std::string destination = _internal::JoinPath(
            _internal::PathFromUrl(m_model.Destination.m_url), taskModel.Destination);

        auto context = std::make_shared<Blobs::_detail::DownloadRangeToMemoryTask::TaskContext>(
            source, destination);
        context->FileSize = taskModel.ObjectSize;
        context->NumChunks = static_cast<int>(
            (taskModel.ObjectSize + taskModel.ChunkSize - 1) / taskModel.ChunkSize);
        context->NumChunks = std::max(context->NumChunks, 1);

        context->NumDownloadedChunks = 0;
        std::string doneBits;

        if (taskModel.ExtendedAttributes.count("_subtasks") != 0)
        {
          doneBits = taskModel.ExtendedAttributes.at("_subtasks");
        }
        for (int i = 0; i < context->NumChunks; ++i)
        {
          if (!doneBits.empty() && doneBits[i] != '0')
          {
            context->NumDownloadedChunks.fetch_add(1, std::memory_order_relaxed);
            ++bitmapOffset;
            continue;
          }
          auto task = m_rootTask->CreateTask<Blobs::_detail::DownloadRangeToMemoryTask>(
              _internal::TaskType::NetworkDownload);
          task->Context = context;
          task->Offset = i * taskModel.ChunkSize;
          task->Length = static_cast<size_t>(
              std::min<uint64_t>(context->FileSize - task->Offset, taskModel.ChunkSize));
          task->MemoryCost = task->Length;
          task->JournalContext = JournalContext{jobPart, bitmapOffset};
          tasks.emplace_back(std::move(task));
          ++bitmapOffset;
        }
      }
    }
    else
    {
      AZURE_NOT_IMPLEMENTED();
    }
    return tasks;
  }

}}} // namespace Azure::Storage::_detail
