#pragma once

#include "transport.hpp"
#include <string>

namespace Azure { namespace Core { namespace _internal { namespace Amqp { namespace Network {

  class SaslTransport : public Azure::Core::_internal::Amqp::Network::Transport {

  public:
    // Configure the transport using SASL plain.
    SaslTransport(
        std::string const& saslKeyName,
        std::string const& saslKey,
        std::string const& hostName,
        uint16_t hostPort);
    // Configure the transport using SASL Anonymous.
    SaslTransport(std::string const& hostName, uint16_t hostPort);
    ~SaslTransport() = default;
  };

}}}}} // namespace Azure::Core::_internal::Amqp::Network
