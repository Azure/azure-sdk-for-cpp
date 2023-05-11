// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include <azure/core/credentials/credentials.hpp>

#include "azure/core/amqp/models/amqp_protocol.hpp"
#include "common/async_operation_queue.hpp"
#include "connection_string_credential.hpp"
#include "models/amqp_value.hpp"
#include "session.hpp"

#include <chrono>
#include <memory>
#include <string>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  class ConnectionImpl;
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _internal {
  class Session;

  struct ConnectionOptions
  {
    //    std::chrono::seconds IdleTimeout{std::chrono::seconds(1)};
    uint32_t MaxFrameSize{512};
    uint16_t MaxSessions{65535};

    Azure::Core::Amqp::Models::AmqpValue Properties;

    std::chrono::seconds Timeout{0};

    /** Enable tracing from the uAMQP stack.
     */
    bool EnableTrace{false};

    /** Defines the ID of the container for this connection. If empty, a unique 128 bit value will
     * be used.
     */
    std::string ContainerId;

    std::string HostName;
    uint16_t Port{_detail::AmqpsPort}; // Assume TLS port by default.

    // Default transport to be used. Normally only needed for socket listeners which need to
    // specify the listening socket characteristics.
    std::shared_ptr<Azure::Core::Amqp::Network::_internal::Transport> Transport{};

    // Optional SASL plain credentials.
    std::shared_ptr<Azure::Core::Amqp::_internal::SaslPlainConnectionStringCredential>
        SaslCredentials{};
  };

  class Error;

  enum class ConnectionState
  {
    Start,
    HeaderReceived,
    HeaderSent,
    HeaderExchanged,
    OpenPipe,
    OcPipe,
    OpenReceived,
    OpenSent,
    ClosePipe,
    Opened,
    CloseReceived,
    CloseSent,
    Discarding,
    End,
    Error
  };

  class Connection;

  /** @brief The ConnectionEvents interface defines a series of events triggered on a connection
   * object.
   */
  struct ConnectionEvents
  {
    /** @brief Called when the connection state changes.
     *
     * @param connection The connection object whose state changed.
     * @param newState The new state of the connection.
     * @param oldState The previous state of the connection.
     */
    virtual void OnConnectionStateChanged(
        Connection const& connection,
        ConnectionState newState,
        ConnectionState oldState)
        = 0;

    /** @brief Called when a new endpoint connects to the connection.
     *
     * @param connection The connection object.
     * @param endpoint The endpoint that connected.
     * @return true if the endpoint was accepted, false otherwise.
     *
     * @remarks Note that this function should only be overriden if the application is listening
     * on the connection.
     */
    virtual bool OnNewEndpoint(Connection const& connection, Endpoint& endpoint)
    {
      (void)connection;
      (void)endpoint;
      return false;
    }

    /** @brief called when an I/O error has occurred on the connection.
     *
     * @param connection The connection object.
     */
    virtual void OnIoError(Connection const& connection) = 0;
  };

  class Connection final {
  public:
    Connection(
        std::shared_ptr<Azure::Core::Amqp::Network::_internal::Transport> transport,
        ConnectionOptions const& options,
        ConnectionEvents* eventHandler = nullptr);

    Connection(
        std::string const& requestUri,
        ConnectionOptions const& options,
        ConnectionEvents* eventHandler = nullptr);

    ~Connection();

    Connection(Connection const&) = default;
    Connection& operator=(Connection const&) = default;
    Connection(Connection&&) noexcept = default;
    Connection& operator=(Connection&&) = default;
    Connection(std::shared_ptr<Azure::Core::Amqp::_detail::ConnectionImpl> impl) : m_impl{impl} {}

    std::shared_ptr<Azure::Core::Amqp::_detail::ConnectionImpl> GetImpl() const { return m_impl; }

    void Open();
    void Listen();

    void Close(
        std::string const& condition,
        std::string const& description,
        Azure::Core::Amqp::Models::AmqpValue info);

    void Poll() const;

    uint32_t GetMaxFrameSize() const;
    uint32_t GetRemoteMaxFrameSize() const;
    void SetMaxFrameSize(uint32_t frameSize);
    uint16_t GetMaxChannel() const;
    void SetMaxChannel(uint16_t frameSize);
    std::chrono::milliseconds GetIdleTimeout() const;
    void SetIdleTimeout(std::chrono::milliseconds timeout);
    void SetRemoteIdleTimeoutEmptyFrameSendRatio(double idleTimeoutEmptyFrameSendRatio);

    void SetProperties(Azure::Core::Amqp::Models::AmqpValue properties);
    Azure::Core::Amqp::Models::AmqpValue GetProperties() const;
    uint64_t HandleDeadlines(); // ???
    Endpoint CreateEndpoint();
    void StartEndpoint(Endpoint const& endpoint);

    uint16_t GetEndpointIncomingChannel(Endpoint endpoint);
    void DestroyEndpoint(Endpoint endpoint);

    void SetTrace(bool enableTrace);

  private:
    std::shared_ptr<Azure::Core::Amqp::_detail::ConnectionImpl> m_impl;
  };
}}}} // namespace Azure::Core::Amqp::_internal
