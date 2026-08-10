// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../../../../amqp/private/unique_handle.hpp"
#include "azure/core/amqp/internal/common/global_state.hpp"
#include "azure/core/amqp/internal/connection.hpp"
#include "azure/core/amqp/internal/network/transport.hpp"

#include <azure/core/context.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/url.hpp>

#include <azure_uamqp_c/connection.h>

#include <chrono>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#if defined(_MSC_VER)
#define _azure_ACQUIRES_LOCK(...) _Acquires_exclusive_lock_(__VA_ARGS__)
#else
#define _azure_ACQUIRES_LOCK(...)
#endif

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  using AmqpConnectionImplementation = CONNECTION_INSTANCE_TAG;

  template <> struct UniqueHandleHelper<AmqpConnectionImplementation>
  {
    static void FreeAmqpConnection(AmqpConnectionImplementation* obj);

    using type
        = Core::_internal::BasicUniqueHandle<AmqpConnectionImplementation, FreeAmqpConnection>;
  };

}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  using UniqueAmqpConnection
      = UniqueHandle<Azure::Core::Amqp::_detail::AmqpConnectionImplementation>;

  std::ostream& operator<<(std::ostream& os, CONNECTION_STATE state);

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

  class ConnectionImpl final : public std::enable_shared_from_this<ConnectionImpl>,
                               public Common::_detail::Pollable {
  public:
    ConnectionImpl(
        std::shared_ptr<Network::_detail::TransportImpl> transport,
        _internal::ConnectionOptions const& options,
        _internal::ConnectionEvents* eventHandler,
        _internal::ConnectionEndpointEvents* endpointEvents);

    ConnectionImpl(
        std::string const& hostName,
        std::shared_ptr<const Credentials::TokenCredential> tokenCredential,
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

    void Open(Azure::Core::Context const&);
    void Listen();

    void Close(Azure::Core::Context const&);
    void Close(
        std::string const& condition,
        std::string const& description,
        Models::AmqpValue info,
        Azure::Core::Context const&);

    void Poll() override;
    std::string GetHost() const { return m_hostName; }
    uint16_t GetPort() const { return m_port; }

    uint32_t GetMaxFrameSize() const;
    uint16_t GetMaxChannel() const;
    std::chrono::milliseconds GetIdleTimeout() const;
    uint32_t GetRemoteMaxFrameSize() const;
    void SetIdleEmptyFrameSendPercentage(double idleTimeoutEmptyFrameSendRatio);

    Models::AmqpMap GetProperties() const;
    std::shared_ptr<const Credentials::TokenCredential> GetCredential() const
    {
      return m_credential;
    }
    void EnableAsyncOperation(bool enable);
    bool IsAsyncOperation() { return m_enableAsyncOperation; }
    bool IsTraceEnabled() { return m_options.EnableTrace; }
    bool IsSasCredential() const;

    // Authenticate the audience on this connection using the provided session.
    Azure::Core::Credentials::AccessToken AuthenticateAudience(
        std::shared_ptr<SessionImpl> session,
        std::string const& audience,
        Azure::Core::Context const& context);

    // Stop the token refresh thread. This is safe to call more than once, and
    // the connection calls it from Close and from the destructor.
    void StopTokenRefresh();

    using LockType = std::recursive_mutex;

    _azure_ACQUIRES_LOCK(m_amqpMutex) std::unique_lock<LockType> Lock()
    {
      return std::unique_lock<LockType>(m_amqpMutex);
    }

  private:
    std::shared_ptr<Network::_detail::TransportImpl> m_transport;
    UniqueAmqpConnection m_connection{};
    std::string m_hostName;
    uint16_t m_port{};
    std::string m_containerId;
    _internal::ConnectionOptions m_options;
    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<std::unique_ptr<_internal::Session>>
        m_newSessionQueue;
    _internal::ConnectionEvents* m_eventHandler{};
    _internal::ConnectionEndpointEvents* m_endpointEvents{};
    _internal::ConnectionState m_connectionState = _internal::ConnectionState::Start;

    LockType m_amqpMutex;
    bool m_enableAsyncOperation = false;
    bool m_isClosing = false;

    bool m_connectionOpened = false;
    std::atomic<uint32_t> m_openCount{0};

    // mutex protecting the token acquisition process.
    std::mutex m_tokenMutex;
    std::shared_ptr<const Credentials::TokenCredential> m_credential{};
    std::map<std::string, Credentials::AccessToken> m_tokenStore;

    // Serializes the CBS operation itself. uAMQP names the CBS links after the
    // node, so every claims based security object on this connection attaches a
    // link called "$cbs-sender" and one called "$cbs-receiver". AMQP 1.0 section
    // 2.6.1 requires a link name to be unique for one direction between two
    // containers, so only one of these objects may exist at a time. The refresh
    // thread does its work without the token mutex, so this mutex is what keeps
    // the refresh and a caller apart.
    //
    // Lock order: a caller takes m_tokenMutex and then this mutex. The refresh
    // thread takes this mutex only while it does not hold m_tokenMutex.
    std::mutex m_cbsMutex;

    // The session that authenticated each audience. The pointer is weak, so the
    // refresh thread never keeps a session alive.
    std::map<std::string, std::weak_ptr<SessionImpl>> m_tokenSessions;
    // The thread that replaces each cached token before the token expires.
    std::thread m_tokenRefreshThread;
    std::condition_variable m_tokenRefreshCv;
    bool m_tokenRefreshStop{false};
    // Cancelled on shutdown, to stop a CBS operation that is in flight.
    Azure::Core::Context m_tokenRefreshContext;

    void StartTokenRefresh();
    void TokenRefreshThread();
    void RefreshTokenForAudience(
        std::string const& audienceUrl,
        std::unique_lock<std::mutex>& lock);

    ConnectionImpl(
        _internal::ConnectionEvents* eventHandler,
        _internal::ConnectionOptions const& options);

    void SetState(_internal::ConnectionState newState) { m_connectionState = newState; }
    static void OnConnectionStateChangedFn(
        void* context,
        CONNECTION_STATE newState,
        CONNECTION_STATE oldState);
    // Note: We cannot take ownership of this instance tag.
    static bool OnNewEndpointFn(void* context, ENDPOINT_HANDLE endpoint);
    static void OnIOErrorFn(void* context);
  };
}}}} // namespace Azure::Core::Amqp::_detail
