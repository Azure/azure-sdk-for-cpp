// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/connection.hpp"
#include "azure/core/amqp/common/global_state.hpp"
#include "azure/core/amqp/network/private/transport_impl.hpp"
#include "azure/core/amqp/network/socket_transport.hpp"
#include "azure/core/amqp/network/tls_transport.hpp"
#include "azure/core/amqp/private/connection_impl.hpp"
#include <azure/core/url.hpp>
#include <azure/core/uuid.hpp>
#include <memory>

#include <azure_uamqp_c/connection.h>

namespace Azure { namespace Core { namespace _internal { namespace Amqp {

  // Create a connection with an existing networking Transport.
  Connection::Connection(
      std::shared_ptr<Network::Transport> transport,
      ConnectionEvents* eventHandler,
      ConnectionOptions const& options)
      : m_impl{std::make_shared<_detail::ConnectionImpl>(transport, eventHandler, options)}
  {
    m_impl->FinishConstruction();
  }

  // Create a connection with a request URI and options.
  Connection::Connection(
      std::string const& requestUri,
      ConnectionEvents* eventHandler,
      ConnectionOptions const& options)
      : m_impl{std::make_shared<_detail::ConnectionImpl>(requestUri, eventHandler, options)}
  {
    m_impl->FinishConstruction();
  }

  // Connection::Connection(ConnectionEvents* eventHandler, ConnectionOptions const& options)
  //     : m_impl{std::make_shared<_detail::ConnectionImpl>(eventHandler, options)}
  //{
  //   m_impl->FinishConstruction();
  // }

  Connection::~Connection() {}

  void Connection::Poll() const { m_impl->Poll(); }

  void Connection::Listen() { m_impl->Listen(); }
  void Connection::SetTrace(bool enableTrace) { m_impl->SetTrace(enableTrace); }
  void Connection::Open() { m_impl->Open(); }
  void Connection::Close(
      std::string const& condition,
      std::string const& description,
      Azure::Core::Amqp::Models::Value value)
  {
    m_impl->Close(condition, description, value);
  }
  uint32_t Connection::GetMaxFrameSize() const { return m_impl->GetMaxFrameSize(); }
  void Connection::SetMaxFrameSize(uint32_t maxFrameSize) { m_impl->SetMaxFrameSize(maxFrameSize); }
  uint32_t Connection::GetRemoteMaxFrameSize() const { return m_impl->GetRemoteMaxFrameSize(); }
  uint16_t Connection::GetMaxChannel() const { return m_impl->GetMaxChannel(); }
  void Connection::SetMaxChannel(uint16_t channel) { m_impl->SetMaxChannel(channel); }
  std::chrono::milliseconds Connection::GetIdleTimeout() const { return m_impl->GetIdleTimeout(); }
  void Connection::SetIdleTimeout(std::chrono::milliseconds timeout)
  {
    m_impl->SetIdleTimeout(timeout);
  }
  void Connection::SetRemoteIdleTimeoutEmptyFrameSendRatio(double idleTimeoutEmptyFrameSendRatio)
  {
    return m_impl->SetRemoteIdleTimeoutEmptyFrameSendRatio(idleTimeoutEmptyFrameSendRatio);
  }

  void Connection::SetProperties(Azure::Core::Amqp::Models::Value properties)
  {
    m_impl->SetProperties(properties);
  }
  Azure::Core::Amqp::Models::Value Connection::GetProperties() const
  {
    return m_impl->GetProperties();
  }

  namespace _detail {

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
        std::shared_ptr<Network::Transport> transport,
        ConnectionEvents* eventHandler,
        ConnectionOptions const& options)
        : m_hostName{options.HostName}, m_options{options}, m_eventHandler{eventHandler}
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
        ConnectionEvents* eventHandler,
        ConnectionOptions const& options)
        : m_options{options}, m_eventHandler{eventHandler}
    {
      EnsureGlobalStateInitialized();

      if (options.SaslCredentials)
      {
        throw std::runtime_error("Sasl Credentials should not be provided with a request URI.");
      }
      Azure::Core::Url requestUrl(requestUri);
      std::shared_ptr<Network::Transport> requestTransport;
      if (requestUrl.GetScheme() == "amqp")
      {
        m_transport = std::make_shared<Azure::Core::_internal::Amqp::Network::SocketTransport>(
            requestUrl.GetHost(), requestUrl.GetPort() ? requestUrl.GetPort() : 5672);
      }
      else if (requestUrl.GetScheme() == "amqps")
      {
        m_transport = std::make_shared<Azure::Core::_internal::Amqp::Network::TlsTransport>(
            requestUrl.GetHost(), requestUrl.GetPort() ? requestUrl.GetPort() : 5671);
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
      if (m_connection)
      {
        connection_destroy(m_connection);
        m_connection = nullptr;
      }
    }

    void ConnectionImpl::FinishConstruction()
    {
      std::string containerId{m_options.ContainerId};
      if (containerId.empty())
      {
        containerId = Azure::Core::Uuid::CreateUuid().ToString();
      }

      m_connection = connection_create2(
          *m_transport->GetImpl(),
          m_hostName.c_str(),
          containerId.c_str(),
          OnNewEndpointFn,
          this,
          OnConnectionStateChangedFn,
          this,
          OnIoErrorFn,
          this);
      SetTrace(m_options.EnableTrace);
      //    SetIdleTimeout(options.IdleTimeout);

      //    SetMaxFrameSize(options.MaxFrameSize);
      //    SetMaxChannel(options.MaxSessions);
      //    SetProperties(options.Properties);

      //    std::string SASLType; // Not a string - fill in later.
      //    std::chrono::seconds Timeout{0};
    }

    void ConnectionImpl::Poll() const
    {
      if (m_transport)
      {
        //      xio_dowork(*m_transport);
        connection_dowork(m_connection);
      }
    }

    ConnectionState ConnectionStateFromCONNECTION_STATE(CONNECTION_STATE state)
    {
      switch (state)
      {
        case CONNECTION_STATE_START:
          return ConnectionState::Start;
        case CONNECTION_STATE_CLOSE_PIPE:
          return ConnectionState::ClosePipe;
        case CONNECTION_STATE_CLOSE_RCVD:
          return ConnectionState::CloseReceived;
        case CONNECTION_STATE_END:
          return ConnectionState::End;
        case CONNECTION_STATE_HDR_RCVD:
          return ConnectionState::HeaderReceived;
        case CONNECTION_STATE_HDR_SENT:
          return ConnectionState::HeaderSent;
        case CONNECTION_STATE_HDR_EXCH:
          return ConnectionState::HeaderExchanged;
        case CONNECTION_STATE_OPEN_PIPE:
          return ConnectionState::OpenPipe;
        case CONNECTION_STATE_OC_PIPE:
          return ConnectionState::OcPipe;
        case CONNECTION_STATE_OPEN_RCVD:
          return ConnectionState::OpenReceived;
        case CONNECTION_STATE_OPEN_SENT:
          return ConnectionState::OpenSent;
        case CONNECTION_STATE_OPENED:
          return ConnectionState::Opened;
        case CONNECTION_STATE_CLOSE_SENT:
          return ConnectionState::CloseSent;
        case CONNECTION_STATE_DISCARDING:
          return ConnectionState::Discarding;
        case CONNECTION_STATE_ERROR:
          return ConnectionState::Error;
        default:
          throw std::logic_error("Unknown connection state");
      }
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
      Endpoint endpoint(newEndpoint);
      if (cn->m_eventHandler)
      {
        return cn->m_eventHandler->OnNewEndpoint(cn->shared_from_this(), endpoint);
      }
      return false;
    }

    void ConnectionImpl::OnIoErrorFn(void* context)
    {
      ConnectionImpl* cn = static_cast<ConnectionImpl*>(context);
      if (cn->m_eventHandler)
      {
        return cn->m_eventHandler->OnIoError(cn->shared_from_this());
      }
    }
    void ConnectionImpl::Open()
    {
      if (connection_open(m_connection))
      {
        throw std::runtime_error("Could not open connection.");
      }
    }

    void ConnectionImpl::Listen()
    {
      if (connection_listen(m_connection))
      {
        throw std::runtime_error("Could not listen on connection.");
      }
    }

    void ConnectionImpl::SetTrace(bool setTrace) { connection_set_trace(m_connection, setTrace); }

    void ConnectionImpl::Close(
        const std::string& condition,
        const std::string& description,
        Azure::Core::Amqp::Models::Value info)
    {
      if (!m_connection)
      {
        throw std::logic_error("Connection already closed.");
      }

      if (connection_close(
              m_connection,
              (condition.empty() ? nullptr : condition.c_str()),
              (description.empty() ? nullptr : description.c_str()),
              info))
      {
        throw std::runtime_error("Could not close connection.");
      }
    }

    void ConnectionImpl::SetMaxFrameSize(uint32_t maxSize)
    {
      if (connection_set_max_frame_size(m_connection, maxSize))
      {
        throw std::runtime_error("COuld not set max frame size.");
      }
    }
    uint32_t ConnectionImpl::GetMaxFrameSize() const
    {
      uint32_t maxSize;
      if (connection_get_max_frame_size(m_connection, &maxSize))
      {
        throw std::runtime_error("COuld not set max frame size.");
      }
      return maxSize;
    }

    void ConnectionImpl::SetMaxChannel(uint16_t maxChannel)
    {
      if (connection_set_channel_max(m_connection, maxChannel))
      {
        throw std::runtime_error("COuld not set max frame size.");
      }
    }
    uint16_t ConnectionImpl::GetMaxChannel() const
    {
      uint16_t maxChannel;
      if (connection_get_channel_max(m_connection, &maxChannel))
      {
        throw std::runtime_error("COuld not set max frame size.");
      }
      return maxChannel;
    }

    void ConnectionImpl::SetIdleTimeout(std::chrono::milliseconds idleTimeout)
    {
      if (connection_set_idle_timeout(m_connection, static_cast<milliseconds>(idleTimeout.count())))
      {
        throw std::runtime_error("COuld not set idle timeout.");
      }
    }
    std::chrono::milliseconds ConnectionImpl::GetIdleTimeout() const
    {
      milliseconds ms;

      if (connection_get_idle_timeout(m_connection, &ms))
      {
        throw std::runtime_error("COuld not set max frame size.");
      }
      return std::chrono::milliseconds(ms);
    }

    void ConnectionImpl::SetProperties(Azure::Core::Amqp::Models::Value value)
    {
      if (connection_set_properties(m_connection, value))
      {
        throw std::runtime_error("COuld not set idle timeout.");
      }
    }
    Azure::Core::Amqp::Models::Value ConnectionImpl::GetProperties() const
    {
      AMQP_VALUE value;
      if (connection_get_properties(m_connection, &value))
      {
        throw std::runtime_error("COuld not get properties.");
      }
      return value;
    }

    uint32_t ConnectionImpl::GetRemoteMaxFrameSize() const
    {
      uint32_t maxFrameSize;
      if (connection_get_remote_max_frame_size(m_connection, &maxFrameSize))
      {
        throw std::runtime_error("Could not get remote max frame size.");
      }
      return maxFrameSize;
    }
    void ConnectionImpl::SetRemoteIdleTimeoutEmptyFrameSendRatio(double ratio)
    {
      if (connection_set_remote_idle_timeout_empty_frame_send_ratio(m_connection, ratio))
      {
        throw std::runtime_error("Could not set remote idle timeout.");
      }
    }

  } // namespace _detail
}}}} // namespace Azure::Core::_internal::Amqp
