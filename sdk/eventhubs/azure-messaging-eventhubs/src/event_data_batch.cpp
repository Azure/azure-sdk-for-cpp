// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/messaging/eventhubs/event_data_batch.hpp"

#include "private/event_data_models_private.hpp"

namespace Azure { namespace Messaging { namespace EventHubs {

  void EventDataBatch::AddMessage(Azure::Messaging::EventHubs::Models::EventData& message)
  {
    auto amqpMessage = _detail::EventDataFactory::EventDataToAmqpMessage(message);
    AddAmqpMessage(amqpMessage);
  }

}}} // namespace Azure::Messaging::EventHubs
