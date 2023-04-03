// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

// LCOV_EXCL_START
#include "azure/core/amqp/network/sasl_transport.hpp"
#include "azure/core/amqp/network/tls_transport.hpp"
#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/socketio.h>
#include <azure_c_shared_utility/tlsio.h>
#include <azure_uamqp_c/sasl_anonymous.h>
#include <azure_uamqp_c/sasl_plain.h>
#include <azure_uamqp_c/saslclientio.h>

Azure::Core::Amqp::Network::_internal::SaslTransport::SaslTransport(
    std::string const& saslKeyName,
    std::string const& saslKey,
    std::string const& hostName,
    uint16_t hostPort,
    TransportEvents* eventHandler)
    : Transport(eventHandler)
{
  XIO_HANDLE underlying_io;
  // We assume that localhost connections will be using raw sockets, not TLS.
  if (hostName == "localhost")
  {
    SOCKETIO_CONFIG socketConfig{hostName.c_str(), hostPort, nullptr};

    underlying_io = xio_create(socketio_get_interface_description(), &socketConfig);
  }
  else
  {
    auto tlsioConfig = platform_get_default_tlsio();
    TLSIO_CONFIG tlsConfig{};
    tlsConfig.hostname = hostName.c_str();
    tlsConfig.port = hostPort;
    underlying_io = xio_create(tlsioConfig, &tlsConfig);
  }

  SASL_PLAIN_CONFIG saslPlainConfig{};
  saslPlainConfig.authcid = saslKeyName.c_str();
  saslPlainConfig.passwd = saslKey.c_str();
  auto saslMechanism = saslmechanism_create(saslplain_get_interface(), &saslPlainConfig);

  SASLCLIENTIO_CONFIG saslConfig;
  saslConfig.underlying_io = underlying_io;
  saslConfig.sasl_mechanism = saslMechanism;
  SetInstance(xio_create(saslclientio_get_interface_description(), &saslConfig));
}

Azure::Core::Amqp::Network::_internal::SaslTransport::SaslTransport(
    std::string const& hostName,
    uint16_t hostPort,
    TransportEvents* eventHandler)
    : Transport(eventHandler)
{
  XIO_HANDLE underlying_io;
  // We assume that localhost connections will be using raw sockets, not TLS.
  if (hostName == "localhost")
  {
    SOCKETIO_CONFIG socketConfig{hostName.c_str(), hostPort, nullptr};

    underlying_io = xio_create(socketio_get_interface_description(), &socketConfig);
  }
  else
  {
    auto tlsioConfig = platform_get_default_tlsio();
    TLSIO_CONFIG tlsConfig{};
    tlsConfig.hostname = hostName.c_str();
    tlsConfig.port = hostPort;
    underlying_io = xio_create(tlsioConfig, &tlsConfig);
  }

  auto saslMechanism = saslmechanism_create(saslanonymous_get_interface(), nullptr);

  SASLCLIENTIO_CONFIG saslConfig;
  saslConfig.underlying_io = underlying_io;
  saslConfig.sasl_mechanism = saslMechanism;
  SetInstance(xio_create(saslclientio_get_interface_description(), &saslConfig));
}
// LCOV_EXCL_END
