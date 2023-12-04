// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../../amqp/private/unique_handle.hpp"
#include "azure/core/amqp/models/amqp_properties.hpp"

#include <azure_uamqp_c/amqp_definitions_sequence_no.h>

#include <azure_uamqp_c/amqp_definitions_properties.h>

#include <type_traits>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  template <> struct UniqueHandleHelper<std::remove_pointer<PROPERTIES_HANDLE>::type>
  {
    static void FreeAmqpProperties(PROPERTIES_HANDLE obj);

    using type = Core::_internal::
        BasicUniqueHandle<std::remove_pointer<PROPERTIES_HANDLE>::type, FreeAmqpProperties>;
  };

}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {
  using UniquePropertiesHandle
      = Amqp::_detail::UniqueHandle<std::remove_pointer<PROPERTIES_HANDLE>::type>;

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
    static MessageProperties FromUamqp(UniquePropertiesHandle const& properties);
    static UniquePropertiesHandle ToUamqp(MessageProperties const& properties);
  };
}}}}} // namespace Azure::Core::Amqp::Models::_detail
