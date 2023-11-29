// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../../amqp/private/unique_handle.hpp"

#include <azure_uamqp_c/amqpvalue.h>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  template <> struct UniqueHandleHelper<std::remove_pointer<AMQP_VALUE>::type>
  {
    // @cond INTERNAL
    static void FreeAmqpValue(AMQP_VALUE value);
    using type
        = Core::_internal::BasicUniqueHandle<std::remove_pointer<AMQP_VALUE>::type, FreeAmqpValue>;
    // @endcond
  };

  template <> struct UniqueHandleHelper<std::remove_pointer<AMQPVALUE_DECODER_HANDLE>::type>
  {
    // @cond INTERNAL
    static void FreeAmqpDecoder(AMQPVALUE_DECODER_HANDLE value);
    using type = Core::_internal::
        BasicUniqueHandle<std::remove_pointer<AMQPVALUE_DECODER_HANDLE>::type, FreeAmqpDecoder>;
    // @endcond
  };
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {
  using UniqueAmqpValueHandle = Amqp::_detail::UniqueHandle<std::remove_pointer<AMQP_VALUE>::type>;
  using UniqueAmqpDecoderHandle
      = Amqp::_detail::UniqueHandle<std::remove_pointer<AMQPVALUE_DECODER_HANDLE>::type>;

  class AmqpValueFactory final {
  public:
    static AmqpValue FromUamqp(UniqueAmqpValueHandle const& value);
    static AmqpValue FromUamqp(UniqueAmqpValueHandle&& value);
    static AmqpValue FromUamqp(AmqpValueImpl&& value);
    // Returns the internal AMQP value handle, without referencing it.
    static AMQP_VALUE ToUamqp(AmqpValue const& value);
  };
  std::ostream& operator<<(std::ostream& os, AMQP_VALUE_DATA_TAG* const value);

  class AmqpValueImpl final {
    friend class Azure::Core::Amqp::Models::AmqpValue;
    friend class AmqpValueFactory;

  public:
    AmqpValueImpl(UniqueAmqpValueHandle&& value) noexcept : m_value(std::move(value)) {}
    AmqpValueImpl(AmqpValueImpl const& other);
    AmqpValueImpl(AmqpValueImpl&& other) noexcept;

    operator AMQP_VALUE() const noexcept { return m_value.get(); }

  private:
    UniqueAmqpValueHandle m_value;
  };

}}}}} // namespace Azure::Core::Amqp::Models::_detail
