// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Enable declaration of strerror_s.
#define __STDC_WANT_LIB_EXT1__ 1

#include "azure/core/amqp/internal/message_receiver.hpp"

#include "../models/private/message_impl.hpp"
#include "../models/private/value_impl.hpp"
#include "azure/core/amqp/internal/link.hpp"
#include "azure/core/amqp/internal/models/messaging_values.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"
#include "message_receiver_impl.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/platform.hpp>

#if ENABLE_UAMQP
#include <azure_uamqp_c/message_receiver.h>
#endif

#include <memory>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;
using namespace Azure::Core::Amqp::_internal;

namespace Azure { namespace Core { namespace Amqp { namespace _internal {

  std::ostream& operator<<(std::ostream& stream, ReceiverSettleMode settleMode)
  {
    switch (settleMode)
    {
      case ReceiverSettleMode::First:
        stream << "First";
        break;
      case ReceiverSettleMode::Second:
        stream << "Second";
        break;
    }
    return stream;
  }

  MessageReceiver::~MessageReceiver() noexcept {}

  void MessageReceiver::Open(Context const& context)
  {
    if (m_impl)
    {
      m_impl->Open(context);
    }
    else
    {
      AZURE_ASSERT_FALSE("MessageReceiver::Open called on moved message receiver.");
    }
  }
  void MessageReceiver::Close(Context const& context)
  {
    if (m_impl)
    {
      m_impl->Close(context);
    }
  }
  std::string MessageReceiver::GetSourceName() const { return m_impl->GetSourceName(); }
  std::pair<std::shared_ptr<const Models::AmqpMessage>, Models::_internal::AmqpError>
  MessageReceiver::WaitForIncomingMessage(Azure::Core::Context const& context)
  {
    if (m_impl)
    {
      return m_impl->WaitForIncomingMessage(context);
    }
    else
    {
      AZURE_ASSERT_FALSE(
          "MessageReceiver::WaitForIncomingMessage called on moved message receiver.");
      Azure::Core::_internal::AzureNoReturnPath(
          "MessageReceiver::WaitForIncomingMessage called on moved message receiver.");
    }
  }

  std::pair<std::shared_ptr<const Models::AmqpMessage>, Models::_internal::AmqpError>
  MessageReceiver::TryWaitForIncomingMessage()
  {
    if (m_impl)
    {
      return m_impl->TryWaitForIncomingMessage();
    }
    else
    {
      AZURE_ASSERT_FALSE(
          "MessageReceiver::TryWaitForIncomingMessage called on moved message receiver.");
      Azure::Core::_internal::AzureNoReturnPath(
          "MessageReceiver::TryWaitForIncomingMessage called on moved message receiver.");
    }
  }

  std::string MessageReceiver::GetLinkName() const { return m_impl->GetLinkName(); }
  std::ostream& operator<<(std::ostream& stream, _internal::MessageReceiverState state)
  {
    switch (state)
    {
      case _internal::MessageReceiverState::Invalid:
        stream << "Invalid";
        break;
      case _internal::MessageReceiverState::Idle:
        stream << "Idle";
        break;
      case _internal::MessageReceiverState::Opening:
        stream << "Opening";
        break;
      case _internal::MessageReceiverState::Open:
        stream << "Open";
        break;
      case _internal::MessageReceiverState::Closing:
        stream << "Closing";
        break;
      case _internal::MessageReceiverState::Error:
        stream << "Error";
        break;
    }
    return stream;
  }

#if defined(_azure_TESTING_BUILD)
#if ENABLE_UAMQP
  void MessageReceiver::EnableLinkPolling()
  {
    if (m_impl)
    {
      m_impl->EnableLinkPolling();
    }
    else
    {
      AZURE_ASSERT_FALSE("MessageReceiver::EnableLinkPolling called on moved message receiver.");
    }
  }
#endif
#endif

}}}} // namespace Azure::Core::Amqp::_internal
