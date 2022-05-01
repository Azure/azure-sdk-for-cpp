// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>

namespace Azure { namespace Storage { namespace DataMovement { namespace _internal {
  enum class TaskType
  {
    DiskIO,
    NetworkUpload,
    NetworkDownload,
    Other, // other tasks will be run ASAP
  };

  // Task should be idempotent
  struct Task
  {
    TaskType Type;

    size_t MemoryCost = 0;
    size_t MemoryGiveBack = 0;

    std::function<void()> func;  // func shouldn't throw
  };

}}}} // namespace Azure::Storage::DataMovement::_internal
