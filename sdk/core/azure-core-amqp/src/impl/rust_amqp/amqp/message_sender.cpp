// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// cspell: words amqpmessagesender amqpmessagesenderoptions

#include "../../../models/private/error_impl.hpp"
#include "../../../models/private/message_impl.hpp"
#include "../../../models/private/target_impl.hpp"
#include "../../../models/private/value_impl.hpp"
#include "azure/core/amqp/internal/common/completion_operation.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"
#include "azure/core/internal/unique_handle.hpp"
#include "private/connection_impl.hpp"
#include "private/message_sender_impl.hpp"
#include "private/session_impl.hpp"
#include "rust_amqp_wrapper.h"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/platform.hpp>

using namespace Azure::Core::Amqp::_detail::RustInterop;

#include <memory>

using namespace Azure::Core::Diagnostics;
using namespace Azure::Core::Diagnostics::_internal;

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  void UniqueHandleHelper<RustAmqpMessageSender>::FreeMessageSender(RustAmqpMessageSender* value)
  {
    amqpmessagesender_destroy(value);
  }

  template <> struct UniqueHandleHelper<RustInterop::RustAmqpSenderOptions>
  {
    static void FreeSenderOptions(RustInterop::RustAmqpSenderOptions* value)
    {
      amqpmessagesenderoptions_destroy(value);
    }

    using type
        = Core::_internal::BasicUniqueHandle<RustInterop::RustAmqpSenderOptions, FreeSenderOptions>;
  };
  template <> struct UniqueHandleHelper<RustInterop::RustAmqpSenderOptionsBuilder>
  {
    static void FreeSenderOptionsBuilder(RustInterop::RustAmqpSenderOptionsBuilder* value)
    {
      amqpmessagesenderoptions_builder_destroy(value);
    }

    using type = Core::_internal::
        BasicUniqueHandle<RustInterop::RustAmqpSenderOptionsBuilder, FreeSenderOptionsBuilder>;
  };

}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  using UniqueSenderOptions = UniqueHandle<RustInterop::RustAmqpSenderOptions>;
  using UniqueSenderOptionsBuilder = UniqueHandle<RustInterop::RustAmqpSenderOptionsBuilder>;

  MessageSenderImpl::MessageSenderImpl(
      std::shared_ptr<_detail::SessionImpl> session,
      Models::_internal::MessageTarget const& target,
      _internal::MessageSenderOptions const& options)
      : m_session{session}, m_target{target}, m_options{options},
        m_messageSender{amqpmessagesender_create()}
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

    UniqueSenderOptionsBuilder optionsBuilder{amqpmessagesenderoptions_builder_create()};

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

    UniqueSenderOptions options{amqpmessagesenderoptions_builder_build(optionsBuilder.get())};

    if (amqpmessagesender_attach(
            callContext.GetCallContext(),
            m_messageSender.get(),
            m_session->GetAmqpSession().get(),
            m_options.Name.c_str(),
            Models::_detail::AmqpTargetFactory::ToImplementation(m_target),
            options.get()))
    {
      throw std::runtime_error("Could not open Message Sender: " + callContext.GetError());
    }
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
      m_senderOpen = false;
    }
    (void)context;
  }

  Models::_internal::AmqpError MessageSenderImpl::Send(
      Models::AmqpMessage const& message,
      Context const& context)
  {
    throw std::runtime_error("Not implemented");
    (void)context;
    (void)message;
  }

}}}} // namespace Azure::Core::Amqp::_detail
