// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/internal/message_sender.hpp"
#include "claims_based_security_impl.hpp"
#include "link_impl.hpp"
#include "unique_handle.hpp"

#include <azure_uamqp_c/message_sender.h>

#include <tuple>

#define SENDER_SYNCHRONOUS_CLOSE 0

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  template <> struct UniqueHandleHelper<MESSAGE_SENDER_INSTANCE_TAG>
  {
    static void FreeMessageSender(MESSAGE_SENDER_HANDLE obj);

    using type = Core::_internal::BasicUniqueHandle<MESSAGE_SENDER_INSTANCE_TAG, FreeMessageSender>;
  };
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  using UniqueMessageSender = UniqueHandle<MESSAGE_SENDER_INSTANCE_TAG>;

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
        _internal::MessageSenderOptions const& options,
        _internal::MessageSenderEvents* events);
    MessageSenderImpl(
        std::shared_ptr<_detail::SessionImpl> session,
        _internal::LinkEndpoint& endpoint,
        Models::_internal::MessageTarget const& target,
        _internal::MessageSenderOptions const& options,
        _internal::MessageSenderEvents* events);
    virtual ~MessageSenderImpl() noexcept;

    MessageSenderImpl(MessageSenderImpl const&) = delete;
    MessageSenderImpl& operator=(MessageSenderImpl const&) = delete;
    MessageSenderImpl(MessageSenderImpl&&) noexcept = delete;
    MessageSenderImpl& operator=(MessageSenderImpl&&) noexcept = delete;

    void Open(Context const& context);
    void Close();
    std::tuple<_internal::MessageSendStatus, Models::_internal::AmqpError> Send(
        Models::AmqpMessage const& message,
        Context const& context);

    std::uint64_t GetMaxMessageSize() const;

  private:
    static void OnMessageSenderStateChangedFn(
        void* context,
        MESSAGE_SENDER_STATE newState,
        MESSAGE_SENDER_STATE oldState);

    void CreateLink();
    void CreateLink(_internal::LinkEndpoint& endpoint);
    void PopulateLinkProperties();
    void QueueSendInternal(
        Models::AmqpMessage const& message,
        Azure::Core::Amqp::_internal::MessageSender::MessageSendCompleteCallback onSendComplete,
        Context const& context);

    bool m_senderOpen{false};
    UniqueMessageSender m_messageSender{};
    std::shared_ptr<_detail::LinkImpl> m_link;
    _internal::MessageSenderEvents* m_events;
    Models::_internal::AmqpError m_savedMessageError;
    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<
        Azure::Core::Amqp::_internal::MessageSendStatus,
        Models::_internal::AmqpError>
        m_sendCompleteQueue;

#if SENDER_SYNCHRONOUS_CLOSE
    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<Models::_internal::AmqpError>
        m_closeQueue;
#endif
    _internal::MessageSenderState m_currentState{};

    std::shared_ptr<_detail::SessionImpl> m_session;
    Models::_internal::MessageTarget m_target;
    _internal::MessageSenderOptions m_options;
  };
}}}} // namespace Azure::Core::Amqp::_detail
