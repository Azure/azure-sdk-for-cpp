// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/models/amqp_header.hpp"

#include "azure/core/amqp/models/amqp_value.hpp"
#include "private/header_impl.hpp"
#include "private/value_impl.hpp"

#if ENABLE_UAMQP

#include <azure_uamqp_c/amqp_definitions_milliseconds.h>

#include <azure_uamqp_c/amqp_definitions_header.h>
#elif ENABLE_RUST_AMQP
#include "azure/core/amqp/internal/common/runtime_context.hpp"

using namespace Azure::Core::Amqp::RustInterop::_detail;
#endif

#include <chrono>
#include <iostream>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  // @cond
  void UniqueHandleHelper<HeaderImplementation>::FreeAmqpHeader(HeaderImplementation* handle)
  {
    header_destroy(handle);
  }
  // @endcond
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models {
  bool MessageHeader::operator==(MessageHeader const& that) const noexcept
  {
    return this->DeliveryCount == that.DeliveryCount && this->Durable == that.Durable
        && this->IsFirstAcquirer == that.IsFirstAcquirer && this->Priority == that.Priority
        && this->TimeToLive.HasValue() == that.TimeToLive.HasValue()
        && (this->TimeToLive.HasValue() ? this->TimeToLive.Value() == that.TimeToLive.Value()
                                        : true);
  }

  MessageHeader _detail::MessageHeaderFactory::FromImplementation(
      _detail::UniqueMessageHeaderHandle const& handle)
  {
    MessageHeader rv;
    bool boolValue;
    if (handle)
    {
      if (!header_get_durable(handle.get(), &boolValue))
      {
        rv.Durable = boolValue;
      }

      uint8_t uint8Value;
      if (!header_get_priority(handle.get(), &uint8Value))
      {
        rv.Priority = uint8Value;
      }

#if ENABLE_UAMQP
      milliseconds millisecondsValue;
      if (!header_get_ttl(handle.get(), &millisecondsValue))
      {
        rv.TimeToLive = std::chrono::milliseconds(millisecondsValue);
      }
#else
      uint64_t millisecondsValue;
      if (!header_get_ttl(handle.get(), &millisecondsValue))
      {
        rv.TimeToLive = std::chrono::milliseconds(millisecondsValue);
      }
#endif

      if (!header_get_first_acquirer(handle.get(), &boolValue))
      {
        rv.IsFirstAcquirer = boolValue;
      }

      uint32_t uint32Value;
      if (!header_get_delivery_count(handle.get(), &uint32Value))
      {
        rv.DeliveryCount = uint32Value;
      }
    }
    return rv;
  }

  _detail::UniqueMessageHeaderHandle _detail::MessageHeaderFactory::ToImplementation(
      MessageHeader const& header)
  {
    _detail::UniqueMessageHeaderHandle rv{header_create()};
#if ENABLE_UAMQP
    if (header.Durable)
    {
      if (header_set_durable(rv.get(), header.Durable))
      {
        throw std::runtime_error("Could not set durable value.");
      }
    }
    if (header.Priority != 4)
    {
      if (header_set_priority(rv.get(), header.Priority))
      {
        throw std::runtime_error("Could not set priority value.");
      }
    }
    if (header.TimeToLive.HasValue())
    {
      if (header_set_ttl(rv.get(), static_cast<milliseconds>(header.TimeToLive.Value().count())))
      {
        throw std::runtime_error("Could not set header TTL.");
      }
    }

    if (header.IsFirstAcquirer)
    {
      if (header_set_first_acquirer(rv.get(), header.IsFirstAcquirer))
      {
        throw std::runtime_error("Could not set first acquirer value.");
      }
    }
    if (header.DeliveryCount != 0)
    {
      if (header_set_delivery_count(rv.get(), header.DeliveryCount))
      {
        throw std::runtime_error("Could not set delivery count value.");
      }
    }
#elif ENABLE_RUST_AMQP
    if (header.Durable)
    {
      Common::_detail::InvokeAmqpApi(header_set_durable, rv, header.Durable);
    }
    if (header.Priority != 4)
    {
      Common::_detail::InvokeAmqpApi(header_set_priority, rv, header.Priority);
    }
    if (header.TimeToLive.HasValue())
    {
      Common::_detail::InvokeAmqpApi(header_set_ttl, rv, header.TimeToLive.Value().count());
    }

    if (header.IsFirstAcquirer)
    {
      Common::_detail::InvokeAmqpApi(header_set_first_acquirer, rv, header.IsFirstAcquirer);
    }
    if (header.DeliveryCount != 0)
    {
      Common::_detail::InvokeAmqpApi(header_set_delivery_count, rv, header.DeliveryCount);
    }
#endif
    return rv;
  }

  std::ostream& operator<<(std::ostream& os, MessageHeader const& header)
  {
    os << "Header{";
    os << "durable=" << header.Durable;
    os << ", priority=" << std::dec << static_cast<int>(header.Priority);
    if (header.TimeToLive.HasValue())
    {
      os << ", ttl=" << header.TimeToLive.Value().count() << " milliseconds";
    }
    os << ", firstAcquirer=" << header.IsFirstAcquirer;
    os << ", deliveryCount=" << header.DeliveryCount;
    os << "}";
    return os;
  }

  bool MessageHeader::ShouldSerialize() const noexcept
  {
    return Durable || (Priority != 4) || TimeToLive.HasValue() || IsFirstAcquirer
        || (DeliveryCount != 0);
  }

  size_t MessageHeader::GetSerializedSize(MessageHeader const& header)
  {
    auto handle = _detail::MessageHeaderFactory::ToImplementation(header);
    AmqpValue propertiesAsValue{_detail::AmqpValueFactory::FromImplementation(
        _detail::UniqueAmqpValueHandle{amqpvalue_create_header(handle.get())})};
    return AmqpValue::GetSerializedSize(propertiesAsValue);
  }

  std::vector<uint8_t> MessageHeader::Serialize(MessageHeader const& header)
  {
    auto handle = _detail::MessageHeaderFactory::ToImplementation(header);
    AmqpValue headerAsValue{_detail::AmqpValueFactory::FromImplementation(
        Models::_detail::UniqueAmqpValueHandle{amqpvalue_create_header(handle.get())})};
    return Models::AmqpValue::Serialize(headerAsValue);
  }

  MessageHeader MessageHeader::Deserialize(std::uint8_t const* data, size_t size)
  {
    AmqpValue value{AmqpValue::Deserialize(data, size)};

    Azure::Core::Amqp::_detail::HeaderImplementation* handle;
    if (amqpvalue_get_header(_detail::AmqpValueFactory::ToImplementation(value), &handle))
    {
      throw std::runtime_error("Could not convert value to AMQP Header.");
    }
    _detail::UniqueMessageHeaderHandle uniqueHandle{handle};
    handle = nullptr;
    return _detail::MessageHeaderFactory::FromImplementation(uniqueHandle);
  }

}}}} // namespace Azure::Core::Amqp::Models
