// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Enable declaration of strerror_s.
#define __STDC_WANT_LIB_EXT1__ 1

#include "azure/core/amqp/internal/message_receiver.hpp"

#include "../../../models/private/message_impl.hpp"
#include "../../../models/private/value_impl.hpp"
#include "azure/core/amqp/internal/link.hpp"
#include "azure/core/amqp/internal/models/messaging_values.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"
#include "private/message_receiver_impl.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/platform.hpp>

#include <memory>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;
using namespace Azure::Core::Amqp::_internal;

using namespace Azure::Core::Amqp::RustInterop::_detail;

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  void UniqueHandleHelper<RustInterop::_detail::RustAmqpMessageReceiver>::FreeMessageReceiver(
      RustInterop::_detail::RustAmqpMessageReceiver* value)
  {
    amqpmessagereceiver_destroy(value);
  }
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  /** Configure the MessageReceiver for receiving messages from a service instance.
   */
  MessageReceiverImpl::MessageReceiverImpl(
      std::shared_ptr<_detail::SessionImpl> session,
      Models::_internal::MessageSource const& source,
      MessageReceiverOptions const& options)
      : m_options{options}, m_source{source}, m_session{session}
  {
  }

  std::pair<std::shared_ptr<Models::AmqpMessage>, Models::_internal::AmqpError>
  MessageReceiverImpl::WaitForIncomingMessage(Context const& context)
  {
    throw std::runtime_error("Not implemented");
    (void)context;
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
    if (m_link)
    {
      m_link.reset();
    }
    m_messageQueue.Clear();
  }

  void MessageReceiverImpl::Open(Azure::Core::Context const& context)
  {
    if (m_options.AuthenticationRequired)
    {
      m_session->GetConnection()->AuthenticateAudience(
          m_session, static_cast<std::string>(m_source.GetAddress()), context);
    }
    throw std::runtime_error("Not yet implemented");
  }

  void MessageReceiverImpl::Close(Context const& context)
  {
    throw std::runtime_error("Not yet implemented");
    (void)context;
  }

}}}} // namespace Azure::Core::Amqp::_detail
