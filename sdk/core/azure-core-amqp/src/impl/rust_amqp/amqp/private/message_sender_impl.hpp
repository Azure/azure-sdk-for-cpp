// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/internal/message_sender.hpp"
#include "rust_amqp_wrapper.h"
#include "unique_handle.hpp"

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  template <>
  struct UniqueHandleHelper<Azure::Core::Amqp::RustInterop::_detail::RustAmqpMessageSender>
  {
    static void FreeMessageSender(
        Azure::Core::Amqp::RustInterop::_detail::RustAmqpMessageSender* value);

    using type = Core::_internal::BasicUniqueHandle<
        Azure::Core::Amqp::RustInterop::_detail::RustAmqpMessageSender,
        FreeMessageSender>;
  };
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  using UniqueMessageSender = UniqueHandle<Azure::Core::Amqp::RustInterop::_detail::RustAmqpMessageSender>;

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
        _internal::MessageSenderOptions const& options);
    virtual ~MessageSenderImpl() noexcept;

    MessageSenderImpl(MessageSenderImpl const&) = delete;
    MessageSenderImpl& operator=(MessageSenderImpl const&) = delete;
    MessageSenderImpl(MessageSenderImpl&&) noexcept = delete;
    MessageSenderImpl& operator=(MessageSenderImpl&&) noexcept = delete;

    Models::_internal::AmqpError Open(Context const& context);
    void Close(Context const& context);
    Models::_internal::AmqpError Send(Models::AmqpMessage const& message, Context const& context);

    std::uint64_t GetMaxMessageSize() const;

  private:
    bool m_senderOpen{false};
    UniqueMessageSender m_messageSender{};

    std::shared_ptr<_detail::SessionImpl> m_session;
    Models::_internal::MessageTarget m_target;
    _internal::MessageSenderOptions m_options;
  };
}}}} // namespace Azure::Core::Amqp::_detail
