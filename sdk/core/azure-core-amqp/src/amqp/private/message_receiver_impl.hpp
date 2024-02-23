// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/internal/message_receiver.hpp"
#include "link_impl.hpp"
#include "session_impl.hpp"
#include "unique_handle.hpp"

#include <azure_uamqp_c/amqpvalue.h>
#include <azure_uamqp_c/message.h>
#include <azure_uamqp_c/message_receiver.h>

#include <memory>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  template <> struct UniqueHandleHelper<MESSAGE_RECEIVER_INSTANCE_TAG>
  {
    static void FreeMessageReceiver(MESSAGE_RECEIVER_HANDLE obj);

    using type
        = Core::_internal::BasicUniqueHandle<MESSAGE_RECEIVER_INSTANCE_TAG, FreeMessageReceiver>;
  };
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  using UniqueMessageReceiver = UniqueHandle<MESSAGE_RECEIVER_INSTANCE_TAG>;

  class MessageReceiverFactory final {
  public:
    static Azure::Core::Amqp::_internal::MessageReceiver CreateFromInternal(
        std::shared_ptr<MessageReceiverImpl> receiverImpl)
    {
      return Azure::Core::Amqp::_internal::MessageReceiver(receiverImpl);
    }
  };

  class MessageReceiverImpl final : public std::enable_shared_from_this<MessageReceiverImpl> {
  public:
    MessageReceiverImpl(
        std::shared_ptr<_detail::SessionImpl> session,
        Models::_internal::MessageSource const& receiverSource,
        _internal::MessageReceiverOptions const& options,
        _internal::MessageReceiverEvents* receiverEvents = nullptr);
    MessageReceiverImpl(
        std::shared_ptr<_detail::SessionImpl> session,
        _internal::LinkEndpoint& linkEndpoint,
        Models::_internal::MessageSource const& receiverSource,
        _internal::MessageReceiverOptions const& options,
        _internal::MessageReceiverEvents* receiverEvents = nullptr);
    ~MessageReceiverImpl() noexcept;

    MessageReceiverImpl(MessageReceiverImpl const&) = delete;
    MessageReceiverImpl& operator=(MessageReceiverImpl const&) = delete;
    MessageReceiverImpl(MessageReceiverImpl&&) = delete;
    MessageReceiverImpl& operator=(MessageReceiverImpl&&) = delete;

    operator bool() const { return (m_messageReceiver != nullptr); }

    void Open(Context const& context);
    void Close(Context const& context);
    std::string GetLinkName() const;
    std::string GetSourceName() const { return static_cast<std::string>(m_source.GetAddress()); }

    std::pair<std::shared_ptr<Models::AmqpMessage>, Models::_internal::AmqpError>
    WaitForIncomingMessage(Context const& context);

    std::pair<std::shared_ptr<Models::AmqpMessage>, Models::_internal::AmqpError>
    TryWaitForIncomingMessage();

  private:
    UniqueMessageReceiver m_messageReceiver{};
    bool m_receiverOpen{false};
    std::shared_ptr<_detail::LinkImpl> m_link;
    _internal::MessageReceiverOptions m_options;
    Models::_internal::MessageSource m_source;
    std::shared_ptr<_detail::SessionImpl> m_session;
    Models::_internal::AmqpError m_savedMessageError{};
    _internal::MessageReceiverState m_currentState{};

    Azure::Core::Amqp::Common::_internal::
        AsyncOperationQueue<std::shared_ptr<Models::AmqpMessage>, Models::_internal::AmqpError>
            m_messageQueue;

    // When we close a uAMQP messagereceiver, the link is left in the half closed state. We need to
    // wait for the link to be fully closed before we can close the session. This queue will hold
    // the close operation until the link is fully closed.
    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<Models::_internal::AmqpError>
        m_closeQueue;

    _internal::MessageReceiverEvents* m_eventHandler{};

    static AMQP_VALUE OnMessageReceivedFn(const void* context, MESSAGE_HANDLE message);

    virtual Models::AmqpValue OnMessageReceived(
        std::shared_ptr<Models::AmqpMessage> const& message);

    void OnLinkDetached(Models::_internal::AmqpError const& error);

    static void OnMessageReceiverStateChangedFn(
        const void* context,
        MESSAGE_RECEIVER_STATE newState,
        MESSAGE_RECEIVER_STATE oldState);

    void CreateLink();
    void CreateLink(_internal::LinkEndpoint& endpoint);
    void PopulateLinkProperties();
  };
}}}} // namespace Azure::Core::Amqp::_detail
