// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/connection.hpp"
#include "../network/private/transport_impl.hpp"
#include "azure/core/amqp/common/global_state.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"
#include "azure/core/amqp/network/socket_transport.hpp"
#include "azure/core/amqp/network/tls_transport.hpp"
#include "private/connection_impl.hpp"
#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/url.hpp>
#include <azure/core/uuid.hpp>
#include <azure_uamqp_c/connection.h>
#include <memory>

void Azure::Core::_internal::UniqueHandleHelper<CONNECTION_INSTANCE_TAG>::FreeAmqpConnection(
    CONNECTION_HANDLE value)
{
  connection_destroy(value);
}

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

namespace Azure { namespace Core { namespace Amqp { namespace _internal {

  // Create a connection with an existing networking Transport.
  Connection::Connection(
      std::shared_ptr<Network::_internal::Transport> transport,
      ConnectionOptions const& options,
      ConnectionEvents* eventHandler)
      : m_impl{std::make_shared<Azure::Core::Amqp::_detail::ConnectionImpl>(
          transport->GetImpl(),
          options,
          eventHandler)}
  {
    m_impl->FinishConstruction();
  }

  // Create a connection with a request URI and options.
  Connection::Connection(
      std::string const& requestUri,
      ConnectionOptions const& options,
      ConnectionEvents* eventHandler)
      : m_impl{std::make_shared<Azure::Core::Amqp::_detail::ConnectionImpl>(
          requestUri,
          options,
          eventHandler)}
  {
    m_impl->FinishConstruction();
  }

  Connection::~Connection() {}

  void Connection::Poll() const { m_impl->Poll(); }

  void Connection::Listen() { m_impl->Listen(); }
  void Connection::Open() { m_impl->Open(); }
  void Connection::Close(
      std::string const& condition,
      std::string const& description,
      Azure::Core::Amqp::Models::AmqpValue value)
  {
    m_impl->Close(condition, description, value);
  }
  uint32_t Connection::GetMaxFrameSize() const { return m_impl->GetMaxFrameSize(); }
  uint32_t Connection::GetRemoteMaxFrameSize() const { return m_impl->GetRemoteMaxFrameSize(); }
  uint16_t Connection::GetMaxChannel() const { return m_impl->GetMaxChannel(); }
  std::chrono::milliseconds Connection::GetIdleTimeout() const { return m_impl->GetIdleTimeout(); }
  Azure::Core::Amqp::Models::AmqpMap Connection::GetProperties() const
  {
    return m_impl->GetProperties();
  }
  void Connection::SetIdleEmptyFrameSendPercentage(double ratio)
  {
    m_impl->SetIdleEmptyFrameSendPercentage(ratio);
  }
}}}} // namespace Azure::Core::Amqp::_internal

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
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

  // Create a connection with an existing networking Transport.
  ConnectionImpl::ConnectionImpl(
      std::shared_ptr<Network::_detail::TransportImpl> transport,
      _internal::ConnectionOptions const& options,
      _internal::ConnectionEvents* eventHandler)
      : m_hostName{"localhost"}, m_options{options}, m_eventHandler{eventHandler}
  {
    if (options.SaslCredentials)
    {
      throw std::runtime_error("Sasl Credentials should not be provided with a transport.");
    }
    EnsureGlobalStateInitialized();
    m_transport = transport;
  }

  // Create a connection with a request URI and options.
  ConnectionImpl::ConnectionImpl(
      std::string const& requestUri,
      _internal::ConnectionOptions const& options,
      _internal::ConnectionEvents* eventHandler)
      : m_options{options}, m_eventHandler{eventHandler}
  {
    EnsureGlobalStateInitialized();

    if (options.SaslCredentials)
    {
      throw std::runtime_error("Sasl Credentials should not be provided with a request URI.");
    }
    Azure::Core::Url requestUrl(requestUri);
    std::shared_ptr<Network::_internal::Transport> requestTransport;
    if (requestUrl.GetScheme() == "amqp")
    {
      Log::Write(Logger::Level::Informational, "Creating socket connection transport.");
      Azure::Core::Amqp::Network::_internal::SocketTransport transport{
          requestUrl.GetHost(),
          requestUrl.GetPort() ? requestUrl.GetPort() : static_cast<std::uint16_t>(AmqpPort)};
      m_transport = transport.GetImpl();
    }
    else if (requestUrl.GetScheme() == "amqps")
    {
      Log::Write(Logger::Level::Informational, "Creating TLS socket connection transport.");
      Azure::Core::Amqp::Network::_internal::TlsTransport transport{
          requestUrl.GetHost(),
          requestUrl.GetPort() ? requestUrl.GetPort() : static_cast<std::uint16_t>(AmqpsPort)};
      m_transport = transport.GetImpl();
    }
    else
    {
      throw std::runtime_error("Unknown connection scheme: " + requestUrl.GetScheme() + ".");
    }
    m_hostName = requestUrl.GetHost();
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
        OnIoErrorFn,
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
            static_cast<Azure::Core::Amqp::Models::_detail::UniqueAmqpValueHandle>(
                m_options.Properties)
                .get()))
    {
      throw std::runtime_error("Failed to set connection properties.");
    }
  }

  void ConnectionImpl::Poll() const
  {
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
          connection->shared_from_this(),
          ConnectionStateFromCONNECTION_STATE(newState),
          ConnectionStateFromCONNECTION_STATE(oldState));
    }
  }

  bool ConnectionImpl::OnNewEndpointFn(void* context, ENDPOINT_HANDLE newEndpoint)
  {
    ConnectionImpl* cn = static_cast<ConnectionImpl*>(context);
    _internal::Endpoint endpoint(newEndpoint);
    if (cn->m_eventHandler)
    {
      return cn->m_eventHandler->OnNewEndpoint(cn->shared_from_this(), endpoint);
    }
    return false; // LCOV_EXCL_LINE
  }

  void ConnectionImpl::OnIoErrorFn(void* context) // LCOV_EXCL_LINE
  { // LCOV_EXCL_LINE
    ConnectionImpl* cn = static_cast<ConnectionImpl*>(context); // LCOV_EXCL_LINE
    if (cn->m_eventHandler) // LCOV_EXCL_LINE
    { // LCOV_EXCL_LINE
      return cn->m_eventHandler->OnIoError(cn->shared_from_this()); // LCOV_EXCL_LINE
    } // LCOV_EXCL_LINE
  } // LCOV_EXCL_LINE

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
      Azure::Core::Amqp::Models::AmqpValue info)
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

  Azure::Core::Amqp::Models::AmqpMap ConnectionImpl::GetProperties() const
  {
    AMQP_VALUE value;
    if (connection_get_properties(m_connection.get(), &value))
    {
      throw std::runtime_error("COuld not get properties."); // LCOV_EXCL_LINE
    }
    return Azure::Core::Amqp::Models::AmqpValue{value}.AsMap();
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

}}}} // namespace Azure::Core::Amqp::_detail
