// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include "claims_based_security.hpp"
#include "common/async_operation_queue.hpp"
#include "connection_string_credential.hpp"
#include "link.hpp"
#include "models/amqp_error.hpp"
#include "models/amqp_message.hpp"
#include "models/amqp_value.hpp"
#include "session.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/nullable.hpp>

#include <vector>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  class MessageReceiverImpl;
  class MessageReceiverFactory;
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _internal {
  enum class MessageReceiverState
  {
    Invalid,
    Idle,
    Opening,
    Open,
    Closing,
    Error,
  };

  enum class ReceiverSettleMode
  {
    First,
    Second,
  };

  class MessageReceiver;

  struct MessageReceiverOptions final
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
    ReceiverSettleMode SettleMode{ReceiverSettleMode::First};

    /** @brief The target for the link associated with the message receiver. */
    Models::_internal::MessageTarget MessageTarget;

    /** @brief The initial delivery count for the link associated with the message receiver. */
    Nullable<uint32_t> InitialDeliveryCount;

    /** @brief The Maximum message size for the link associated with the message receiver. */
    Nullable<uint64_t> MaxMessageSize;

    /** @brief If true, the message receiver will generate low level events */
    bool EnableTrace{false};

    /** @brief If true, require that the message sender be authenticated with the service. */
    bool AuthenticationRequired{true};
  };

  class MessageReceiverEvents {
  protected:
    ~MessageReceiverEvents() = default;

  public:
    virtual void OnMessageReceiverStateChanged(
        MessageReceiver const& receiver,
        MessageReceiverState newState,
        MessageReceiverState oldState)
        = 0;
    virtual Models::AmqpValue OnMessageReceived(
        MessageReceiver const& receiver,
        Models::AmqpMessage const& message)
        = 0;
    virtual void OnMessageReceiverDisconnected(Models::_internal::AmqpError const& error) = 0;
  };

  /** @brief MessageReceiver
   *
   * The MessageReceiver class is responsible for receiving messages from a remote AMQP node. It is
   * constructed by the Session::CreateMessageReceiver method.
   *
   * The message receiver operates in one of two possible models.
   *
   * In the first model, the message receiver caller registers for incoming messages by providing a
   * MessageReceiverEvents callback object, and processes incoming messages in the OnMessageReceived
   * method.
   *
   * In the second model, the caller calls the WaitForIncomingMessage method to wait for the next
   * incoming message.
   *
   * The primary difference between the two models is that the first model allows the caller to
   * alter the disposition of a message when it is received, the second model accepts all incoming
   * messages.
   *
   * @remarks If the caller provides a MessageReceiverEvents callback, then the
   * WaitForIncomingMessage API will throw an exception.
   *
   */
  class MessageReceiver final {
  public:
    ~MessageReceiver() noexcept;

    MessageReceiver(MessageReceiver const&) = default;
    MessageReceiver& operator=(MessageReceiver const&) = default;
    MessageReceiver(MessageReceiver&&) = default;
    MessageReceiver& operator=(MessageReceiver&&) = default;

    /** @brief Opens the message receiver.
     *
     * @param context The context for cancelling operations.
     */
    void Open(Context const& context = {});

    /** @brief Closes the message receiver.
     *
     */
    void Close();

    /** @brief Gets the name of the underlying link.
     *
     * @return The name of the underlying link object.
     */
    std::string GetLinkName() const;

    /** @brief Gets the Address of the message receiver's source node.
     *
     * @return The name of the source node.
     */
    std::string GetSourceName() const;

    /** @brief Waits until a message has been received.
     *
     * @param context The context for cancelling operations.
     *
     * @return A pair of the received message and the error if any.
     */
    std::pair<Azure::Nullable<Models::AmqpMessage>, Models::_internal::AmqpError>
    WaitForIncomingMessage(Context const& context = {});

  private:
    MessageReceiver(std::shared_ptr<_detail::MessageReceiverImpl> impl) : m_impl{impl} {}
    friend class _detail::MessageReceiverFactory;
    std::shared_ptr<_detail::MessageReceiverImpl> m_impl;
  };
}}}} // namespace Azure::Core::Amqp::_internal
