// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/tasks/download_blob_to_file_task.hpp"

#include "azure/storage/datamovement/task_shared_status.hpp"
#include "azure/storage/datamovement/transfer_engine.hpp"
#include "azure/storage/datamovement/utilities.hpp"

namespace Azure { namespace Storage { namespace Blobs { namespace _detail {

  namespace {
    constexpr size_t g_WritePieceLength = 8 * 1024 * 1024;
  } // namespace

  DownloadRangeToMemoryTask::TaskContext::~TaskContext()
  {
    size_t memoryToDeallocate = 0;
    {
      std::lock_guard<std::mutex> guard(WriteChunksMutex);
      for (auto& p : ChunksToWrite)
      {
        memoryToDeallocate += p.second->MemoryGiveBack;
      }
      ChunksToWrite.clear();
    }
    if (memoryToDeallocate != 0)
    {
      _internal::Task releaseMemoryTask
          = std::make_unique<_internal::DummyTask>(_internal::TaskType::Other);
      releaseMemoryTask->MemoryGiveBack = memoryToDeallocate;
      TransferEngine->ReclaimAllocatedResource(releaseMemoryTask);
    }
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
        TransferFailed(Context->Source.GetUrl(), _internal::PathToUrl(Context->Destination));
      }
      return;
    }

    std::unique_ptr<WriteChunk> writeChunk = std::make_unique<WriteChunk>();
    writeChunk->Buffer = std::move(buffer);
    writeChunk->Offset = this->Offset;
    writeChunk->Length = Length;
    std::swap(writeChunk->MemoryGiveBack, this->MemoryGiveBack);
    writeChunk->JournalContext = std::move(JournalContext);

    auto writeToFileTask = CreateTask<WriteToFileTask>(_internal::TaskType::DiskIO);
    {
      std::lock_guard<std::mutex> guard(Context->WriteChunksMutex);
      Context->ChunksToWrite.emplace(writeChunk->Offset, std::move(writeChunk));
      if (!Context->WriteTaskRunning)
      {

        while (!Context->ChunksToWrite.empty()
               && Context->ChunksToWrite.begin()->first == Context->OffsetToWrite)
        {
          auto& chunk = Context->ChunksToWrite.begin()->second;
          Context->OffsetToWrite += chunk->Length;
          writeToFileTask->MemoryGiveBack += chunk->MemoryGiveBack;
          writeToFileTask->ChunksToWrite.push_back(std::move(chunk));
          Context->ChunksToWrite.erase(Context->ChunksToWrite.begin());
        }
        if (!writeToFileTask->ChunksToWrite.empty())
        {
          Context->WriteTaskRunning = true;
        }
      }
      if (!writeToFileTask->ChunksToWrite.empty())
      {
        writeToFileTask->Context = Context;
        SharedStatus->TransferEngine->AddTask(std::move(writeToFileTask));
      }
    }
  }

  void WriteToFileTask::Execute() noexcept
  {
    if (Context->Failed.load(std::memory_order_relaxed))
    {
      return;
    }

    {
      std::lock_guard<std::mutex> guard(Context->FileWriterMutex);
      if (!Context->FileWriter)
      {
        // TODO: we should truncate the original file if this is the first write to the file, and
        // should NOT truncate if this is a resumed job.
        // A better way is to download to a non-existing temp file, and overwrite destination after
        // download is finished.
        if (ChunksToWrite[0]->Offset == 0)
        {
          Context->FileWriter
              = std::make_unique<Storage::_internal::FileWriter>(Context->Destination);
        }
        else
        {
          Context->FileWriter
              = std::make_unique<Storage::_internal::FileWriter>(Context->Destination, false);
        }
      }
    }

    for (auto& chunk : ChunksToWrite)
    {
      int64_t offset = chunk->Offset;
      size_t length = chunk->Length;
      uint8_t* bufferPointer = chunk->Buffer.get();
      try
      {
        while (length > 0)
        {
          size_t thisWriteLength = std::min(length, g_WritePieceLength);
          Context->FileWriter->Write(bufferPointer, thisWriteLength, offset);
          bufferPointer += thisWriteLength;
          offset += thisWriteLength;
          length -= thisWriteLength;
        }
      }
      catch (std::exception&)
      {
        bool firstFailure = !Context->Failed.exchange(true, std::memory_order_relaxed);
        if (firstFailure)
        {
          TransferFailed(Context->Source.GetUrl(), _internal::PathToUrl(Context->Destination));
        }
        return;
      }

      int numDownloadedBlocks
          = Context->NumDownloadedChunks.fetch_add(1, std::memory_order_relaxed) + 1;
      JournalContext = std::move(chunk->JournalContext);
      if (numDownloadedBlocks == Context->NumChunks)
      {
        TransferSucceeded(chunk->Length, 1);
        return;
      }
      else
      {
        TransferSucceeded(chunk->Length, 0);
      }
    }

    auto writeToFileTask = CreateTask<WriteToFileTask>(_internal::TaskType::DiskIO);
    {
      std::lock_guard<std::mutex> guard(Context->WriteChunksMutex);
      while (!Context->ChunksToWrite.empty()
             && Context->ChunksToWrite.begin()->first == Context->OffsetToWrite)
      {
        auto& chunk = Context->ChunksToWrite.begin()->second;
        Context->OffsetToWrite += chunk->Length;
        writeToFileTask->MemoryGiveBack += chunk->MemoryGiveBack;
        writeToFileTask->ChunksToWrite.push_back(std::move(chunk));
        Context->ChunksToWrite.erase(Context->ChunksToWrite.begin());
      }
      if (writeToFileTask->ChunksToWrite.empty())
      {
        Context->WriteTaskRunning = false;
      }
    }
    if (!writeToFileTask->ChunksToWrite.empty())
    {
      writeToFileTask->Context = Context;
      SharedStatus->TransferEngine->AddTask(std::move(writeToFileTask));
    }
  }
}}}} // namespace Azure::Storage::Blobs::_detail
