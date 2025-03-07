// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#if ENABLE_UAMQP
#include "transport.hpp"

#include <azure/core/platform.hpp>

#include <exception>

namespace Azure { namespace Core { namespace Amqp { namespace Network { namespace _internal {

  /** @brief Factory used to create a socket connection to the remote node. */
  class SocketTransportFactory final {

  public:
    /**
     * @brief Creates a socket connection to the remote node.
     *
     * @param hostName Fully qualified domain name or IP address of the remote node.
     * @param hostPort Port number of the remote node.
     * @param eventHandler Optional pointer to the event handler to be notified of transport events.
     *
     * @return The created transport.
     */
    static Transport Create(
        std::string const& hostName,
        uint16_t hostPort,
        TransportEvents* eventHandler = nullptr);

    SocketTransportFactory() = delete;
  };

}}}}} // namespace Azure::Core::Amqp::Network::_internal
#endif
