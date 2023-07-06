// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>

namespace Azure { namespace Messaging { namespace EventHubs { namespace _detail {

  /// @brief The default maximum size for a single receive operation.
  constexpr const std::uint32_t DefaultMaxSize = 5000;
  /// @brief The default consumer group name.
  constexpr const char* DefaultConsumerGroup = "$Default";

  constexpr const char* PartitionKeyAnnotation = "x-opt-partition-key";
  constexpr const char* SequenceNumberAnnotation = "x-opt-sequence-number";
  constexpr const char* OffsetNumberAnnotation = "x-opt-offset";
  constexpr const char* EnqueuedTimeAnnotation = "x-opt-enqueued-time";
}}}} // namespace Azure::Messaging::EventHubs::_detail