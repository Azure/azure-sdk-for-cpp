// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/internal/message_receiver.hpp"
#if ENABLE_UAMQP
#include "link_impl.hpp"
#endif
#include "rust_amqp_wrapper.h"
#include "session_impl.hpp"
#include "unique_handle.hpp"

#include <memory>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  template <> struct UniqueHandleHelper<RustInterop::_detail::RustAmqpMessageReceiver>
  {
    static void FreeMessageReceiver(RustInterop::_detail::RustAmqpMessageReceiver* obj);

    using type = Core::_internal::
        BasicUniqueHandle<RustInterop::_detail::RustAmqpMessageReceiver, FreeMessageReceiver>;
  };
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  using UniqueMessageReceiver = UniqueHandle<RustInterop::_detail::RustAmqpMessageReceiver>;
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
        _internal::MessageReceiverOptions const& options);
    ~MessageReceiverImpl() noexcept;

    MessageReceiverImpl(MessageReceiverImpl const&) = delete;
    MessageReceiverImpl& operator=(MessageReceiverImpl const&) = delete;
    MessageReceiverImpl(MessageReceiverImpl&&) = delete;
    MessageReceiverImpl& operator=(MessageReceiverImpl&&) = delete;
    /** Open the receiver. */
    void Open(Context const& context);
    void Close(Context const& context);
    std::string GetSourceName() const { return static_cast<std::string>(m_source.GetAddress()); }

    std::pair<std::shared_ptr<Models::AmqpMessage>, Models::_internal::AmqpError>
    WaitForIncomingMessage(Context const& context);

    std::pair<std::shared_ptr<Models::AmqpMessage>, Models::_internal::AmqpError>
    TryWaitForIncomingMessage();

  private:
    bool m_receiverOpen{false};
    std::shared_ptr<_detail::LinkImpl> m_link;
    _internal::MessageReceiverOptions m_options;
    Models::_internal::MessageSource m_source;
    std::shared_ptr<_detail::SessionImpl> m_session;
    bool m_deferLinkPolling{false};

    bool m_linkPollingEnabled{false};
    std::mutex m_mutableState;
  };
}}}} // namespace Azure::Core::Amqp::_detail
