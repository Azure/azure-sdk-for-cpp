// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#include "azure/core/amqp/network/tls_transport.hpp"

#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/tlsio.h>
#include <string>

namespace Azure { namespace Core { namespace _internal { namespace Amqp { namespace Network {

  TlsTransport::TlsTransport(std::string const& host, uint16_t port, TransportEvents* eventHandler)
      : Transport(eventHandler)
  {
    TLSIO_CONFIG tlsConfig{host.c_str(), port, nullptr};

    auto tlsio_interface = platform_get_default_tlsio();

    SetInstance(xio_create(tlsio_interface, &tlsConfig));
  }

  TlsTransport::TlsTransport(TransportEvents* eventHandler) : Transport(eventHandler) {}

}}}}} // namespace Azure::Core::_internal::Amqp::Network
