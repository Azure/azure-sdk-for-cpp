// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/message_receiver.hpp"
#include "claims_based_security_impl.hpp"
#include "connection_impl.hpp"
#include "link_impl.hpp"
#include "message_receiver_impl.hpp"
#include "session_impl.hpp"

#include <azure/core/credentials/credentials.hpp>

#include <azure_uamqp_c/amqpvalue.h>
#include <azure_uamqp_c/message.h>
#include <azure_uamqp_c/message_receiver.h>

#include <memory>
#include <vector>

namespace Azure { namespace Core { namespace _internal {
  template <> struct UniqueHandleHelper<MESSAGE_RECEIVER_INSTANCE_TAG>
  {
    static void FreeMessageReceiver(MESSAGE_RECEIVER_HANDLE obj);

    using type = BasicUniqueHandle<MESSAGE_RECEIVER_INSTANCE_TAG, FreeMessageReceiver>;
  };
}}} // namespace Azure::Core::_internal

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  using UniqueMessageReceiver = Azure::Core::_internal::UniqueHandle<MESSAGE_RECEIVER_INSTANCE_TAG>;

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
    void Close();
    std::string GetLinkName() const;
    std::string GetSourceName() const { return static_cast<std::string>(m_source.GetAddress()); }

    std::pair<Azure::Nullable<Models::AmqpMessage>, Models::_internal::AmqpError>
    WaitForIncomingMessage(Context const& context);

  private:
    UniqueMessageReceiver m_messageReceiver{};
    bool m_receiverOpen{false};
    bool m_listeningReceiver{false};
    std::shared_ptr<_detail::LinkImpl> m_link;
    _internal::MessageReceiverOptions m_options;
    Models::_internal::MessageSource m_source;
    std::shared_ptr<_detail::SessionImpl> m_session;
    Models::_internal::AmqpError m_savedMessageError{};

    Azure::Core::Amqp::Common::_internal::
        AsyncOperationQueue<Models::AmqpMessage, Models::_internal::AmqpError>
            m_messageQueue;

    _internal::MessageReceiverEvents* m_eventHandler{};

    static AMQP_VALUE OnMessageReceivedFn(const void* context, MESSAGE_HANDLE message);

    virtual Models::AmqpValue OnMessageReceived(Models::AmqpMessage message);

    static void OnMessageReceiverStateChangedFn(
        const void* context,
        MESSAGE_RECEIVER_STATE newState,
        MESSAGE_RECEIVER_STATE oldState);

    void CreateLink();
    void CreateLink(_internal::LinkEndpoint& endpoint);
    void PopulateLinkProperties();
  };
}}}} // namespace Azure::Core::Amqp::_detail
