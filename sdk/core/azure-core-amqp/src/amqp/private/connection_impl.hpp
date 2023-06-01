// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include "azure/core/amqp/connection.hpp"
#include "azure/core/amqp/network/transport.hpp"

#include <azure/core/credentials/credentials.hpp>

#include <azure_uamqp_c/connection.h>

#include <chrono>
#include <memory>
#include <string>

template <> struct Azure::Core::_internal::UniqueHandleHelper<CONNECTION_INSTANCE_TAG>
{
  static void FreeAmqpConnection(CONNECTION_HANDLE obj);

  using type
      = Azure::Core::_internal::BasicUniqueHandle<CONNECTION_INSTANCE_TAG, FreeAmqpConnection>;
};

using UniqueAmqpConnection = Azure::Core::_internal::UniqueHandle<CONNECTION_INSTANCE_TAG>;

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  class ClaimsBasedSecurity;

  class ConnectionFactory final {
  public:
    static Azure::Core::Amqp::_internal::Connection CreateFromInternal(
        std::shared_ptr<ConnectionImpl> connectionImpl)
    {
      return Azure::Core::Amqp::_internal::Connection(connectionImpl);
    }

    static std::shared_ptr<ConnectionImpl> GetImpl(
        Azure::Core::Amqp::_internal::Connection const& connection)
    {
      return connection.m_impl;
    }
  };

  class ConnectionImpl final : public std::enable_shared_from_this<ConnectionImpl> {
  public:
    ConnectionImpl(
        std::shared_ptr<Network::_detail::TransportImpl> transport,
        _internal::ConnectionOptions const& options,
        _internal::ConnectionEvents* eventHandler);

    ConnectionImpl(
        std::string const& hostName,
        std::shared_ptr<Credentials::TokenCredential> tokenCredential,
        _internal::ConnectionOptions const& options,
        _internal::ConnectionEvents* eventHandler);

    virtual ~ConnectionImpl();

    // Because m_connection has a pointer back to the Connection object, we cannot move or delete
    // Connection objects.
    ConnectionImpl(ConnectionImpl const&) = delete;
    ConnectionImpl& operator=(ConnectionImpl const&) = delete;
    ConnectionImpl(ConnectionImpl&&) noexcept = delete;
    ConnectionImpl& operator=(ConnectionImpl&&) = delete;

    /**
     * @brief Complete the construction of the ConnectionImpl object. This is required because the
     * uAMQP call to connection_create/connection_create2 will call the event handler to indicate
     * that the connection was created, but std::enable_shared_from_this requires that the
     * std::shared_ptr containing the Connection be fully created.
     *
     * If the call to connection_create/connection_create2 is made from the constructor of the
     * ConnectionImpl, the shared_ptr will not have been fully constructed, causing a crash.
     */
    void FinishConstruction();

    operator CONNECTION_HANDLE() const { return m_connection.get(); }

    void Open();
    void Listen();

    void Close(
        std::string const& condition,
        std::string const& description,
        Models::AmqpValue info);

    void Poll();

    std::string GetHost() const { return m_hostName; }
    uint16_t GetPort() const { return m_port; }

    uint32_t GetMaxFrameSize() const;
    uint32_t GetRemoteMaxFrameSize() const;
    uint16_t GetMaxChannel() const;
    std::chrono::milliseconds GetIdleTimeout() const;
    void SetIdleEmptyFrameSendPercentage(double idleTimeoutEmptyFrameSendRatio);

    void SetProperties(Models::AmqpValue properties);
    Models::AmqpMap GetProperties() const;
    std::shared_ptr<Credentials::TokenCredential> GetCredential() const { return m_credential; }

  private:
    std::shared_ptr<Network::_detail::TransportImpl> m_transport;
    UniqueAmqpConnection m_connection{};
    std::string m_hostName;
    uint16_t m_port;
    std::string m_containerId;
    _internal::ConnectionOptions m_options;
    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<std::unique_ptr<_internal::Session>>
        m_newSessionQueue;
    _internal::ConnectionEvents* m_eventHandler{};
    _internal::ConnectionState m_connectionState = _internal::ConnectionState::Start;
    std::shared_ptr<Credentials::TokenCredential> m_credential{};

    ConnectionImpl(
        _internal::ConnectionEvents* eventHandler,
        _internal::ConnectionOptions const& options);

    void SetState(_internal::ConnectionState newState) { m_connectionState = newState; }

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
    static void OnIOErrorFn(void* context);
  };
}}}} // namespace Azure::Core::Amqp::_detail
