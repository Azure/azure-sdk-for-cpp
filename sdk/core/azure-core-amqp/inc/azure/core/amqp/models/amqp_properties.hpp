// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "amqp_value.hpp"
#include <azure/core/nullable.hpp>
#include <chrono>
#include <vector>

struct PROPERTIES_INSTANCE_TAG;

namespace Azure { namespace Core { namespace _internal {

  template <> struct UniqueHandleHelper<PROPERTIES_INSTANCE_TAG>
  {
    static void FreeAmqpProperties(PROPERTIES_INSTANCE_TAG* obj);

    using type
        = Azure::Core::_internal::BasicUniqueHandle<PROPERTIES_INSTANCE_TAG, FreeAmqpProperties>;
  };
}}} // namespace Azure::Core::_internal

namespace Azure { namespace Core { namespace Amqp { namespace Models {

    class MessageProperties final {
      // uAMQP interop functions.
    public:
      MessageProperties(PROPERTIES_INSTANCE_TAG* properties); //: m_properties(properties) {}
      operator PROPERTIES_INSTANCE_TAG*() const;

    public:
      MessageProperties();
      ~MessageProperties();

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

      friend std::ostream& operator<<(std::ostream&, MessageProperties const&);
    };

}}}} // namespace Azure::Core::Amqp::Models
