// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../../amqp/private/unique_handle.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"

#if ENABLE_UAMQP
#include <azure_uamqp_c/amqpvalue.h>
#endif
#if ENABLE_RUST_AMQP
#include "../rust_amqp/rust_wrapper/rust_amqp_wrapper.h"

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
#if ENABLE_RUST_AMQP
  template <> struct UniqueHandleHelper<RustInterop::AmqpValue>
  {
    // @cond INTERNAL
    static void FreeAmqpValue(RustInterop::AmqpValue* value);
    using type = Core::_internal::BasicUniqueHandle<RustInterop::AmqpValue, FreeAmqpValue>;
    // @endcond
  };
#endif

}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {

#if ENABLE_UAMQP
  using AmqpValueImplementation = std::remove_pointer<AMQP_VALUE>::type;
  using AmqpValueImplementationType = AMQP_TYPE;
#elif ENABLE_RUST_AMQP
  using AmqpValueImplementation = Azure::Core::Amqp::_detail::RustInterop::AmqpValue;
  using AmqpValueImplementationType = Azure::Core::Amqp::_detail::RustInterop::AmqpValueType;
#endif

  using UniqueAmqpValueHandle = Amqp::_detail::UniqueHandle<AmqpValueImplementation>;
#if ENABLE_UAMQP
  using UniqueAmqpDecoderHandle
      = Amqp::_detail::UniqueHandle<std::remove_pointer<AMQPVALUE_DECODER_HANDLE>::type>;
#endif

  class AmqpValueFactory final {
  public:
    static AmqpValue FromImplementation(UniqueAmqpValueHandle const& value);
    static AmqpValue FromImplementation(UniqueAmqpValueHandle&& value);
    static AmqpValue FromImplementation(AmqpValueImpl&& value);
    // Returns the internal AMQP value handle, without referencing it.
    static AmqpValueImplementation* ToImplementation(AmqpValue const& value);
  };
  std::ostream& operator<<(std::ostream& os, AmqpValueImplementation const value);
  class AmqpValueImpl final {
    friend class Azure::Core::Amqp::Models::AmqpValue;
    friend class AmqpValueFactory;

  public:
    AmqpValueImpl(UniqueAmqpValueHandle&& value) noexcept : m_value(std::move(value)) {}
#if ENABLE_RUST_AMQP
    AmqpValueImpl() = default;
#endif
    AmqpValueImpl(AmqpValueImpl const& other);
    AmqpValueImpl(AmqpValueImpl&& other) noexcept;

    operator AmqpValueImplementation*() const noexcept { return m_value.get(); }

  private:
    UniqueAmqpValueHandle m_value;
  };
  std::ostream& operator<<(std::ostream& os, AmqpValueImplementation value);
  std::ostream& operator<<(std::ostream& os, AmqpValueImplementationType const value);
}}}}} // namespace Azure::Core::Amqp::Models::_detail
