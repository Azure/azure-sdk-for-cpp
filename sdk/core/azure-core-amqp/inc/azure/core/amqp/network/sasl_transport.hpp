// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "transport.hpp"
#include <string>

namespace Azure { namespace Core { namespace Amqp { namespace Network { namespace _internal {

  class SaslTransport final : public Transport {

  public:
    // Configure the transport using SASL plain.
    SaslTransport(
        std::string const& saslKeyName,
        std::string const& saslKey,
        std::string const& hostName,
        uint16_t hostPort,
        TransportEvents* eventHandler = nullptr);
    // Configure the transport using SASL Anonymous.
    SaslTransport(
        std::string const& hostName,
        uint16_t hostPort,
        TransportEvents* eventHandler = nullptr);
    ~SaslTransport() = default;
  };

}}}}} // namespace Azure::Core::Amqp::Network::_internal
