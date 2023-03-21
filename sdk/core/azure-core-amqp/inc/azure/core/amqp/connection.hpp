// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include "common/async_operation_queue.hpp"
#include "connection_string_credential.hpp"
#include "models/amqp_value.hpp"
#include "session.hpp"
#include <azure/core/credentials/credentials.hpp>
#include <chrono>
#include <memory>
#include <string>

namespace Azure { namespace Core { namespace _internal { namespace Amqp {

  namespace _detail {
    class ConnectionImpl;
  }

  class Session;

  struct ConnectionOptions
  {
    //    std::chrono::seconds IdleTimeout{std::chrono::seconds(1)};
    uint32_t MaxFrameSize{512};
    uint16_t MaxSessions{65535};

    Azure::Core::Amqp::Models::Value Properties;

    std::string SASLType; // Not a string - fill in later.

    std::chrono::seconds Timeout{0};

    /** Enable tracing from the uAMQP stack.
     */
    bool EnableTrace{false};

    /** Defines the ID of the container for this connection. If empty, a unique 128 bit value will
     * be used.
     */
    std::string ContainerId;

    std::string HostName;
    uint16_t Port{5671}; // Assume TLS port by default.

    // Default transport to be used. Normally only needed for socket listeners which need to specify
    // the listening socket characteristics.
    std::shared_ptr<Azure::Core::_internal::Amqp::Network::Transport> Transport{};

    // Optional SASL plain credentials.
    std::shared_ptr<Azure::Core::_internal::Amqp::SaslPlainConnectionStringCredential>
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

  struct ConnectionEvents
  {
    virtual void OnEndpointFrameReceived(
        Connection const& connection,
        Azure::Core::Amqp::Models::Value const& value,
        uint32_t framePayloadSize,
        uint8_t* payloadBytes)
        = 0;
    virtual void OnConnectionStateChanged(
        Connection const& connection,
        ConnectionState newState,
        ConnectionState oldState)
        = 0;
    virtual bool OnNewEndpoint(Connection const& connection, Endpoint& endpoint) = 0;
    virtual void OnIoError(Connection const& connection) = 0;
    virtual ~ConnectionEvents() = default;
  };

  class Connection final {
  public:
    //    using OnEndpointFrameReceivedCallback
    //        = std::function<void(Models::Value, uint32_t framePayloadSize, uint8_t*
    //        payloadBytes)>;
    //    using OnConnectionStateChangedCallback
    //        = std::function<void(ConnectionState newState, ConnectionState previousState)>;

    Connection(
        std::shared_ptr<Azure::Core::_internal::Amqp::Network::Transport> transport,
        ConnectionEvents* eventHandler,
        ConnectionOptions const& options);

    Connection(
        std::string const& requestUri,
        ConnectionEvents* eventHandler,
        ConnectionOptions const& options);

    Connection(ConnectionEvents* eventHandler, ConnectionOptions const& options);

    ~Connection();

    // Because m_connection has a pointer back to the Connection object, we cannot move or delete
    // Connection objects.
    Connection(Connection const&) = delete;
    Connection& operator=(Connection const&) = delete;
    Connection(Connection&&) noexcept = delete;
    Connection& operator=(Connection&&) = delete;
    Connection(std::shared_ptr<_detail::ConnectionImpl> impl) : m_impl{impl} {}

    std::shared_ptr<_detail::ConnectionImpl> GetImpl() const { return m_impl; }

    void Open();
    void Listen();

    void Close(
        std::string const& condition,
        std::string const& description,
        Azure::Core::Amqp::Models::Value info);

    void Poll() const;

    uint32_t GetMaxFrameSize() const;
    uint32_t GetRemoteMaxFrameSize() const;
    void SetMaxFrameSize(uint32_t frameSize);
    uint16_t GetMaxChannel() const;
    void SetMaxChannel(uint16_t frameSize);
    std::chrono::milliseconds GetIdleTimeout() const;
    void SetIdleTimeout(std::chrono::milliseconds timeout);
    void SetRemoteIdleTimeoutEmptyFrameSendRatio(double idleTimeoutEmptyFrameSendRatio);

    void SetProperties(Azure::Core::Amqp::Models::Value properties);
    Azure::Core::Amqp::Models::Value GetProperties() const;
    uint64_t HandleDeadlines(); // ???
    Endpoint CreateEndpoint();
    void StartEndpoint(Endpoint const& endpoint);

    uint16_t GetEndpointIncomingChannel(Endpoint endpoint);
    void DestroyEndpoint(Endpoint endpoint);
    // void EncodeFrame(
    //     Endpoint endpoint,
    //     Azure::Core::Amqp::Models::Value performative,
    //     std::vector<Azure::Core::Amqp::Models::BinaryData> payloads,
    //     Azure::Core::_internal::Amqp::Network::Transport::TransportSendCompleteFn
    //     onSendComplete);

    void SetTrace(bool enableTrace);

  private:
    std::shared_ptr<_detail::ConnectionImpl> m_impl;
  };
}}}} // namespace Azure::Core::_internal::Amqp
