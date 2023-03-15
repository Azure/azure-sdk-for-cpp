// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#include "azure/core/amqp/network/socket_transport.hpp"

#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/socketio.h>
#include <exception>
#include <stdexcept>

namespace Azure { namespace Core { namespace _internal { namespace Amqp { namespace Network {

  SocketTransport::SocketTransport(std::string const& host, uint16_t port) : Transport()
  {
    SOCKETIO_CONFIG socketConfig{host.c_str(), port};

    SetInstance(xio_create(socketio_get_interface_description(), &socketConfig));
  }

  SocketTransport::SocketTransport() : Transport() {}

}}}}} // namespace Azure::Core::_internal::Amqp::Network
