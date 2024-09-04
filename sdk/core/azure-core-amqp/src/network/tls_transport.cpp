// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/network/tls_transport.hpp"

#include "private/transport_impl.hpp"

#if ENABLE_UAMQP
#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/tlsio.h>
#endif

#include <string>

namespace Azure { namespace Core { namespace Amqp { namespace Network { namespace _internal {

  Transport TlsTransportFactory::Create(
      std::string const& host,
      uint16_t port,
      TransportEvents* eventHandler)
  {
#if ENABLE_UAMQP
    TLSIO_CONFIG tlsConfig{};
    tlsConfig.hostname = host.c_str();
    tlsConfig.port = port;

    auto tlsio_interface = platform_get_default_tlsio();

    return _detail::TransportImpl::CreateFromXioHandle(
        xio_create(tlsio_interface, &tlsConfig), eventHandler);
#else
    (void)host;
    (void)port;
    (void)eventHandler;
    throw std::runtime_error("Not implemented.");

#endif // ENABLE_UAMQP
  }

}}}}} // namespace Azure::Core::Amqp::Network::_internal
