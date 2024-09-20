// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#if ENABLE_RUST_AMQP
#include "../src/amqp/private/unique_handle.hpp"
#include "rust_amqp_wrapper.h"

#include <azure/core/azure_assert.hpp>

#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include <thread>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  using RustRuntimeContext = RustInterop::RuntimeContext;

  template <> struct UniqueHandleHelper<RustRuntimeContext>
  {
    static void FreeRuntimeContext(RustRuntimeContext* obj);

    using type = Core::_internal::BasicUniqueHandle<RustRuntimeContext, FreeRuntimeContext>;
  };
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Common { namespace _detail {

  using UniqueRustRuntimeContext = Azure::Core::Amqp::_detail::UniqueHandleHelper<
      Azure::Core::Amqp::_detail::RustRuntimeContext>::type;

  /**
   * @brief Represents the an implementation of the rust multithreaded runtime.
   *
   * Needed to implement blocking Rust API calls.
   */
  class RustRuntimeContext final {

    UniqueRustRuntimeContext m_runtimeContext;

  public:
    RustRuntimeContext()
        : m_runtimeContext{Azure::Core::Amqp::_detail::RustInterop::runtime_context_new()}
    {
    }

    Azure::Core::Amqp::_detail::RustRuntimeContext* GetRuntimeContext()
    {
      return m_runtimeContext.get();
    }
  };

}}}}} // namespace Azure::Core::Amqp::Common::_detail
#endif // ENABLE_RUST_AMQP
