// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once
#include "models/event_data.hpp"
#include "models/event_data_batch_models.hpp"

#include <azure/core/amqp/models/amqp_message.hpp>
#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

#include <mutex>
#include <stdexcept>

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
    const std::string anyPartitionId = "";
    const std::string PartitionKeyAnnotation = "x-opt-partition-key";
    const std::string SequenceNumberAnnotation = "x-opt-sequence-number";
    const std::string OffsetNumberAnnotation = "x-opt-offset";
    const std::string EnqueuedTimeAnnotation = "x-opt-enqueued-time";

    std::mutex m_rwMutex;
    std::string m_partitionID;
    std::string m_partitionKey;
    uint64_t m_maxBytes;
    std::vector<std::vector<uint8_t>> m_marshalledMessages;
    // Annotation properties
    const uint32_t BatchedMessageFormat = 0x80013700;

    Azure::Core::Amqp::Models::AmqpMessage m_batchEnvelope;
    size_t m_currentSize;

  public:
    /** @brief Constructs an EventDataBatch from another EventDataBatch
     *
     * @param other The EventDataBatch to copy
     */
    EventDataBatch(EventDataBatch const& other)
        // Copy constructor cannot be defaulted because of m_rwMutex.
        : m_rwMutex{}, m_partitionID{other.m_partitionID}, m_partitionKey{other.m_partitionKey},
          m_maxBytes{other.m_maxBytes}, m_marshalledMessages{other.m_marshalledMessages},
          m_batchEnvelope{other.m_batchEnvelope}, m_currentSize(other.m_currentSize){};

    /** Copy an EventDataBatch to another EventDataBatch */
    EventDataBatch& operator=(EventDataBatch const& other)
    {
      // Assignment operator cannot be defaulted because of m_rwMutex.
      if (this != &other)
      {
        m_partitionID = other.m_partitionID;
        m_partitionKey = other.m_partitionKey;
        m_maxBytes = other.m_maxBytes;
        m_marshalledMessages = other.m_marshalledMessages;
        m_batchEnvelope = other.m_batchEnvelope;
        m_currentSize = other.m_currentSize;
      }
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
        throw std::runtime_error("Either PartionID or PartitionKey can be set.");
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

    /** @brief Sets the partition ID for the data batch
     *
     * @param partitionID The partition ID to set
     */
    void SetPartitionID(std::string partitionID) { m_partitionID = partitionID; }

    /** @brief Sets the partition key for the data batch
     *
     * @param partitionKey The partition key to set
     */
    void SetPartitionKey(std::string partitionKey) { m_partitionKey = partitionKey; }

    /** @brief Sets the maximum size of the data batch */
    void SetMaxBytes(uint64_t maxBytes) { m_maxBytes = maxBytes; }

    /** @brief Gets the partition ID for the data batch
     *
     * @return std::string
     */
    std::string GetPartitionID() const { return m_partitionID; }

    /** @brief Gets the partition key for the data batch
     * @return std::string
     */
    std::string GetPartitionKey() const { return m_partitionKey; }

    /** @brief Gets the maximum size of the data batch
     *
     * @return uint64_t
     */
    uint64_t GetMaxBytes() const { return m_maxBytes; }

    /** @brief Adds a message to the data batch
     *
     * @param message The message to add to the batch
     */
    void AddMessage(Azure::Core::Amqp::Models::AmqpMessage& message) { AddAmqpMessage(message); }

    /** @brief Adds a message to the data batch
     *
     * @param message The message to add to the batch
     */
    void AddMessage(Azure::Messaging::EventHubs::Models::EventData& message);

    /** @brief Gets the number of messages in the batch
     *
     */
    size_t CurrentSize()
    {
      std::lock_guard<std::mutex> lock(m_rwMutex);
      return m_currentSize;
    }

    /** @brief Serializes the EventDataBatch to a single AmqpMessage to be sent to the EventHubs
     * service.
     *
     * @return Azure::Core::Amqp::Models::AmqpMessage
     */
    Azure::Core::Amqp::Models::AmqpMessage ToAmqpMessage() const
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
            PartitionKeyAnnotation, Azure::Core::Amqp::Models::AmqpValue(m_partitionKey));
      }

      std::vector<Azure::Core::Amqp::Models::AmqpBinaryData> messageList;
      for (auto const& marshalledMessage : m_marshalledMessages)
      {
        Azure::Core::Amqp::Models::AmqpBinaryData data(marshalledMessage);
        messageList.push_back(data);
      }

      returnValue.SetBody(messageList);
      Azure::Core::Diagnostics::_internal::Log::Stream(
          Azure::Core::Diagnostics::Logger::Level::Informational)
          << "EventDataBatch::ToAmqpMessage: " << returnValue << std::endl;
      return returnValue;
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

      if (!m_partitionKey.empty())
      {
        message.MessageAnnotations.emplace(
            PartitionKeyAnnotation, Azure::Core::Amqp::Models::AmqpValue(m_partitionKey));
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

    size_t CalculateActualSizeForPayload(std::vector<uint8_t> const& payload)
    {
      const size_t vbin8Overhead = 5;
      const size_t vbin32Overhead = 8;

      if (payload.size() < 256)
      {
        return payload.size() + vbin8Overhead;
      }
      return payload.size() + vbin32Overhead;
    }

    Azure::Core::Amqp::Models::AmqpMessage CreateBatchEnvelope(
        Azure::Core::Amqp::Models::AmqpMessage const& message)
    {
      // Create the batch envelope from the prototype message. This copies all the attributes
      // *except* the body attribute to the batch envelope.
      Azure::Core::Amqp::Models::AmqpMessage batchEnvelope{message};
      batchEnvelope.BodyType = Azure::Core::Amqp::Models::MessageBodyType::None;
      batchEnvelope.MessageFormat = BatchedMessageFormat;
      return batchEnvelope;
    }
  };
}}} // namespace Azure::Messaging::EventHubs
