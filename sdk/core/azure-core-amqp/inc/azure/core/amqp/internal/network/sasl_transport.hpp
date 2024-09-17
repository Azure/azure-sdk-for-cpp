// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#if ENABLE_UAMQP
#include "transport.hpp"

#include <string>
namespace Azure { namespace Core { namespace Amqp { namespace Network { namespace _internal {

  /** @brief Factory to create a transport using SASL authentication.
   *
   */
  class SaslTransportFactory final {

  public:
    /** @brief Create a transport using SASL Plain.
     *
     * @param[in] saslKeyName The SASL key name.
     * @param[in] saslKey The SASL key.
     * @param[in] hostName The host name.
     * @param[in] hostPort The host port.
     * @param[in] eventHandler The transport event handler.
     *
     */
    static Transport Create(
        std::string const& saslKeyName,
        std::string const& saslKey,
        std::string const& hostName,
        uint16_t hostPort,
        TransportEvents* eventHandler = nullptr);

    /** @brief Create a transport using SASL Anonymous
     *
     * @param[in] hostName The host name.
     * @param[in] hostPort The host port.
     * @param[in] eventHandler The transport event handler.
     *
     */
    static Transport Create(
        std::string const& hostName,
        uint16_t hostPort,
        TransportEvents* eventHandler = nullptr);

    SaslTransportFactory() = delete;
  };

}}}}} // namespace Azure::Core::Amqp::Network::_internal
#endif
