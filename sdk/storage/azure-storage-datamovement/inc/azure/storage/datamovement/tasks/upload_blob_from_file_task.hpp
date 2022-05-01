// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>
#include <memory>

#include <azure/storage/blobs.hpp>

#include "azure/storage/datamovement/task.hpp"

namespace Azure { namespace Storage { namespace DataMovement { namespace _internal {

  struct UploadBlobFromFileTask : public TaskBase
  {
    UploadBlobFromFileTask(
        TaskType type,
        Scheduler* scheduler,
        const std::string& source,
        const Blobs::BlobClient& destination)
        : TaskBase(type, scheduler), Context(std::make_shared<TaskContext>(source, destination))
    {
    }

    struct TaskContext
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
    };
    std::shared_ptr<TaskContext> Context;

    void Execute() override;
  };

  struct ReadFileRangeToMemoryTask : public TaskBase
  {
    using TaskBase::TaskBase;

    std::shared_ptr<UploadBlobFromFileTask::TaskContext> Context;
    int BlockId;
    int64_t Offset;
    size_t Length;

    void Execute() override;
  };

  struct StageBlockTask : public TaskBase
  {
    using TaskBase::TaskBase;

    std::shared_ptr<UploadBlobFromFileTask::TaskContext> Context;
    int BlockId;
    size_t Length;
    std::unique_ptr<uint8_t[]> Buffer;

    void Execute() override;
  };

}}}} // namespace Azure::Storage::DataMovement::_internal