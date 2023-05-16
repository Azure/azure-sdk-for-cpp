// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/network/tls_transport.hpp"
#include "private/transport_impl.hpp"

#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/tlsio.h>
#include <string>

namespace Azure { namespace Core { namespace Amqp { namespace Network { namespace _internal {

  TlsTransport::TlsTransport(std::string const& host, uint16_t port, TransportEvents* eventHandler)
      : Transport(eventHandler)
  {
    TLSIO_CONFIG tlsConfig{};
    tlsConfig.hostname = host.c_str();
    tlsConfig.port = port;

    auto tlsio_interface = platform_get_default_tlsio();

    m_impl->SetInstance(xio_create(tlsio_interface, &tlsConfig));
  }

  TlsTransport::TlsTransport(TransportEvents* eventHandler) : Transport(eventHandler) {}

}}}}} // namespace Azure::Core::Amqp::Network::_internal
