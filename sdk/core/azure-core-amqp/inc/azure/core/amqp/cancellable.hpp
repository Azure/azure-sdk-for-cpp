// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <vector>
#if 0
struct ASYNC_OPERATION_INSTANCE_TAG;

namespace Azure { namespace Core { namespace Amqp { namespace _internal {
  class Cancellable final {
  public:
    Cancellable(ASYNC_OPERATION_INSTANCE_TAG* asyncOperation) : m_operation{asyncOperation} {}
    ~Cancellable();

    Cancellable(Cancellable const&) = delete;
    Cancellable& operator=(Cancellable const&) = delete;
    Cancellable(Cancellable&& that) noexcept : m_operation{that.m_operation}
    {
      that.m_operation = nullptr;
    }
    Cancellable& operator=(Cancellable&& that) noexcept
    {
      m_operation = that.m_operation;
      that.m_operation = nullptr;
      return *this;
    }

    void Cancel();

  private:
    ASYNC_OPERATION_INSTANCE_TAG* m_operation;
  };
}}}} // namespace Azure::Core::Amqp::_internal
#endif