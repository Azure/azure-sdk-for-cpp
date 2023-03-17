// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#pragma once

#include "Transport.hpp"
#include <exception>

struct XIO_INSTANCE_TAG;

namespace Azure { namespace Core { namespace _internal { namespace Amqp { namespace Network {

  class SocketTransport final : public Azure::Core::_internal::Amqp::Network::Transport {

  public:
    SocketTransport();
    SocketTransport(std::string const& hostName, uint16_t hostPort, TransportEvents* eventHandler=nullptr);
    ~SocketTransport() = default;
  };

}}}}} // namespace Azure::Core::_internal::Amqp::Network
