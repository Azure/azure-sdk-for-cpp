// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "transport.hpp"
#include <exception>

namespace Azure { namespace Core { namespace Amqp { namespace Network { namespace _internal {

  /** A transport that detects the AMQP header on an incoming message. Used when listening for
   * incoming AMQP connections and messages.
   */
  class AmqpHeaderDetectTransport final : public Transport {

  public:
    /** Construct a new instance of the Amqp Header Detect Transport.
     * 	 *
     * 	 * @param parentTransport The parent transport to read from.
     * 	 * @param eventHandler The event handler to notify when the AMQP header is detected.
     * 	 *
     */
    AmqpHeaderDetectTransport(
        std::shared_ptr<Transport> parentTransport,
        TransportEvents* eventHandler);
    ~AmqpHeaderDetectTransport() = default;
  };

}}}}} // namespace Azure::Core::Amqp::Network::_internal
