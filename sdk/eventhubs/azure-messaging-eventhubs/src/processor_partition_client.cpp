// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "azure/messaging/eventhubs/processor_partition_client.hpp"

#include "private/eventhubs_constants.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

#include <iomanip>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

namespace Azure { namespace Messaging { namespace EventHubs {

  ProcessorPartitionClient::~ProcessorPartitionClient()
  {
    // Only log the destructor if the partition client has a value.
    if (!m_partitionId.empty())
    {
      Log::Stream(Logger::Level::Verbose) << "~ProcessorPartitionClient() for " << m_partitionId;
      Log::Stream(Logger::Level::Verbose)
          << "PartitionClient is " << (m_partitionClient ? "not " : "") << "null.";
    }
  }

  void ProcessorPartitionClient::UpdateCheckpoint(
      Azure::Core::Amqp::Models::AmqpMessage const& amqpMessage,
      Core::Context const& context)
  {
    Azure::Nullable<int64_t> sequenceNumber;

    Azure::Nullable<std::string> offset;

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
      if (pair.first == _detail::OffsetAnnotation)
      {
        if (pair.second.GetType() == Azure::Core::Amqp::Models::AmqpValueType::String)
          offset = static_cast<std::string>(pair.second);
      }
    }

    Models::Checkpoint checkpoint
        = {m_consumerClientDetails.ConsumerGroup,
           m_consumerClientDetails.EventHubName,
           m_consumerClientDetails.FullyQualifiedNamespace,
           m_partitionId,
           offset,
           sequenceNumber};

    m_checkpointStore->UpdateCheckpoint(checkpoint, context);
  }

  void ProcessorPartitionClient::UpdateCheckpoint(
      std::shared_ptr<const Models::ReceivedEventData> const& eventData,
      Core::Context const& context)
  {
    uint64_t sequenceNumber{};
    if (!eventData->SequenceNumber.HasValue())
    {
      throw std::runtime_error("Event does not have a sequence number.");
    }
    if (eventData->SequenceNumber.HasValue())
    {
      sequenceNumber = eventData->SequenceNumber.Value();
    }
    std::string offset{};
    if (!eventData->Offset.HasValue())
    {
      offset = eventData->Offset.Value();
    }

    Models::Checkpoint checkpoint;
    checkpoint.ConsumerGroup = m_consumerClientDetails.ConsumerGroup;
    checkpoint.FullyQualifiedNamespaceName = m_consumerClientDetails.FullyQualifiedNamespace;
    checkpoint.PartitionId = m_partitionId;
    checkpoint.EventHubName = m_consumerClientDetails.EventHubName;
    checkpoint.SequenceNumber = sequenceNumber;
    checkpoint.Offset = offset;
    m_checkpointStore->UpdateCheckpoint(checkpoint, context);
  }

}}} // namespace Azure::Messaging::EventHubs
