// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#include "azure/messaging/eventhubs/models/event_data.hpp"
#include "azure/messaging/eventhubs/models/management_models.hpp"
#include "azure/messaging/eventhubs/models/partition_client_models.hpp"
#include "azure/messaging/eventhubs/producer_client.hpp"

#include <azure/core/context.hpp>

#include <chrono>
#include <functional>
#include <memory>
#include <utility>

#if defined(_MSC_VER)
// The OpenTelemetry headers generate a couple of warnings on MSVC in the OTel 1.2 package, suppress
// the warnings across the includes.
#pragma warning(push)
#pragma warning(disable : 4100)
#pragma warning(disable : 4244)
#pragma warning(disable : 6323) // Disable "Use of arithmetic operator on Boolean type" warning.
#endif

#include <opentelemetry/logs/provider.h>
#include <opentelemetry/sdk/logs/logger.h>
#include <opentelemetry/sdk/trace/tracer.h>
#include <opentelemetry/trace/provider.h>
#include <opentelemetry/trace/semantic_conventions.h>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

opentelemetry::nostd::shared_ptr<opentelemetry::logs::Logger> GetLogger();

opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> GetTracer();

std::pair<opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span>, opentelemetry::trace::Scope>
CreateStressSpan(std::string const& name);

std::function<void(Azure::Core::Context const&)> GetSleepAfterFunction(
    std::chrono::system_clock::duration const& duration);

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
      Azure::Core::Context const& context);

private:
  static void AddEndProperty(
      Azure::Messaging::EventHubs::Models::EventData& event,
      uint64_t expectedCount)
  {
    event.Properties["End"] = expectedCount;
  }
};
