// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include <azure/core/amqp.hpp>
#include <mutex>
namespace Azure { namespace Messaging { namespace EventHubs {
  /** @brief EventDataBatchOptions contains optional parameters for the
   * [ProducerClient.NewEventDataBatch] function.
   *
   * @remark If both PartitionKey and PartitionID are nil, Event Hubs will choose an arbitrary
   * partition for any events in this [EventDataBatch].
   */
  struct EventDataBatchOptions
  {

    /** @brief MaxBytes overrides the max size (in bytes) for a batch.
     * By default NewEventDataBatch will use the max message size provided by the service.
     */
    uint64_t MaxBytes;

    /** @brief PartitionKey is hashed to calculate the partition assignment.Messages and message
     * batches with the same PartitionKey are guaranteed to end up in the same partition.
     * Note that if you use this option then PartitionID cannot be set.
     */
    std::string PartitionKey;

    /** @brief PartitionID is the ID of the partition to send these messages to.
     * Note that if you use this option then PartitionKey cannot be set.
     */
    std::string PartitionID;
  };

  /**@brief EventDataBatch is used to efficiently pack up EventData before sending it to Event Hubs.
   *
   * @remark EventDataBatch's are not meant to be created directly. Use
   * [ProducerClient.NewEventDataBatch], which will create them with the proper size limit for your
   * Event Hub.
   */
  class EventDataBatch {
  private:
    std::mutex m_rwMutex;
    std::vector<Azure::Core::Amqp::Models::AmqpMessage> m_messages;
    std::string m_partitionID;
    std::string m_partitionKey;
    uint64_t m_maxBytes;
    const std::string anyPartitionId = "";

  public:
    /** @brief Event Data Batch constructor
     *
     * @param options Options settings for creating the data batch
     */
    EventDataBatch(EventDataBatchOptions options = EventDataBatchOptions())
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
     */
    void AddMessage(Azure::Core::Amqp::Models::AmqpMessage& message)
    {
      std::lock_guard<std::mutex> lock(m_rwMutex);
      if (message.MessageAnnotations["x-opt-partition-key"] == nullptr && !m_partitionKey.empty())
      {
        message.MessageAnnotations["x-opt-partition-key"]
            = Azure::Core::Amqp::Models::AmqpValue(m_partitionKey);
      }
      m_messages.push_back(message);
    }

    /** @brief Gets the number of messages in the batch
     *
     */
    const size_t CurrentSize()
    {
      std::lock_guard<std::mutex> lock(m_rwMutex);
      return m_messages.size();
    }

    /** @brief Gets the messages in the batch
     *
     */
    const std::vector<Azure::Core::Amqp::Models::AmqpMessage> GetMessages()
    {
      std::lock_guard<std::mutex> lock(m_rwMutex);
      /* if (m_messages.size() == 0)
      {
        return nullptr;
      }

      Azure::Core::Amqp::Models::Message message = m_messages[0];
      
      message.ClearBody();
      for (int i = 0; i < m_messages.size(); i++)
      {
        if (m_messages[i].BodyType == Azure::Core::Amqp::Models::MessageBodyType::Data)
        {
          auto data = m_messages[i].GetBodyAsBinary();
          for (auto oneData : data)
          {
            message.SetBody(oneData);
          }
        }
        else if (m_messages[i].BodyType == Azure::Core::Amqp::Models::MessageBodyType::Value)
        {
          auto data = m_messages[i].GetBodyAsAmqpValue();
          message.SetBody(data)//.AsBinary());
        }
        else if (m_messages[i].BodyType == Azure::Core::Amqp::Models::MessageBodyType::Sequence)
        {
          auto data = m_messages[i].GetBodyAsAmqpList();
          message.SetBody(data); //((Azure::Core::Amqp::Models::AmqpValue)data).AsBinary());
        }
      }

      return message;*/

      std::vector<Azure::Core::Amqp::Models::AmqpMessage> messages(m_messages);
      return messages;

    }
  };
}}} // namespace Azure::Messaging::EventHubs