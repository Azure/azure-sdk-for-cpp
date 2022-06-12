// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <mutex>
#include <string>

#include <azure/storage/blobs/blob_client.hpp>
#include <azure/storage/common/internal/file_io.hpp>

#include "azure/storage/datamovement/datamovement_options.hpp"
#include "azure/storage/datamovement/task.hpp"

namespace Azure { namespace Storage { namespace Blobs { namespace _detail {

  struct DownloadRangeToMemoryTask final : public Storage::_internal::TaskBase
  {
    using TaskBase::TaskBase;

    struct TaskContext final
    {
      explicit TaskContext(Blobs::BlobClient source, std::string destination)
          : Source(std::move(source)), Destination(std::move(destination))
      {
      }
      Blobs::BlobClient Source;
      std::string Destination;
      std::mutex FileWriterMutex; // TODO: optimize this if it becomes a bottleneck
      std::unique_ptr<Storage::_internal::FileWriter> FileWriter;
      uint64_t FileSize{0};
      int NumChunks{0};
      std::atomic<int> NumDownloadedChunks{0};
      std::atomic<bool> Failed{false};
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
    int64_t Offset;
    size_t Length;
    std::unique_ptr<uint8_t[]> Buffer;

    void Execute() noexcept override;
  };
}}}} // namespace Azure::Storage::Blobs::_detail
