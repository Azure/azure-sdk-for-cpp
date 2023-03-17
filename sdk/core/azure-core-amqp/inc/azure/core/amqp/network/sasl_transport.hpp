// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#pragma once

#include "transport.hpp"
#include <string>

namespace Azure { namespace Core { namespace _internal { namespace Amqp { namespace Network {

  class SaslTransport final : public Azure::Core::_internal::Amqp::Network::Transport {

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

}}}}} // namespace Azure::Core::_internal::Amqp::Network
