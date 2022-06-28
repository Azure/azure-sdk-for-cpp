// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/tasks/download_blob_to_file_task.hpp"

#include "azure/storage/datamovement/task_shared_status.hpp"
#include "azure/storage/datamovement/transfer_engine.hpp"
#include "azure/storage/datamovement/utilities.hpp"

namespace Azure { namespace Storage { namespace Blobs { namespace _detail {

  namespace {
    constexpr size_t WritePieceLength = 8 * 1024 * 1024;
  } // namespace

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
      std::lock_guard<std::mutex> guard(Context->m_writeChunksMutex);
      Context->m_chunksToWrite.insert({writeChunk->Offset, std::move(writeChunk)});
      if (!Context->writeTaskRunning)
      {
        auto iter = Context->m_chunksToWrite.begin();
        while ((iter != Context->m_chunksToWrite.end()) && (iter->first == Context->OffsetToWrite))
        {
          Context->OffsetToWrite += iter->second->Length;
          writeToFileTask->MemoryGiveBack += iter->second->MemoryGiveBack;
          writeToFileTask->chunksToWrite.push_back(std::move(iter->second));
          Context->m_chunksToWrite.erase(iter);
          iter = Context->m_chunksToWrite.begin();
        }
        if (!writeToFileTask->chunksToWrite.empty())
        {
          Context->writeTaskRunning = true;
        }
      }
    }

    if (!writeToFileTask->chunksToWrite.empty())
    {
      writeToFileTask->Context = Context;
      SharedStatus->TransferEngine->AddTask(std::move(writeToFileTask));
    }
  }

  void WriteToFileTask::Execute() noexcept
  {
    if (Context->Failed.load(std::memory_order_relaxed))
    {
      return;
    }

    {
      if (!Context->FileWriter)
      {
        std::lock_guard<std::mutex> guard(Context->FileWriterMutex);
        if (!Context->FileWriter)
        {
          Context->FileWriter
              = std::make_unique<Storage::_internal::FileWriter>(Context->Destination);
        }
      }
    }

    for (auto chunkToWrite = chunksToWrite.begin(); chunkToWrite != chunksToWrite.end();
         ++chunkToWrite)
    {
      int64_t offset = (*chunkToWrite)->Offset;
      size_t length = (*chunkToWrite)->Length;
      uint8_t* bufferPointer = (*chunkToWrite)->Buffer.get();
      try
      {
        while (length > 0)
        {
          size_t thisWriteLength = std::min(length, WritePieceLength);
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
      JournalContext = std::move((*chunkToWrite)->JournalContext);
      if (numDownloadedBlocks == Context->NumChunks)
      {
        TransferSucceeded((*chunkToWrite)->Length, 1);
        return;
      }
      else
      {
        TransferSucceeded((*chunkToWrite)->Length, 0);
      }
    }

    auto writeToFileTask = CreateTask<WriteToFileTask>(_internal::TaskType::DiskIO);
    {
      std::lock_guard<std::mutex> guard(Context->m_writeChunksMutex);
      auto iter = Context->m_chunksToWrite.begin();
      while ((iter != Context->m_chunksToWrite.end()) && (iter->first == Context->OffsetToWrite))
      {
        Context->OffsetToWrite += iter->second->Length;
        writeToFileTask->MemoryGiveBack += iter->second->MemoryGiveBack;
        writeToFileTask->chunksToWrite.push_back(std::move(iter->second));
        Context->m_chunksToWrite.erase(iter);
        iter = Context->m_chunksToWrite.begin();
      }
      if (writeToFileTask->chunksToWrite.empty())
      {
        Context->writeTaskRunning = false;
      }
    }

    if (!writeToFileTask->chunksToWrite.empty())
    {
      writeToFileTask->Context = Context;
      SharedStatus->TransferEngine->AddTask(std::move(writeToFileTask));
    }
  }
}}}} // namespace Azure::Storage::Blobs::_detail
