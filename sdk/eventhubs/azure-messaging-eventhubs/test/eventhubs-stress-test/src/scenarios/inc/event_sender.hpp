// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "opentelemetry_helpers.hpp"

#include <azure/core/context.hpp>
#include <azure/messaging/eventhubs/models/event_data.hpp>
#include <azure/messaging/eventhubs/models/management_models.hpp>
#include <azure/messaging/eventhubs/models/partition_client_models.hpp>
#include <azure/messaging/eventhubs/producer_client.hpp>

#include <memory>

struct EventSenderOptions
{
  std::string PartitionId;
  std::uint32_t MessageLimit;
  std::uint32_t NumberOfExtraBytes;
};

class EventSender {
public:
  static std::pair<
      Azure::Messaging::EventHubs::Models::StartPosition,
      Azure::Messaging::EventHubs::Models::EventHubPartitionProperties>
  SendEventsToPartition(
      std::unique_ptr<Azure::Messaging::EventHubs::ProducerClient> const& producerClient,
      EventSenderOptions const& senderOptions,
      Azure::Core::Context const& context)
  {
    auto sendEventsScope{CreateStressSpan("SendEventsToPartition")};

    std::cout << "[BEGIN] Sending " << senderOptions.MessageLimit << " messages to partition "
              << senderOptions.PartitionId << ", with messages of size "
              << senderOptions.NumberOfExtraBytes << std::endl;

    Azure::Messaging::EventHubs::Models::EventHubPartitionProperties beforeSendProps;
    {
      auto getPropertiesSpan{
          CreateStressSpan("SendEventsToPartition::GetPartitionProperties begin")};
      beforeSendProps = producerClient->GetPartitionProperties(senderOptions.PartitionId, context);
    }
    std::vector<uint8_t> bodyData(senderOptions.NumberOfExtraBytes, 'a');

    Azure::Messaging::EventHubs::EventDataBatchOptions batchOptions;
    batchOptions.PartitionId = senderOptions.PartitionId;
    Azure::Messaging::EventHubs::EventDataBatch batch{producerClient->CreateBatch(batchOptions)};

    for (uint32_t j = 0; j < senderOptions.MessageLimit; ++j)
    {
      Azure::Messaging::EventHubs::Models::EventData event;
      event.Body = bodyData;
      event.Properties["Number"] = j;
      event.Properties["PartitionID"]
          = static_cast<Azure::Core::Amqp::Models::AmqpValue>(senderOptions.PartitionId);
      if (j == senderOptions.MessageLimit)
      {
        AddEndProperty(event, senderOptions.MessageLimit);
      }

      {
        auto batchAddMessageSpan{CreateStressSpan("SendEventsToPartition::BatchTryAddMessage")};
        if (!batch.TryAddMessage(event))
        {
          if (batch.CurrentSize() == 0)
          {
            std::cerr << "Single message could not fit in batch";
            throw std::runtime_error("Single message could not fit in batch");
          }
          auto sendBatchSpan{CreateStressSpan("SendBatch")};
          {
            producerClient->Send(batch, context);
          }
          batch = producerClient->CreateBatch(batchOptions);
          j -= 1; // Retry adding the same message.
        }
      }
    }
    if (batch.CurrentSize() > 0)
    {
      auto sendBatchSpan{CreateStressSpan("SendBatch")};
      {
        sendBatchSpan.first->AddEvent("Send events", {{"event count", senderOptions.MessageLimit}});
        producerClient->Send(batch, context);
      }
    }
    {
      auto getPartitionPropertiesSpan{CreateStressSpan("GetPartitionProperties")};
      auto afterSendProps
          = producerClient->GetPartitionProperties(senderOptions.PartitionId, context);
      getPartitionPropertiesSpan.first->AddEvent(
          "After Properties", {{"sequenceNumber", beforeSendProps.LastEnqueuedSequenceNumber}});

      Azure::Messaging::EventHubs::Models::StartPosition afterStartPosition;
      afterStartPosition.Inclusive = false;
      afterStartPosition.SequenceNumber = beforeSendProps.LastEnqueuedSequenceNumber;

      std::cout << "[END] Sending " << senderOptions.MessageLimit << " messages to partition "
                << senderOptions.PartitionId << " with messages of size "
                << senderOptions.NumberOfExtraBytes << "b" << std::endl;
      return std::make_pair(afterStartPosition, afterSendProps);
    }
  }

private:
  static void AddEndProperty(
      Azure::Messaging::EventHubs::Models::EventData& event,
      uint64_t expectedCount)
  {
    event.Properties["End"] = expectedCount;
  }
};
