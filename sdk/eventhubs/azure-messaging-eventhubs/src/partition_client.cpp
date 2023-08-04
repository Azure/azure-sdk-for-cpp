// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/messaging/eventhubs/partition_client.hpp"

#include "azure/messaging/eventhubs/eventhubs_exception.hpp"
#include "private/eventhubs_utilities.hpp"
#include "private/retry_operation.hpp"

#include <azure/core/amqp.hpp>

namespace Azure { namespace Messaging { namespace EventHubs {
  /** Receive events from the partition.
   *
   * @param maxMessages The maximum number of messages to receive.
   * @param context A context to control the request lifetime.
   * @return A vector of received events.
   *
   */
  std::vector<Models::ReceivedEventData> PartitionClient::ReceiveEvents(
      uint32_t maxMessages,
      Core::Context const& context)
  {
    std::vector<Models::ReceivedEventData> messages;
    // bool prefetchDisabled = m_prefetchCount < 0;

    while (messages.size() < maxMessages && !context.IsCancelled())
    {
      auto message = m_receivers[0].WaitForIncomingMessage(context);
      if (message.first.HasValue())
      {
        messages.push_back(Models::ReceivedEventData{message.first.Value()});
      }
      else
      {
        throw _detail::EventHubsExceptionFactory::CreateEventHubsException(message.second);
      }
    }
    return messages;
  }
}}} // namespace Azure::Messaging::EventHubs
