// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/network/sasl_transport.hpp"

#include "azure/core/amqp/internal/network/tls_transport.hpp"
#include "private/transport_impl.hpp"

#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/socketio.h>
#include <azure_c_shared_utility/tlsio.h>
#include <azure_uamqp_c/sasl_anonymous.h>
#include <azure_uamqp_c/sasl_plain.h>
#include <azure_uamqp_c/saslclientio.h>

#include <utility>

// cspell: ignore saslclientio
Azure::Core::Amqp::Network::_internal::Transport
Azure::Core::Amqp::Network::_internal::SaslTransportFactory::Create(
    std::string const& saslKeyName,
    std::string const& saslKey,
    std::string const& hostName,
    uint16_t hostPort,
    TransportEvents* eventHandler)
{
  Azure::Core::Amqp::Network::_detail::UniqueXioHandle underlyingXio;
  // We assume that localhost connections will be using raw sockets, not TLS.
  if (hostName == "localhost")
  {
    SOCKETIO_CONFIG socketConfig{hostName.c_str(), hostPort, nullptr};

    underlyingXio.reset(xio_create(socketio_get_interface_description(), &socketConfig));
  }
  else
  {
    auto tlsioConfig = platform_get_default_tlsio();
    TLSIO_CONFIG tlsConfig{};
    tlsConfig.hostname = hostName.c_str();
    tlsConfig.port = hostPort;
    underlyingXio.reset(xio_create(tlsioConfig, &tlsConfig));
  }

  SASL_PLAIN_CONFIG saslPlainConfig{};
  saslPlainConfig.authcid = saslKeyName.c_str();
  saslPlainConfig.passwd = saslKey.c_str();
  Azure::Core::Amqp::Network::_detail::UniqueSaslMechanismHandle saslMechanism{
      saslmechanism_create(saslplain_get_interface(), &saslPlainConfig)};

  SASLCLIENTIO_CONFIG saslConfig;
  saslConfig.underlying_io = underlyingXio.get();
  saslConfig.sasl_mechanism = saslMechanism.get();
  auto transport = _detail::TransportImpl::CreateFromXioHandle(
      xio_create(saslclientio_get_interface_description(), &saslConfig), eventHandler);
  transport->AdoptUnderlyingXio(std::move(underlyingXio));
  transport->AdoptSaslMechanism(std::move(saslMechanism));
  return transport;
}

Azure::Core::Amqp::Network::_internal::Transport
Azure::Core::Amqp::Network::_internal::SaslTransportFactory::Create(
    std::string const& hostName,
    uint16_t hostPort,
    TransportEvents* eventHandler)
{
  Azure::Core::Amqp::Network::_detail::UniqueXioHandle underlyingXio;
  // We assume that localhost connections will be using raw sockets, not TLS.
  if (hostName == "localhost")
  {
    SOCKETIO_CONFIG socketConfig{hostName.c_str(), hostPort, nullptr};

    underlyingXio.reset(xio_create(socketio_get_interface_description(), &socketConfig));
  }
  else
  {
    auto tlsioConfig = platform_get_default_tlsio();
    TLSIO_CONFIG tlsConfig{};
    tlsConfig.hostname = hostName.c_str();
    tlsConfig.port = hostPort;
    underlyingXio.reset(xio_create(tlsioConfig, &tlsConfig));
  }

  Azure::Core::Amqp::Network::_detail::UniqueSaslMechanismHandle saslMechanism{
      saslmechanism_create(saslanonymous_get_interface(), nullptr)};

  SASLCLIENTIO_CONFIG saslConfig;
  saslConfig.underlying_io = underlyingXio.get();
  saslConfig.sasl_mechanism = saslMechanism.get();
  auto transport = _detail::TransportImpl::CreateFromXioHandle(
      xio_create(saslclientio_get_interface_description(), &saslConfig), eventHandler);
  transport->AdoptUnderlyingXio(std::move(underlyingXio));
  transport->AdoptSaslMechanism(std::move(saslMechanism));
  return transport;
}
