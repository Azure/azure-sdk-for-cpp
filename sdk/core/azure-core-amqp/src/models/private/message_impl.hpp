// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/models/amqp_message.hpp"

#include <azure/core/internal/unique_handle.hpp>

#include <azure_uamqp_c/message.h>

#include <type_traits>

namespace Azure { namespace Core { namespace _internal {
  template <> struct UniqueHandleHelper<std::remove_pointer<MESSAGE_HANDLE>::type>
  {
    static void FreeAmqpMessage(MESSAGE_HANDLE obj);

    using type = BasicUniqueHandle<std::remove_pointer<MESSAGE_HANDLE>::type, FreeAmqpMessage>;
  };
}}} // namespace Azure::Core::_internal

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {

  using UniqueMessageHandle
      = Azure::Core::_internal::UniqueHandle<std::remove_pointer<MESSAGE_HANDLE>::type>;

  /**
   * @brief uAMQP interoperability functions to convert a Message to a uAMQP
   * MESSAGE_HANDLE and back.
   *
   * @remarks This class should not be used directly. It is used by the uAMQP interoperability
   * layer.
   */
  class AmqpMessageFactory final {
    AmqpMessageFactory() = delete;

  public:
    static std::shared_ptr<AmqpMessage> FromUamqp(MESSAGE_HANDLE message);
    static UniqueMessageHandle ToUamqp(AmqpMessage const& message);
  };
}}}}} // namespace Azure::Core::Amqp::Models::_detail
