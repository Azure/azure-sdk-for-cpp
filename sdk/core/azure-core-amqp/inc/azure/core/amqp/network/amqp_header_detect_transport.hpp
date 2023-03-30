// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "transport.hpp"
#include <exception>

struct XIO_INSTANCE_TAG;

namespace Azure { namespace Core { namespace Amqp { namespace Network { namespace _internal {

  class AmqpHeaderTransport final : public Transport {

  public:
    AmqpHeaderTransport(XIO_INSTANCE_TAG* parentTransport, TransportEvents* eventHandler);
    ~AmqpHeaderTransport() = default;
  };

}}}}} // namespace Azure::Core::Amqp::Network::_internal
