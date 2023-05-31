// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/session.hpp"

#include "azure/core/amqp/connection.hpp"
#include "azure/core/amqp/link.hpp"
#include "private/claims_based_security_impl.hpp"
#include "private/connection_impl.hpp"
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

  Session::Session(
      Connection const& parentConnection,
      Endpoint& newEndpoint,
      SessionOptions const& options,
      SessionEvents* eventHandler)
      : m_impl{std::make_shared<_detail::SessionImpl>(
          _detail::ConnectionFactory::GetImpl(parentConnection),
          newEndpoint,
          options,
          eventHandler)}
  {
  }

  Session::Session(
      Connection const& parentConnection,
      std::shared_ptr<Credentials::TokenCredential> tokenCredential,
      SessionOptions const& options,
      SessionEvents* eventHandler)
      : m_impl{std::make_shared<_detail::SessionImpl>(
          _detail::ConnectionFactory::GetImpl(parentConnection),
          tokenCredential,
          options,
          eventHandler)}
  {
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
      std::shared_ptr<Credentials::TokenCredential> tokenCredential,
      _internal::SessionOptions const& options,
      _internal::SessionEvents* eventHandler)
      : m_connectionToPoll(connection),
        m_session{session_create(*connection, SessionImpl::OnLinkAttachedFn, this)},
        m_options{options}, m_eventHandler{eventHandler}, m_credential{tokenCredential}

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
    if (m_credential)
    {
      bool isSasToken
          = m_credential->GetCredentialName() == "ServiceBusSasConnectionStringCredential";
      Credentials::TokenRequestContext requestContext;
      if (isSasToken)
      {
        requestContext.MinimumExpiration = std::chrono::minutes(60);
      }
      requestContext.Scopes = m_options.AuthenticationScopes;
      std::string tokenAudience = audience;
      // If the caller provided a URL, use that url, otherwise build the URL to be used.
      if ((audience.find("amqps://") != 0) && (audience.find("amqp://") != 0))
      {
        tokenAudience = "amqps://" + m_connectionToPoll->GetHost() + "/" + audience;
      }
      Authenticate(isSasToken, requestContext, tokenAudience, context);
    }
  }
  void SessionImpl::Authenticate(
      bool isSasToken,
      Credentials::TokenRequestContext const& tokenRequestContext,
      std::string const& audience,
      Context const& context)
  {
    if (m_credential)
    {
      m_claimsBasedSecurity = std::make_shared<ClaimsBasedSecurityImpl>(shared_from_this());
      auto accessToken = m_credential->GetToken(tokenRequestContext, context);
      Log::Write(
          Logger::Level::Informational,
          "Authenticate with audience: " + audience + ", token: " + accessToken.Token);
      m_claimsBasedSecurity->SetTrace(true);
      if (m_claimsBasedSecurity->Open(context) == CbsOpenResult::Ok)
      {
        m_cbsOpen = true;
        auto result = m_claimsBasedSecurity->PutToken(
            (isSasToken ? CbsTokenType::Sas : CbsTokenType::Jwt),
            audience,
            accessToken.Token,
            context);
        if (std::get<0>(result) == CbsOperationResult::Ok)
        {
          m_tokenStore.emplace(std::make_pair(audience, accessToken));
        }
      }
      else
      {
        throw std::runtime_error("Could not put Claims Based Security token."); // LCOV_EXCL_LINE
      }
    }
  }

  std::string SessionImpl::GetSecurityToken(std::string const& audience) const
  {
    if (m_credential)
    {
      if (m_tokenStore.find(audience) == m_tokenStore.end())
      {
        Credentials::TokenRequestContext requestContext;
        bool isSasToken
            = m_credential->GetCredentialName() == "ServiceBusSasConnectionStringCredential";
        if (isSasToken)
        {
          requestContext.MinimumExpiration = std::chrono::minutes(60);
        }
        requestContext.Scopes = m_options.AuthenticationScopes;
        return m_credential->GetToken(requestContext, {}).Token;
      }
      return m_tokenStore.at(audience).Token;
    }
    return "";
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
