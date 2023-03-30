// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/network/socket_transport.hpp"

#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/socketio.h>
#include <exception>
#include <stdexcept>

namespace Azure { namespace Core { namespace Amqp { namespace Network { namespace _internal {

  SocketTransport::SocketTransport(
      std::string const& host,
      uint16_t port,
      TransportEvents* eventHandler)
      : Transport(eventHandler)
  {
    SOCKETIO_CONFIG socketConfig{host.c_str(), port, nullptr};

    SetInstance(xio_create(socketio_get_interface_description(), &socketConfig));
  }

}}}}} // namespace Azure::Core::Amqp::Network::_internal
