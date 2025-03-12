// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <cstdint>
#include <string>

namespace Azure { namespace Messaging { namespace EventHubs { namespace _detail {

  /// @brief The default maximum size for a single receive operation.
  constexpr const std::uint32_t DefaultMaxSize = 5000;

  constexpr const char* PartitionKeyAnnotation = "x-opt-partition-key";
  constexpr const char* SequenceNumberAnnotation = "x-opt-sequence-number";
  constexpr const char* OffsetAnnotation = "x-opt-offset";
  constexpr const char* EnqueuedTimeAnnotation = "x-opt-enqueued-time";

  constexpr const char* EventHubsServiceScheme = "amqps://";
  constexpr const char* EventHubsServiceScheme_Emulator = "amqp://";
  constexpr const char* EventHubsConsumerGroupsPath = "/ConsumerGroups/";
}}}} // namespace Azure::Messaging::EventHubs::_detail
