#include "azure/core/amqp/connection.hpp"
#include "azure/core/amqp/session.hpp"
#include "azure/core/amqp/common/global_state.hpp"
#include "azure/core/amqp/network/socket_transport.hpp"
#include "azure/core/amqp/network/tls_transport.hpp"
#include <azure/core/url.hpp>
#include <azure/core/uuid.hpp>
#include <memory>

#include <azure_uamqp_c/connection.h>

namespace Azure { namespace Core { namespace _internal { namespace Amqp {

  namespace {
    void EnsureGlobalStateInitialized()
    {
      // Force the global instance to exist. This is required to ensure that uAMQP and
      // azure-c-shared-utility is
      auto globalInstance = Common::_detail::GlobalState::GlobalStateInstance();
      (void)globalInstance;
    }
  } // namespace

  // Create a conection with an existing networking Transport.
  Connection::Connection(
      std::shared_ptr<Network::Transport> transport,
      ConnectionEvents* eventHandler,
      ConnectionOptions const& options)
      : m_eventHandler{eventHandler}
  {
    if (options.SaslCredentials)
    {
      throw std::runtime_error("Sasl Credentials should not be provided with a transport.");
    }
    EnsureGlobalStateInitialized();
    m_transport = transport;
    CreateUnderlyingConnection(options.HostName, options);
  }

  // Create a connection with a request URI and options.
  Connection::Connection(
      std::string const& requestUri,
      ConnectionEvents* eventHandler,
      ConnectionOptions const& options)
      : m_eventHandler{eventHandler}
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
      m_transport = std::make_shared<Network::SocketTransport>(
          requestUrl.GetHost(), requestUrl.GetPort() ? requestUrl.GetPort() : 5672);
    }
    else if (requestUrl.GetScheme() == "amqps")
    {
      m_transport = std::make_shared<Network::TlsTransport>(
          requestUrl.GetHost(), requestUrl.GetPort() ? requestUrl.GetPort() : 5671);
    }
    CreateUnderlyingConnection(requestUrl.GetHost(), options);
  }

  Connection::Connection(ConnectionEvents* eventHandler, ConnectionOptions const& options)
      : m_eventHandler{eventHandler}
  {
    EnsureGlobalStateInitialized();

    if (options.SaslCredentials)
    {
      m_transport = options.SaslCredentials->GetTransport();
      m_credential = options.SaslCredentials;
    }
    else
    {
      m_transport = std::make_shared<Network::TlsTransport>(
          options.HostName, options.Port);
    }
    CreateUnderlyingConnection(options.HostName, options);
  }
  Connection::~Connection()
  {
    if (m_connection)
    {
      connection_destroy(m_connection);
      m_connection = nullptr;
    }
  }

  void Connection::CreateUnderlyingConnection(
      std::string const& hostName,
      ConnectionOptions const& options)
  {
    std::string containerId{options.ContainerId};
    if (containerId.empty())
    {
      containerId = Azure::Core::Uuid::CreateUuid().ToString();
    }

    m_connection = connection_create2(
        *m_transport,
        hostName.c_str(),
        containerId.c_str(),
        OnNewEndpointFn,
        this,
        OnConnectionStateChangedFn,
        this,
        OnIoErrorFn,
        this);
    SetTrace(options.EnableTrace);
    //    SetIdleTimeout(options.IdleTimeout);

    //    SetMaxFrameSize(options.MaxFrameSize);
    //    SetMaxChannel(options.MaxSessions);
    //    SetProperties(options.Properties);

    //    std::string SASLType; // Not a string - fill in later.
    //    std::chrono::seconds Timeout{0};
  }

  void Connection::Poll() const
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

  void Connection::OnConnectionStateChangedFn(
      void* context,
      CONNECTION_STATE newState,
      CONNECTION_STATE oldState)
  {
    Connection* connection = static_cast<Connection*>(context);

    if (connection->m_eventHandler)
    {
      connection->m_eventHandler->OnConnectionStateChanged(
          *connection,
          ConnectionStateFromCONNECTION_STATE(newState),
          ConnectionStateFromCONNECTION_STATE(oldState));
    }
  }

  bool Connection::OnNewEndpointFn(void* context, ENDPOINT_HANDLE newEndpoint)
  {
    Connection* cn = static_cast<Connection*>(context);
    Endpoint endpoint(newEndpoint);
    if (cn->m_eventHandler)
    {
      return cn->m_eventHandler->OnNewEndpoint(*cn, endpoint);
    }
    return false;
  }

  void Connection::OnIoErrorFn(void* context)
  {
    Connection* cn = static_cast<Connection*>(context);
    if (cn->m_eventHandler)
    {
      return cn->m_eventHandler->OnIoError(*cn);
    }
  }
  void Connection::Open()
  {
    if (connection_open(m_connection))
    {
      throw std::runtime_error("Could not open connection.");
    }
  }

  void Connection::Listen()
  {
    if (connection_listen(m_connection))
    {
      throw std::runtime_error("Could not listen on connection.");
    }
  }

  void Connection::SetTrace(bool setTrace) { connection_set_trace(m_connection, setTrace); }

  void Connection::Close(
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

  void Connection::SetMaxFrameSize(uint32_t maxSize)
  {
    if (connection_set_max_frame_size(m_connection, maxSize))
    {
      throw std::runtime_error("COuld not set max frame size.");
    }
  }
  uint32_t Connection::GetMaxFrameSize()
  {
    uint32_t maxSize;
    if (connection_get_max_frame_size(m_connection, &maxSize))
    {
      throw std::runtime_error("COuld not set max frame size.");
    }
    return maxSize;
  }

  void Connection::SetMaxChannel(uint16_t maxChannel)
  {
    if (connection_set_channel_max(m_connection, maxChannel))
    {
      throw std::runtime_error("COuld not set max frame size.");
    }
  }
  uint16_t Connection::GetMaxChannel()
  {
    uint16_t maxChannel;
    if (connection_get_channel_max(m_connection, &maxChannel))
    {
      throw std::runtime_error("COuld not set max frame size.");
    }
    return maxChannel;
  }

  void Connection::SetIdleTimeout(std::chrono::milliseconds idleTimeout)
  {
    if (connection_set_idle_timeout(m_connection, static_cast<milliseconds>(idleTimeout.count())))
    {
      throw std::runtime_error("COuld not set idle timeout.");
    }
  }
  std::chrono::milliseconds Connection::GetIdleTimeout()
  {
    milliseconds ms;

    if (connection_get_idle_timeout(m_connection, &ms))
    {
      throw std::runtime_error("COuld not set max frame size.");
    }
    return std::chrono::milliseconds(ms);
  }

  void Connection::SetProperties(Azure::Core::Amqp::Models::Value value)
  {
    if (connection_set_properties(m_connection, value))
    {
      throw std::runtime_error("COuld not set idle timeout.");
    }
  }
  Azure::Core::Amqp::Models::Value Connection::GetProperties()
  {
    AMQP_VALUE value;
    if (connection_get_properties(m_connection, &value))
    {
      throw std::runtime_error("COuld not get properties.");
    }
    return value;
  }

  uint32_t Connection::GetRemoteMaxFrameSize()
  {
    uint32_t maxFrameSize;
    if (connection_get_remote_max_frame_size(m_connection, &maxFrameSize))
    {
      throw std::runtime_error("Could not get remot max frame size.");
    }
    return maxFrameSize;
  }
  void Connection::SetRemoteIdleTimeoutEmptyFrameSendRatio(double ratio)
  {
    if (connection_set_remote_idle_timeout_empty_frame_send_ratio(m_connection, ratio))
    {
      throw std::runtime_error("Could not set remote itle timeout.");
    }
  }

}}}} // namespace Azure::Core::_internal::Amqp
