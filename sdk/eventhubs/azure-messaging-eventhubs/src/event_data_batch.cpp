// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/messaging/eventhubs/event_data_batch.hpp"

#include "private/eventhubs_constants.hpp"
#include "private/eventhubs_utilities.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

namespace Azure { namespace Messaging { namespace EventHubs {

  bool EventDataBatch::TryAdd(Azure::Messaging::EventHubs::Models::EventData const& message)
  {
    return TryAddAmqpMessage(message.GetRawAmqpMessage());
  }

  Azure::Core::Amqp::Models::AmqpMessage EventDataBatch::ToAmqpMessage() const
  {
    Azure::Core::Amqp::Models::AmqpMessage returnValue{m_batchEnvelope};
    if (m_marshalledMessages.empty())
    {
      throw std::runtime_error("No messages added to the batch.");
    }

    // Make sure that the partition key in the message is the current partition key.
    if (!m_partitionKey.empty())
    {
      returnValue.DeliveryAnnotations.emplace(
          _detail::PartitionKeyAnnotation, Azure::Core::Amqp::Models::AmqpValue(m_partitionKey));
    }

    std::vector<Azure::Core::Amqp::Models::AmqpBinaryData> messageList;
    for (auto const& marshalledMessage : m_marshalledMessages)
    {
      Azure::Core::Amqp::Models::AmqpBinaryData data(marshalledMessage);
      messageList.push_back(data);
    }

    returnValue.SetBody(messageList);
    return returnValue;
  }

  bool EventDataBatch::TryAddAmqpMessage(
      std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage const> const& message)
  {
    Azure::Core::Amqp::Models::AmqpMessage messageToSend{*message};

    // Fix up some properties in the message to send if they have not been already set.
    if (message->Properties.MessageId.IsNull())
    {
      messageToSend.Properties.MessageId
          = Azure::Core::Amqp::Models::AmqpValue(Azure::Core::Uuid::CreateUuid().ToString());
    }

    if (!m_partitionKey.empty())
    {
      messageToSend.MessageAnnotations.emplace(
          _detail::PartitionKeyAnnotation, Azure::Core::Amqp::Models::AmqpValue(m_partitionKey));
    }

    auto serializedMessage = Azure::Core::Amqp::Models::AmqpMessage::Serialize(messageToSend);

    std::lock_guard<std::mutex> lock(m_rwMutex);

    if (m_marshalledMessages.empty())
    {
      // The first message is special - we use its properties and annotations on the envelope for
      // the batch message.
      m_batchEnvelope = CreateBatchEnvelope(message);
      m_currentSize = serializedMessage.size();
    }
    auto actualPayloadSize = CalculateActualSizeForPayload(serializedMessage);
    if (m_currentSize + actualPayloadSize > m_maxBytes.Value())
    {
      Log::Stream(Logger::Level::Informational)
          << "Batch is full. Cannot add more messages. "
          << "Message size: " << actualPayloadSize << " size: " << m_currentSize
          << " Max size: " << m_maxBytes.Value() << std::endl;
      // If we don't have any messages and we can't add this one, then we can't add it at all.
      // Discard the contents of the batch.
      if (m_marshalledMessages.empty())
      {
        m_currentSize = 0;
        m_batchEnvelope = nullptr;
      }
      return false;
    }

    m_currentSize += actualPayloadSize;
    m_marshalledMessages.push_back(serializedMessage);
    return true;
  }

}}} // namespace Azure::Messaging::EventHubs

namespace Azure { namespace Messaging { namespace EventHubs { namespace _detail {
  EventDataBatch EventDataBatchFactory::CreateEventDataBatch(EventDataBatchOptions const& options)
  {
    return EventDataBatch{options};
  }
}}}} // namespace Azure::Messaging::EventHubs::_detail
