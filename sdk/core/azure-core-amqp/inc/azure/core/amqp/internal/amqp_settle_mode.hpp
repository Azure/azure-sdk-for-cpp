// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <iostream>

namespace Azure { namespace Core { namespace Amqp { namespace _internal {

  enum class SenderSettleMode
  {
    Unsettled,
    Settled,
    Mixed,
  };

  std::ostream& operator<<(std::ostream& os, SenderSettleMode mode);

  enum class ReceiverSettleMode
  {
    First,
    Second,
  };
  std::ostream& operator<<(std::ostream& os, ReceiverSettleMode mode);

}}}} // namespace Azure::Core::Amqp::_internal
