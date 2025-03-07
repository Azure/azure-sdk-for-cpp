// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../../amqp/private/unique_handle.hpp"
#include "azure/core/amqp/models/amqp_properties.hpp"

#if ENABLE_UAMQP
extern "C"
{
  typedef struct PROPERTIES_INSTANCE_TAG* PROPERTIES_HANDLE;
}
#elif ENABLE_RUST_AMQP
#include "rust_amqp_wrapper.h"
#endif // ENABLE_UAMQP
#include <type_traits>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  using PropertiesImplementation = std::remove_pointer<PROPERTIES_HANDLE>::type;
#elif ENABLE_RUST_AMQP
  using PropertiesImplementation = Azure::Core::Amqp::RustInterop::_detail::RustMessageProperties;
#endif

  template <> struct UniqueHandleHelper<PropertiesImplementation>
  {
    static void FreeAmqpProperties(PropertiesImplementation* obj);

    using type = Core::_internal::BasicUniqueHandle<PropertiesImplementation, FreeAmqpProperties>;
  };
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {

  using UniquePropertiesHandle
      = Amqp::_detail::UniqueHandle<Azure::Core::Amqp::_detail::PropertiesImplementation>;
  /**
   * @brief uAMQP interoperability functions to convert a MessageProperties to a uAMQP
   * PROPERTIES_HANDLE and back.
   *
   * @remarks This class should not be used directly. It is used by the uAMQP interoperability
   * layer.
   */
  class MessagePropertiesFactory final {
    MessagePropertiesFactory() = delete;

  public:
    static MessageProperties FromImplementation(UniquePropertiesHandle const& properties);
    static UniquePropertiesHandle ToImplementation(MessageProperties const& properties);
  };
}}}}} // namespace Azure::Core::Amqp::Models::_detail
