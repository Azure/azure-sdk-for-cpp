// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// cspell: words amqpmessagereceiver amqpmessagereceiveroptions amqp Amqp

//// Enable declaration of strerror_s.
// #define __STDC_WANT_LIB_EXT1__ 1

#include "azure/core/amqp/internal/message_receiver.hpp"

#include "../../../models/private/message_impl.hpp"
#include "../../../models/private/source_impl.hpp"
#include "../../../models/private/target_impl.hpp"
#include "../../../models/private/value_impl.hpp"
#include "../../../models/private/error_impl.hpp"
#include "azure/core/amqp/internal/link.hpp"
#include "azure/core/amqp/internal/models/messaging_values.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"
#include "private/message_receiver_impl.hpp"
#include "private/session_impl.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/platform.hpp>

#include <memory>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;
using namespace Azure::Core::Amqp::_internal;
using namespace Azure::Core::Amqp::Common::_detail;

using namespace Azure::Core::Amqp::RustInterop::_detail;

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  void UniqueHandleHelper<RustInterop::_detail::RustAmqpMessageReceiver>::FreeMessageReceiver(
      RustInterop::_detail::RustAmqpMessageReceiver* value)
  {
    amqpmessagereceiver_destroy(value);
  }

  template <> struct UniqueHandleHelper<RustInterop::_detail::RustAmqpMessageReceiverOptions>
  {
    static void FreeMessageReceiverOptions(
        RustInterop::_detail::RustAmqpMessageReceiverOptions* obj);

    using type = Core::_internal::BasicUniqueHandle<
        RustInterop::_detail::RustAmqpMessageReceiverOptions,
        FreeMessageReceiverOptions>;
  };

  void UniqueHandleHelper<RustInterop::_detail::RustAmqpMessageReceiverOptions>::
      FreeMessageReceiverOptions(RustInterop::_detail::RustAmqpMessageReceiverOptions* value)
  {
    amqpmessagereceiveroptions_destroy(value);
  }

}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  using UniqueMessageReceiverOptions
      = UniqueHandle<RustInterop::_detail::RustAmqpMessageReceiverOptions>;

  /** Configure the MessageReceiver for receiving messages from a service instance.
   */
  MessageReceiverImpl::MessageReceiverImpl(
      std::shared_ptr<_detail::SessionImpl> session,
      Models::_internal::MessageSource const& source,
      MessageReceiverOptions const& options)
      : m_receiver{amqpmessagereceiver_create()}, m_options{options}, m_source{source},
        m_session{session}
  {
  }

  std::pair<std::shared_ptr<Models::AmqpMessage>, Models::_internal::AmqpError>
  MessageReceiverImpl::WaitForIncomingMessage(Context const& context)
  {
    Common::_detail::CallContext callContext(
        Common::_detail::GlobalStateHolder::GlobalStateInstance()->GetRuntimeContext(), context);

    auto message = Models::_detail::AmqpMessageFactory::FromImplementation(
        amqpmessagereceiver_receive_message(callContext.GetCallContext(), m_receiver.get()));

    return std::make_pair(message, Models::_internal::AmqpError{});
  }
 
  std::pair<std::shared_ptr<Models::AmqpMessage>, Models::_internal::AmqpError>
  MessageReceiverImpl::TryWaitForIncomingMessage()
  {
    throw std::runtime_error("Not implemented");
  }

  MessageReceiverImpl::~MessageReceiverImpl() noexcept
  {
    auto lock{m_session->GetConnection()->Lock()};

    if (m_receiverOpen)
    {
      AZURE_ASSERT_MSG(m_receiverOpen, "MessageReceiverImpl is being destroyed while open.");
      Azure::Core::_internal::AzureNoReturnPath(
          "MessageReceiverImpl is being destroyed while open.");
    }
  }

  // Nullable<uint32_t> InitialDeliveryCount;

  // Nullable<uint64_t> MaxMessageSize;
  // uint32_t MaxLinkCredit{};

  // Models::AmqpMap Properties;

  void MessageReceiverImpl::Open(Azure::Core::Context const& context)
  {
    if (m_options.AuthenticationRequired)
    {
      m_session->GetConnection()->AuthenticateAudience(
          m_session, static_cast<std::string>(m_source.GetAddress()), context);
    }

    UniqueMessageReceiverOptions options{amqpmessagereceiveroptions_create()};

    InvokeAmqpApi(amqpmessagereceiveroptions_set_name, options, m_options.Name.c_str());

    RustReceiverSettleMode settleMode;
    switch (m_options.SettleMode)
    {
      case ReceiverSettleMode::First:
        settleMode = RustReceiverSettleMode::First;
        break;
      case ReceiverSettleMode::Second:
        settleMode = RustReceiverSettleMode::Second;
        break;
    }
    InvokeAmqpApi(amqpmessagereceiveroptions_set_receiver_settle_mode, options, settleMode);
    InvokeAmqpApi(
        amqpmessagereceiveroptions_set_target,
        options,
        Models::_detail::AmqpTargetFactory::ToImplementation(m_options.MessageTarget));
    InvokeAmqpApi(
        amqpmessagereceiveroptions_set_properties,
        options,
        Models::_detail::AmqpValueFactory::ToImplementation(m_options.Properties.AsAmqpValue()));

    Common::_detail::CallContext callContext(
        Common::_detail::GlobalStateHolder::GlobalStateInstance()->GetRuntimeContext(), context);

    if (amqpmessagereceiver_attach(
            callContext.GetCallContext(),
            m_receiver.get(),
            m_session->GetAmqpSession().get(),
            Models::_detail::AmqpSourceFactory::ToImplementation(m_source),
            options.get()))
    {
      throw std::runtime_error("Failed to attach message receiver: " + callContext.GetError());
    }
    else
    {
      m_receiverOpen = true;
    }
  }
  void MessageReceiverImpl::Close(Context const& context)
  {
    Common::_detail::CallContext callContext(
        Common::_detail::GlobalStateHolder::GlobalStateInstance()->GetRuntimeContext(), context);

    // Even if the detach fails, we still want to consider the receiver closed.
    m_receiverOpen = false;
    if (amqpmessagereceiver_detach_and_release(callContext.GetCallContext(), m_receiver.release()))
    {
      throw std::runtime_error("Failed to detach message receiver: " + callContext.GetError());
    }
  }
}}}} // namespace Azure::Core::Amqp::_detail
