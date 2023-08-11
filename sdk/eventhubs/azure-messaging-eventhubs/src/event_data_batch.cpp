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

  bool EventDataBatch::AddMessage(Azure::Messaging::EventHubs::Models::EventData const& message)
  {
    return AddAmqpMessage(message.GetRawAmqpMessage());
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
      std::stringstream ss;
      _detail::EventHubsUtilities::LogRawBuffer(ss, marshalledMessage);
      Log::Stream(Logger::Level::Informational) << "Add marshalled AMQP message:" << ss.str();
      Azure::Core::Amqp::Models::AmqpBinaryData data(marshalledMessage);
      messageList.push_back(data);
    }

    returnValue.SetBody(messageList);
    return returnValue;
  }

  bool EventDataBatch::AddAmqpMessage(Azure::Core::Amqp::Models::AmqpMessage message)
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

    Log::Stream(Logger::Level::Informational) << "Insert AMQP message: " << message;
    auto serializedMessage = Azure::Core::Amqp::Models::AmqpMessage::Serialize(message);

    if (m_marshalledMessages.size() == 0)
    {
      // The first message is special - we use its properties and annotations on the envelope for
      // the batch message.
      m_batchEnvelope = CreateBatchEnvelope(message);
      m_currentSize = serializedMessage.size();
    }
    auto actualPayloadSize = CalculateActualSizeForPayload(serializedMessage);
    if (m_currentSize + actualPayloadSize > m_maxBytes.Value())
    {
      m_currentSize = 0;
      m_batchEnvelope = nullptr;
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
