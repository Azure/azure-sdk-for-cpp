// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/models/amqp_header.hpp"

#include <azure/core/internal/unique_handle.hpp>
#include <azure/core/nullable.hpp>

#include <azure_uamqp_c/amqp_definitions_milliseconds.h>

#include <azure_uamqp_c/amqp_definitions_header.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <vector>

namespace Azure { namespace Core { namespace _internal {
  template <> struct UniqueHandleHelper<std::remove_pointer<HEADER_HANDLE>::type>
  {
    static void FreeAmqpHeader(HEADER_HANDLE obj);

    using type = BasicUniqueHandle<std::remove_pointer<HEADER_HANDLE>::type, FreeAmqpHeader>;
  };
}}} // namespace Azure::Core::_internal

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {
  using UniqueMessageHeaderHandle
      = Azure::Core::_internal::UniqueHandle<std::remove_pointer<HEADER_HANDLE>::type>;

  /**
   * @brief uAMQP interoperability functions to convert a MessageHeader to a uAMQP HEADER_HANDLE
   * and back.
   *
   * @remarks This class should not be used directly. It is used by the uAMQP interoperability
   * layer.
   */
  struct MessageHeaderFactory
  {
    static MessageHeader FromUamqp(_detail::UniqueMessageHeaderHandle const& properties);
    static _detail::UniqueMessageHeaderHandle ToUamqp(MessageHeader const& properties);
  };
}}}}} // namespace Azure::Core::Amqp::Models::_detail


