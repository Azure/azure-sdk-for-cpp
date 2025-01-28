// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#include "models/event_data.hpp"

#include <azure/core/amqp/models/amqp_message.hpp>
#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

#include <mutex>
#include <stdexcept>

// cspell: words vbin

namespace Azure { namespace Messaging { namespace EventHubs { namespace _detail {
  class EventDataBatchFactory;
}}}} // namespace Azure::Messaging::EventHubs::_detail

namespace Azure { namespace Messaging { namespace EventHubs {

  /** @brief EventDataBatchOptions contains optional parameters for the
   * [ProducerClient.CreateEventDataBatch] function.
   *
   * @remark If both PartitionKey and PartitionId are empty, Event Hubs will choose an arbitrary
   * partition for any events in this [EventDataBatch].
   */
  struct EventDataBatchOptions final
  {

    /** @brief MaxBytes overrides the max size (in bytes) for a batch.
     * By default CreateEventDataBatch will use the max message size provided by the service.
     */
    Azure::Nullable<std::uint64_t> MaxBytes;

    /** @brief PartitionKey is hashed to calculate the partition assignment.Messages and message
     * batches with the same PartitionKey are guaranteed to end up in the same partition.
     * Note that if you use this option then PartitionId cannot be set.
     */
    std::string PartitionKey;

    /** @brief PartitionId is the ID of the partition to send these messages to.
     * Note that if you use this option then PartitionKey cannot be set.
     */
    std::string PartitionId;
  };

  /**@brief EventDataBatch is used to efficiently pack up EventData before sending it to Event Hubs.
   *
   * @remark EventDataBatch's are not meant to be created directly. Use
   * [ProducerClient.CreateEventDataBatch], which will create them with the proper size limit for
   * your Event Hub.
   */
  class EventDataBatch final {
  private:
    std::mutex m_rwMutex;
    std::string m_partitionId;
    std::string m_partitionKey;
    Azure::Nullable<std::uint64_t> m_maxBytes;
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
        : m_rwMutex{}, m_partitionId{other.m_partitionId}, m_partitionKey{other.m_partitionKey},
          m_maxBytes{other.m_maxBytes}, m_marshalledMessages{other.m_marshalledMessages},
          m_batchEnvelope{other.m_batchEnvelope}, m_currentSize(other.m_currentSize){};

    /** Copy an EventDataBatch to another EventDataBatch */
    EventDataBatch& operator=(EventDataBatch const& other)
    {
      // Assignment operator cannot be defaulted because of m_rwMutex.
      if (this != &other)
      {
        m_partitionId = other.m_partitionId;
        m_partitionKey = other.m_partitionKey;
        m_maxBytes = other.m_maxBytes;
        m_marshalledMessages = other.m_marshalledMessages;
        m_batchEnvelope = other.m_batchEnvelope;
        m_currentSize = other.m_currentSize;
      }
      return *this;
    }

    /** @brief Gets the partition ID for the data batch
     *
     * @return std::string
     */
    std::string GetPartitionId() const { return m_partitionId; }

    /** @brief Gets the partition key for the data batch
     * @return std::string
     */
    std::string GetPartitionKey() const { return m_partitionKey; }

    /** @brief Gets the maximum size of the data batch
     *
     * @return uint64_t
     */
    uint64_t GetMaxBytes() const { return m_maxBytes.Value(); }

    /** @brief Attempts to add a raw AMQP message to the data batch
     *
     * @param message The AMQP message to add to the batch
     *
     * @returns true if the message was added to the batch, false otherwise.
     */
    _azure_NODISCARD bool TryAdd(
        std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage const> const& message)
    {
      return TryAddAmqpMessage(message);
    }

    /** @brief Attempts to add a message to the data batch
     *
     * @param message The message to add to the batch
     *
     * @returns true if the message was added to the batch, false otherwise.
     */
    _azure_NODISCARD bool TryAdd(Azure::Messaging::EventHubs::Models::EventData const& message);

    /** @brief Gets the number of messages in the batch
     *
     */
    size_t NumberOfEvents()
    {
      std::lock_guard<std::mutex> lock(m_rwMutex);
      return m_marshalledMessages.size();
    }

    /** @brief Serializes the EventDataBatch to a single AmqpMessage to be sent to the EventHubs
     * service.
     *
     * @return Azure::Core::Amqp::Models::AmqpMessage
     */
    Azure::Core::Amqp::Models::AmqpMessage ToAmqpMessage() const;

  private:
    bool TryAddAmqpMessage(
        std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage const> const& message);

    static size_t CalculateActualSizeForPayload(std::vector<uint8_t> const& payload)
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
        std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage const> const& message) const
    {
      // Create the batch envelope from the prototype message. This copies all the attributes
      // *except* the body attribute to the batch envelope.
      Azure::Core::Amqp::Models::AmqpMessage batchEnvelope{*message};
      batchEnvelope.BodyType = Azure::Core::Amqp::Models::MessageBodyType::None;
      batchEnvelope.MessageFormat = BatchedMessageFormat;
      return batchEnvelope;
    }

    /** @brief Event Data Batch constructor
     *
     * @param options Options settings for creating the data batch
     */
    EventDataBatch(EventDataBatchOptions const& options = {})
        : m_partitionId{options.PartitionId}, m_partitionKey{options.PartitionKey},
          m_maxBytes{options.MaxBytes}, m_marshalledMessages{}, m_batchEnvelope{}, m_currentSize{0}
    {
      if (!options.PartitionId.empty() && !options.PartitionKey.empty())
      {
        throw std::runtime_error("Either PartitionID or PartitionKey can be set, but not both.");
      }

      if (options.PartitionId.empty())
      {
        m_partitionId = ""; // "" means "any partition ID"
      }
    };

    friend class _detail::EventDataBatchFactory;
  };
}}} // namespace Azure::Messaging::EventHubs
