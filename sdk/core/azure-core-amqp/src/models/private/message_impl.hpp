// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../../amqp/private/unique_handle.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"

#if ENABLE_UAMQP
extern "C"
{
  typedef struct MESSAGE_INSTANCE_TAG* MESSAGE_HANDLE;
}
//#include <azure_uamqp_c/message.h>
#elif ENABLE_RUST_AMQP
#include "rust_amqp_wrapper.h"
#endif

#include <type_traits>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  using MessageImplementation = std::remove_pointer<MESSAGE_HANDLE>::type;
#elif ENABLE_RUST_AMQP
  using MessageImplementation = Azure::Core::Amqp::_detail::RustInterop::RustAmqpMessage;
  using MessageBuilderImplementation = Azure::Core::Amqp::_detail::RustInterop::RustAmqpMessageBuilder;
#endif

  template <> struct UniqueHandleHelper<MessageImplementation>
  {
    static void FreeAmqpMessage(MessageImplementation* obj);

    using type = Core::_internal::BasicUniqueHandle<MessageImplementation, FreeAmqpMessage>;
  };

  #if ENABLE_RUST_AMQP
  template <> struct UniqueHandleHelper<MessageBuilderImplementation>
  {
    static void FreeAmqpMessageBuilder(MessageBuilderImplementation* obj);

    using type = Core::_internal::BasicUniqueHandle<MessageBuilderImplementation, FreeAmqpMessageBuilder>;
  };
#endif
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {
  using UniqueMessageHandle
      = Amqp::_detail::UniqueHandle<Azure::Core::Amqp::_detail::MessageImplementation>;
  #if ENABLE_RUST_AMQP
  using UniqueMessageBuilderHandle
      = Amqp::_detail::UniqueHandle<Azure::Core::Amqp::_detail::MessageBuilderImplementation>;
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
    static std::shared_ptr<AmqpMessage> FromImplementation(
        Azure::Core::Amqp::_detail::MessageImplementation* message);
    static UniqueMessageHandle ToImplementation(AmqpMessage const& message);
  };
}}}}} // namespace Azure::Core::Amqp::Models::_detail
