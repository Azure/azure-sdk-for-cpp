// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../../amqp/private/unique_handle.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"

#if ENABLE_UAMQP
#include <azure_uamqp_c/message.h>
#endif

#include <type_traits>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  template <> struct UniqueHandleHelper<std::remove_pointer<MESSAGE_HANDLE>::type>
  {
    static void FreeAmqpMessage(MESSAGE_HANDLE obj);

    using type = Core::_internal::
        BasicUniqueHandle<std::remove_pointer<MESSAGE_HANDLE>::type, FreeAmqpMessage>;
  };
#endif
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {
#if ENABLE_UAMQP
  using UniqueMessageHandle
      = Amqp::_detail::UniqueHandle<std::remove_pointer<MESSAGE_HANDLE>::type>;
#endif
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
#if ENABLE_UAMQP
    static std::shared_ptr<AmqpMessage> FromUamqp(MESSAGE_HANDLE message);
    static UniqueMessageHandle ToUamqp(AmqpMessage const& message);
#endif
  };
}}}}} // namespace Azure::Core::Amqp::Models::_detail
