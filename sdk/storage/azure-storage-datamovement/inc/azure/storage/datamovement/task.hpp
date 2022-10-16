// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>

namespace Azure { namespace Storage {
  namespace _detail {
    struct JobPart;
    struct JournalContext final
    {
      std::weak_ptr<_detail::JobPart> JobPart;
      size_t BitmapOffset = 0;
    };
  } // namespace _detail
  namespace _internal {
    class TaskSharedStatus;

    enum class TaskType
    {
      DiskIO,
      NetworkUpload,
      NetworkDownload,
      Other, // other tasks will be run ASAP
    };

    // Task should be idempotent
    struct TaskBase
    {
      explicit TaskBase(TaskType type) noexcept : Type(type) {}
      TaskBase(const TaskBase&) = delete;
      TaskBase(TaskBase&&) noexcept = default;
      TaskBase& operator=(const TaskBase&) = delete;
      TaskBase& operator=(TaskBase&&) = delete;

      TaskType Type;
      std::shared_ptr<TaskSharedStatus> SharedStatus;

      // This value should be initialized when task is created. Corresponding resource is reduced
      // when the task enters ready queue. If the task is successfully executed, resource is added
      // back by MemoryGiveBack. If the task isn't successfully executed, the resource needs to be
      // added back when the task gets out of ready queue (getting paused, cancelled, failed etc.)
      size_t MemoryCost = 0;
      // This value should be initialized when resource is allocated. This amount of resource will
      // be given back when the task is destructed. If you want to pass on the resource to a child
      // task, copy this value to child task and then set the value of current task to zero.
      size_t MemoryGiveBack = 0;

      template <class T, class... Args> std::unique_ptr<T> CreateTask(TaskType type, Args&&... args)
      {
        auto task = std::make_unique<T>(type, std::forward<Args>(args)...);
        task->SharedStatus = SharedStatus;
        return task;
      }

      virtual ~TaskBase() {}
      virtual void Execute() noexcept = 0;

      _detail::JournalContext JournalContext;
      void TransferSucceeded(int64_t bytesTransferred, int64_t numFiles = 1) const;
      void TransferFailed(std::string sourceUrl, std::string destinationUrl, int64_t numFiles = 1)
          const;
      void Transferkipped(int64_t numFiles = 1) const;
    };

    using Task = std::unique_ptr<TaskBase>;

    struct DummyTask final : public Storage::_internal::TaskBase
    {
      using TaskBase::TaskBase;
      void Execute() noexcept override { AZURE_UNREACHABLE_CODE(); }
    };
  } // namespace _internal
}} // namespace Azure::Storage
