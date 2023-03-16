// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#pragma once

#include <chrono>
#include <memory>
#include <string>

#include "azure/core/amqp/connection.hpp"
#include <azure/core/credentials/credentials.hpp>
#include <azure_uamqp_c/connection.h>

namespace Azure { namespace Core { namespace _internal { namespace Amqp { namespace _detail {

  class ConnectionImpl final : std::enable_shared_from_this<ConnectionImpl> {
  public:
    ConnectionImpl(
        std::shared_ptr<Azure::Core::_internal::Amqp::Network::Transport> transport,
        ConnectionEvents* eventHandler,
        ConnectionOptions const& options);

    ConnectionImpl(
        std::string const& requestUri,
        ConnectionEvents* eventHandler,
        ConnectionOptions const& options);
    ConnectionImpl(ConnectionEvents* eventHandler, ConnectionOptions const& options);

    virtual ~ConnectionImpl();

    // Because m_connection has a pointer back to the Connection object, we cannot move or delete
    // Connection objects.
    ConnectionImpl(ConnectionImpl const&) = delete;
    ConnectionImpl& operator=(ConnectionImpl const&) = delete;
    ConnectionImpl(Connection&&) noexcept = delete;
    ConnectionImpl& operator=(ConnectionImpl&&) = delete;

    operator CONNECTION_HANDLE() const { return m_connection; }

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
    void EncodeFrame(
        Endpoint endpoint,
        Azure::Core::Amqp::Models::Value performative,
        std::vector<Azure::Core::Amqp::Models::BinaryData> payloads,
        Azure::Core::_internal::Amqp::Network::Transport::TransportSendCompleteFn onSendComplete);

    void SetTrace(bool enableTrace);

  private:
    std::shared_ptr<Network::Transport> m_transport;
    CONNECTION_HANDLE m_connection{};
    std::string m_hostName;
    std::string m_containerId;
    Common::AsyncOperationQueue<std::unique_ptr<Session>> m_newSessionQueue;
    ConnectionEvents* m_eventHandler{};
    CredentialType m_credentialType;
    std::shared_ptr<ConnectionStringCredential> m_credential{};
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_tokenCredential{};

    ConnectionImpl(
        ConnectionEvents* eventHandler,
        CredentialType credentialType,
        ConnectionOptions const& options);

    void CreateUnderlyingConnection(std::string const& hostName, ConnectionOptions const& options);

    static void OnEndpointFrameReceivedFn(
        void* context,
        AMQP_VALUE value,
        uint32_t framePayloadSize,
        uint8_t* payloadBytes);
    static void OnConnectionStateChangedFn(
        void* context,
        CONNECTION_STATE newState,
        CONNECTION_STATE oldState);
    // Note: We cannot take ownership of this instance tag.
    static bool OnNewEndpointFn(void* context, ENDPOINT_HANDLE endpoint);
    static void OnIoErrorFn(void* context);
  };
}}}}} // namespace Azure::Core::_internal::Amqp::_detail
