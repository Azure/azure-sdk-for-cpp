// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Enable declaration of strerror_s.
#define __STDC_WANT_LIB_EXT1__ 1

#include "../models/private/error_impl.hpp"
#include "../models/private/message_impl.hpp"
#include "../models/private/value_impl.hpp"
#include "azure/core/amqp/internal/common/completion_operation.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"
#include "connection_impl.hpp"
#include "message_sender_impl.hpp"
#include "session_impl.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/platform.hpp>

#include <memory>

using namespace Azure::Core::Diagnostics;
using namespace Azure::Core::Diagnostics::_internal;

namespace Azure { namespace Core { namespace Amqp { namespace _internal {
  std::ostream& operator<<(std::ostream& stream, SenderSettleMode settleMode)
  {
    switch (settleMode)
    {
      case SenderSettleMode::Settled:
        stream << "Settled";
        break;
      case SenderSettleMode::Unsettled:
        stream << "Unsettled";
        break;
      case SenderSettleMode::Mixed:
        stream << "Mixed";
        break;
    }
    return stream;
  }

  Models::_internal::AmqpError MessageSender::Open(Context const& context)
  {
    return m_impl->Open(false, context);
  }

  Models::_internal::AmqpError MessageSender::HalfOpen(Context const& context)
  {
    return m_impl->Open(true, context);
  }

  void MessageSender::Close(Context const& context) { m_impl->Close(context); }
  #if ENABLE_UAMQP
  std::tuple<MessageSendStatus, Models::_internal::AmqpError> MessageSender::Send(
      Models::AmqpMessage const& message,
      Context const& context)
  {
    return m_impl->Send(message, context);
  }
  #elif ENABLE_RUST_AMQP
  Models::_internal::AmqpError MessageSender::Send(
      Models::AmqpMessage const& message,
      Context const& context)
  {
    return m_impl->Send(message, context);
  }
#endif

  std::uint64_t MessageSender::GetMaxMessageSize() const { return m_impl->GetMaxMessageSize(); }
  std::string MessageSender::GetLinkName() const { return m_impl->GetLinkName(); }
  MessageSender::~MessageSender() noexcept {}

}}}} // namespace Azure::Core::Amqp::_internal
