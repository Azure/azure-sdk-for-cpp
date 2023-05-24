// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include "common/async_operation_queue.hpp"
#include "connection_string_credential.hpp"
#include "endpoint.hpp"
#include "models/amqp_value.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  class SessionImpl;
  class SessionFactory;
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _internal {

  class Connection;
  enum class SessionRole;

  enum class ExpiryPolicy
  {
    LinkDetach,
    SessionEnd,
    ConnectionClose,
    Never,
  };

  enum class SessionState
  {
    Unmapped,
    BeginSent,
    BeginReceived,
    Mapped,
    EndSent,
    EndReceived,
    Discarding,
    Error,
  };

  enum class SessionSendTransferResult
  {
    Ok,
    Error,
    Busy,
  };

  class Session;
  struct SessionEvents
  {
    virtual bool OnLinkAttached(
        Session const& session,
        LinkEndpoint& newLink,
        std::string const& name,
        SessionRole role,
        Models::AmqpValue const& source,
        Models::AmqpValue const& target,
        Models::AmqpValue const& properties)
        = 0;
  };

  struct SessionOptions final
  {
    /** @brief The Audience to which an authentication operation applies when using Claims Based
     * Authentication. */
    std::vector<std::string> AuthenticationScopes;

    /** @brief Represents the initial incoming window size for the sender. See [AMQP Session Flow
     * Control](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#doc-session-flow-control)
     * for more information.*/
    Nullable<uint32_t> InitialIncomingWindowSize;
    /** @brief Represents the initial outgoing window size for the sender. See [AMQP Session Flow
     * Control](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#doc-session-flow-control)
     * for more information.*/
    Nullable<uint32_t> InitialOutgoingWindowSize;

    /** @brief Represents the maximum number of link handles which can be used on the session. See
     * [AMQP Session Flow
     * Control](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#doc-session-flow-control)
     * for more information.*/
    Nullable<uint32_t> MaximumLinkCount;
  };

  class Session final {
  public:
    /** @brief Create a new AMQP Session object on the specified parent connection.
     *
     * @param parentConnection - Connection upon which to create the session.
     * @param credential - Credential to use for authentication.
     * @param options - Options to use when creating the session.
     * @param eventHandler - Event handler for session events.
     */
    Session(
        Connection const& parentConnection,
        std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
        SessionOptions const& options = {},
        SessionEvents* eventHandler = nullptr);

    /** @brief Construct a new session associated with the specified connection over the specified
     * endpoint.
     *
     * @param parentConnection - Connection upon which to create the session.
     * @param newEndpoint - AMQP Endpoint from which to create the session.
     * @param eventHandler - Event handler for session events.
     *
     * @remarks Note that this function is normally only called from a application listening for
     * incoming connections, not from an AMQP client.
     */
    Session(
        Connection const& parentConnection,
        Endpoint& newEndpoint,
        SessionOptions const& options = {},
        SessionEvents* eventHandler = nullptr);

    /** @brief Destroys the session object. */
    ~Session() noexcept;

    /** @brief Returns the current value of the incoming window.
     *
     * @returns The current incoming message window.
     */
    uint32_t GetIncomingWindow() const;
    /** @brief Returns the current value of the outgoing window.
     *
     * @returns The current outgoing message window.
     */
    uint32_t GetOutgoingWindow() const;

    /** @brief Returns the maximum number of links currently configured.
     *
     * @returns The current maximum number of links configured.
     */
    uint32_t GetHandleMax() const;

    void Begin();
    void End(std::string const& condition_value, std::string const& description);

    friend class _detail::SessionFactory;

  private:
    /** @brief Construct a new Session object from an existing implementation instance.
     *
     * @param impl - Implementation object
     *
     * @remarks This function is used internally by the library and is not intended for use by any
     * client.
     */
    Session(std::shared_ptr<_detail::SessionImpl> impl) : m_impl{impl} {}

    std::shared_ptr<_detail::SessionImpl> m_impl;
  };

}}}} // namespace Azure::Core::Amqp::_internal
