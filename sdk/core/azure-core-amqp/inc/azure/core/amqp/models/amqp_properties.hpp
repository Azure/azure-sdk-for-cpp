// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "amqp_value.hpp"
#include <azure/core/internal/unique_handle.hpp>
#include <azure/core/nullable.hpp>
#include <chrono>
#include <vector>

struct PROPERTIES_INSTANCE_TAG;

template <> struct Azure::Core::_internal::UniqueHandleHelper<PROPERTIES_INSTANCE_TAG>
{
  static void FreeAmqpProperties(PROPERTIES_INSTANCE_TAG* obj);

  using type
      = Azure::Core::_internal::BasicUniqueHandle<PROPERTIES_INSTANCE_TAG, FreeAmqpProperties>;
};
namespace Azure { namespace Core { namespace Amqp { namespace Models {

  using UniquePropertiesHandle = Azure::Core::_internal::UniqueHandle<PROPERTIES_INSTANCE_TAG>;

  struct MessageProperties final
  {
    MessageProperties() = default;
    ~MessageProperties() = default;

    Azure::Nullable<AmqpValue> MessageId;
    Azure::Nullable<AmqpValue> CorrelationId;
    Azure::Nullable<std::vector<uint8_t>> UserId;
    Azure::Nullable<AmqpValue> To;
    Azure::Nullable<std::string> Subject;
    Azure::Nullable<AmqpValue> ReplyTo;
    Azure::Nullable<std::string> ContentType;
    Azure::Nullable<std::string> ContentEncoding;
    Azure::Nullable<std::chrono::system_clock::time_point> AbsoluteExpiryTime;
    Azure::Nullable<std::chrono::system_clock::time_point> CreationTime;
    Azure::Nullable<std::string> GroupId;
    Azure::Nullable<uint32_t> GroupSequence;
    Azure::Nullable<std::string> ReplyToGroupId;

    bool operator==(MessageProperties const&) const noexcept;
    bool ShouldSerialize() const noexcept;

    static size_t GetSerializedSize(MessageProperties const& properties);
    static std::vector<uint8_t> Serialize(MessageProperties const& properties);
    static MessageProperties Deserialize(uint8_t const* data, size_t size);
  };
  std::ostream& operator<<(std::ostream&, MessageProperties const&);
}}}} // namespace Azure::Core::Amqp::Models

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {
    /**
     * @brief uAMQP interoperability functions to convert a MessageProperties to a uAMQP
     * PROPERTIES_HANDLE and back.
     *
     * @remarks This class should not be used directly. It is used by the uAMQP interoperability
     * layer.
     */
    class MessagePropertiesFactory {
    public:
      static MessageProperties FromUamqp(UniquePropertiesHandle const& properties);
      static UniquePropertiesHandle ToUamqp(MessageProperties const& properties);
    };
}}}}} // namespace Azure::Core::Amqp::Models::_internal
