// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include "cancellable.hpp"
#include "claims_based_security.hpp"
#include "common/async_operation_queue.hpp"
#include "connection.hpp"
#include "link.hpp"
#include "models/amqp_error.hpp"
#include "models/amqp_message.hpp"
#include "models/amqp_value.hpp"

#include <azure/core/nullable.hpp>

#include <tuple>

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
  enum class MessageSenderState
  {
    Invalid,
    Idle,
    Opening,
    Open,
    Closing,
    Error,
  };

  enum class SenderSettleMode
  {
    Unsettled,
    Settled,
    Mixed,
  };

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
    virtual void OnMessageSenderDisconnected(Models::_internal::AmqpError const& error) = 0;
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
     */
    void Open(Context const& context = {});

    /** @brief Closes a message sender.
     *
     */
    void Close();

    /** @brief Send a message synchronously to the target of the message sender.
     *
     * @param message The message to send.
     * @param context The context to use for the operation.
     *
     * @return A tuple containing the status of the send operation and the send disposition.
     */
    std::tuple<MessageSendStatus, Models::AmqpValue> Send(
        Models::AmqpMessage const& message,
        Context const& context = {});

    /** @brief Queue a message to be sent to the target of the message sender.
     */
    void QueueSend(
        Models::AmqpMessage const& message,
        MessageSendCompleteCallback onSendComplete,
        Context const& context = {});

  private:
    /** @brief Construct a MessageSender from a low level message sender implementation.
     *
     * @remarks This function should never be called by a user. It is used internally by the SDK.
     */
    MessageSender(std::shared_ptr<_detail::MessageSenderImpl> sender) : m_impl{sender} {}

    friend class _detail::MessageSenderFactory;
    std::shared_ptr<_detail::MessageSenderImpl> m_impl;
  };
}}}} // namespace Azure::Core::Amqp::_internal
