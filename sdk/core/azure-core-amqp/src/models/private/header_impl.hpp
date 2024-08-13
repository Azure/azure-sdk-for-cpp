// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../../amqp/private/unique_handle.hpp"
#include "azure/core/amqp/models/amqp_header.hpp"

#include <azure/core/nullable.hpp>

#if ENABLE_UAMQP
#include <azure_uamqp_c/amqp_definitions_milliseconds.h>

#include <azure_uamqp_c/amqp_definitions_header.h>
#endif

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <vector>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  template <> struct UniqueHandleHelper<std::remove_pointer<HEADER_HANDLE>::type>
  {
    static void FreeAmqpHeader(HEADER_HANDLE obj);

    using type = Core::_internal::
        BasicUniqueHandle<std::remove_pointer<HEADER_HANDLE>::type, FreeAmqpHeader>;
  };
#endif
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {
#if ENABLE_UAMQP
  using UniqueMessageHeaderHandle
      = Amqp::_detail::UniqueHandle<std::remove_pointer<HEADER_HANDLE>::type>;
#endif

  /**
   * @brief uAMQP interoperability functions to convert a MessageHeader to a uAMQP HEADER_HANDLE
   * and back.
   *
   * @remarks This class should not be used directly. It is used by the uAMQP interoperability
   * layer.
   */
  struct MessageHeaderFactory
  {
#if ENABLE_UAMQP
    static MessageHeader FromImplementation(_detail::UniqueMessageHeaderHandle const& properties);
    static _detail::UniqueMessageHeaderHandle ToImplementation(MessageHeader const& properties);
#endif
  };
}}}}} // namespace Azure::Core::Amqp::Models::_detail
