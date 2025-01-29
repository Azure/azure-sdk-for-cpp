// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// cspell: words amqpmessagesender amqpmessagesenderoptions

#include "../../../models/private/error_impl.hpp"
#include "../../../models/private/message_impl.hpp"
#include "../../../models/private/source_impl.hpp"
#include "../../../models/private/target_impl.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"
#include "azure/core/internal/unique_handle.hpp"
#include "private/connection_impl.hpp"
#include "private/message_sender_impl.hpp"
#include "private/session_impl.hpp"
#include "rust_amqp_wrapper.h"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

using namespace Azure::Core::Amqp::RustInterop::_detail;

#include <memory>

using namespace Azure::Core::Diagnostics;
using namespace Azure::Core::Diagnostics::_internal;

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  void UniqueHandleHelper<RustAmqpMessageSender>::FreeMessageSender(RustAmqpMessageSender* value)
  {
    amqpmessagesender_destroy(value);
  }

  template <> struct UniqueHandleHelper<RustAmqpSenderOptions>
  {
    static void FreeSenderOptions(RustAmqpSenderOptions* value)
    {
      amqpmessagesenderoptions_destroy(value);
    }

    using type = Core::_internal::BasicUniqueHandle<RustAmqpSenderOptions, FreeSenderOptions>;
  };

}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  using UniqueSenderOptions = UniqueHandle<RustAmqpSenderOptions>;

  MessageSenderImpl::MessageSenderImpl(
      std::shared_ptr<_detail::SessionImpl> session,
      Models::_internal::MessageTarget const& target,
      _internal::MessageSenderOptions const& options)
      : m_messageSender{amqpmessagesender_create()}, m_session{session}, m_target{target},
        m_options{options}
  {
  }

  MessageSenderImpl::~MessageSenderImpl() noexcept
  {
    if (m_senderOpen)
    {
      AZURE_ASSERT_MSG(m_senderOpen, "MessageSenderImpl is being destroyed while open.");
      Azure::Core::_internal::AzureNoReturnPath("MessageSenderImpl is being destroyed while open.");
    }
  }

  std::uint64_t MessageSenderImpl::GetMaxMessageSize() const
  {
    Common::_detail::CallContext callContext(
        Common::_detail::GlobalStateHolder::GlobalStateInstance()->GetRuntimeContext(), {});

    if (!m_senderOpen)
    {
      throw std::runtime_error("Message sender is not open.");
    }
    // Get the max message size from the link (which is the max frame size for the link
    // endpoint) and the peer (which is the max frame size for the other end of the connection).
    //
    uint64_t maxSize;
    if (amqpmessagesender_get_max_message_size(
            callContext.GetCallContext(), m_messageSender.get(), &maxSize))
    {
      throw std::runtime_error("Failed to get max message size: " + callContext.GetError());
    }
    return maxSize;
  }

  Models::_internal::AmqpError MessageSenderImpl::Open(Context const& context)
  {
    Common::_detail::CallContext callContext(
        Common::_detail::GlobalStateHolder::GlobalStateInstance()->GetRuntimeContext(), {});

    UniqueSenderOptions senderOptions{amqpmessagesenderoptions_create()};

    Models::_internal::AmqpError rv;
    if (m_options.EnableTrace)
    {
      Log::Stream(Logger::Level::Verbose)
          << "Opening message sender. Authenticate if needed with audience: " << m_target;
    }

    if (m_options.AuthenticationRequired)
    {
      // If we need to authenticate with either ServiceBus or BearerToken, now is the time to do
      // it.
      m_session->GetConnection()->AuthenticateAudience(
          m_session, static_cast<std::string>(m_target.GetAddress()), context);
    }

    if (m_options.InitialDeliveryCount)
    {
      Common::_detail::InvokeAmqpApi(
          amqpmessagesenderoptions_set_initial_delivery_count,
          senderOptions,
          m_options.InitialDeliveryCount.Value());
    }

    if (m_options.MaxLinkCredits)
    {
      //        optionsBuilder.reset(amqpmessagesenderoptions_builder_set_)
    }

    if (m_options.MaxMessageSize)
    {
      Common::_detail::InvokeAmqpApi(
          amqpmessagesenderoptions_set_max_message_size,
          senderOptions,
          m_options.MaxMessageSize.Value());
    }

    Common::_detail::InvokeAmqpApi(
        amqpmessagesenderoptions_set_source,
        senderOptions,
        Models::_detail::AmqpSourceFactory::ToImplementation(m_options.MessageSource));

    RustSenderSettleMode senderSettleMode = RustSenderSettleMode::Settled;
    switch (m_options.SettleMode)
    {
      case _internal::SenderSettleMode::Mixed:
        senderSettleMode = RustSenderSettleMode::Mixed;
        break;
      case _internal::SenderSettleMode::Settled:
        senderSettleMode = RustSenderSettleMode::Settled;
        break;
      case _internal::SenderSettleMode::Unsettled:
        senderSettleMode = RustSenderSettleMode::Unsettled;
        break;
    }

    Common::_detail::InvokeAmqpApi(
        amqpmessagesenderoptions_set_sender_settle_mode, senderOptions, senderSettleMode);

    if (amqpmessagesender_attach(
            callContext.GetCallContext(),
            m_messageSender.get(),
            m_session->GetAmqpSession().get(),
            m_options.Name.c_str(),
            Models::_detail::AmqpTargetFactory::ToImplementation(m_target),
            senderOptions.get()))
    {
      throw std::runtime_error("Could not open Message Sender: " + callContext.GetError());
    }
    m_senderOpen = true;
    return {};
  }

  void MessageSenderImpl::Close(Context const& context)
  {
    if (m_senderOpen)
    {
      if (m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose) << "Closing message sender.";
      }
      Common::_detail::CallContext callContext(
          Common::_detail::GlobalStateHolder::GlobalStateInstance()->GetRuntimeContext(), context);

      if (amqpmessagesender_detach_and_release(
              callContext.GetCallContext(), m_messageSender.release()))
      {
        throw std::runtime_error("Could not close Message Sender: " + callContext.GetError());
      }
      m_senderOpen = false;
    }
    else
    {
      throw std::runtime_error("Message sender is not open.");
    }
  }

  Models::_internal::AmqpError MessageSenderImpl::Send(
      Models::AmqpMessage const& message,
      Context const& context)
  {
    if (m_senderOpen)
    {
      Common::_detail::CallContext callContext(
          Common::_detail::GlobalStateHolder::GlobalStateInstance()->GetRuntimeContext(), context);

      RustAmqpSendOptions sendOptions{};

      if (message.MessageFormat != 0)
      {
        sendOptions.message_format = &message.MessageFormat;
      }

      if (amqpmessagesender_send(
              callContext.GetCallContext(),
              m_messageSender.get(),
              Models::_detail::AmqpMessageFactory::ToImplementation(message).get(),
              &sendOptions))
      {
        throw std::runtime_error("Could not send message: " + callContext.GetError());
      }
      return {};
    }
    else
    {
      throw std::runtime_error("Message sender is not open.");
    }
  }

}}}} // namespace Azure::Core::Amqp::_detail
