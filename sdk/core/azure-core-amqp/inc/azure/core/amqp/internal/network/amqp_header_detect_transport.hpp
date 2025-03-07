// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#if ENABLE_UAMQP
#include "transport.hpp"

#include <exception>

namespace Azure { namespace Core { namespace Amqp { namespace Network { namespace _internal {

  /** @brief Factory to create a transport which detects the AMQP header on an incoming message.
   * Used when listening for incoming AMQP connections and messages.
   */
  class AmqpHeaderDetectTransportFactory final {

  public:
    /** Construct a new instance of an Amqp Header Detect Transport.
     *
     * @param parentTransport The parent transport to read from.
     * @param eventHandler The event handler to notify when the AMQP header is detected.
     *
     */
    static Transport Create(
        std::shared_ptr<Transport> parentTransport,
        TransportEvents* eventHandler);
    AmqpHeaderDetectTransportFactory() = delete;
  };

}}}}} // namespace Azure::Core::Amqp::Network::_internal

#endif
