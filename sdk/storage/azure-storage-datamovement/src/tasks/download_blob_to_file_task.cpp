// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/tasks/download_blob_to_file_task.hpp"

#include "azure/storage/datamovement/task_shared_status.hpp"
#include "azure/storage/datamovement/transfer_engine.hpp"
#include "azure/storage/datamovement/utilities.hpp"

namespace Azure { namespace Storage { namespace Blobs { namespace _detail {

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
        TransferFailed(Context->Source.GetUrl(), _internal::GetPathUrl(Context->Destination));
      }
      return;
    }

    auto writeToFileTask = CreateTask<WriteToFileTask>(_internal::TaskType::DiskIO);
    writeToFileTask->Context = Context;
    writeToFileTask->Buffer = std::move(buffer);
    writeToFileTask->Offset = this->Offset;
    writeToFileTask->Length = Length;
    std::swap(writeToFileTask->MemoryGiveBack, this->MemoryGiveBack);
    writeToFileTask->JournalContext = std::move(JournalContext);

    SharedStatus->TransferEngine->AddTask(std::move(writeToFileTask));
  }

  void WriteToFileTask::Execute() noexcept
  {
    if (Context->Failed.load(std::memory_order_relaxed))
    {
      return;
    }

    try
    {
      {
        std::lock_guard<std::mutex> guard(Context->FileWriterMutex);
        if (!Context->FileWriter)
        {
          Context->FileWriter
              = std::make_unique<Storage::_internal::FileWriter>(Context->Destination);
        }
      }
      Context->FileWriter->Write(this->Buffer.get(), this->Length, this->Offset);
    }
    catch (std::exception&)
    {
      bool firstFailure = !Context->Failed.exchange(true, std::memory_order_relaxed);
      if (firstFailure)
      {
        TransferFailed(Context->Source.GetUrl(), _internal::GetPathUrl(Context->Destination));
      }
      return;
    }

    int numDownloadedBlocks
        = Context->NumDownloadedChunks.fetch_add(1, std::memory_order_relaxed) + 1;
    if (numDownloadedBlocks == Context->NumChunks)
    {
      TransferSucceeded(Length, 1);
    }
    else
    {
      TransferSucceeded(Length, 0);
    }
  }
}}}} // namespace Azure::Storage::Blobs::_detail
