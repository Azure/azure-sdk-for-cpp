// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#if ENABLE_UAMQP

#include "transport.hpp"

#include <string>

namespace Azure { namespace Core { namespace Amqp { namespace Network { namespace _internal {

  /** @brief Factory to create a TLS connection to a remote node.
   */
  class TlsTransportFactory final {
  public:
    /** @brief Creates a TLS transport.
     * 	 *
     * 	 * @param hostName Host name to connect to.
     * 	 * @param hostPort Port to connect to.
     * 	 * @param eventHandler Optional event handler.
     * 	 * @return Transport.
     * 	 *
     */
    static Transport Create(
        std::string const& hostName,
        uint16_t hostPort,
        TransportEvents* eventHandler = nullptr);

    TlsTransportFactory() = delete;
  };
}}}}} // namespace Azure::Core::Amqp::Network::_internal
#endif
