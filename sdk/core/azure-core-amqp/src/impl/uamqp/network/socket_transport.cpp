// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/network/socket_transport.hpp"

#include "private/transport_impl.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/socketio.h>

#include <exception>
#include <stdexcept>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

namespace Azure { namespace Core { namespace Amqp { namespace Network { namespace _internal {

  Transport SocketTransportFactory::Create(
      std::string const& host,
      uint16_t port,
      TransportEvents* eventHandler)
  {
    Log::Stream(Logger::Level::Verbose)
        << "Create socket transport for host " << host << " port: " << port;

    SOCKETIO_CONFIG socketConfig{host.c_str(), port, nullptr};
    return _detail::TransportImpl::CreateFromXioHandle(
        xio_create(socketio_get_interface_description(), &socketConfig), eventHandler);
  }

}}}}} // namespace Azure::Core::Amqp::Network::_internal
