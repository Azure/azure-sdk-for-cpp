// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/tasks/download_page_blob_to_file_task.hpp"

#include <azure/core/platform.hpp>

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#include <winioctl.h>
#endif

#include "azure/storage/datamovement/task_shared_status.hpp"
#include "azure/storage/datamovement/transfer_engine.hpp"
#include "azure/storage/datamovement/utilities.hpp"

namespace Azure { namespace Storage { namespace Blobs { namespace _detail {
  namespace {
    constexpr size_t g_WritePieceLength = 8 * 1024 * 1024;
  }

  void DownloadPageBlobRangeToMemoryTask::Execute() noexcept
  {
    if (Context->Failed.load(std::memory_order_relaxed))
    {
      return;
    }

    size_t bufferLength = 0;
    for (const auto& range : Ranges)
    {
      bufferLength += static_cast<size_t>(range.Length.Value());
    }
    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(bufferLength);

    uint8_t* bufferPointer = buffer.get();
    for (const auto& range : Ranges)
    {
      Azure::Storage::Blobs::DownloadBlobOptions options;
      options.Range = range;

      try
      {
        auto downloadResult = Context->Source.Download(options).Value;
        size_t bytesRead = downloadResult.BodyStream->ReadToCount(
            bufferPointer, static_cast<size_t>(range.Length.Value()));
        if (bytesRead != static_cast<size_t>(range.Length.Value()))
        {
          throw std::runtime_error("Failed to download blob chunk.");
        }
        bufferPointer += range.Length.Value();
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
    }

    auto writeToFileTask
        = CreateTask<WritePageBlobRangesToSparseFileTask>(_internal::TaskType::DiskIO);
    writeToFileTask->Context = Context;
    writeToFileTask->Ranges = std::move(Ranges);
    writeToFileTask->Buffer = std::move(buffer);
    std::swap(writeToFileTask->MemoryGiveBack, this->MemoryGiveBack);
    writeToFileTask->JournalContext = std::move(JournalContext);
    SharedStatus->TransferEngine->AddTask(std::move(writeToFileTask));
  }

  void WritePageBlobRangesToSparseFileTask::Execute() noexcept
  {
    if (Context->Failed.load(std::memory_order_relaxed))
    {
      return;
    }

    try
    {
      std::lock_guard<std::mutex> guard(Context->FileWriterMutex);
      if (!Context->FileWriter)
      {
        if (Context->NumDownloadedChunks == 0)
        {
          Context->FileWriter
              = std::make_unique<Storage::_internal::FileWriter>(Context->Destination);

#if defined(AZ_PLATFORM_WINDOWS)
          DWORD useless;
          BOOL ret = DeviceIoControl(
              Context->FileWriter->GetHandle(), FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &useless, NULL);
          if (!ret)
          {
            throw std::runtime_error("Failed to set sparse file.");
          }
#endif

#if defined(AZ_PLATFORM_WINDOWS)
          LARGE_INTEGER size;
          size.QuadPart = Context->FileSize;
          ret = SetFilePointerEx(Context->FileWriter->GetHandle(), size, NULL, FILE_BEGIN);
          if (!ret)
          {
            throw std::runtime_error("Failed to seek file.");
          }
          ret = SetEndOfFile(Context->FileWriter->GetHandle());
          if (!ret)
          {
            throw std::runtime_error("Failed to resize file.");
          }
#else
          int ret = ftruncate(Context->FileWriter->GetHandle(), Context->FileSize);
          if (ret != 0)
          {
            throw std::runtime_error("Failed to resize file.");
          }
#endif
        }
        else
        {
          Context->FileWriter
              = std::make_unique<Storage::_internal::FileWriter>(Context->Destination, false);
        }
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
    Storage::_internal::FileWriter* writer = Context->FileWriter.get();

    uint8_t* bufferPointer = Buffer.get();
    int64_t totalWriteLength = 0;
    for (const auto& range : Ranges)
    {
      int64_t offset = range.Offset;
      size_t length = static_cast<size_t>(range.Length.Value());
      totalWriteLength += length;
      try
      {
        while (length > 0)
        {
          size_t thisWriteLength = std::min(length, g_WritePieceLength);
          writer->Write(bufferPointer, thisWriteLength, offset);
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
    }

    int numDownloadedBlocks
        = Context->NumDownloadedChunks.fetch_add(1, std::memory_order_relaxed) + 1;
    if (numDownloadedBlocks == Context->NumChunks)
    {
      TransferSucceeded(totalWriteLength, 1);
    }
    else
    {
      TransferSucceeded(totalWriteLength, 0);
    }
  }

}}}} // namespace Azure::Storage::Blobs::_detail