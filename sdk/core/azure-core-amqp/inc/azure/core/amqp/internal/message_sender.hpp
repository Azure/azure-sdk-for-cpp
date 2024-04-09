// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/internal/amqp_settle_mode.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"
#include "cancellable.hpp"
#include "claims_based_security.hpp"
#include "common/async_operation_queue.hpp"
#include "connection.hpp"
#include "link.hpp"
#include "models/amqp_error.hpp"

#include <azure/core/nullable.hpp>

#include <tuple>

#if defined(_azure_TESTING_BUILD)
// Define the test classes dependant on this class here.
namespace Azure { namespace Core { namespace Amqp { namespace Tests { namespace MessageTests {
  class AmqpServerMock;
  class MockServiceEndpoint;
}}}}} // namespace Azure::Core::Amqp::Tests::MessageTests
#endif // _azure_TESTING_BUILDs

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  class MessageSenderImpl;
  class MessageSenderFactory;
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _internal {
  enum class MessageSendStatus
  {
    Invalid,
    Ok,
    Error,
    Timeout,
    Cancelled,
  };
  std::ostream& operator<<(std::ostream& stream, MessageSendStatus status);

  enum class MessageSenderState
  {
    Invalid,
    Idle,
    Opening,
    Open,
    Closing,
    Error,
  };
  std::ostream& operator<<(std::ostream& stream, MessageSenderState state);

  class MessageSender;
  class MessageSenderEvents {
  protected:
    ~MessageSenderEvents() = default;

  public:
    virtual void OnMessageSenderStateChanged(
        MessageSender const& sender,
        MessageSenderState newState,
        MessageSenderState oldState)
        = 0;
    virtual void OnMessageSenderDisconnected(
        MessageSender const& sender,
        Models::_internal::AmqpError const& error)
        = 0;
  };

  struct MessageSenderOptions final
  {
    /** @brief The name of the link associated with the message sender.
     *
     * Links are named so that they can be recovered when communication is interrupted. Link names
     * MUST uniquely identify the link amongst all links of the same direction between the two
     * participating containers. Link names are only used when attaching a link, so they can be
     * arbitrarily long without a significant penalty.
     *
     */
    std::string Name;

    /** @brief The settle mode for the link associated with the message sender.
     *
     * This field indicates how the deliveries sent over the link SHOULD be settled. When this
     * field is set to "mixed", the unsettled map MUST be sent even if it is empty. When this
     * field is set to "settled", the value of the unsettled map MUST NOT be sent. See
     * http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transactions-v1.0-os.html#doc-idp145616
     * for more details.
     *
     */
    SenderSettleMode SettleMode{};

    /** @brief The source for the link associated with the message sender. */
    Models::_internal::MessageSource MessageSource;

    /** @brief The Maximum message size for the link associated with the message sender. */
    Nullable<uint64_t> MaxMessageSize;

    /** @brief The link maximum credits.
     *
     * Each message sent over a link reduces the link-credit by one. When the link-credit reaches
     * zero, no more messages can be sent until the sender receives a disposition indicating that at
     * least one message has been settled. The sender MAY send as many messages as it likes before
     * receiving a disposition, but it MUST NOT send more messages than the link-credit. The sender
     * MUST NOT send any messages after sending a disposition that indicates an error.
     *
     */
    uint32_t MaxLinkCredits{};

    /** @brief The initial delivery count for the link associated with the message.
     *
     * The delivery-count is initialized by the sender when a link endpoint is created, and is
     * incremented whenever a message is sent. Note that the value of this field can be
     * overwritten by the remote peer if a link endpoint is attached with a transfer-id that
     * indicates that the peer's value for the delivery-count is ahead of the sender's value.
     *
     */
    Nullable<uint32_t> InitialDeliveryCount;

    /** @brief If true, the message sender will log trace events. */
    bool EnableTrace{false};

    /** @brief If true, require that the message sender be authenticated with the service. */
    bool AuthenticationRequired{true};
  };

  class MessageSender final {
  public:
    using MessageSendCompleteCallback
        = std::function<void(MessageSendStatus sendResult, Models::AmqpValue const& deliveryState)>;

    ~MessageSender() noexcept;

    MessageSender(MessageSender const&) = default;
    MessageSender& operator=(MessageSender const&) = default;
    MessageSender(MessageSender&&) noexcept = default;
    MessageSender& operator=(MessageSender&&) noexcept = default;

    /** @brief Opens a message sender.
     *
     * @param context The context to use for the operation.
     *
     * @note: If the call to `Open` succeeds, the caller is responsible for calling `Close`
     * before the MessageSender object is destroyed. Failing to
     *
     * @return An error if the operation was not successful.
     */
    _azure_NODISCARD Models::_internal::AmqpError Open(Context const& context = {});

    /** @brief Closes a message sender.
     *
     */
    void Close(Context const& context = {});

    /** @brief Gets the name of the underlying link.
     *
     * @return The name of the underlying link object.
     */
    std::string GetLinkName() const;

    /** @brief Returns the link negotiated maximum message size
     *
     * @return The negotiated maximum message size.
     */
    std::uint64_t GetMaxMessageSize() const;

    /** @brief Send a message synchronously to the target of the message sender.
     *
     * @param message The message to send.
     * @param context The context to use for the operation.
     *
     * @return A tuple containing the status of the send operation and the send disposition.
     */
    _azure_NODISCARD std::tuple<MessageSendStatus, Models::_internal::AmqpError> Send(
        Models::AmqpMessage const& message,
        Context const& context = {});

  private:
    // Half-open the message sender (does not block waiting on the Open to complete).
    _azure_NODISCARD Models::_internal::AmqpError HalfOpen(Context const& context = {});
    /** @brief Construct a MessageSender from a low level message sender implementation.
     *
     * @remarks This function should never be called by a user. It is used internally by the SDK.
     */
    MessageSender(std::shared_ptr<_detail::MessageSenderImpl> sender) : m_impl{sender} {}

    friend class _detail::MessageSenderFactory;
    std::shared_ptr<_detail::MessageSenderImpl> m_impl;
#if _azure_TESTING_BUILD
    friend class Azure::Core::Amqp::Tests::MessageTests::AmqpServerMock;
    friend class Azure::Core::Amqp::Tests::MessageTests::MockServiceEndpoint;
    friend class Azure::Core::Amqp::Tests::MessageTests::MessageListenerEvents;
#endif // _azure_TESTING_BUILD
  };
}}}} // namespace Azure::Core::Amqp::_internal
