// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/messaging/eventhubs/models/event_data.hpp"

namespace Azure { namespace Messaging { namespace EventHubs { namespace _detail {
  class EventDataFactory {
  public:
    static Azure::Core::Amqp::Models::AmqpMessage EventDataToAmqpMessage(
        Models::EventData const& message);
    static void EventBodyToAmqpMessageBody(
        Models::EventDataBody const& body,
        Azure::Core::Amqp::Models::AmqpMessage& messageToUpdate);

  private:
    EventDataFactory() = delete;
  };

}}}} // namespace Azure::Messaging::EventHubs::_detail
