// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#pragma once

#include "Transport.hpp"
#include <string>

namespace Azure { namespace Core { namespace _internal { namespace Amqp { namespace Network {

  class TlsTransport : public Transport {

  public:
    TlsTransport();
    TlsTransport(std::string const& hostName, uint16_t hostPort);
    ~TlsTransport() = default;
  };

}}}}} // namespace Azure::Core::_internal::Amqp::Network
