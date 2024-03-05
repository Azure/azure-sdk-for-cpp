// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/internal/models/amqp_error.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"
#include "common/async_operation_queue.hpp"
#include "connection_string_credential.hpp"
#include "endpoint.hpp"
#include "models/message_source.hpp"
#include "models/message_target.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#if defined(_azure_TESTING_BUILD)
// Define the test classes dependant on this class here.
namespace Azure { namespace Core { namespace Amqp { namespace Tests {
  namespace MessageTests {
    class AmqpServerMock;
    class MockServiceEndpoint;
    class MessageListenerEvents;
  } // namespace MessageTests

  class TestSessions_SimpleSession_Test;
  class TestSessions_SessionProperties_Test;
  class TestSessions_SessionBeginEnd_Test;
  class TestSessions_MultipleSessionBeginEnd_Test;
  class TestLinks_LinkAttachDetach_Test;
  class TestSocketListenerEvents;
  class LinkSocketListenerEvents;
  class TestMessages_SenderSendAsync_Test;
}}}} // namespace Azure::Core::Amqp::Tests
#endif // _azure_TESTING_BUILD
#if defined(SAMPLES_BUILD)
namespace LocalServerSample {
class SampleEvents;
} // namespace LocalServerSample
#endif // SAMPLES_BUILD

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  class SessionImpl;
  class SessionFactory;
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _internal {

  class Connection;
  enum class SessionRole;
  class MessageSender;
  struct MessageSenderOptions;
  class MessageSenderEvents;
  class MessageReceiver;
  struct MessageReceiverOptions;
  class MessageReceiverEvents;
  class ManagementClient;
  struct ManagementClientOptions;
  class ManagementClientEvents;

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
  class SessionEvents {
  protected:
    ~SessionEvents() = default;

  public:
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
    /** @brief Destroys the session object. */
    ~Session() noexcept;

    /** @brief Creates a MessageSender
     *
     * @param target - The target to which the message will be sent.
     * @param options - Options to configure the MessageSender.
     * @param events - Event Handler used to capture message sender events.
     *
     * @returns A MessageSender object.
     *
     */
    MessageSender CreateMessageSender(
        Models::_internal::MessageTarget const& target,
        MessageSenderOptions const& options,
        MessageSenderEvents* events) const;

    /** @brief Creates a MessageReceiver
     *
     * @param receiverSource - The source from which to receive messages.
     * @param options - Options to configure the MessageReceiver.
     * @param receiverEvents - Event Handler used to capture message receiverevents.
     *
     * @returns A MessageSender object.
     *
     */
    MessageReceiver CreateMessageReceiver(
        Models::_internal::MessageSource const& receiverSource,
        MessageReceiverOptions const& options,
        MessageReceiverEvents* receiverEvents = nullptr) const;

    ManagementClient CreateManagementClient(
        std::string const& managementInstancePath,
        ManagementClientOptions const& options,
        ManagementClientEvents* managementEvents = nullptr) const;

  private:
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

    /** @brief Begins operations on the session.
     *
     * @remarks Note that this function is not intended for use by AMQP clients, it is intended for
     * use by AMQP listeners.
     *
     */
    void Begin();

    /** @brief Ends operations on the session.
     *
     * @remarks Note that this function is not intended for use by AMQP clients, it is intended for
     * use by AMQP listeners.
     *
     */
    void End(std::string const& condition_value = {}, std::string const& description = {});

    /** @brief Sends a detach message on the specified link endpoint.
     *
     * @param linkEndpoint - Link endpoint to detach.
     * @param closeLink - Whether to close the link after sending the detach.
     * @param error - Error description to send with the detach.
     *
     * @remarks Note that this function is not intended for use by AMQP clients, it is intended for
     * use by AMQP listeners.
     *
     */
    void SendDetach(
        LinkEndpoint const& linkEndpoint,
        bool closeLink,
        Models::_internal::AmqpError const& error) const;

    /** @brief Creates a MessageSender for use in a message listener.
     *
     * @param endpoint - Endpoint associated with this message sender.
     * @param target - The target to which the message will be sent.
     * @param options - Options to configure the MessageSender.
     * @param events - Event Handler used to capture message sender events.
     *
     * @returns A MessageSender object.
     *
     * @remarks Note that this function is not intended for use by AMQP clients, it is intended for
     * use by AMQP listeners.
     *
     */
    MessageSender CreateMessageSender(
        LinkEndpoint& endpoint,
        Models::_internal::MessageTarget const& target,
        MessageSenderOptions const& options,
        MessageSenderEvents* events) const;

    /** @brief Creates a MessageReceiver for use in a message listener.
     *
     * @param receiverSource - The source from which to receive messages.
     * @param options - Options to configure the MessageReceiver.
     * @param receiverEvents - Event Handler used to capture message receiverevents.
     *
     * @returns A MessageSender object.
     *
     */
    MessageReceiver CreateMessageReceiver(
        LinkEndpoint& linkEndpoint,
        Models::_internal::MessageSource const& receiverSource,
        MessageReceiverOptions const& options,
        MessageReceiverEvents* receiverEvents = nullptr) const;

    friend class _detail::SessionFactory;

#if _azure_TESTING_BUILD
    friend class Azure::Core::Amqp::Tests::MessageTests::AmqpServerMock;
    friend class Azure::Core::Amqp::Tests::MessageTests::MockServiceEndpoint;
    friend class Azure::Core::Amqp::Tests::MessageTests::MessageListenerEvents;
    friend class Azure::Core::Amqp::Tests::TestSocketListenerEvents;
    friend class Azure::Core::Amqp::Tests::LinkSocketListenerEvents;
    friend class Azure::Core::Amqp::Tests::TestSessions_SimpleSession_Test;
    friend class Azure::Core::Amqp::Tests::TestSessions_SessionProperties_Test;
    friend class Azure::Core::Amqp::Tests::TestSessions_SessionBeginEnd_Test;
    friend class Azure::Core::Amqp::Tests::TestSessions_MultipleSessionBeginEnd_Test;
    friend class Azure::Core::Amqp::Tests::TestLinks_LinkAttachDetach_Test;

    friend class Azure::Core::Amqp::Tests::TestMessages_SenderSendAsync_Test;
#endif // _azure_TESTING_BUILD
#if SAMPLES_BUILD
    friend class LocalServerSample::SampleEvents;
#endif // SAMPLES_BUILD
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
