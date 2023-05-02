// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/network/socket_transport.hpp"
#include "azure/core/amqp/network/private/transport_impl.hpp"
#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/socketio.h>
#include <exception>
#include <stdexcept>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

namespace Azure { namespace Core { namespace Amqp { namespace Network { namespace _internal {

  SocketTransport::SocketTransport(
      std::string const& host,
      uint16_t port,
      TransportEvents* eventHandler)
      : Transport(eventHandler)
  {
    Log::Write(
        Logger::Level::Verbose,
        "Create socket transport for host " + host + " port: " + std::to_string(port));

    SOCKETIO_CONFIG socketConfig{host.c_str(), port, nullptr};
    m_impl->SetInstance(xio_create(socketio_get_interface_description(), &socketConfig));
  }

}}}}} // namespace Azure::Core::Amqp::Network::_internal
