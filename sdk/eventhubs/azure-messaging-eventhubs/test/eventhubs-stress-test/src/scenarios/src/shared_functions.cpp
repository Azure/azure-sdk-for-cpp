// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "shared_functions.hpp"

#include "eventhubs_stress_scenarios.hpp"

#include <iostream>
#include <thread>

std::function<void(Azure::Core::Context const&)> GetSleepAfterFunction(
    std::chrono::system_clock::duration const& duration)
{
  return [duration](Azure::Core::Context const& context) {
    if (duration.count() > 0)
    {
      std::cout << "Sleeping for " << duration.count() << " seconds" << std::endl;
      context.ThrowIfCancelled();
      std::this_thread::sleep_for(duration);
      std::cout << "DOne sleeping for " << duration.count() << " seconds" << std::endl;
    }
  };
}

opentelemetry::nostd::shared_ptr<opentelemetry::logs::Logger> GetLogger()
{
  auto logger{opentelemetry::logs::Provider::GetLoggerProvider()->GetLogger(EventHubsLoggerName)};
  return logger;
}

opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> GetTracer()
{
  return opentelemetry::trace::Provider::GetTracerProvider()->GetTracer(EventHubsLoggerName);
}

std::pair<opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span>, opentelemetry::trace::Scope>
CreateStressSpan(std::string const& name)
{
  auto tracer = GetTracer();
  opentelemetry::trace::StartSpanOptions options;
  options.parent = tracer->GetCurrentSpan()->GetContext();
  options.kind = opentelemetry::trace::SpanKind::kClient;
  auto newSpan = tracer->StartSpan(name, options);

  auto scope{tracer->WithActiveSpan(newSpan)};

  return std::make_pair<
      opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span>,
      opentelemetry::trace::Scope>(std::move(newSpan), std::move(scope));
}

std::pair<
    Azure::Messaging::EventHubs::Models::StartPosition,
    Azure::Messaging::EventHubs::Models::EventHubPartitionProperties>
EventSender::SendEventsToPartition(
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
    auto getPropertiesSpan{CreateStressSpan("SendEventsToPartition::GetPartitionProperties begin")};
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
      if (!batch.TryAdd(event))
      {
        if (batch.NumberOfEvents() == 0)
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
      batchAddMessageSpan.first->SetStatus(opentelemetry::trace::StatusCode::kOk, "OK");
    }
  }
  if (batch.NumberOfEvents() > 0)
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
