// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../../amqp/private/unique_handle.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"

#if ENABLE_UAMQP
#include <azure_uamqp_c/amqpvalue.h>
#endif // ENABLE_UAMQP

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
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
#endif

}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {
#if ENABLE_UAMQP
  using UniqueAmqpValueHandle = Amqp::_detail::UniqueHandle<std::remove_pointer<AMQP_VALUE>::type>;
  using UniqueAmqpDecoderHandle
      = Amqp::_detail::UniqueHandle<std::remove_pointer<AMQPVALUE_DECODER_HANDLE>::type>;
#endif

  class AmqpValueFactory final {
  public:
#if ENABLE_UAMQP
    static AmqpValue FromUamqp(UniqueAmqpValueHandle const& value);
    static AmqpValue FromUamqp(UniqueAmqpValueHandle&& value);
    static AmqpValue FromUamqp(AmqpValueImpl&& value);
    // Returns the internal AMQP value handle, without referencing it.
    static AMQP_VALUE ToUamqp(AmqpValue const& value);
#endif // ENABLE_UAMQP
  };
#if ENABLE_UAMQP
  std::ostream& operator<<(std::ostream& os, AMQP_VALUE_DATA_TAG* const value);
#endif
  class AmqpValueImpl final {
    friend class Azure::Core::Amqp::Models::AmqpValue;
    friend class AmqpValueFactory;

  public:
#if ENABLE_UAMQP
    AmqpValueImpl(UniqueAmqpValueHandle&& value) noexcept : m_value(std::move(value)) {}
#else
    AmqpValueImpl() = default;
#endif
    AmqpValueImpl(AmqpValueImpl const& other);
    AmqpValueImpl(AmqpValueImpl&& other) noexcept;

#if ENABLE_UAMQP
    operator AMQP_VALUE() const noexcept { return m_value.get(); }
#endif

  private:
#if ENABLE_UAMQP
    UniqueAmqpValueHandle m_value;
#endif
  };
#if ENABLE_UAMQP
  std::ostream& operator<<(std::ostream& os, AMQP_TYPE value);
  std::ostream& operator<<(std::ostream& os, AMQP_VALUE const value);
#endif
}}}}} // namespace Azure::Core::Amqp::Models::_detail
