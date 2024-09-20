// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../../amqp/private/unique_handle.hpp"
#include "azure/core/amqp/models/amqp_header.hpp"

#include <azure/core/nullable.hpp>

#if ENABLE_UAMQP
typedef struct HEADER_INSTANCE_TAG* HEADER_HANDLE;

#elif ENABLE_RUST_AMQP
#include "rust_amqp_wrapper.h"
#endif

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <vector>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  using HeaderImplementation = std::remove_pointer<HEADER_HANDLE>::type;
#elif ENABLE_RUST_AMQP
  using HeaderImplementation = Azure::Core::Amqp::_detail::RustInterop::RustMessageHeader;
#endif

  template <> struct UniqueHandleHelper<HeaderImplementation>
  {
    static void FreeAmqpHeader(HeaderImplementation* obj);

    using type = Core::_internal::BasicUniqueHandle<HeaderImplementation, FreeAmqpHeader>;
  };
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {
  using UniqueMessageHeaderHandle
      = Amqp::_detail::UniqueHandle<Azure::Core::Amqp::_detail::HeaderImplementation>;

  /**
   * @brief uAMQP interoperability functions to convert a MessageHeader to a uAMQP HEADER_HANDLE
   * and back.
   *
   * @remarks This class should not be used directly. It is used by the uAMQP interoperability
   * layer.
   */
  struct MessageHeaderFactory
  {
    static MessageHeader FromImplementation(_detail::UniqueMessageHeaderHandle const& properties);
    static _detail::UniqueMessageHeaderHandle ToImplementation(MessageHeader const& properties);
  };
}}}}} // namespace Azure::Core::Amqp::Models::_detail
