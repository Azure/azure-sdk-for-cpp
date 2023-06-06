// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/session.hpp"

#include "azure/core/amqp/connection.hpp"
#include "azure/core/amqp/link.hpp"
#include "private/claims_based_security_impl.hpp"
#include "private/connection_impl.hpp"
#include "private/management_impl.hpp"
#include "private/message_receiver_impl.hpp"
#include "private/message_sender_impl.hpp"
#include "private/session_impl.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

#include <azure_uamqp_c/session.h>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

void Azure::Core::_internal::UniqueHandleHelper<SESSION_INSTANCE_TAG>::FreeAmqpSession(
    SESSION_HANDLE value)
{
  session_destroy(value);
}

namespace Azure { namespace Core { namespace Amqp { namespace _internal {
  Endpoint::~Endpoint()
  {
    if (m_endpoint)
    {
      connection_destroy_endpoint(m_endpoint);
    }
  }

  Session::~Session() noexcept {}

  uint32_t Session::GetIncomingWindow() const { return m_impl->GetIncomingWindow(); }
  uint32_t Session::GetOutgoingWindow() const { return m_impl->GetOutgoingWindow(); }
  uint32_t Session::GetHandleMax() const { return m_impl->GetHandleMax(); }

  void Session::Begin() { m_impl->Begin(); }
  void Session::End(std::string const& condition_value, std::string const& description)
  {
    m_impl->End(condition_value, description);
  }

  MessageSender Session::CreateMessageSender(
      Models::_internal::MessageTarget const& target,
      MessageSenderOptions const& options,
      MessageSenderEvents* events) const
  {
    return _detail::MessageSenderFactory::CreateFromInternal(
        std::make_shared<_detail::MessageSenderImpl>(m_impl, target, options, events));
  }
  MessageSender Session::CreateMessageSender(
      LinkEndpoint& endpoint,
      Models::_internal::MessageTarget const& target,
      MessageSenderOptions const& options,
      MessageSenderEvents* events) const
  {
    return _detail::MessageSenderFactory::CreateFromInternal(
        std::make_shared<_detail::MessageSenderImpl>(m_impl, endpoint, target, options, events));
  }

  MessageReceiver Session::CreateMessageReceiver(
      Models::_internal::MessageSource const& receiverSource,
      MessageReceiverOptions const& options,
      MessageReceiverEvents* events) const
  {
    return _detail::MessageReceiverFactory::CreateFromInternal(
        std::make_shared<_detail::MessageReceiverImpl>(m_impl, receiverSource, options, events));
  }

  MessageReceiver Session::CreateMessageReceiver(
      LinkEndpoint& endpoint,
      Models::_internal::MessageSource const& receiverSource,
      MessageReceiverOptions const& options,
      MessageReceiverEvents* events) const
  {
    return _detail::MessageReceiverFactory::CreateFromInternal(
        std::make_shared<_detail::MessageReceiverImpl>(
            m_impl, endpoint, receiverSource, options, events));
  }

  ManagementClient Session::CreateManagementClient(
      std::string const& entityPath,
      ManagementClientOptions const& options,
      ManagementClientEvents* events) const
  {
    return _detail::ManagementClientFactory::CreateFromInternal(
        std::make_shared<_detail::ManagementClientImpl>(m_impl, entityPath, options, events));
  }

}}}} // namespace Azure::Core::Amqp::_internal

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  SessionImpl::~SessionImpl() noexcept {}

  SessionImpl::SessionImpl(
      std::shared_ptr<_detail::ConnectionImpl> connection,
      _internal::Endpoint& endpoint,
      _internal::SessionOptions const& options,
      _internal::SessionEvents* eventHandler)
      : m_connectionToPoll(connection), m_session{session_create_from_endpoint(
                                            *connection,
                                            EndpointFactory::Release(endpoint),
                                            SessionImpl::OnLinkAttachedFn,
                                            this)},
        m_options{options}, m_eventHandler{eventHandler}
  {
    if (options.MaximumLinkCount.HasValue())
    {
      if (session_set_handle_max(m_session.get(), options.MaximumLinkCount.Value()))
      {
        throw std::runtime_error("Could not set handle max."); // LCOV_EXCL_LINE
      }
    }
    if (options.InitialIncomingWindowSize.HasValue())
    {
      if (session_set_incoming_window(m_session.get(), options.InitialIncomingWindowSize.Value()))
      {
        throw std::runtime_error("Could not set incoming window"); // LCOV_EXCL_LINE
      }
    }
    if (options.InitialOutgoingWindowSize.HasValue())
    {
      if (session_set_outgoing_window(m_session.get(), options.InitialOutgoingWindowSize.Value()))
      {
        throw std::runtime_error("Could not set outgoing window"); // LCOV_EXCL_LINE
      }
    }
  }

  SessionImpl::SessionImpl(
      std::shared_ptr<_detail::ConnectionImpl> connection,
      _internal::SessionOptions const& options,
      _internal::SessionEvents* eventHandler)
      : m_connectionToPoll(connection),
        m_session{session_create(*connection, SessionImpl::OnLinkAttachedFn, this)},
        m_options{options}, m_eventHandler{eventHandler}

  {
    if (options.MaximumLinkCount.HasValue())
    {
      if (session_set_handle_max(m_session.get(), options.MaximumLinkCount.Value()))
      {
        throw std::runtime_error("Could not set handle max."); // LCOV_EXCL_LINE
      }
    }
    if (options.InitialIncomingWindowSize.HasValue())
    {
      if (session_set_incoming_window(m_session.get(), options.InitialIncomingWindowSize.Value()))
      {
        throw std::runtime_error("Could not set incoming window"); // LCOV_EXCL_LINE
      }
    }
    if (options.InitialOutgoingWindowSize.HasValue())
    {
      if (session_set_outgoing_window(m_session.get(), options.InitialOutgoingWindowSize.Value()))
      {
        throw std::runtime_error("Could not set outgoing window"); // LCOV_EXCL_LINE
      }
    }
  }

  uint32_t SessionImpl::GetIncomingWindow()
  {
    uint32_t window;
    if (session_get_incoming_window(m_session.get(), &window))
    {
      throw std::runtime_error("Could not get incoming window"); // LCOV_EXCL_LINE
    }
    return window;
  }

  uint32_t SessionImpl::GetOutgoingWindow()
  {
    uint32_t window;
    if (session_get_outgoing_window(m_session.get(), &window))
    {
      throw std::runtime_error("Could not get outgoing window"); // LCOV_EXCL_LINE
    }
    return window;
  }

  uint32_t SessionImpl::GetHandleMax()
  {
    uint32_t max;
    if (session_get_handle_max(m_session.get(), &max))
    {
      throw std::runtime_error("Could not get handle max."); // LCOV_EXCL_LINE
    }
    return max;
  }

  void SessionImpl::Begin()
  {
    if (session_begin(m_session.get()))
    {
      throw std::runtime_error("Could not begin session"); // LCOV_EXCL_LINE
    }
  }
  void SessionImpl::End(const std::string& condition, const std::string& description)
  {
    // When we end the session, it clears all the links, so we need to ensure that the
    //    m_newLinkAttachedQueue.Clear();
    if (session_end(
            m_session.get(),
            condition.empty() ? nullptr : condition.c_str(),
            description.empty() ? nullptr : description.c_str()))
    {
      throw std::runtime_error("Could not begin session"); // LCOV_EXCL_LINE
    }
  }

  void SessionImpl::AuthenticateIfNeeded(std::string const& audience, Context const& context)
  {
    if (GetConnection()->GetCredential())
    {
      std::string tokenAudience = audience;
      // If the caller provided a URL, use that url, otherwise build the URL to be used.
      if ((audience.find("amqps://") != 0) && (audience.find("amqp://") != 0))
      {
        tokenAudience = "amqps://" + m_connectionToPoll->GetHost() + "/" + audience;
      }
      Authenticate(tokenAudience, context);
    }
  }
  void SessionImpl::Authenticate(std::string const& audience, Context const& context)
  {
    if (!m_claimsBasedSecurity)
    {
      m_claimsBasedSecurity = std::make_shared<ClaimsBasedSecurityImpl>(shared_from_this());
    }
    auto accessToken = GetConnection()->GetSecurityToken(audience, context);
    Log::Stream(Logger::Level::Informational)
        << "Authenticate with audience: " << audience << ", token: " << accessToken << std::endl;

    m_claimsBasedSecurity->SetTrace(GetConnection()->EnableTrace());
    if (!m_cbsOpen)
    {
      auto cbsOpenStatus = m_claimsBasedSecurity->Open(context);
      if (cbsOpenStatus == CbsOpenResult::Ok)
      {
        m_cbsOpen = true;
      }
      else
      {
        throw std::runtime_error("Could not open Claims Based Security object."); // LCOV_EXCL_LINE
      }
    }
    auto result = m_claimsBasedSecurity->PutToken(
        (GetConnection()->IsSasCredential() ? CbsTokenType::Sas : CbsTokenType::Jwt),
        audience,
        accessToken,
        context);
    if (std::get<0>(result) != CbsOperationResult::Ok)
    {
      throw std::runtime_error("Could not put Claims Based Security token."); // LCOV_EXCL_LINE
    }
  }

  bool SessionImpl::OnLinkAttachedFn(
      void* context,
      LINK_ENDPOINT_INSTANCE_TAG* newLinkEndpoint,
      const char* name,
      bool role,
      AMQP_VALUE_DATA_TAG* source,
      AMQP_VALUE_DATA_TAG* target,
      AMQP_VALUE_DATA_TAG* properties)
  {
    SessionImpl* session = static_cast<SessionImpl*>(context);
    _internal::LinkEndpoint linkEndpoint(LinkEndpointFactory::CreateLinkEndpoint(newLinkEndpoint));
    if (session->m_eventHandler)
    {
      return session->m_eventHandler->OnLinkAttached(
          _detail::SessionFactory::CreateFromInternal(session->shared_from_this()),
          linkEndpoint,
          name,
          role == role_receiver ? Azure::Core::Amqp::_internal::SessionRole::Receiver
                                : Azure::Core::Amqp::_internal::SessionRole::Sender,
          source,
          target,
          properties);
    }
    else
    {
      // Even if we don't have any actions to take, if we return false, the connection will be
      // aborted.
      return true;
    }
    static_cast<void>(role);
  }

  _internal::Endpoint EndpointFactory::CreateEndpoint(ENDPOINT_HANDLE endpoint)
  {
    return _internal::Endpoint(endpoint);
  }
  _internal::LinkEndpoint LinkEndpointFactory::CreateLinkEndpoint(LINK_ENDPOINT_HANDLE endpoint)
  {
    return _internal::LinkEndpoint(endpoint);
  }

}}}} // namespace Azure::Core::Amqp::_detail
