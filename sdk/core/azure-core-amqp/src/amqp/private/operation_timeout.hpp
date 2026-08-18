// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/context.hpp>
#include <azure/core/datetime.hpp>

#include <chrono>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  // The deadline for one AMQP operation that the caller did not bound. A
  // context with no deadline never cancels, so a transport that stops
  // answering makes that caller wait forever.
  constexpr std::chrono::seconds DefaultOperationTimeout{60};

  // The parentheses around max stop the Windows max macro.
  inline bool ContextHasDeadline(Azure::Core::Context const& context)
  {
    return context.GetDeadline() != (Azure::DateTime::max)();
  }

  // GetDeadline takes the earliest deadline on the parent chain, so a caller
  // deadline that is later than the default must not go through WithDeadline
  // here. That call would shorten it.
  inline Azure::Core::Context ContextWithOperationDeadline(
      Azure::Core::Context const& context,
      std::chrono::system_clock::time_point now)
  {
    if (ContextHasDeadline(context))
    {
      return context;
    }
    return context.WithDeadline(Azure::DateTime(now + DefaultOperationTimeout));
  }

}}}} // namespace Azure::Core::Amqp::_detail
