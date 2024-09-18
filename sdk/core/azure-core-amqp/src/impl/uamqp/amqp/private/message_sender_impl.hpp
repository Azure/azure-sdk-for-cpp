// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/internal/message_sender.hpp"
#include "link_impl.hpp"
#include "../../../../amqp/private/unique_handle.hpp"

#if ENABLE_UAMQP
#include <azure_uamqp_c/message_sender.h>
#endif

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  template <> struct UniqueHandleHelper<MESSAGE_SENDER_INSTANCE_TAG>
  {
    static void FreeMessageSender(MESSAGE_SENDER_HANDLE obj);

    using type = Core::_internal::BasicUniqueHandle<MESSAGE_SENDER_INSTANCE_TAG, FreeMessageSender>;
  };
#endif
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  using UniqueMessageSender = UniqueHandle<MESSAGE_SENDER_INSTANCE_TAG>;
#endif

  class MessageSenderFactory final {
  public:
    static Azure::Core::Amqp::_internal::MessageSender CreateFromInternal(
        std::shared_ptr<MessageSenderImpl> senderImpl)
    {
      return Azure::Core::Amqp::_internal::MessageSender(senderImpl);
    }
  };

  class MessageSenderImpl : public std::enable_shared_from_this<MessageSenderImpl> {
  public:
    MessageSenderImpl(
        std::shared_ptr<_detail::SessionImpl> session,
        Models::_internal::MessageTarget const& target,
        _internal::MessageSenderOptions const& options
#if ENABLE_UAMQP
        ,
        _internal::MessageSenderEvents* events = nullptr
#endif
    );
#if ENABLE_UAMQP
    MessageSenderImpl(
        std::shared_ptr<_detail::SessionImpl> session,
        _internal::LinkEndpoint& endpoint,
        Models::_internal::MessageTarget const& target,
        _internal::MessageSenderOptions const& options,
        _internal::MessageSenderEvents* events);
#endif
    virtual ~MessageSenderImpl() noexcept;

    MessageSenderImpl(MessageSenderImpl const&) = delete;
    MessageSenderImpl& operator=(MessageSenderImpl const&) = delete;
    MessageSenderImpl(MessageSenderImpl&&) noexcept = delete;
    MessageSenderImpl& operator=(MessageSenderImpl&&) noexcept = delete;

    Models::_internal::AmqpError Open(bool blockingOpen, Context const& context);
    void Close(Context const& context);
    std::tuple<_internal::MessageSendStatus, Models::_internal::AmqpError> Send(
        Models::AmqpMessage const& message,
        Context const& context);

    std::uint64_t GetMaxMessageSize() const;

    std::string GetLinkName() const;

  private:
#if ENABLE_UAMQP
    static void OnMessageSenderStateChangedFn(
        void* context,
        MESSAGE_SENDER_STATE newState,
        MESSAGE_SENDER_STATE oldState);
#endif
    void CreateLink();
    void CreateLink(_internal::LinkEndpoint& endpoint);
    void PopulateLinkProperties();
    void QueueSendInternal(
        Models::AmqpMessage const& message,
        Azure::Core::Amqp::_internal::MessageSender::MessageSendCompleteCallback onSendComplete,
        Context const& context);
    void OnLinkDetached(Models::_internal::AmqpError const& error);

    bool m_senderOpen{false};
#if ENABLE_UAMQP
    UniqueMessageSender m_messageSender{};
#endif
    std::shared_ptr<_detail::LinkImpl> m_link;
    _internal::MessageSenderEvents* m_events;
    Models::_internal::AmqpError m_savedMessageError;
    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<
        Azure::Core::Amqp::_internal::MessageSendStatus,
        Models::_internal::AmqpError>
        m_sendCompleteQueue;

    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<Models::_internal::AmqpError>
        m_openQueue;
    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<Models::_internal::AmqpError>
        m_closeQueue;
    _internal::MessageSenderState m_currentState{};

    std::shared_ptr<_detail::SessionImpl> m_session;
    Models::_internal::MessageTarget m_target;
    _internal::MessageSenderOptions m_options;
  };
}}}} // namespace Azure::Core::Amqp::_detail
