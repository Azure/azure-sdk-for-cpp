// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/policies/policy.hpp"

#include "azure/core/credentials/credentials.hpp"
#include "azure/core/internal/credentials/authorization_challenge_parser.hpp"

#include <chrono>

using Azure::Core::Http::Policies::_internal::BearerTokenAuthenticationPolicy;

using Azure::Core::Context;
using Azure::Core::Credentials::AuthenticationException;
using Azure::Core::Credentials::TokenRequestContext;
using Azure::Core::Credentials::_detail::AuthorizationChallengeHelper;
using Azure::Core::Http::RawResponse;
using Azure::Core::Http::Policies::NextHttpPolicy;

std::unique_ptr<RawResponse> BearerTokenAuthenticationPolicy::Send(
    Request& request,
    NextHttpPolicy nextPolicy,
    Context const& context) const
{
  if (request.GetUrl().GetScheme() != "https")
  {
    throw AuthenticationException(
        "Bearer token authentication is not permitted for non TLS protected (https) endpoints.");
  }

  auto result = AuthorizeAndSendRequest(request, nextPolicy, context);
  {
    auto const& response = *result;
    auto const& challenge = AuthorizationChallengeHelper::GetChallenge(response);
    if (!challenge.empty() && AuthorizeRequestOnChallenge(challenge, request, context))
    {
      result = nextPolicy.Send(request, context);
    }
  }

  return result;
}

std::unique_ptr<RawResponse> BearerTokenAuthenticationPolicy::AuthorizeAndSendRequest(
    Request& request,
    NextHttpPolicy& nextPolicy,
    Context const& context) const
{
  AuthenticateAndAuthorizeRequest(request, m_tokenRequestContext, context);
  return nextPolicy.Send(request, context);
}

bool BearerTokenAuthenticationPolicy::AuthorizeRequestOnChallenge(
    std::string const& challenge,
    Request& request,
    Context const& context) const
{
  static_cast<void>(challenge);
  static_cast<void>(request);
  static_cast<void>(context);

  return false;
}

void BearerTokenAuthenticationPolicy::AuthenticateAndAuthorizeRequest(
    Request& request,
    Credentials::TokenRequestContext const& tokenRequestContext,
    Context const& context) const
{
  std::lock_guard<std::mutex> lock(m_accessTokenMutex);

  if (tokenRequestContext.TenantId != m_accessTokenContext.TenantId
      || tokenRequestContext.Scopes != m_accessTokenContext.Scopes
      || std::chrono::system_clock::now()
          > (m_accessToken.ExpiresOn - tokenRequestContext.MinimumExpiration))
  {
    m_accessToken = m_credential->GetToken(tokenRequestContext, context);
    m_accessTokenContext = tokenRequestContext;
  }

  request.SetHeader("authorization", "Bearer " + m_accessToken.Token);
}
