#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <vector>

struct ASYNC_OPERATION_INSTANCE_TAG;

namespace Azure { namespace Core { namespace _internal { namespace Amqp {

  class Cancellable {
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
}}}} // namespace Azure::Core::_internal::Amqp