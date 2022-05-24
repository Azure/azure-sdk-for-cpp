// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/tasks/download_blob_to_file_task.hpp"

#include "azure/storage/datamovement/job_properties.hpp"
#include "azure/storage/datamovement/scheduler.hpp"
#include "azure/storage/datamovement/utilities.hpp"

namespace Azure { namespace Storage { namespace Blobs { namespace _detail {

  namespace {
    constexpr uint64_t ChunkSize = 8 * 1024 * 1024;
    static_assert(ChunkSize < static_cast<uint64_t>(std::numeric_limits<size_t>::max()), "");

  } // namespace

  // TODO: Should not allow expection thrown out from the method, should add some error handling.
  void DownloadBlobToFileTask::Execute() noexcept
  {
    try
    {
      auto properties = Context->Source.GetProperties().Value;
      Context->FileSize = properties.BlobSize;
    }
    catch (std::exception&)
    {
      SharedStatus->TaskFailedCallback(
          1, Context->Source.GetUrl(), _internal::GetFileUrl(Context->Destination));
      return;
    }

    const uint64_t fileSize = Context->FileSize;

    if (!Context->FileWriter)
    {
      // TODO: check if file already exists and last modified time depending on overwrite behavior
      try
      {
        Context->FileWriter
            = std::make_unique<Storage::_internal::FileWriter>(Context->Destination);
      }
      catch (std::exception&)
      {
        SharedStatus->TaskFailedCallback(
            1, Context->Source.GetUrl(), _internal::GetFileUrl(Context->Destination));
        return;
      }
    }

    if (fileSize == 0)
    {
      SharedStatus->TaskTransferedCallback(1, Context->FileSize);
      return;
    }

    Context->NumChunks = static_cast<int>((fileSize + ChunkSize - 1) / ChunkSize);
    std::vector<Storage::_internal::Task> subtasks;
    for (int index = 0; index < Context->NumChunks; ++index)
    {
      auto downloadRangeTask
          = CreateTask<DownloadRangeToMemoryTask>(Storage::_internal::TaskType::NetworkDownload);
      downloadRangeTask->Context = Context;
      downloadRangeTask->Offset = index * ChunkSize;
      downloadRangeTask->Length
          = static_cast<size_t>(std::min(ChunkSize, fileSize - index * ChunkSize));
      downloadRangeTask->MemoryCost = downloadRangeTask->Length;
      subtasks.push_back(std::move(downloadRangeTask));
    }
    SharedStatus->Scheduler->AddTasks(std::move(subtasks));
  }

  void DownloadRangeToMemoryTask::Execute() noexcept
  {
    if (Context->Failed.load(std::memory_order_relaxed))
    {
      return;
    }

    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(Length);
    Azure::Storage::Blobs::DownloadBlobOptions options;
    options.Range = Core::Http::HttpRange();
    options.Range->Offset = this->Offset;
    options.Range->Length = this->Length;
    // TODO: should check for source blob changing to avoid data corruption.
    try
    {
      auto downloadResult = Context->Source.Download(options).Value;
      size_t bytesRead = downloadResult.BodyStream->ReadToCount(buffer.get(), this->Length);
      if (bytesRead != this->Length)
      {
        throw std::runtime_error("Failed to download blob chunk.");
      }
    }
    catch (std::exception&)
    {
      bool firstFailure = !Context->Failed.exchange(true, std::memory_order_relaxed);
      if (firstFailure)
      {
        SharedStatus->TaskFailedCallback(
            1, Context->Source.GetUrl(), _internal::GetFileUrl(Context->Destination));
      }
      return;
    }

    auto writeToFileTask = CreateTask<WriteToFileTask>(_internal::TaskType::DiskIO);
    writeToFileTask->Context = Context;
    writeToFileTask->Buffer = std::move(buffer);
    writeToFileTask->Offset = this->Offset;
    writeToFileTask->Length = Length;
    std::swap(writeToFileTask->MemoryGiveBack, this->MemoryGiveBack);

    SharedStatus->Scheduler->AddTask(std::move(writeToFileTask));
  }

  void WriteToFileTask::Execute() noexcept
  {
    if (Context->Failed.load(std::memory_order_relaxed))
    {
      return;
    }

    try
    {
      Context->FileWriter->Write(this->Buffer.get(), this->Length, this->Offset);
    }
    catch (std::exception&)
    {
      bool firstFailure = !Context->Failed.exchange(true, std::memory_order_relaxed);
      if (firstFailure)
      {
        SharedStatus->TaskFailedCallback(
            1, Context->Source.GetUrl(), _internal::GetFileUrl(Context->Destination));
      }
      return;
    }

    int numDownloadedBlocks
        = Context->NumDownloadedChunks.fetch_add(1, std::memory_order_relaxed) + 1;
    if (numDownloadedBlocks == Context->NumChunks)
    {
      SharedStatus->TaskTransferedCallback(1, Context->FileSize);
    }
  }
}}}} // namespace Azure::Storage::Blobs::_detail
