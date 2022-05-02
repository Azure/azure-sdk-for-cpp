// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>
#include <memory>

#include <azure/storage/blobs.hpp>
#include <azure/storage/common/internal/file_io.hpp>

namespace Azure { namespace Storage { namespace DataMovement { namespace _internal {
  class Scheduler;

  enum class TaskType
  {
    DiskIO,
    NetworkUpload,
    NetworkDownload,
    Other, // other tasks will be run ASAP
  };

  // Task should be idempotent
  // Root task should be serializable and de-serializable
  struct TaskBase
  {
    TaskBase(TaskType type, Scheduler* scheduler) : Type(type), m_scheduler(scheduler) {}
    TaskType Type;

    size_t MemoryCost = 0;
    size_t MemoryGiveBack = 0;

    virtual ~TaskBase() {}
    virtual void Execute() = 0;
    virtual std::string Serialize() { return std::string(); }

  protected:
    Scheduler* m_scheduler;
  };

  using Task = std::unique_ptr<TaskBase>;

  Task Deserialize(const char*);

}}}} // namespace Azure::Storage::DataMovement::_internal
