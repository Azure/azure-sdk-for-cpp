// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/internal/unique_handle.hpp>

#include <azure_uamqp_c/amqpvalue.h>

namespace Azure { namespace Core { namespace _internal {
  template <> struct UniqueHandleHelper<std::remove_pointer<AMQP_VALUE>::type>
  {
    /**
     * Free a uAMQP Value object.
     *
     * @param obj value handle to free.
     */
    static void FreeAmqpValue(AMQP_VALUE obj);
    using type = BasicUniqueHandle<std::remove_pointer<AMQP_VALUE>::type, FreeAmqpValue>;
  };

  template <> struct UniqueHandleHelper<std::remove_pointer<AMQPVALUE_DECODER_HANDLE>::type>
  {
    /**
     * Free a uAMQP Decoder object.
     *
     * @param obj Decoder handle to free.
     */
    static void FreeAmqpDecoder(AMQPVALUE_DECODER_HANDLE obj);
    using type
        = BasicUniqueHandle<std::remove_pointer<AMQPVALUE_DECODER_HANDLE>::type, FreeAmqpDecoder>;
  };

}}} // namespace Azure::Core::_internal

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {
  using UniqueAmqpValueHandle
      = Azure::Core::_internal::UniqueHandle<std::remove_pointer<AMQP_VALUE>::type>;
  using UniqueAmqpDecoderHandle
      = Azure::Core::_internal::UniqueHandle<std::remove_pointer<AMQPVALUE_DECODER_HANDLE>::type>;

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

    operator AMQP_VALUE_DATA_TAG*() const noexcept { return m_value.get(); }

  private:
    UniqueAmqpValueHandle m_value;
  };

}}}}} // namespace Azure::Core::Amqp::Models::_detail
