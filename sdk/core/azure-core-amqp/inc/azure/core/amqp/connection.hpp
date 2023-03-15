// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#pragma once

#include <chrono>
#include <memory>
#include <string>

#include "models/amqp_value.hpp"
#include "network/Transport.hpp"
#include "session.hpp"
#include "common/async_operation_queue.hpp"
#include "connection_string_credential.hpp"
#include <azure/core/credentials/credentials.hpp>

struct CONNECTION_INSTANCE_TAG;
struct ENDPOINT_INSTANCE_TAG;
struct AMQP_VALUE_DATA_TAG;
enum CONNECTION_STATE_TAG : int;

namespace Azure { namespace Core { namespace _internal { namespace Amqp {

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
        Azure::Core::Amqp::Models::Value value,
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

    virtual ~Connection();

    // Because m_connection has a pointer back to the Connection object, we cannot move or delete
    // Connection objects.
    Connection(Connection const&) = delete;
    Connection& operator=(Connection const&) = delete;
    Connection(Connection&&) noexcept = delete;
    Connection& operator=(Connection&&) = delete;

    operator CONNECTION_INSTANCE_TAG*() const { return m_connection; }

    void Open();
    void Listen();

    void Close(
        std::string const& condition,
        std::string const& description,
        Azure::Core::Amqp::Models::Value info);

    void Poll() const;

    uint32_t GetMaxFrameSize();
    uint32_t GetRemoteMaxFrameSize();
    void SetMaxFrameSize(uint32_t frameSize);
    uint16_t GetMaxChannel();
    void SetMaxChannel(uint16_t frameSize);
    std::chrono::milliseconds GetIdleTimeout();
    void SetIdleTimeout(std::chrono::milliseconds timeout);
    void SetRemoteIdleTimeoutEmptyFrameSendRatio(double idleTimeoutEmptyFrameSendRatio);

    void SetProperties(Azure::Core::Amqp::Models::Value properties);
    Azure::Core::Amqp::Models::Value GetProperties();
    uint64_t HandleDeadlines(); // ???
    Endpoint CreateEndpoint();
    void StartEndpoint(Endpoint const& endpoint);

    uint16_t GetEndpointIncomingChannel(Endpoint endpoint);
    void DestroyEndpoint(Endpoint endpoint);
    void EncodeFrame(
        Endpoint endpoint,
        Azure::Core::Amqp::Models::Value performative,
        std::vector<Azure::Core::Amqp::Models::BinaryData> payloads,
        Azure::Core::_internal::Amqp::Network::Transport::TransportSendCompleteFn onSendComplete);

    void SetTrace(bool enableTrace);

  private:
    std::shared_ptr<Network::Transport> m_transport;
    CONNECTION_INSTANCE_TAG* m_connection{};
    std::string m_hostName;
    std::string m_containerId;
    Common::AsyncOperationQueue<std::unique_ptr<Session>> m_newSessionQueue;
    ConnectionEvents* m_eventHandler{};
    CredentialType m_credentialType;
    std::shared_ptr<ConnectionStringCredential> m_credential{};
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_tokenCredential{};

    Connection(
        ConnectionEvents* eventHandler,
        CredentialType credentialType,
        ConnectionOptions const& options);

    void CreateUnderlyingConnection(std::string const& hostName, ConnectionOptions const& options);

    static void OnEndpointFrameReceivedFn(
        void* context,
        AMQP_VALUE_DATA_TAG* value,
        uint32_t framePayloadSize,
        uint8_t* payloadBytes);
    static void OnConnectionStateChangedFn(
        void* context,
        CONNECTION_STATE_TAG newState,
        CONNECTION_STATE_TAG oldState);
    // Note: We cannot take ownership of this instance tag.
    static bool OnNewEndpointFn(void* context, ENDPOINT_INSTANCE_TAG* endpoint);
    static void OnIoErrorFn(void* context);

#if 0
    
    MOCKABLE_FUNCTION(, CONNECTION_HANDLE, connection_create, XIO_HANDLE, io, const char*, hostname, const char*, container_id, ON_NEW_ENDPOINT, on_new_endpoint, void*, callback_context);
    MOCKABLE_FUNCTION(, CONNECTION_HANDLE, connection_create2, XIO_HANDLE, xio, const char*, hostname, const char*, container_id, ON_NEW_ENDPOINT, on_new_endpoint, void*, callback_context, ON_CONNECTION_STATE_CHANGED, on_connection_state_changed, void*, on_connection_state_changed_context, ON_IO_ERROR, on_io_error, void*, on_io_error_context);
    MOCKABLE_FUNCTION(, void, connection_destroy, CONNECTION_HANDLE, connection);
    

    MOCKABLE_FUNCTION(, ON_CONNECTION_CLOSED_EVENT_SUBSCRIPTION_HANDLE, connection_subscribe_on_connection_close_received, CONNECTION_HANDLE, connection, ON_CONNECTION_CLOSE_RECEIVED, on_connection_close_received, void*, context);
    MOCKABLE_FUNCTION(, void, connection_unsubscribe_on_connection_close_received, ON_CONNECTION_CLOSED_EVENT_SUBSCRIPTION_HANDLE, event_subscription);

#endif
  };
}}}} // namespace Azure::Core::_internal::Amqp
