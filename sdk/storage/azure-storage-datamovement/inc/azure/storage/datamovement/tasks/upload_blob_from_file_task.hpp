// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>
#include <memory>
#include <string>

#include <azure/storage/blobs.hpp>
#include <azure/storage/common/internal/file_io.hpp>

#include "azure/storage/datamovement/task.hpp"

namespace Azure { namespace Storage { namespace Blobs { namespace _detail {

  struct UploadBlobFromFileTask final : public Storage::_internal::TaskBase
  {
    explicit UploadBlobFromFileTask(
        _internal::TaskType type,
        const std::string& source,
        const Blobs::BlobClient& destination) noexcept
        : TaskBase(type), Context(std::make_shared<TaskContext>(source, destination))
    {
    }

    struct TaskContext final
    {
      explicit TaskContext(std::string source, Blobs::BlobClient destination)
          : Source(std::move(source)), Destination(std::move(destination))
      {
      }
      std::string Source;
      Blobs::BlobClient Destination;
      std::unique_ptr<Storage::_internal::FileReader> FileReader;
      uint64_t FileSize{0};
      int NumBlocks{0};
      std::atomic<int> NumStagedBlocks{0};
      std::atomic<bool> Failed{false};
    };
    std::shared_ptr<TaskContext> Context;

    void Execute() noexcept override;
  };

  struct ReadFileRangeToMemoryTask final : public Storage::_internal::TaskBase
  {
    using TaskBase::TaskBase;

    std::shared_ptr<UploadBlobFromFileTask::TaskContext> Context;
    int BlockId;
    int64_t Offset;
    size_t Length;

    void Execute() noexcept override;
  };

  struct StageBlockTask final : public Storage::_internal::TaskBase
  {
    using TaskBase::TaskBase;

    std::shared_ptr<UploadBlobFromFileTask::TaskContext> Context;
    int BlockId;
    size_t Length;
    std::unique_ptr<uint8_t[]> Buffer;

    void Execute() noexcept override;
  };

}}}} // namespace Azure::Storage::Blobs::_detail
