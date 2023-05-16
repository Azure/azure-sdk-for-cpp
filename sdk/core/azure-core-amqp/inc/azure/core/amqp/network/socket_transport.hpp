// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "transport.hpp"
#include <azure/core/platform.hpp>
#include <exception>

namespace Azure { namespace Core { namespace Amqp { namespace Network { namespace _internal {

  class SocketTransport final : public Transport {

  public:
    SocketTransport(
        std::string const& hostName,
        uint16_t hostPort,
        TransportEvents* eventHandler = nullptr);
    ~SocketTransport() = default;
  };

}}}}} // namespace Azure::Core::Amqp::Network::_internal
