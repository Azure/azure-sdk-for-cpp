// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// cspell: words amqpconnection amqpconnectionoptions amqpconnectionoptionsbuilder

#include "azure/core/amqp/internal/connection.hpp"

#include "azure/core/amqp/internal/common/global_state.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"
#include "claims_based_security_impl.hpp"
#include "connection_impl.hpp"
#include "session_impl.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

#include <memory>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

namespace Azure { namespace Core { namespace Amqp { namespace _internal {

  // Create a connection with an existing networking Transport.
#if ENABLE_UAMQP
  Connection::Connection(
      Network::_internal::Transport const& transport,
      ConnectionOptions const& options,
      ConnectionEvents* eventHandler,
      ConnectionEndpointEvents* endpointEventHandler)
      : m_impl{std::make_shared<_detail::ConnectionImpl>(
          transport.GetImpl(),
          options,
          eventHandler,
          endpointEventHandler)}
  {
    m_impl->FinishConstruction();
  }
#endif

  // Create a connection with a request URI and options.
  Connection::Connection(
      std::string const& hostName,
      std::shared_ptr<Credentials::TokenCredential> credential,
      ConnectionOptions const& options
#if ENABLE_UAMQP
      ,
      ConnectionEvents* eventHandler
#endif
      )
      : m_impl
  {
    std::make_shared<_detail::ConnectionImpl>(
        hostName,
        credential,
        options
#if ENABLE_UAMQP
        ,
        eventHandler
#endif
    )
  }
  {
    m_impl->FinishConstruction();
  }

  Connection::~Connection() {}

  Session Connection::CreateSession(
      SessionOptions const& sessionOptions
#if ENABLE_UAMQP
      ,
      SessionEvents* sessionEvents
#endif
  ) const
  {
    return Azure::Core::Amqp::_detail::SessionFactory::CreateFromInternal(
        std::make_shared<_detail::SessionImpl>(
            m_impl,
            sessionOptions
#if ENABLE_UAMQP
            ,
            sessionEvents
#endif
            ));
  }

#if ENABLE_UAMQP
  Session Connection::CreateSession(
      Endpoint& endpoint,
      SessionOptions const& sessionOptions,
      SessionEvents* sessionEvents) const
  {
    return Azure::Core::Amqp::_detail::SessionFactory::CreateFromInternal(
        std::make_shared<_detail::SessionImpl>(m_impl, endpoint, sessionOptions, sessionEvents));
  }
  void Connection::Poll() { m_impl->Poll(); }

  void Connection::Listen() { m_impl->Listen(); }
#endif // ENABLE_UAMQP

  void Connection::Open(Azure::Core::Context const& context) { m_impl->Open(context); }
  void Connection::Close(Azure::Core::Context const& context) { m_impl->Close(context); }
  void Connection::Close(
      std::string const& condition,
      std::string const& description,
      Models::AmqpValue value,
      Azure::Core::Context const& context)
  {
    m_impl->Close(condition, description, value, context);
  }
  uint32_t Connection::GetMaxFrameSize() const { return m_impl->GetMaxFrameSize(); }
#if ENABLE_UAMQP
  uint32_t Connection::GetRemoteMaxFrameSize() const { return m_impl->GetRemoteMaxFrameSize(); }
#endif
  uint16_t Connection::GetMaxChannel() const { return m_impl->GetMaxChannel(); }
  std::string Connection::GetHost() const { return m_impl->GetHost(); }
  uint16_t Connection::GetPort() const { return m_impl->GetPort(); }
  std::chrono::milliseconds Connection::GetIdleTimeout() const { return m_impl->GetIdleTimeout(); }
  Models::AmqpMap Connection::GetProperties() const { return m_impl->GetProperties(); }
#if ENABLE_UAMQP
  void Connection::SetIdleEmptyFrameSendPercentage(double ratio)
  {
    m_impl->SetIdleEmptyFrameSendPercentage(ratio);
  }
#endif
}}}} // namespace Azure::Core::Amqp::_internal

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  bool ConnectionImpl::IsSasCredential() const
  {
    if (GetCredential())
    {
      return GetCredential()->GetCredentialName() == "ServiceBusSasConnectionStringCredential";
    }
    return false;
  }

  // Ensure that we have a token for the provided audience.
  // If we don't, authenticate the audience with the service using the provided session.
  // Note that the granularity of
  Credentials::AccessToken ConnectionImpl::AuthenticateAudience(
      std::shared_ptr<SessionImpl> session,
      std::string const& audience,
      Azure::Core::Context const& context)
  {
    if (GetCredential())
    {
      std::string audienceUrl = audience;
      if (m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose) << "Authenticate connection for audience " << audience;
      }
      // If the audience looks like a URL for AMQP, AMQPS, or SB, we can use the URL as
      // provided.
      if ((audience.find("amqps://") != 0) && (audience.find("amqp://") != 0)
          && (audience.find("sb://") != 0))
      {
        audienceUrl = "amqps://" + GetHost();
        // The provided audience may begin with a /, if not, we need to add the separator.
        if (audience.front() != '/')
        {
          audienceUrl += "/";
        }
        audienceUrl += audience;
        if (m_options.EnableTrace)
        {
          Log::Stream(Logger::Level::Verbose)
              << "Initial audience is not URL, using " << audienceUrl;
        }
      }

      std::unique_lock<std::mutex> lock(m_tokenMutex);
      // If we have authenticated this audience, we're done and can return success.
      auto token = m_tokenStore.find(audienceUrl);
      if (token != m_tokenStore.end())
      {
        if (m_options.EnableTrace)
        {
          Log::Stream(Logger::Level::Verbose) << "Using cached token for " << audienceUrl;
        }
        return token->second;
      }
      // We've not authenticated this audience.
      // Authenticate it with the server

      if (m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose)
            << "No cached token for " << audienceUrl << ", Authenticating.";
      }

      auto claimsBasedSecurity = std::make_shared<ClaimsBasedSecurityImpl>(session);
      auto cbsOpenStatus = claimsBasedSecurity->Open(context);
      if (cbsOpenStatus != CbsOpenResult::Ok)
      {
        throw std::runtime_error("Could not open Claims Based Security object.");
      }

      try
      {
        Credentials::TokenRequestContext requestContext;

        requestContext.Scopes = m_options.AuthenticationScopes;
        auto accessToken{GetCredential()->GetToken(requestContext, context)};

        auto result = claimsBasedSecurity->PutToken(
            (IsSasCredential() ? CbsTokenType::Sas : CbsTokenType::Jwt),
            audienceUrl,
            accessToken.Token,
            context);
        if (std::get<0>(result) != CbsOperationResult::Ok)
        {
          throw Azure::Core::Credentials::AuthenticationException(
              "Could not authenticate client. Error Status: " + std::to_string(std::get<1>(result))
              + " reason: " + std::get<2>(result));
        }
        Log::Stream(Logger::Level::Verbose) << "Close CBS object";
        claimsBasedSecurity->Close(context);
        if (m_options.EnableTrace)
        {
          Log::Stream(Logger::Level::Verbose)
              << "Authenticated connection for audience " << audienceUrl << " successfully.";
        }

        m_tokenStore.emplace(audienceUrl, accessToken);
        return accessToken;
      }
      catch (...)
      {
        // Ensure that the claims based security object is closed before we leave this scope.
        claimsBasedSecurity->Close(context);
        throw;
      }
    }
    else
    {
      Log::Stream(Logger::Level::Verbose) << "No credential, returning empty token.";
      // If the connection is unauthenticated, then just return an empty access token.
      return {};
    }
  }
}}}} // namespace Azure::Core::Amqp::_detail
