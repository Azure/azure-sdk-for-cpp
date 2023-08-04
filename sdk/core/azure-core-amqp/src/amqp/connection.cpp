// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/connection.hpp"

#include "../network/private/transport_impl.hpp"
#include "azure/core/amqp/common/global_state.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"
#include "azure/core/amqp/network/socket_transport.hpp"
#include "azure/core/amqp/network/tls_transport.hpp"
#include "private/claims_based_security_impl.hpp"
#include "private/connection_impl.hpp"
#include "private/session_impl.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/url.hpp>
#include <azure/core/uuid.hpp>

#include <azure_uamqp_c/connection.h>

#include <memory>

namespace Azure { namespace Core { namespace _internal {
  void UniqueHandleHelper<CONNECTION_INSTANCE_TAG>::FreeAmqpConnection(CONNECTION_HANDLE value)
  {
    connection_destroy(value);
  }

}}} // namespace Azure::Core::_internal

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

namespace Azure { namespace Core { namespace Amqp { namespace _internal {

  // Create a connection with an existing networking Transport.
  Connection::Connection(
      Network::_internal::Transport const& transport,
      ConnectionOptions const& options,
      ConnectionEvents* eventHandler)
      : m_impl{
          std::make_shared<_detail::ConnectionImpl>(transport.GetImpl(), options, eventHandler)}
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
      _internal::ConnectionEvents* eventHandler)
      : m_hostName{"localhost"}, m_options{options}, m_eventHandler{eventHandler}
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
    // If the connection is going away, we don't want to generate any more events on it.
    if (m_eventHandler)
    {
      m_eventHandler = nullptr;
    }
  }

  void ConnectionImpl::FinishConstruction()
  {
    std::string containerId{m_options.ContainerId};
    if (containerId.empty())
    {
      containerId = Azure::Core::Uuid::CreateUuid().ToString();
    }

    m_connection.reset(connection_create2(
        *m_transport,
        m_hostName.c_str(),
        containerId.c_str(),
        OnNewEndpointFn,
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
            static_cast<Models::_detail::UniqueAmqpValueHandle>(m_options.Properties).get()))
    {
      throw std::runtime_error("Failed to set connection properties.");
    }
  }

  void ConnectionImpl::Poll()
  {
    if (m_connectionState == _internal::ConnectionState::Error
        || m_connectionState == _internal::ConnectionState::End)
    {
      throw std::runtime_error("Connection cannot be polled in the current state.");
    }
    if (m_connection)
    {
      connection_dowork(m_connection.get());
    }
  }

  namespace {

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

    if (connection->m_eventHandler)
    {
      connection->m_eventHandler->OnConnectionStateChanged(
          ConnectionFactory::CreateFromInternal(connection->shared_from_this()),
          ConnectionStateFromCONNECTION_STATE(newState),
          ConnectionStateFromCONNECTION_STATE(oldState));
    }
    connection->SetState(ConnectionStateFromCONNECTION_STATE(newState));
  }

  bool ConnectionImpl::OnNewEndpointFn(void* context, ENDPOINT_HANDLE newEndpoint)
  {
    ConnectionImpl* cn = static_cast<ConnectionImpl*>(context);
    _internal::Endpoint endpoint(EndpointFactory::CreateEndpoint(newEndpoint));
    if (cn->m_eventHandler)
    {
      return cn->m_eventHandler->OnNewEndpoint(
          ConnectionFactory::CreateFromInternal(cn->shared_from_this()), endpoint);
    }
    return false; // LCOV_EXCL_LINE
  }

  // LCOV_EXCL_START
  void ConnectionImpl::OnIOErrorFn(void* context)
  {
    ConnectionImpl* cn = static_cast<ConnectionImpl*>(context);
    if (cn->m_eventHandler)
    {
      return cn->m_eventHandler->OnIOError(
          ConnectionFactory::CreateFromInternal(cn->shared_from_this()));
    }
  }
  // LCOV_EXCL_STOP

  void ConnectionImpl::Open()
  {
    if (connection_open(m_connection.get()))
    {
      throw std::runtime_error("Could not open connection."); // LCOV_EXCL_LINE
    }
  }

  void ConnectionImpl::Listen()
  {
    if (connection_listen(m_connection.get()))
    {
      throw std::runtime_error("Could not listen on connection."); // LCOV_EXCL_LINE
    }
  }

  void ConnectionImpl::Close(
      const std::string& condition,
      const std::string& description,
      Models::AmqpValue info)
  {
    if (!m_connection)
    {
      throw std::logic_error("Connection already closed."); // LCOV_EXCL_LINE
    }

    if (connection_close(
            m_connection.get(),
            (condition.empty() ? nullptr : condition.c_str()),
            (description.empty() ? nullptr : description.c_str()),
            info))
    {
      throw std::runtime_error("Could not close connection.");
    }
  }

  uint32_t ConnectionImpl::GetMaxFrameSize() const
  {
    uint32_t maxSize;
    if (connection_get_max_frame_size(m_connection.get(), &maxSize))
    {
      throw std::runtime_error("COuld not get max frame size."); // LCOV_EXCL_LINE
    }
    return maxSize;
  }

  uint16_t ConnectionImpl::GetMaxChannel() const
  {
    uint16_t maxChannel;
    if (connection_get_channel_max(m_connection.get(), &maxChannel))
    {
      throw std::runtime_error("COuld not get channel max."); // LCOV_EXCL_LINE
    }
    return maxChannel;
  }

  std::chrono::milliseconds ConnectionImpl::GetIdleTimeout() const
  {
    milliseconds ms;

    if (connection_get_idle_timeout(m_connection.get(), &ms))
    {
      throw std::runtime_error("COuld not set max frame size."); // LCOV_EXCL_LINE
    }
    return std::chrono::milliseconds(ms);
  }

  Models::AmqpMap ConnectionImpl::GetProperties() const
  {
    AMQP_VALUE value;
    if (connection_get_properties(m_connection.get(), &value))
    {
      throw std::runtime_error("COuld not get properties."); // LCOV_EXCL_LINE
    }
    return Models::AmqpValue{value}.AsMap();
  }

  uint32_t ConnectionImpl::GetRemoteMaxFrameSize() const
  {
    uint32_t maxFrameSize;
    if (connection_get_remote_max_frame_size(m_connection.get(), &maxFrameSize))
    {
      throw std::runtime_error("Could not get remote max frame size."); // LCOV_EXCL_LINE
    }
    return maxFrameSize;
  }

  void ConnectionImpl::SetIdleEmptyFrameSendPercentage(double ratio)
  {
    if (connection_set_remote_idle_timeout_empty_frame_send_ratio(m_connection.get(), ratio))
    {
      throw std::runtime_error(
          "Could not set remote idle timeout send frame ratio."); // LCOV_EXCL_LINE
    }
  }

  bool ConnectionImpl::IsSasCredential() const
  {
    if (GetCredential())
    {
      return GetCredential()->GetCredentialName() == "ServiceBusSasConnectionStringCredential";
    }
    return false;
  }

  std::string ConnectionImpl::GetSecurityToken(
      std::string const& audience,
      Azure::Core::Context const& context) const
  {
    if (GetCredential())
    {
      if (m_tokenStore.find(audience) == m_tokenStore.end())
      {
        Credentials::TokenRequestContext requestContext;
        bool isSasToken = IsSasCredential();
        if (isSasToken)
        {
          requestContext.MinimumExpiration = std::chrono::minutes(60);
        }
        requestContext.Scopes = m_options.AuthenticationScopes;
        return GetCredential()->GetToken(requestContext, context).Token;
      }
      return m_tokenStore.at(audience).Token;
    }
    return "";
  }

}}}} // namespace Azure::Core::Amqp::_detail
