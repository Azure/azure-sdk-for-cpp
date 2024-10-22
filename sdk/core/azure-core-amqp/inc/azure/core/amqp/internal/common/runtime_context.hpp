// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#if ENABLE_RUST_AMQP
#include "../src/amqp/private/unique_handle.hpp"
#include "rust_amqp_wrapper.h"

#include <azure/core/context.hpp>

#include <memory>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  using RustRuntimeContext = Azure::Core::Amqp::RustInterop::_detail::RuntimeContext;
  using RustCallContext = Azure::Core::Amqp::RustInterop::_detail::RustCallContext;

  template <> struct UniqueHandleHelper<RustRuntimeContext>
  {
    static void FreeRuntimeContext(RustRuntimeContext* obj);

    using type = Core::_internal::BasicUniqueHandle<RustRuntimeContext, FreeRuntimeContext>;
  };
  template <> struct UniqueHandleHelper<RustCallContext>
  {
    static void FreeCallContext(RustCallContext* obj);

    using type = Core::_internal::BasicUniqueHandle<RustCallContext, FreeCallContext>;
  };
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Common { namespace _detail {

  using UniqueRustRuntimeContext = Azure::Core::Amqp::_detail::UniqueHandleHelper<
      Azure::Core::Amqp::_detail::RustRuntimeContext>::type;

  using UniqueRustCallContext = Azure::Core::Amqp::_detail::UniqueHandleHelper<
      Azure::Core::Amqp::_detail::RustCallContext>::type;

  /**
   * @brief Represents the an implementation of the rust multithreaded runtime.
   *
   * Needed to implement blocking Rust API calls.
   */
  class RustRuntimeContext final {

    UniqueRustRuntimeContext m_runtimeContext;

  public:
    RustRuntimeContext()
        : m_runtimeContext{Azure::Core::Amqp::RustInterop::_detail::runtime_context_new()}
    {
    }

    Azure::Core::Amqp::_detail::RustRuntimeContext* GetRuntimeContext() const
    {
      return m_runtimeContext.get();
    }
  };

  class CallContext final {
  public:
    /** Construct a new CallContext object.
     *
     * @param runtimeContext - pointer to the Rust runtime for this process.
     * @param context - Azure Context for this operation.
     *
     * @note This class does *NOT* take ownership of the runtime - that is because the lifetime of
     * all CallContext objects MUST be shorter than the lifetime of the GlobalState object which
     * actually holds the RustRuntimeContext.
     *
     */

    CallContext(
        Azure::Core::Amqp::RustInterop::_detail::RuntimeContext* runtimeContext,
        Azure::Core::Context const& context)
        : m_callContext{Azure::Core::Amqp::RustInterop::_detail::call_context_new(runtimeContext)},
          m_context(context)
    {
    }

    CallContext()
        : m_callContext{Azure::Core::Amqp::RustInterop::_detail::call_context_new(nullptr)}
    {
    }

    Azure::Core::Amqp::_detail::RustCallContext* GetCallContext() const
    {
      return m_callContext.get();
    }

    std::string GetError() const
    {
      auto err = call_context_get_error(GetCallContext());
      if (err)
      {
        //        auto err{rust_error_get_message(error.get())};
        std::string errorString{err};
        Azure::Core::Amqp::RustInterop::_detail::rust_string_delete(err);
        return errorString;
      }
      else
      {
        return "No current Error.";
      }
    }

  private:
    UniqueRustCallContext m_callContext;
    Azure::Core::Context m_context;
  };

  /**
   * Invoke a Rust AMQP builder API, checking for error.
   *
   * @param api - Flat C API to invoke. The first parameter MUST be a RustCallContext object, the
   * second parameter must be a Rust client object.
   * @param builder - Unique Pointer to a Rust builder object.
   * @param args - remaining arguments to the flat C API.
   *
   * This function will check the return from the API, and if it is null, will throw an exception
   * with error information from the callContext.
   *
   */
  template <typename Api, typename T, typename... Args>
  void InvokeBuilderApi(Api& api, T& builder, Args&&... args)
  {
    CallContext callContext;
    auto raw = api(callContext.GetCallContext(), builder.release(), std::forward<Args>(args)...);
    if (!raw)
    {
      throw std::runtime_error("Error processing builder API: " + callContext.GetError());
    }
    builder.reset(raw);
  }

}}}}} // namespace Azure::Core::Amqp::Common::_detail
#endif // ENABLE_RUST_AMQP
