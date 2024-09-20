// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Enable declaration of strerror_s.
#define __STDC_WANT_LIB_EXT1__ 1

#include "../../../models/private/error_impl.hpp"
#include "../../../models/private/message_impl.hpp"
#include "../../../models/private/value_impl.hpp"
#include "azure/core/amqp/internal/common/completion_operation.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"
#include "private/connection_impl.hpp"
#include "private/message_sender_impl.hpp"
#include "private/session_impl.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/platform.hpp>

#if ENABLE_UAMQP
#include <azure_uamqp_c/message_sender.h>
#endif

#include <memory>

using namespace Azure::Core::Diagnostics;
using namespace Azure::Core::Diagnostics::_internal;

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  void UniqueHandleHelper<MESSAGE_SENDER_INSTANCE_TAG>::FreeMessageSender(
      MESSAGE_SENDER_HANDLE value)
  {
    messagesender_destroy(value);
  }
#endif
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  MessageSenderImpl::MessageSenderImpl(
      std::shared_ptr<_detail::SessionImpl> session,
      Models::_internal::MessageTarget const& target,
      _internal::MessageSenderOptions const& options)
      : m_session{session}, m_target{target}, m_options{options}
  {
  }

  MessageSenderImpl::~MessageSenderImpl() noexcept
  {

    auto lock{m_session->GetConnection()->Lock()};
    if (m_senderOpen)
    {
      AZURE_ASSERT_MSG(m_senderOpen, "MessageSenderImpl is being destroyed while open.");
      Azure::Core::_internal::AzureNoReturnPath("MessageSenderImpl is being destroyed while open.");
    }

    if (m_link)
    {

      m_link.reset();
    }
  }

  void MessageSenderImpl::CreateLink()
  {
    m_link = std::make_shared<_detail::LinkImpl>(
        m_session,
        m_options.Name,
        _internal::SessionRole::Sender, // This is the role of the link, not the endpoint.
        m_options.MessageSource,
        m_target);
    PopulateLinkProperties();
  }

  /* Populate link properties from options. */
  void MessageSenderImpl::PopulateLinkProperties()
  {
    // Populate link options from options.
    if (m_options.InitialDeliveryCount.HasValue())
    {
      m_link->SetInitialDeliveryCount(m_options.InitialDeliveryCount.Value());
    }
    if (m_options.MaxMessageSize.HasValue())
    {
      m_link->SetMaxMessageSize(m_options.MaxMessageSize.Value());
    }
    else
    {
      m_link->SetMaxMessageSize((std::numeric_limits<uint64_t>::max)());
    }
    if (m_options.MaxLinkCredits != 0)
    {
      m_link->SetMaxLinkCredit(m_options.MaxLinkCredits);
    }
    m_link->SetSenderSettleMode(m_options.SettleMode);
  }

  std::uint64_t MessageSenderImpl::GetMaxMessageSize() const
  {
    if (!m_senderOpen)
    {
      throw std::runtime_error("Message sender is not open.");
    }
    // Get the max message size from the link (which is the max frame size for the link
    // endpoint) and the peer (which is the max frame size for the other end of the connection).
    //
    auto linkSize{m_link->GetMaxMessageSize()};
    auto peerSize{m_link->GetPeerMaxMessageSize()};

    // Return the smaller of the two values
    return (std::min)(linkSize, peerSize);
  }

  Models::_internal::AmqpError MessageSenderImpl::Open(bool halfOpen, Context const& context)
  {
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
    throw std::runtime_error("Not implemented");
    (void)halfOpen;
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
    throw std::runtime_error("Not implemented");
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

  std::string MessageSenderImpl::GetLinkName() const { return m_link->GetName(); }

}}}} // namespace Azure::Core::Amqp::_detail
