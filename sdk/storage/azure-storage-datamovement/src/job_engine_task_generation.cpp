// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/job_engine.hpp"

#include <algorithm>
#include <vector>

#include <azure/core/azure_assert.hpp>
#include <azure/core/internal/json/json.hpp>

#include "azure/storage/blobs/blob_options.hpp"
#include "azure/storage/datamovement/tasks/async_copy_blob_task.hpp"
#include "azure/storage/datamovement/tasks/download_blob_to_file_task.hpp"
#include "azure/storage/datamovement/tasks/download_page_blob_to_file_task.hpp"
#include "azure/storage/datamovement/tasks/upload_blob_from_file_task.hpp"
#include "azure/storage/datamovement/utilities.hpp"

namespace Azure { namespace Storage { namespace _detail {

  namespace {
    constexpr size_t g_UploadBlockSize = 8 * 1024 * 1024;
    constexpr size_t g_DownloadBlockSize = 8 * 1024 * 1024;
    constexpr size_t g_NumSubtasksPerPart = 50000;
    constexpr size_t g_MaxTasksGenerated = 1000000;
    constexpr int32_t g_ListBlobsPageSize = 250;

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
      if (_internal::IsDirectory(dirPath))
      {
        return;
      }
      // TODO: need decide what to do when path conflicts with file
      const auto parent = GetParentDir(dirPath);
      if (!parent.empty())
      {
        CreateDirectoryIfNotExists(parent);
      }

      _internal::CreateDirectory(dirPath);
    }

    void PageBlobTaskDecoration(TaskModel& task, const Blobs::PageBlobClient& pageBlobClient)
    {
      auto subtasksDescription = Core::Json::_internal::json::array();

      std::vector<Azure::Core::Http::HttpRange> subtaskRanges;
      int64_t subtaskDownloadSize = 0;
      int64_t totalDownloadSize = 0;

      auto flushSubtask = [&]() {
        auto description = Core::Json::_internal::json::array();
        for (const auto& range : subtaskRanges)
        {
          description.push_back(range.Offset);
          description.push_back(range.Length.Value());
        }
        subtaskRanges.clear();
        if (!description.empty())
        {
          subtasksDescription.push_back(std::move(description));
        }
        totalDownloadSize += subtaskDownloadSize;
        subtaskDownloadSize = 0;
      };
      std::function<void(const Azure::Core::Http::HttpRange&)> addRange;
      addRange = [&](const Azure::Core::Http::HttpRange& range) {
        constexpr int64_t MergeThreshold = 512;
        AZURE_ASSERT(range.Length.Value() > 0);
        int64_t distance = subtaskRanges.empty()
            ? std::numeric_limits<int64_t>::max()
            : range.Offset - (subtaskRanges.back().Offset + subtaskRanges.back().Length.Value());

        if (range.Length.Value() > task.ChunkSize)
        {
          if (distance > 0 && distance <= MergeThreshold
              && task.ChunkSize - subtaskDownloadSize > distance)
          {
            Azure::Core::Http::HttpRange newRange;
            newRange.Offset = range.Offset - distance;
            newRange.Length = distance + range.Length.Value();
            addRange(newRange);
          }
          else if (task.ChunkSize - subtaskDownloadSize > 0)
          {
            Azure::Core::Http::HttpRange newRange;
            newRange.Offset = range.Offset;
            newRange.Length = task.ChunkSize - subtaskDownloadSize;
            addRange(newRange);
            Azure::Core::Http::HttpRange newRange2;
            newRange2.Offset = range.Offset + newRange.Length.Value();
            newRange2.Length = range.Length.Value() - newRange.Length.Value();
            addRange(newRange2);
          }
          else
          {
            flushSubtask();
            addRange(range);
          }
        }
        else
        { // range.Length.Value() <= task.ChunkSize
          if (distance <= MergeThreshold
              && subtaskDownloadSize + distance + range.Length.Value() <= task.ChunkSize)
          {
            subtaskRanges.back().Length.Value() += distance + range.Length.Value();
            subtaskDownloadSize += distance + range.Length.Value();
          }
          else if (
              distance > MergeThreshold
              && subtaskDownloadSize + range.Length.Value() <= task.ChunkSize)
          {
            subtaskRanges.push_back(range);
            subtaskDownloadSize += range.Length.Value();
          }
          else
          {
            flushSubtask();
            addRange(range);
          }
        }
      };

      for (auto rangePage = pageBlobClient.GetPageRanges(); rangePage.HasPage();
           rangePage.MoveToNextPage())
      {
        for (const auto& range : rangePage.PageRanges)
        {
          addRange(range);
        }
      }
      flushSubtask();

      if (totalDownloadSize < task.ObjectSize)
      {
        if (subtasksDescription.empty())
        {
          subtasksDescription.push_back(Core::Json::_internal::json::array());
        }
        task.ExtendedAttributes["page_ranges"] = subtasksDescription.dump();
        task.NumSubtasks = static_cast<int32_t>(subtasksDescription.size());
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

    const auto transferType = _internal::JobModel::DeduceTransferType(m_model);
    if (transferType == TransferType::SingleUpload)
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
    else if (transferType == TransferType::DirectoryUpload)
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
    else if (transferType == TransferType::SingleDownload)
    {
      AZURE_ASSERT(gen.Source.empty());
      AZURE_ASSERT(gen.Destination.empty());
      AZURE_ASSERT(gen.ContinuationToken.empty());
      // TODO: It's not a good idea to invoke a network request when generating tasks, as this
      // function is executed in single thread so it's likely to become a bottleneck. Find some way
      // to optimize this. This also applies to downloading directory, downloading page blob etc.
      auto blobProperties = m_model.Source.m_blobClient.Value().GetProperties().Value;
      int64_t fileSize = blobProperties.BlobSize;

      TaskModel task;
      task.ObjectSize = fileSize;
      task.ChunkSize = g_DownloadBlockSize;
      task.NumSubtasks
          = static_cast<int32_t>((fileSize + g_UploadBlockSize - 1) / g_DownloadBlockSize);
      task.NumSubtasks = std::max(task.NumSubtasks, 1);
      if (blobProperties.BlobType == Blobs::Models::BlobType::BlockBlob)
      {
        task.SourceType = TaskModel::ResourceType::BlockBlob;
      }
      else if (blobProperties.BlobType == Blobs::Models::BlobType::AppendBlob)
      {
        task.SourceType = TaskModel::ResourceType::AppendBlob;
      }
      else if (blobProperties.BlobType == Blobs::Models::BlobType::PageBlob)
      {
        task.SourceType = TaskModel::ResourceType::PageBlob;
        PageBlobTaskDecoration(task, m_model.Source.m_blobClient.Value().AsPageBlobClient());
      }
      taskGenerated(std::move(task));
    }
    else if (
        transferType == TransferType::DirectoryDownload
        || transferType == TransferType::DirectoryCopy)
    {
      std::string rootDirectory;
      std::string currentDirectory;
      if (transferType == TransferType::DirectoryDownload)
      {
        rootDirectory = _internal::PathFromUrl(m_model.Destination.m_url);
        currentDirectory = rootDirectory;
        CreateDirectoryIfNotExists(currentDirectory);
      }

      partGens.push_back(gen);

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
        options.PageSizeHint = g_ListBlobsPageSize;
        options.ContinuationToken = std::move(currGen.ContinuationToken);

        std::vector<_internal::Task> subtasks;
        auto result = m_model.Source.m_blobFolder.Value().m_blobContainerClient.ListBlobs(options);
        for (auto& blobItem : result.Blobs)
        {
          TaskModel task;
          const std::string blobName = blobItem.Name.substr(options.Prefix->length());
          if (transferType == TransferType::DirectoryDownload)
          {
            std::string parentDir = GetParentDir(_internal::JoinPath(rootDirectory, blobName));
            if (parentDir != currentDirectory)
            {
              CreateDirectoryIfNotExists(parentDir);
              currentDirectory = std::move(parentDir);
            }

            task.Source = _internal::JoinPath(currGen.Source, blobName);
            task.Destination = _internal::JoinPath(currGen.Destination, blobName);
            task.ObjectSize = blobItem.BlobSize;
            task.ChunkSize = g_DownloadBlockSize;
            task.NumSubtasks = static_cast<int32_t>(
                (blobItem.BlobSize + g_DownloadBlockSize - 1) / g_DownloadBlockSize);
            task.NumSubtasks = std::max(task.NumSubtasks, 1);
            if (blobItem.BlobType == Blobs::Models::BlobType::BlockBlob)
            {
              task.SourceType = TaskModel::ResourceType::BlockBlob;
            }
            else if (blobItem.BlobType == Blobs::Models::BlobType::AppendBlob)
            {
              task.SourceType = TaskModel::ResourceType::AppendBlob;
            }
            else if (blobItem.BlobType == Blobs::Models::BlobType::PageBlob)
            {
              task.SourceType = TaskModel::ResourceType::PageBlob;
              PageBlobTaskDecoration(task, m_model.Source.m_blobClient.Value().AsPageBlobClient());
            }
          }
          else
          {
            task.NumSubtasks = 1;
            task.Source = _internal::JoinPath(currGen.Source, blobName);
            task.Destination = _internal::JoinPath(currGen.Destination, blobName);
          }
          taskGenerated(std::move(task));
        }

        if (result.NextPageToken.HasValue())
        {
          currGen.ContinuationToken = std::move(result.NextPageToken.Value());
          partGens.push_back(std::move(currGen));
        }
      } while (!partGens.empty() && totalNumNewSubtasks < g_MaxTasksGenerated);
    }
    else if (transferType == TransferType::SingleCopy)
    {
      AZURE_ASSERT(gen.Source.empty());
      AZURE_ASSERT(gen.Destination.empty());
      AZURE_ASSERT(gen.ContinuationToken.empty());

      TaskModel task;
      task.NumSubtasks = 1;
      taskGenerated(std::move(task));
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

    const auto transferType = _internal::JobModel::DeduceTransferType(m_model);
    if (transferType == TransferType::SingleUpload || transferType == TransferType::DirectoryUpload)
    {
      for (auto& taskModel : taskModels)
      {
        std::string source
            = _internal::JoinPath(_internal::PathFromUrl(m_model.Source.m_url), taskModel.Source);
        auto destination = transferType == TransferType::SingleUpload
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
        transferType == TransferType::SingleDownload
        || transferType == TransferType::DirectoryDownload)
    {
      for (auto& taskModel : taskModels)
      {
        auto source = transferType == TransferType::SingleDownload
            ? m_model.Source.m_blobClient.Value()
            : m_model.Source.m_blobFolder.Value().GetBlobClient(taskModel.Source);
        std::string destination = _internal::JoinPath(
            _internal::PathFromUrl(m_model.Destination.m_url), taskModel.Destination);

        if (taskModel.ExtendedAttributes.count("page_ranges") == 0)
        {
          auto context = std::make_shared<Blobs::_detail::DownloadRangeToMemoryTask::TaskContext>(
              source, destination);
          context->TransferEngine = m_rootTask->SharedStatus->TransferEngine;
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
            if (context->OffsetToWrite == -1)
            {
              context->OffsetToWrite = task->Offset;
            }
            task->Length = static_cast<size_t>(
                std::min<uint64_t>(context->FileSize - task->Offset, taskModel.ChunkSize));
            task->MemoryCost = task->Length;
            task->JournalContext = JournalContext{jobPart, bitmapOffset};
            tasks.push_back(std::move(task));
            ++bitmapOffset;
          }
        }
        else
        {
          auto context
              = std::make_shared<Blobs::_detail::DownloadPageBlobRangeToMemoryTask::TaskContext>(
                  source.AsPageBlobClient(), destination);
          context->FileSize = taskModel.ObjectSize;
          context->NumChunks = taskModel.NumSubtasks;

          context->NumDownloadedChunks = 0;
          std::string doneBits;

          if (taskModel.ExtendedAttributes.count("_subtasks") != 0)
          {
            doneBits = taskModel.ExtendedAttributes.at("_subtasks");
          }
          auto pageRanges
              = Core::Json::_internal::json::parse(taskModel.ExtendedAttributes.at("page_ranges"));
          AZURE_ASSERT(pageRanges.size() == static_cast<size_t>(context->NumChunks));
          AZURE_ASSERT(pageRanges.size() == static_cast<size_t>(taskModel.NumSubtasks));
          for (int i = 0; i < context->NumChunks; ++i)
          {
            if (!doneBits.empty() && doneBits[i] != '0')
            {
              context->NumDownloadedChunks.fetch_add(1, std::memory_order_relaxed);
              ++bitmapOffset;
              continue;
            }
            auto task = m_rootTask->CreateTask<Blobs::_detail::DownloadPageBlobRangeToMemoryTask>(
                _internal::TaskType::NetworkDownload);
            task->Context = context;
            task->MemoryCost = 0;
            for (size_t j = 0; j < pageRanges[i].size(); j += 2)
            {
              Azure::Core::Http::HttpRange range;
              range.Offset = pageRanges[i][j];
              range.Length = pageRanges[i][j + 1];
              task->MemoryCost += range.Length.Value();
              task->Ranges.push_back(std::move(range));
            }
            task->JournalContext = JournalContext{jobPart, bitmapOffset};
            tasks.push_back(std::move(task));
            ++bitmapOffset;
          }
        }
      }
    }
    else if (
        transferType == TransferType::SingleCopy || transferType == TransferType::DirectoryCopy)
    {
      for (auto& taskModel : taskModels)
      {
        auto source = transferType == TransferType::SingleCopy
            ? m_model.Source.m_blobClient.Value()
            : m_model.Source.m_blobFolder.Value().GetBlobClient(taskModel.Source);
        auto destination = transferType == TransferType::SingleCopy
            ? m_model.Destination.m_blobClient.Value()
            : m_model.Destination.m_blobFolder.Value().GetBlobClient(taskModel.Destination);

        AZURE_ASSERT(taskModel.NumSubtasks == 1);
        auto task = m_rootTask->CreateTask<Blobs::_detail::AsyncCopyBlobTask>(
            _internal::TaskType::NetworkUpload, source, destination);
        task->JournalContext = JournalContext{jobPart, bitmapOffset};
        tasks.emplace_back(std::move(task));
        ++bitmapOffset;
      }
    }
    else
    {
      AZURE_NOT_IMPLEMENTED();
    }
    return tasks;
  }

}}} // namespace Azure::Storage::_detail
