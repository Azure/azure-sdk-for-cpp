// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/messaging/eventhubs/models/event_data.hpp"

namespace Azure { namespace Messaging { namespace EventHubs { namespace _detail {
  class EventDataFactory {
  public:
    static Azure::Core::Amqp::Models::AmqpMessage EventDataToAmqpMessage(
        Models::EventData const& message);

  private:
    EventDataFactory() = delete;
  };

}}}} // namespace Azure::Messaging::EventHubs::_detail
