// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once
#include "models/amqp_message.hpp"
#include "models/event_data_batch_models.hpp"
#include <azure/core/amqp.hpp>

#include <mutex>

// cspell: words vbin

namespace Azure { namespace Messaging { namespace EventHubs {
  /**@brief EventDataBatch is used to efficiently pack up EventData before sending it to Event Hubs.
   *
   * @remark EventDataBatch's are not meant to be created directly. Use
   * [ProducerClient.NewEventDataBatch], which will create them with the proper size limit for your
   * Event Hub.
   */
  class EventDataBatch {
  private:
    std::mutex m_rwMutex;
    std::string m_partitionID;
    std::string m_partitionKey;
    uint64_t m_maxBytes;
    const std::string anyPartitionId = "";
    std::vector<std::vector<uint8_t>> m_marshalledMessages;
    // Annotation properties
    const std::string PartitionKeyAnnotation = "x-opt-partition-key";
    const std::string SequenceNumberAnnotation = "x-opt-sequence-number";
    const std::string OffsetNumberAnnotation = "x-opt-offset";
    const std::string EnqueuedTimeAnnotation = "x-opt-enqueued-time";

    Azure::Core::Amqp::Models::AmqpMessage m_batchEnvelope;
    uint64_t m_currentSize;

  public:
    EventDataBatch(EventDataBatch const& other)
        : m_partitionID(other.m_partitionID), m_partitionKey(other.m_partitionKey),
          m_maxBytes(other.m_maxBytes), m_marshalledMessages(other.m_marshalledMessages),
          m_batchEnvelope(other.m_batchEnvelope), m_currentSize(other.m_currentSize)
    {
    }

    EventDataBatch& operator=(EventDataBatch const& other)
    {
        if (&other == this)
        {
		return *this;
	  }
	  m_partitionID = other.m_partitionID;
	  m_partitionKey = other.m_partitionKey;
	  m_maxBytes = other.m_maxBytes;
	  m_marshalledMessages = other.m_marshalledMessages;
	  m_batchEnvelope = other.m_batchEnvelope;
	  m_currentSize = other.m_currentSize;
	  return *this;
	}
    /** @brief Event Data Batch constructor
     *
     * @param options Options settings for creating the data batch
     */
    EventDataBatch(Models::EventDataBatchOptions options = {})
    {
      SetPartitionID(anyPartitionId);

      if (!options.PartitionID.empty() && !options.PartitionKey.empty())
      {
        throw std::exception("Either PartionID or PartitionKey can be set.");
      }

      if (!options.PartitionID.empty())
      {
        SetPartitionID(options.PartitionID);
      }
      else if (!options.PartitionKey.empty())
      {
        SetPartitionKey(options.PartitionKey);
      }

      if (options.MaxBytes == 0)
      {
        SetMaxBytes(std::numeric_limits<uint16_t>::max());
      }
      else
      {
        SetMaxBytes(options.MaxBytes);
      }
    };

    void SetPartitionID(std::string partitionID) { m_partitionID = partitionID; }
    void SetPartitionKey(std::string partitionKey) { m_partitionKey = m_partitionKey; }
    void SetMaxBytes(uint64_t maxBytes) { m_maxBytes = maxBytes; }

    std::string GetPartitionID() { return m_partitionID; }
    std::string GetPartitionKey() { return m_partitionKey; }
    uint64_t GetMaxBytes() { return m_maxBytes; }

    /** @brief Adds a message to the data batch
     *
     * @param message The message to add to the batch
     */
    void AddMessage(Azure::Messaging::EventHubs::Models::AmqpAnnotatedMessage& message)
    {
      auto amqpMessage = message.ToAMQPMessage();

      AddAmqpMessage(amqpMessage);
    }

    /** @brief Adds a message to the data batch
     *
     * @param message The message to add to the batch
     */
    void AddMessage(Azure::Messaging::EventHubs::Models::EventData& message)
    {
      auto amqpMessage = message.ToAMQPMessage();

      AddAmqpMessage(amqpMessage);
    }

    /** @brief Gets the number of messages in the batch
     *
     */
    const size_t CurrentSize()
    {
      std::lock_guard<std::mutex> lock(m_rwMutex);
      return m_currentSize;
    }

    Azure::Core::Amqp::Models::AmqpMessage ToAmqpMessage()
    {
      if (m_marshalledMessages.size() == 0)
      {
        throw std::exception("No messages added to the batch.");
      }

     /* if (!m_partitionKey.empty())
      {
        m_batchEnvelope.DeliveryAnnotations.emplace(
            PartitionKeyAnnotation, Azure::Core::Amqp::Models::AmqpValue(m_partitionKey));
      }*/
      std::vector<Azure::Core::Amqp::Models::AmqpBinaryData> messageList;
      for (auto marshalledMessage : m_marshalledMessages)
      {
          Azure::Core::Amqp::Models::AmqpBinaryData data(marshalledMessage);
        messageList.push_back(data);
	  }

      m_batchEnvelope.SetBody(messageList);

      return m_batchEnvelope;
    }

  private:
    void AddAmqpMessage(Azure::Core::Amqp::Models::AmqpMessage& message)
    {
      std::lock_guard<std::mutex> lock(m_rwMutex);

      if (!message.Properties.MessageId.HasValue())
      {
        message.Properties.MessageId
            = Azure::Core::Amqp::Models::AmqpValue(Azure::Core::Uuid::CreateUuid().ToString());
      }

      /* if (!m_partitionKey.empty())
      {
        message.DeliveryAnnotations.emplace(
            PartitionKeyAnnotation, Azure::Core::Amqp::Models::AmqpValue(m_partitionKey));
      }*/

      auto serializedMessage = Azure::Core::Amqp::Models::AmqpMessage::Serialize(message);

      if (m_marshalledMessages.size() == 0)
      {
        m_batchEnvelope = CreateBatchEnvelope(message);
        m_currentSize = Azure::Core::Amqp::Models::AmqpMessage::Serialize(message).size();
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

    uint64_t CalculateActualSizeForPayload(std::vector<uint8_t> payload)
    {
      const uint64_t vbin8Overhead = 5;
      const uint64_t vbin32Overhead = 8;

      if (payload.size() < 256)
      {
        return payload.size() + vbin8Overhead;
      }
      return payload.size() + vbin32Overhead;
    }

    Azure::Core::Amqp::Models::AmqpMessage CreateBatchEnvelope(
        Azure::Core::Amqp::Models::AmqpMessage const& message)
    {
      Azure::Core::Amqp::Models::AmqpMessage batchEnvelope;
      batchEnvelope.Header = message.Header;
      batchEnvelope.DeliveryAnnotations = message.DeliveryAnnotations;
      batchEnvelope.MessageAnnotations = message.MessageAnnotations;
      batchEnvelope.Properties = message.Properties;
      batchEnvelope.ApplicationProperties = message.ApplicationProperties;
      batchEnvelope.Footer = message.Footer;
      return batchEnvelope;
    }
  };
}}} // namespace Azure::Messaging::EventHubs
