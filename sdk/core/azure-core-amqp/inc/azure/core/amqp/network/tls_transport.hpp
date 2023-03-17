// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#pragma once

#include "Transport.hpp"
#include <string>

namespace Azure { namespace Core { namespace _internal { namespace Amqp { namespace Network {

  class TlsTransport final : public Transport {

  public:
    TlsTransport(TransportEvents* eventHandler = nullptr);
    TlsTransport(
        std::string const& hostName,
        uint16_t hostPort,
        TransportEvents* eventHandler = nullptr);
    ~TlsTransport() = default;
  };

}}}}} // namespace Azure::Core::_internal::Amqp::Network
