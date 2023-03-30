// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "transport.hpp"
#include <string>

namespace Azure { namespace Core { namespace Amqp { namespace Network { namespace _internal {

  class TlsTransport final : public Transport {

  public:
    TlsTransport(TransportEvents* eventHandler = nullptr);
    TlsTransport(
        std::string const& hostName,
        uint16_t hostPort,
        TransportEvents* eventHandler = nullptr);
    ~TlsTransport() = default;
  };

}}}}} // namespace Azure::Core::Amqp::Network::_internal
