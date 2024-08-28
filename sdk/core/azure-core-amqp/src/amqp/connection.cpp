// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/connection.hpp"

#include "../models/private/value_impl.hpp"
#include "../network/private/transport_impl.hpp"
#include "azure/core/amqp/internal/common/global_state.hpp"
#include "azure/core/amqp/internal/network/socket_transport.hpp"
#include "azure/core/amqp/internal/network/tls_transport.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"
#include "private/claims_based_security_impl.hpp"
#include "private/connection_impl.hpp"
#include "private/session_impl.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/uuid.hpp>

#if ENABLE_UAMQP
#include <azure_uamqp_c/connection.h>
#endif

#include <memory>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  void UniqueHandleHelper<CONNECTION_INSTANCE_TAG>::FreeAmqpConnection(CONNECTION_HANDLE value)
  {
    connection_destroy(value);
  }
#endif
}}}} // namespace Azure::Core::Amqp::_detail

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

namespace Azure { namespace Core { namespace Amqp { namespace _internal {

  // Create a connection with an existing networking Transport.
  Connection::Connection(
      Network::_internal::Transport const& transport,
      ConnectionOptions const& options,
      ConnectionEvents* eventHandler,
      ConnectionEndpointEvents* endpointEventHandler)
      : m_impl{std::make_shared<_detail::ConnectionImpl>(
          transport.GetImpl(),
          options,
          eventHandler,
          endpointEventHandler)}
  {
    m_impl->FinishConstruction();
  }

  // Create a connection with a request URI and options.
  Connection::Connection(
      std::string const& hostName,
      std::shared_ptr<Credentials::TokenCredential> credential,
      ConnectionOptions const& options,
      ConnectionEvents* eventHandler)
      : m_impl{
          std::make_shared<_detail::ConnectionImpl>(hostName, credential, options, eventHandler)}
  {
    m_impl->FinishConstruction();
  }

  Connection::~Connection() {}

  Session Connection::CreateSession(
      SessionOptions const& sessionOptions,
      SessionEvents* sessionEvents) const
  {
    return Azure::Core::Amqp::_detail::SessionFactory::CreateFromInternal(
        std::make_shared<_detail::SessionImpl>(m_impl, sessionOptions, sessionEvents));
  }

  Session Connection::CreateSession(
      Endpoint& endpoint,
      SessionOptions const& sessionOptions,
      SessionEvents* sessionEvents) const
  {
    return Azure::Core::Amqp::_detail::SessionFactory::CreateFromInternal(
        std::make_shared<_detail::SessionImpl>(m_impl, endpoint, sessionOptions, sessionEvents));
  }

  void Connection::Poll() { m_impl->Poll(); }

  void Connection::Listen() { m_impl->Listen(); }
  void Connection::Open() { m_impl->Open(); }
  void Connection::Close(
      std::string const& condition,
      std::string const& description,
      Models::AmqpValue value)
  {
    m_impl->Close(condition, description, value);
  }
  uint32_t Connection::GetMaxFrameSize() const { return m_impl->GetMaxFrameSize(); }
  uint32_t Connection::GetRemoteMaxFrameSize() const { return m_impl->GetRemoteMaxFrameSize(); }
  uint16_t Connection::GetMaxChannel() const { return m_impl->GetMaxChannel(); }
  std::string Connection::GetHost() const { return m_impl->GetHost(); }
  uint16_t Connection::GetPort() const { return m_impl->GetPort(); }
  std::chrono::milliseconds Connection::GetIdleTimeout() const { return m_impl->GetIdleTimeout(); }
  Models::AmqpMap Connection::GetProperties() const { return m_impl->GetProperties(); }
  void Connection::SetIdleEmptyFrameSendPercentage(double ratio)
  {
    m_impl->SetIdleEmptyFrameSendPercentage(ratio);
  }
  std::ostream& operator<<(std::ostream& stream, ConnectionState state)
  {
    switch (state)
    {
      case Azure::Core::Amqp::_internal::ConnectionState::Start:
        stream << "Start";
        break;
      case Azure::Core::Amqp::_internal::ConnectionState::HeaderReceived:
        stream << "HeaderReceived";
        break;
      case Azure::Core::Amqp::_internal::ConnectionState::HeaderSent:
        stream << "HeaderSent";
        break;
      case Azure::Core::Amqp::_internal::ConnectionState::HeaderExchanged:
        stream << "HeaderExchanged";
        break;
      case Azure::Core::Amqp::_internal::ConnectionState::OpenPipe:
        stream << "OpenPipe";
        break;
      case Azure::Core::Amqp::_internal::ConnectionState::OcPipe:
        stream << "OcPipe";
        break;
      case Azure::Core::Amqp::_internal::ConnectionState::OpenReceived:
        stream << "OpenReceived";
        break;
      case Azure::Core::Amqp::_internal::ConnectionState::OpenSent:
        stream << "OpenSent";
        break;
      case Azure::Core::Amqp::_internal::ConnectionState::ClosePipe:
        stream << "ClosePipe";
        break;
      case Azure::Core::Amqp::_internal::ConnectionState::Opened:
        stream << "Opened";
        break;
      case Azure::Core::Amqp::_internal::ConnectionState::CloseReceived:
        stream << "CloseReceived";
        break;
      case Azure::Core::Amqp::_internal::ConnectionState::CloseSent:
        stream << "CloseSent";
        break;
      case Azure::Core::Amqp::_internal::ConnectionState::Discarding:
        stream << "Discarding";
        break;
      case Azure::Core::Amqp::_internal::ConnectionState::End:
        stream << "End";
        break;
      case Azure::Core::Amqp::_internal::ConnectionState::Error:
        stream << "Error";
        break;
    }
    return stream;
  }

}}}} // namespace Azure::Core::Amqp::_internal

namespace {
void EnsureGlobalStateInitialized()
{
  // Force the global instance to exist. This is required to ensure that uAMQP and
  // azure-c-shared-utility is
  auto globalInstance
      = Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance();
  (void)globalInstance;
}
} // namespace

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  // Create a connection with an existing networking Transport.
  ConnectionImpl::ConnectionImpl(
      std::shared_ptr<Network::_detail::TransportImpl> transport,
      _internal::ConnectionOptions const& options,
      _internal::ConnectionEvents* eventHandler,
      _internal::ConnectionEndpointEvents* endpointEvents)
      : m_hostName{"localhost"}, m_options{options}, m_eventHandler{eventHandler},
        m_endpointEvents{endpointEvents}
  {
    EnsureGlobalStateInitialized();
    m_transport = transport;
  }

  // Create a connection with a request URI and options.
  ConnectionImpl::ConnectionImpl(
      std::string const& hostName,
      std::shared_ptr<Credentials::TokenCredential> credential,
      _internal::ConnectionOptions const& options,
      _internal::ConnectionEvents* eventHandler)
      : m_hostName{hostName}, m_port{options.Port}, m_options{options},
        m_eventHandler{eventHandler}, m_credential{credential}
  {
    EnsureGlobalStateInitialized();

    if (options.Port == _internal::AmqpPort)
    {
      Log::Write(Logger::Level::Informational, "Creating socket connection transport.");
      m_transport
          = Network::_internal::SocketTransportFactory::Create(m_hostName, m_port).GetImpl();
    }
    else if (options.Port == _internal::AmqpTlsPort)
    {
      m_transport = Network::_internal::TlsTransportFactory::Create(m_hostName, m_port).GetImpl();
    }
    else
    {
      Log::Write(
          Logger::Level::Informational,
          "Unknown port specified, assuming socket connection transport.");
      m_transport
          = Network::_internal::SocketTransportFactory::Create(m_hostName, m_port).GetImpl();
    }
  }

  ConnectionImpl::~ConnectionImpl()
  {
    std::unique_lock<LockType> lock(m_amqpMutex);
    if (m_openCount.load() != 0)
    {
      AZURE_ASSERT_MSG(m_openCount.load() == 0, "Connection is being destroyed while polling.");
      Azure::Core::_internal::AzureNoReturnPath("Connection is being destroyed while polling.");
    }
    if (m_connectionOpened)
    {
      AZURE_ASSERT_MSG(!m_connectionOpened, "Connection being destroyed while open.");
      Azure::Core::_internal::AzureNoReturnPath("Connection is being destroyed while open.");
    }
    m_isClosing = true;

    // If the connection is going away, we don't want to generate any more events on it.
    if (m_eventHandler)
    {
      m_eventHandler = nullptr;
    }

#if ENABLE_UAMQP
    m_connection.reset();
#endif
    lock.unlock();
  }

  void ConnectionImpl::FinishConstruction()
  {
    std::string containerId{m_options.ContainerId};
    if (containerId.empty())
    {
      containerId = Azure::Core::Uuid::CreateUuid().ToString();
    }
    m_containerId = containerId;
#if ENABLE_UAMQP

    m_connection.reset(connection_create2(
        *m_transport,
        m_hostName.c_str(),
        containerId.c_str(),
        (m_endpointEvents ? OnNewEndpointFn : nullptr),
        this,
        OnConnectionStateChangedFn,
        this,
        OnIOErrorFn,
        this));
    if (m_options.EnableTrace)
    {
      connection_set_trace(m_connection.get(), m_options.EnableTrace);
    }
    if (connection_set_idle_timeout(
            m_connection.get(), static_cast<milliseconds>(m_options.IdleTimeout.count())))
    {
      throw std::runtime_error("Failed to set idle timeout.");
    }
    if (connection_set_channel_max(m_connection.get(), m_options.MaxChannelCount))
    {
      throw std::runtime_error("Failed to set max channel count.");
    }
    if (connection_set_max_frame_size(m_connection.get(), m_options.MaxFrameSize))
    {
      throw std::runtime_error("Failed to set max frame size.");
    }
    if (connection_set_properties(
            m_connection.get(),
            Models::_detail::AmqpValueFactory::ToImplementation(
                m_options.Properties.AsAmqpValue())))
    {
      throw std::runtime_error("Failed to set connection properties.");
    }
#endif
  }

  void ConnectionImpl::Poll()
  {
    std::unique_lock<LockType> lock(m_amqpMutex);
    if (m_connectionState == _internal::ConnectionState::Error
        || m_connectionState == _internal::ConnectionState::End)
    {
      return;
    }
    if (!m_isClosing)
    {
#if ENABLE_UAMQP
      if (m_connection)
      {
        connection_dowork(m_connection.get());
      }
#endif
    }
  }

  namespace {
#if ENABLE_UAMQP
    const std::map<CONNECTION_STATE, _internal::ConnectionState> UamqpToAmqpConnectionStateMap{
        {CONNECTION_STATE_START, _internal::ConnectionState::Start},
        {CONNECTION_STATE_CLOSE_PIPE, _internal::ConnectionState::ClosePipe},
        {CONNECTION_STATE_CLOSE_RCVD, _internal::ConnectionState::CloseReceived},
        {CONNECTION_STATE_END, _internal::ConnectionState::End},
        {CONNECTION_STATE_HDR_RCVD, _internal::ConnectionState::HeaderReceived},
        {CONNECTION_STATE_HDR_SENT, _internal::ConnectionState::HeaderSent},
        {CONNECTION_STATE_HDR_EXCH, _internal::ConnectionState::HeaderExchanged},
        {CONNECTION_STATE_OPEN_PIPE, _internal::ConnectionState::OpenPipe},
        {CONNECTION_STATE_OC_PIPE, _internal::ConnectionState::OcPipe},
        {CONNECTION_STATE_OPEN_RCVD, _internal::ConnectionState::OpenReceived},
        {CONNECTION_STATE_OPEN_SENT, _internal::ConnectionState::OpenSent},
        {CONNECTION_STATE_OPENED, _internal::ConnectionState::Opened},
        {CONNECTION_STATE_CLOSE_RCVD, _internal::ConnectionState::CloseReceived},
        {CONNECTION_STATE_CLOSE_SENT, _internal::ConnectionState::CloseSent},
        {CONNECTION_STATE_DISCARDING, _internal::ConnectionState::Discarding},
        {CONNECTION_STATE_ERROR, _internal::ConnectionState::Error},
    };
    const std::map<CONNECTION_STATE, std::string> UamqpConnectionStateToStringMap{
        {CONNECTION_STATE_START, "CONNECTION_STATE_START"},
        {CONNECTION_STATE_CLOSE_PIPE, "CONNECTION_STATE_CLOSE_PIPE"},
        {CONNECTION_STATE_CLOSE_RCVD, "CONNECTION_STATE_CLOSE_RCVD"},
        {CONNECTION_STATE_END, "CONNECTION_STATE_END"},
        {CONNECTION_STATE_HDR_RCVD, "CONNECTION_STATE_HDR_RCVD"},
        {CONNECTION_STATE_HDR_SENT, "CONNECTION_STATE_HDR_SENT"},
        {CONNECTION_STATE_HDR_EXCH, "CONNECTION_STATE_HDR_EXCH"},
        {CONNECTION_STATE_OPEN_PIPE, "CONNECTION_STATE_OPEN_PIPE"},
        {CONNECTION_STATE_OC_PIPE, "CONNECTION_STATE_OC_PIPE"},
        {CONNECTION_STATE_OPEN_RCVD, "CONNECTION_STATE_OPEN_RCVD"},
        {CONNECTION_STATE_OPEN_SENT, "CONNECTION_STATE_OPEN_SENT"},
        {CONNECTION_STATE_OPENED, "CONNECTION_STATE_OPENED"},
        {CONNECTION_STATE_CLOSE_RCVD, "CONNECTION_STATE_CLOSE_RCVD"},
        {CONNECTION_STATE_CLOSE_SENT, "CONNECTION_STATE_CLOSE_SENT"},
        {CONNECTION_STATE_DISCARDING, "CONNECTION_STATE_DISCARDING"},
        {CONNECTION_STATE_ERROR, "CONNECTION_STATE_ERROR"},

    };
#endif
  } // namespace

#if ENABLE_UAMQP
  std::ostream& operator<<(std::ostream& os, CONNECTION_STATE state)
  {
    auto val{UamqpConnectionStateToStringMap.find(state)};
    if (val == UamqpConnectionStateToStringMap.end())
    {
      os << "Unknown connection state: "
         << static_cast<std::underlying_type<decltype(state)>::type>(state);
    }
    else
    {
      os << val->second;
    }
    return os;
  }

  _internal::ConnectionState ConnectionStateFromCONNECTION_STATE(CONNECTION_STATE state)
  {
    auto val{UamqpToAmqpConnectionStateMap.find(state)};
    if (val == UamqpToAmqpConnectionStateMap.end())
    {
      throw std::runtime_error("Unknown connection state.");
    }
    return val->second;
  }

  void ConnectionImpl::OnConnectionStateChangedFn(
      void* context,
      CONNECTION_STATE newState,
      CONNECTION_STATE oldState)
  {
    ConnectionImpl* connection = static_cast<ConnectionImpl*>(context);

    if (connection->m_options.EnableTrace)
    {
      Log::Stream(Logger::Level::Verbose)
          << "Connection " << connection->m_containerId << " state changed from " << oldState
          << " to " << newState;
    }
    if (connection->m_eventHandler)
    {
      if (!connection->m_isClosing)
      {
        connection->m_eventHandler->OnConnectionStateChanged(
            ConnectionFactory::CreateFromInternal(connection->shared_from_this()),
            ConnectionStateFromCONNECTION_STATE(newState),
            ConnectionStateFromCONNECTION_STATE(oldState));
      }
    }
    if (newState == CONNECTION_STATE_ERROR || newState == CONNECTION_STATE_END)
    {
      // When the connection transitions into the error or end state, it is no longer pollable.
      if (connection->m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose)
            << "Connection " << connection->m_containerId << " state changed to " << newState;
      }
    }
    connection->SetState(ConnectionStateFromCONNECTION_STATE(newState));
  }

  bool ConnectionImpl::OnNewEndpointFn(void* context, ENDPOINT_HANDLE newEndpoint)
  {
    ConnectionImpl* cn = static_cast<ConnectionImpl*>(context);
    _internal::Endpoint endpoint(EndpointFactory::CreateEndpoint(newEndpoint));
    if (cn->m_endpointEvents)
    {
      return cn->m_endpointEvents->OnNewEndpoint(
          ConnectionFactory::CreateFromInternal(cn->shared_from_this()), endpoint);
    }
    return false;
  }

  void ConnectionImpl::OnIOErrorFn(void* context)
  {
    ConnectionImpl* cn = static_cast<ConnectionImpl*>(context);
    if (!cn->m_isClosing)
    {
      if (cn->m_eventHandler)
      {
        return cn->m_eventHandler->OnIOError(
            ConnectionFactory::CreateFromInternal(cn->shared_from_this()));
      }
    }
  }
#endif
  void ConnectionImpl::EnableAsyncOperation(bool enable)
  {
    m_enableAsyncOperation = enable;
    if (enable)
    {
      if (m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose)
            << "Try to enable async operation on connection: " << this << " ID: " << m_containerId
            << " count: " << m_openCount.load();
      }
      if (m_openCount++ == 0)
      {
        if (m_options.EnableTrace)
        {
          Log::Stream(Logger::Level::Verbose)
              << "Enabled async operation on connection: " << this << " ID: " << m_containerId;
        }
        Common::_detail::GlobalStateHolder::GlobalStateInstance()->AddPollable(shared_from_this());
      }
    }
    else
    {
      AZURE_ASSERT_MSG(m_openCount.load() > 0, "Closing async without opening it first.");
      if (m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose)
            << "Try to disable async operation on connection: " << this << " ID: " << m_containerId
            << " count: " << m_openCount.load();
      }
      if (--m_openCount == 0)
      {
        if (m_options.EnableTrace)
        {
          Log::Stream(Logger::Level::Verbose)
              << "Disabled async operation on connection: " << this << " ID: " << m_containerId;
        }
        Common::_detail::GlobalStateHolder::GlobalStateInstance()->RemovePollable(
            shared_from_this());
      }
    }
  }

  void ConnectionImpl::Open()
  {
    if (m_options.EnableTrace)
    {
      Log::Stream(Logger::Level::Verbose)
          << "ConnectionImpl::Open: " << this << " ID: " << m_containerId;
    }
#if ENABLE_UAMQP
    if (connection_open(m_connection.get()))
    {
      throw std::runtime_error("Could not open connection.");
    }
#endif
    m_connectionOpened = true;
    EnableAsyncOperation(true);
  }

  void ConnectionImpl::Listen()
  {
    Log::Stream(Logger::Level::Verbose)
        << "ConnectionImpl::Listen: " << this << " ID: " << m_containerId;
#if ENABLE_UAMQP
    if (connection_listen(m_connection.get()))
    {
      throw std::runtime_error("Could not listen on connection.");
    }
#endif
    m_connectionOpened = true;

    EnableAsyncOperation(true);
  }

  void ConnectionImpl::Close(
      const std::string& condition,
      const std::string& description,
      Models::AmqpValue info)
  {
    Log::Stream(Logger::Level::Verbose)
        << "ConnectionImpl::Close: " << this << " ID: " << m_containerId;
#if ENABLE_UAMQP
    if (!m_connection)
    {
      throw std::logic_error("Connection not opened.");
    }

    // Stop polling on this connection, we're shutting it down.
    EnableAsyncOperation(false);

    std::unique_lock<LockType> lock(m_amqpMutex);

    if (m_connectionOpened)
    {
      if (connection_close(
              m_connection.get(),
              (condition.empty() ? nullptr : condition.c_str()),
              (description.empty() ? nullptr : description.c_str()),
              Models::_detail::AmqpValueFactory::ToImplementation(info)))
      {
        throw std::runtime_error("Could not close connection.");
      }
    }
#else
    (void)condition;
    (void)description;
    (void)info;
#endif
    m_connectionOpened = false;
  }

  uint32_t ConnectionImpl::GetMaxFrameSize() const
  {
    uint32_t maxSize = {};
#if ENABLE_UAMQP
    if (connection_get_max_frame_size(m_connection.get(), &maxSize))
    {
      throw std::runtime_error("Could not get max frame size.");
    }
#endif
    return maxSize;
  }

  uint16_t ConnectionImpl::GetMaxChannel() const
  {
    uint16_t maxChannel = {};
#if ENABLE_UAMQP
    if (connection_get_channel_max(m_connection.get(), &maxChannel))
    {
      throw std::runtime_error("Could not get channel max.");
    }
#endif
    return maxChannel;
  }

  std::chrono::milliseconds ConnectionImpl::GetIdleTimeout() const
  {
#if ENABLE_UAMQP
    milliseconds ms;
    if (connection_get_idle_timeout(m_connection.get(), &ms))
    {
      throw std::runtime_error("Could not set max frame size.");
    }
    return std::chrono::milliseconds(ms);
#else
    return {};
#endif
  }

  Models::AmqpMap ConnectionImpl::GetProperties() const
  {
#if ENABLE_UAMQP
    AMQP_VALUE value;
    if (connection_get_properties(m_connection.get(), &value))
    {
      throw std::runtime_error("Could not get properties.");
    }
    return Models::_detail::AmqpValueFactory::FromImplementation(
               Models::_detail::UniqueAmqpValueHandle{value})
        .AsMap();
#else
    return {};
#endif
  }

  uint32_t ConnectionImpl::GetRemoteMaxFrameSize() const
  {
    uint32_t maxFrameSize = {};
#if ENABLE_UAMQP
    if (connection_get_remote_max_frame_size(m_connection.get(), &maxFrameSize))
    {
      throw std::runtime_error("Could not get remote max frame size.");
    }
#endif
    return maxFrameSize;
  }

  void ConnectionImpl::SetIdleEmptyFrameSendPercentage(double ratio)
  {
    std::unique_lock<LockType> lock(m_amqpMutex);
#if ENABLE_UAMQP
    if (connection_set_remote_idle_timeout_empty_frame_send_ratio(m_connection.get(), ratio))
    {
      throw std::runtime_error("Could not set remote idle timeout send frame ratio.");
    }
#else
    (void)ratio;
#endif
  }

  bool ConnectionImpl::IsSasCredential() const
  {
    if (GetCredential())
    {
      return GetCredential()->GetCredentialName() == "ServiceBusSasConnectionStringCredential";
    }
    return false;
  }

  // Ensure that we have a token for the provided audience.
  // If we don't, authenticate the audience with the service using the provided session.
  // Note that the granularity of
  Credentials::AccessToken ConnectionImpl::AuthenticateAudience(
      std::shared_ptr<SessionImpl> session,
      std::string const& audience,
      Azure::Core::Context const& context)
  {
    if (GetCredential())
    {
      std::string audienceUrl = audience;
      if (m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose) << "Authenticate connection for audience " << audience;
      }
      // If the audience looks like a URL for AMQP, AMQPS, or SB, we can use the URL as provided.
      if ((audience.find("amqps://") != 0) && (audience.find("amqp://") != 0)
          && (audience.find("sb://") != 0))
      {
        audienceUrl = "amqps://" + GetHost();
        // The provided audience may begin with a /, if not, we need to add the separator.
        if (audience.front() != '/')
        {
          audienceUrl += "/";
        }
        audienceUrl += audience;
        if (m_options.EnableTrace)
        {
          Log::Stream(Logger::Level::Verbose)
              << "Initial audience is not URL, using " << audienceUrl;
        }
      }

      std::unique_lock<std::mutex> lock(m_tokenMutex);
      // If we have authenticated this audience, we're done and can return success.
      auto token = m_tokenStore.find(audienceUrl);
      if (token != m_tokenStore.end())
      {
        if (m_options.EnableTrace)
        {
          Log::Stream(Logger::Level::Verbose) << "Using cached token for " << audienceUrl;
        }
        return token->second;
      }
      // We've not authenticated this audience.
      // Authenticate it with the server

      if (m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose)
            << "No cached token for " << audienceUrl << ", Authenticating.";
      }

      auto claimsBasedSecurity = std::make_shared<ClaimsBasedSecurityImpl>(session);
      auto cbsOpenStatus = claimsBasedSecurity->Open(context);
      if (cbsOpenStatus != CbsOpenResult::Ok)
      {
        throw std::runtime_error("Could not open Claims Based Security object.");
      }

      try
      {
        Credentials::TokenRequestContext requestContext;

        requestContext.Scopes = m_options.AuthenticationScopes;
        auto accessToken{GetCredential()->GetToken(requestContext, context)};

        auto result = claimsBasedSecurity->PutToken(
            (IsSasCredential() ? CbsTokenType::Sas : CbsTokenType::Jwt),
            audienceUrl,
            accessToken.Token,
            context);
        if (std::get<0>(result) != CbsOperationResult::Ok)
        {
          throw Azure::Core::Credentials::AuthenticationException(
              "Could not authenticate client. Error Status: " + std::to_string(std::get<1>(result))
              + " reason: " + std::get<2>(result));
        }
        Log::Stream(Logger::Level::Verbose) << "Close CBS object";
        claimsBasedSecurity->Close(context);
        if (m_options.EnableTrace)
        {
          Log::Stream(Logger::Level::Verbose)
              << "Authenticated connection for audience " << audienceUrl << " successfully.";
        }

        m_tokenStore.emplace(audienceUrl, accessToken);
        return accessToken;
      }
      catch (...)
      {
        // Ensure that the claims based security object is closed before we leave this scope.
        claimsBasedSecurity->Close(context);
        throw;
      }
    }
    else
    {
      Log::Stream(Logger::Level::Verbose) << "No credential, returning empty token.";
      // If the connection is unauthenticated, then just return an empty access token.
      return {};
    }
  }
}}}} // namespace Azure::Core::Amqp::_detail
