// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <azure/storage/blobs/blob_client.hpp>
#include <azure/storage/common/internal/file_io.hpp>

#include "azure/storage/datamovement/datamovement_options.hpp"
#include "azure/storage/datamovement/task.hpp"

namespace Azure { namespace Storage {

  namespace _internal {
    class TransferEngine;
  }
  namespace Blobs { namespace _detail {

    struct WriteChunk
    {
      int64_t Offset;
      size_t Length;
      size_t MemoryGiveBack = 0;
      std::unique_ptr<uint8_t[]> Buffer;
      Azure::Storage::_detail::JournalContext JournalContext;
    };

    struct DownloadRangeToMemoryTask final : public Storage::_internal::TaskBase
    {
      using TaskBase::TaskBase;

      struct TaskContext final
      {
        explicit TaskContext(Blobs::BlobClient source, std::string destination)
            : Source(std::move(source)), Destination(std::move(destination))
        {
        }
        ~TaskContext();

        Blobs::BlobClient Source;
        std::string Destination;

        std::mutex FileWriterMutex; // TODO: optimize this if it becomes a bottleneck
        std::unique_ptr<Storage::_internal::FileWriter> FileWriter;
        uint64_t FileSize{0};
        int NumChunks{0};
        std::atomic<int> NumDownloadedChunks{0};
        std::atomic<bool> Failed{false};

        std::mutex WriteChunksMutex;
        bool WriteTaskRunning{false};
        std::map<int64_t, std::unique_ptr<WriteChunk>> ChunksToWrite;
        int64_t OffsetToWrite{-1};
        _internal::TransferEngine* TransferEngine = nullptr;
      };

      std::shared_ptr<TaskContext> Context;
      int64_t Offset;
      size_t Length;

      void Execute() noexcept override;
    };

    struct WriteToFileTask final : public Storage::_internal::TaskBase
    {
      using TaskBase::TaskBase;

      std::shared_ptr<DownloadRangeToMemoryTask::TaskContext> Context;
      std::vector<std::unique_ptr<WriteChunk>> ChunksToWrite;

      void Execute() noexcept override;
    };
  }} // namespace Blobs::_detail
}} // namespace Azure::Storage
