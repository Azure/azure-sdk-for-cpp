// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/messaging/eventhubs/event_data_batch.hpp"

#include "private/event_data_models_private.hpp"
#include "private/eventhub_constants.hpp"

namespace Azure { namespace Messaging { namespace EventHubs {

  void EventDataBatch::AddMessage(Azure::Messaging::EventHubs::Models::EventData& message)
  {
    auto amqpMessage = _detail::EventDataFactory::EventDataToAmqpMessage(message);
    AddAmqpMessage(amqpMessage);
  }

  Azure::Core::Amqp::Models::AmqpMessage EventDataBatch::ToAmqpMessage() const
  {
    Azure::Core::Amqp::Models::AmqpMessage returnValue{m_batchEnvelope};
    if (m_marshalledMessages.size() == 0)
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
    //      Azure::Core::Diagnostics::_internal::Log::Stream(
    //          Azure::Core::Diagnostics::Logger::Level::Verbose)
    //          << "EventDataBatch::ToAmqpMessage: " << returnValue << std::endl;
    return returnValue;
  }

      void EventDataBatch::AddAmqpMessage(Azure::Core::Amqp::Models::AmqpMessage& message)
  {
    std::lock_guard<std::mutex> lock(m_rwMutex);

    if (!message.Properties.MessageId.HasValue())
    {
      message.Properties.MessageId
          = Azure::Core::Amqp::Models::AmqpValue(Azure::Core::Uuid::CreateUuid().ToString());
    }

    if (!m_partitionKey.empty())
    {
      message.MessageAnnotations.emplace(
          _detail::PartitionKeyAnnotation, Azure::Core::Amqp::Models::AmqpValue(m_partitionKey));
    }

    auto serializedMessage = Azure::Core::Amqp::Models::AmqpMessage::Serialize(message);

    if (m_marshalledMessages.size() == 0)
    {
      // The first message is special - we use its properties and annotations on the envelope for
      // the batch message.
      m_batchEnvelope = CreateBatchEnvelope(message);
      m_currentSize = serializedMessage.size();
    }
    auto actualPayloadSize = CalculateActualSizeForPayload(serializedMessage);
    if (m_currentSize + actualPayloadSize > m_maxBytes)
    {
      m_currentSize = 0;
      m_batchEnvelope = nullptr;

      throw std::runtime_error("EventDataBatch size is too large.");
    }

    m_currentSize += actualPayloadSize;
    m_marshalledMessages.push_back(serializedMessage);
  }



}}} // namespace Azure::Messaging::EventHubs
