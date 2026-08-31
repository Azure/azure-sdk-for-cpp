// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../../../../amqp/private/unique_handle.hpp"
#include "azure/core/amqp/internal/message_sender.hpp"
#include "link_impl.hpp"

#include <azure_uamqp_c/message_sender.h>

#include <atomic>
#include <cstdint>
#include <map>

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
        _internal::MessageSenderEvents* events = nullptr);
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

    Models::_internal::AmqpError Open(bool blockingOpen, Context const& context);
    void Close(Context const& context);
    std::tuple<_internal::MessageSendStatus, Models::_internal::AmqpError> Send(
        Models::AmqpMessage const& message,
        Context const& context);

    std::uint64_t GetMaxMessageSize() const;

    /** @brief Reports whether the peer has taken the link away.
     *
     * Never throws, so a caller can test a cached sender rather than discovering a dead link from
     * a call that fails.
     */
    bool IsLinkDetached() const noexcept
    {
      return m_linkDetached.load(std::memory_order_acquire);
    }

    std::string GetLinkName() const;

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
    void OnLinkDetached(Models::_internal::AmqpError const& error);

    /** @brief Read the negotiated maximum message size from the link and store it.
     *
     * The caller must know the link is attached. Returns the value it stored.
     */
    std::uint64_t CaptureMaxMessageSize() const;

    /** @brief Record that the link reached the attached state.
     *
     * Runs on the uAMQP polling thread, so it swallows any failure rather than letting an
     * exception unwind into C code.
     */
    void OnLinkAttached() noexcept;

    /** @brief Release the link and the async operation on the connection, then mark the sender as
     * closed.
     *
     * Close() calls this on every path, including the paths that throw. A sender that keeps the
     * async operation on the connection stops the process in ~ConnectionImpl, and a sender that
     * keeps the open flag stops the process in ~MessageSenderImpl.
     */
    void CompleteClose() noexcept;

    // The completion state of one send. A send that gave up leaves the uAMQP operation in flight,
    // because uAMQP can call the completion handler two times after a cancel. Each send owns its
    // own state, so a late result completes only the send that started it.
    struct SendOperation final
    {
      Azure::Core::Amqp::Common::_internal::
          AsyncOperationQueue<_internal::MessageSendStatus, Models::_internal::AmqpError>
              Queue;
    };

    bool m_senderOpen{false};

    // The negotiated maximum message size, captured each time the link attaches. Mutable so the
    // const getter can fill it on demand.
    mutable std::atomic<bool> m_maxMessageSizeCached{false};
    mutable std::atomic<std::uint64_t> m_maxMessageSize{0};

    // Set when the peer takes the link away, cleared when a link attaches. Presence in a cache is
    // not liveness, so callers need a test that does not throw.
    std::atomic<bool> m_linkDetached{false};

    UniqueMessageSender m_messageSender{};
    std::shared_ptr<_detail::LinkImpl> m_link;
    _internal::MessageSenderEvents* m_events;
    Models::_internal::AmqpError m_savedMessageError;

    // Every send that waits now. The connection lock guards the map and the id.
    std::map<std::uint64_t, std::shared_ptr<SendOperation>> m_pendingSends;
    std::uint64_t m_nextSendId{0};

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
