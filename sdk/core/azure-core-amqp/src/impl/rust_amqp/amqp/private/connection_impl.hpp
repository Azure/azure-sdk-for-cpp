// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/internal/common/global_state.hpp"
#include "azure/core/amqp/internal/connection.hpp"
#include "azure/core/amqp/internal/network/transport.hpp"
#include "rust_amqp_wrapper.h"
#include "unique_handle.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/url.hpp>

#include <chrono>
#include <memory>
#include <string>

#if defined(_MSC_VER)
#define _azure_ACQUIRES_LOCK(...) _Acquires_exclusive_lock_(__VA_ARGS__)
#else
#define _azure_ACQUIRES_LOCK(...)
#endif

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  using AmqpConnectionImplementation = RustInterop::RustAmqpConnection;
  using AmqpConnectionOptionsImplementation = RustInterop::RustAmqpConnectionOptions;
  using AmqpConnectionOptionsBuilderImplementation = RustInterop::RustAmqpConnectionOptionsBuilder;

  template <> struct UniqueHandleHelper<AmqpConnectionImplementation>
  {
    static void FreeAmqpConnection(AmqpConnectionImplementation* obj);

    using type
        = Core::_internal::BasicUniqueHandle<AmqpConnectionImplementation, FreeAmqpConnection>;
  };

  template <> struct UniqueHandleHelper<AmqpConnectionOptionsImplementation>
  {
    static void FreeAmqpConnectionOptions(AmqpConnectionOptionsImplementation* obj);

    using type = Core::_internal::
        BasicUniqueHandle<AmqpConnectionOptionsImplementation, FreeAmqpConnectionOptions>;
  };

  template <> struct UniqueHandleHelper<AmqpConnectionOptionsBuilderImplementation>
  {
    static void FreeAmqpConnectionOptionsBuilder(AmqpConnectionOptionsBuilderImplementation* obj);

    using type = Core::_internal::BasicUniqueHandle<
        AmqpConnectionOptionsBuilderImplementation,
        FreeAmqpConnectionOptionsBuilder>;
  };

}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  using UniqueAmqpConnection
      = UniqueHandle<Azure::Core::Amqp::_detail::AmqpConnectionImplementation>;
  using UniqueAmqpConnectionOptions
      = UniqueHandle<Azure::Core::Amqp::_detail::AmqpConnectionOptionsImplementation>;
  using UniqueAmqpConnectionOptionsBuilder
      = UniqueHandle<Azure::Core::Amqp::_detail::AmqpConnectionOptionsBuilderImplementation>;

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
        std::string const& hostName,
        std::shared_ptr<Credentials::TokenCredential> tokenCredential,
        _internal::ConnectionOptions const& options);

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

    void Open();

    void Close(
        std::string const& condition = {},
        std::string const& description = {},
        Models::AmqpValue info = {});

    std::string GetHost() const { return m_hostUrl.GetHost(); }
    uint16_t GetPort() const { return m_hostUrl.GetPort(); }

    uint32_t GetMaxFrameSize() const;
    uint16_t GetMaxChannel() const;
    std::chrono::milliseconds GetIdleTimeout() const;

    Models::AmqpMap GetProperties() const;
    std::shared_ptr<Credentials::TokenCredential> GetCredential() const { return m_credential; }
    bool IsTraceEnabled() { return m_options.EnableTrace; }
    bool IsSasCredential() const;

    // Authenticate the audience on this connection using the provided session.
    Azure::Core::Credentials::AccessToken AuthenticateAudience(
        std::shared_ptr<SessionImpl> session,
        std::string const& audience,
        Azure::Core::Context const& context);

    using LockType = std::recursive_mutex;

    _azure_ACQUIRES_LOCK(m_amqpMutex) std::unique_lock<LockType> Lock()
    {
      return std::unique_lock<LockType>(m_amqpMutex);
    }

    _detail::AmqpConnectionImplementation* GetConnection() { return m_connection.get(); }

  private:
    UniqueAmqpConnection m_connection{};
    Azure::Core::Url m_hostUrl;
    UniqueAmqpConnectionOptions m_connectionOptions{};
    std::string m_containerId;
    _internal::ConnectionOptions m_options;
    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<std::unique_ptr<_internal::Session>>
        m_newSessionQueue;

    LockType m_amqpMutex;
    bool m_enableAsyncOperation = false;
    bool m_isClosing = false;

    bool m_connectionOpened = false;
    std::atomic<uint32_t> m_openCount{0};

    // mutex protecting the token acquisition process.
    std::mutex m_tokenMutex;
    std::shared_ptr<Credentials::TokenCredential> m_credential{};
    std::map<std::string, Credentials::AccessToken> m_tokenStore;

    ConnectionImpl(
        _internal::ConnectionEvents* eventHandler,
        _internal::ConnectionOptions const& options);
  };
}}}} // namespace Azure::Core::Amqp::_detail
