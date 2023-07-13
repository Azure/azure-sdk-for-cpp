// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/models/amqp_header.hpp"

#include "azure/core/amqp/models/amqp_value.hpp"

#include <azure_uamqp_c/amqp_definitions_milliseconds.h>

#include <azure_uamqp_c/amqp_definitions_header.h>

#include <chrono>
#include <iostream>

namespace Azure { namespace Core { namespace _internal {
  void UniqueHandleHelper<HEADER_INSTANCE_TAG>::FreeAmqpHeader(HEADER_HANDLE handle)
  {
    header_destroy(handle);
  }
}}} // namespace Azure::Core::_internal

namespace Azure { namespace Core { namespace Amqp { namespace Models {
  bool MessageHeader::operator==(MessageHeader const& that) const noexcept
  {
    return this->DeliveryCount == that.DeliveryCount && this->Durable == that.Durable
        && this->IsFirstAcquirer == that.IsFirstAcquirer && this->Priority == that.Priority
        && this->TimeToLive.HasValue() == that.TimeToLive.HasValue()
        && (this->TimeToLive.HasValue() ? this->TimeToLive.Value() == that.TimeToLive.Value()
                                        : true);
  }

  MessageHeader _internal::MessageHeaderFactory::FromUamqp(UniqueMessageHeaderHandle const& handle)
  {
    MessageHeader rv;
    bool boolValue;
    if (!header_get_durable(handle.get(), &boolValue))
    {
      rv.Durable = boolValue;
    }

    uint8_t uint8Value;
    if (!header_get_priority(handle.get(), &uint8Value))
    {
      rv.Priority = uint8Value;
    }

    milliseconds millisecondsValue;
    if (!header_get_ttl(handle.get(), &millisecondsValue))
    {
      rv.TimeToLive = std::chrono::milliseconds(millisecondsValue);
    }

    if (!header_get_first_acquirer(handle.get(), &boolValue))
    {
      rv.IsFirstAcquirer = boolValue;
    }

    uint32_t uint32Value;
    if (!header_get_delivery_count(handle.get(), &uint32Value))
    {
      rv.DeliveryCount = uint32Value;
    }
    return rv;
  }

  UniqueMessageHeaderHandle _internal::MessageHeaderFactory::ToUamqp(MessageHeader const& header)
  {
    UniqueMessageHeaderHandle rv{header_create()};
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
        throw std::runtime_error("Could not set header TTL."); // LCOV_EXCL_LINE
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

  size_t MessageHeader::GetSerializedSize(MessageHeader const& properties)
  {
    auto handle = _internal::MessageHeaderFactory::ToUamqp(properties);
    AmqpValue propertiesAsValue{amqpvalue_create_header(handle.get())};
    return AmqpValue::GetSerializedSize(propertiesAsValue);
  }

  std::vector<uint8_t> MessageHeader::Serialize(MessageHeader const& header)
  {
    auto handle = _internal::MessageHeaderFactory::ToUamqp(header);
    AmqpValue headerAsValue{amqpvalue_create_header(handle.get())};
    return Models::AmqpValue::Serialize(headerAsValue);
  }

  MessageHeader MessageHeader::Deserialize(std::uint8_t const* data, size_t size)
  {
    AmqpValue value{AmqpValue::Deserialize(data, size)};
    HEADER_HANDLE handle;
    if (amqpvalue_get_header(value, &handle))
    {
      throw std::runtime_error("Could not convert value to AMQP Header.");
    }
    UniqueMessageHeaderHandle uniqueHandle{handle};
    handle = nullptr;
    return _internal::MessageHeaderFactory::FromUamqp(uniqueHandle);
  }

}}}} // namespace Azure::Core::Amqp::Models
