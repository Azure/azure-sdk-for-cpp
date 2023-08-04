// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "azure/messaging/eventhubs/processor_partition_client.hpp"

#include "private/eventhubs_constants.hpp"

namespace Azure { namespace Messaging { namespace EventHubs {

  void ProcessorPartitionClient::UpdateCheckpoint(
      Azure::Core::Amqp::Models::AmqpMessage const& amqpMessage,
      Core::Context const& context)
  {
    Azure::Nullable<int64_t> sequenceNumber;

    Azure::Nullable<int64_t> offsetNumber;

    for (auto const& pair : amqpMessage.MessageAnnotations)
    {
      if (pair.first == _detail::SequenceNumberAnnotation)
      {
        if (pair.second.GetType() == Azure::Core::Amqp::Models::AmqpValueType::Int
            || pair.second.GetType() == Azure::Core::Amqp::Models::AmqpValueType::Uint
            || pair.second.GetType() == Azure::Core::Amqp::Models::AmqpValueType::Long
            || pair.second.GetType() == Azure::Core::Amqp::Models::AmqpValueType::Ulong)
          sequenceNumber = static_cast<int64_t>(pair.second);
      }
      if (pair.first == _detail::OffsetNumberAnnotation)
      {
        if (pair.second.GetType() == Azure::Core::Amqp::Models::AmqpValueType::Int
            || pair.second.GetType() == Azure::Core::Amqp::Models::AmqpValueType::Uint
            || pair.second.GetType() == Azure::Core::Amqp::Models::AmqpValueType::Long
            || pair.second.GetType() == Azure::Core::Amqp::Models::AmqpValueType::Ulong)
          offsetNumber = static_cast<int64_t>(pair.second);
      }
    }

    Models::Checkpoint checkpoint
        = {m_consumerClientDetails.ConsumerGroup,
           m_consumerClientDetails.EventHubName,
           m_consumerClientDetails.FullyQualifiedNamespace,
           m_partitionId,
           sequenceNumber,
           offsetNumber};

    m_checkpointStore->UpdateCheckpoint(checkpoint, context);
  }
}}} // namespace Azure::Messaging::EventHubs
