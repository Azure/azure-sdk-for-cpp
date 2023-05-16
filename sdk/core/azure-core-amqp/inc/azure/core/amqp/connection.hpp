// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include "azure/core/amqp/models/amqp_protocol.hpp"
#include "common/async_operation_queue.hpp"
#include "connection_string_credential.hpp"
#include "models/amqp_value.hpp"
#include "session.hpp"
#include <azure/core/credentials/credentials.hpp>
#include <chrono>
#include <limits>
#include <memory>
#include <string>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  class ConnectionImpl;
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _internal {
  class Session;

  class Error;

  /**
   * @brief The state of the connection.
   *
   * @remarks This enum is used to track the state of the connection. The state machine is
   * implemented in the Connection class. For more information about various connection states, see
   * the [AMQP Connection
   * States](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#doc-idp184736)
   * definition.
   *
   */
  enum class ConnectionState
  {
    /** @brief Start
     * In this state a connection exists, but nothing has been sent or received. This is the
     * state an implementation would be in immediately after performing a socket connect or
     * socket accept.
     */
    Start,
    /** @brief Header Received
     * In this state the connection header has been received from the peer but a connection header
     * has not been sent.
     */
    HeaderReceived,
    /** @brief Header Sent
     * In this state the connection header has been sent to the peer but no connection header has
     * been received.
     */
    HeaderSent,
    /** @brief Header Exchanged
     * In this state the connection header has been sent to the peer and a connection header has
     * been received from the peer.
     */
    HeaderExchanged,
    /** @brief Open Pipe
     * In this state both the connection header and the open frame have been sent but nothing has
     * been received.
     */
    OpenPipe,
    /** @brief OC Pipe
     * In this state, the connection header, the open frame, any pipelined connection traffic, and
     * the close frame have been sent but nothing has been received.
     */
    OcPipe,
    /** @brief Open Received
     * In this state the connection headers have been exchanged. An open frame has been received
     * from the peer but an open frame has not been sent.
     */
    OpenReceived,
    /** @brief Open Sent
     * In this state the connection headers have been exchanged. An open frame has been sent to the
     * peer but no open frame has yet been received.
     */
    OpenSent,
    /** @brief Close Pipe
     * In this state the connection headers have been exchanged. An open frame, any pipelined
     * connection traffic, and the close frame have been sent but no open frame has yet been
     * received from the peer.
     */
    ClosePipe,
    /** @brief Opened
     * In this state the connection header and the open frame have been both sent and received.
     */
    Opened,
    /** @brief Close Received
     * In this state a close frame has been received indicating that the peer has initiated an AMQP
     * close. No further frames are expected to arrive on the connection; however, frames can still
     * be sent. If desired, an implementation MAY do a TCP half-close at this point to shut down the
     * read side of the connection.
     */
    CloseReceived,
    /** @brief Close Sent
     * In this state a close frame has been sent to the peer. It is illegal to write anything more
     * onto the connection, however there could potentially still be incoming frames. If desired, an
     * implementation MAY do a TCP half-close at this point to shutdown the write side of the
     * connection.
     */
    CloseSent,
    /** @brief Discarding
     * The DISCARDING state is a variant of the CLOSE_SENT state where the close is triggered by an
     * error. In this case any incoming frames on the connection MUST be silently discarded until
     * the peer's close frame is received.
     */
    Discarding,
    /** @brief End
     * In this state it is illegal for either endpoint to write anything more onto the connection.
     * The connection can be safely closed and discarded.
     */
    End,

    /** @brief Error
     * In this state an error has occurred on the connection. It is illegal for either endpoint to
     * write anything more onto the connection. The connection can be safely closed and discarded.
     */
    Error
  };

  class Connection;

  /** @brief The ConnectionEvents interface defines a series of events triggered on a connection
   * object.
   */
  struct ConnectionEvents
  {
    /** @brief Called when the connection state changes.
     *
     * @param connection The connection object whose state changed.
     * @param newState The new state of the connection.
     * @param oldState The previous state of the connection.
     */
    virtual void OnConnectionStateChanged(
        Connection const& connection,
        ConnectionState newState,
        ConnectionState oldState)
        = 0;

    /** @brief Called when a new endpoint connects to the connection.
     *
     * @param connection The connection object.
     * @param endpoint The endpoint that connected.
     * @return true if the endpoint was accepted, false otherwise.
     *
     * @remarks Note that this function should only be overriden if the application is listening
     * on the connection.
     */
    virtual bool OnNewEndpoint(Connection const& connection, Endpoint& endpoint)
    {
      (void)connection;
      (void)endpoint;
      return false;
    }

    /** @brief called when an I/O error has occurred on the connection.
     *
     * @param connection The connection object.
     */
    virtual void OnIoError(Connection const& connection) = 0;
  };

  /** @brief Options used to create a connection. */
  struct ConnectionOptions
  {
    /** @brief The idle timeout for the connection.
     *
     * If no frames are received within the timeout, the connection will be closed.
     */
    std::chrono::milliseconds IdleTimeout{std::chrono::milliseconds(60000)};

    /** @brief The maximum frame size for the connection.
     *
     * The maximum frame size is the largest frame that can be received on the connection. During
     * the initial connection negotiation, each peer will send a max frame size. The smaller of the
     * two values will be used as the maximum frame size for the connection.
     *
     * @remarks The maximum frame size must be at least 512 bytes. The default value is the maximum
     * value for a uint32.
     */
    uint32_t MaxFrameSize{std::numeric_limits<uint32_t>::max()};

    /** @brief The maximum number of channels supported.
     *
     * A single connection may have multiple independant sessions active simultaneously up to the
     * negotiated maximum channel count.
     */
    uint16_t MaxChannelCount{65535};

    /** @brief Properties for the connection.
     *
     * The properties map contains a set of fields intended to indicate information about the
     * connection and its container.
     *
     */
    Azure::Core::Amqp::Models::AmqpMap Properties;

    /**
     * Note that the AMQP specification defines the following fields in the open performative which
     * are not supported by the underlying uAMQP stack:
     *
     * - outgoing-locales
     * - incoming-locales
     * - offered-capabilities
     * - desired-capabilities
     *
     */

    /** @brief Enable tracing from the uAMQP stack.
     */
    bool EnableTrace{false};

    /** @brief Defines the ID of the container for this connection. If empty, a unique 128 bit value
     * will be used.
     */
    std::string ContainerId;

    // Optional SASL plain credentials.
    std::shared_ptr<Azure::Core::Amqp::_internal::SaslPlainConnectionStringCredential>
        SaslCredentials{};
  };

  class Connection final {
  public:
    /** @brief Construct a new AMQP Connection.
     *
     * @param requestUri The URI to connect to.
     * @param options The options to use when creating the connection.
     * @param eventHandler The event handler for the connection.
     *
     * @remarks The requestUri must be a valid AMQP URI.
     */
    Connection(
        std::string const& requestUri,
        ConnectionOptions const& options,
        ConnectionEvents* eventHandler = nullptr);

    /** @brief Construct a new AMQP Connection.
     *
     * @param transport The transport to use for the connection.
     * @param options The options to use when creating the connection.
     * @param eventHandler The event handler for the connection.
     *
     * @remarks This constructor should only be used for an AMQP listener - it is not intended for
     * use in a client.
     */
    Connection(
        std::shared_ptr<Azure::Core::Amqp::Network::_internal::Transport> transport,
        ConnectionOptions const& options,
        ConnectionEvents* eventHandler = nullptr);

    /** @brief Destroy an AMQP connection */
    ~Connection();

    /** @brief Opens the current connection.
     *
     * @remarks In general, a customer will not need to call this method, instead the connection
     * will be opened implicitly by a Session object derived from the connection. It primarily
     * exists as a test hook.
     *
     */
    void Open();

    /** @brief Starts listening for incoming connections.
     *
     * @remarks This method should only be called on a connection that was created with a transport
     * object.
     *
     */
    void Listen();

    /** @brief Closes the current connection.
     *
     * @param condition The condition for closing the connection.
     * @param description The description for closing the connection.
     * @param info Additional information for closing the connection.
     *
     * @remarks In general, a customer will not need to call this method, instead the connection
     * will be closed implicitly by a Session object derived from the connection. It primarily
     * exists as a test hook.
     *
     */
    void Close(
        std::string const& condition,
        std::string const& description,
        Azure::Core::Amqp::Models::AmqpValue info);

    void Poll() const;

    /** @brief Gets host configured by the connection.
     *
     * @return The host used in the connection.
     */
    std::string GetHost() const;

    /** @brief Gets the port configured by the connection.
     *
     * @return The port used in the connection.
     */
    uint16_t GetPort() const;

    /** @brief Gets the max frame size configured for the connection.
     *
     * @return The configured maximum frame size for the connection.
     */
    uint32_t GetMaxFrameSize() const;

    /** @brief Gets the max frame size configured for the remote node.
     *
     * @return The configured maximum frame size for the remote node.
     */
    uint32_t GetRemoteMaxFrameSize() const;

    /** @brief Gets the max channel count configured for the connection.
     *
     * @return The configured maximum channel count for the connection.
     */
    uint16_t GetMaxChannel() const;

    /** @brief Gets the idle timeout configured for the connection.
     *
     * @return The configured idle timeout in milliseconds for the connection.
     */
    std::chrono::milliseconds GetIdleTimeout() const;

    /** @brief Gets the properties for the connection.
     *
     * @return The properties for the connection.
     */
    Azure::Core::Amqp::Models::AmqpMap GetProperties() const;

    /** @brief Sets the percentage of the idle timeout before an empty frame is sent to the remote
     * node.
     *
     * @param idleTimeoutEmptyFrameSendRatio The percentage of the idle timeout before an empty
     * frame is sent to the remote node.
     *
     * This field determines when to send empty frames to the remote node to keep the connection
     * alive as a percentage of the remote nodes idle timeout. For
     * example, if the remote node has an idle timeout of 5 minutes, a value of 0.5 will cause an
     * empty frame to be sent every 2.5 minutes.
     *
     * @remarks: The default value for this field is 0.5.
     *
     * @remarks: Note that this is a dynamic property on the connection, it can be set after the
     * connection is opened.
     */
    void SetIdleEmptyFrameSendPercentage(double idleTimeoutEmptyFrameSendRatio);

    /** @brief Create an AMQP Connection from an existing connection implementation.
     *
     * @remarks This constructor is an implementation detail of the AMQP stack and should never be
     * called by clients.
     *
     */
    Connection(std::shared_ptr<Azure::Core::Amqp::_detail::ConnectionImpl> impl) : m_impl{impl} {}

    /** @brief Returns the connection implementation associated with this AMQP Connection.
     *
     * @returns The connection implementation associated with this AMQP Connection.
     *
     * @remarks This method is an implementation detail of the AMQP stack and should never be
     * called by clients.
     *
     */
    std::shared_ptr<Azure::Core::Amqp::_detail::ConnectionImpl> GetImpl() const { return m_impl; }

  private:
    std::shared_ptr<Azure::Core::Amqp::_detail::ConnectionImpl> m_impl;
  };
}}}} // namespace Azure::Core::Amqp::_internal
