// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/policies/policy.hpp"

#include <algorithm>
#include <chrono>
#include <shared_mutex>

using Azure::Core::Context;
using Azure::Core::Url;
using Azure::Core::Credentials::AuthenticationException;
using Azure::Core::Credentials::TokenRequestContext;
using Azure::Core::Http::RawResponse;
using Azure::Core::Http::Request;
using Azure::Core::Http::Policies::NextHttpPolicy;
using Azure::Core::Http::Policies::_internal::BearerTokenAuthenticationPolicy;
using Azure::Core::Http::Policies::_internal::ChallengeBasedAuthenticationPolicy;

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

  auto response = AuthorizeAndSendRequest(request, nextPolicy, context);

  auto const challenge = GetChallenge(*response);
  if (!challenge.empty())
  {
    AuthorizeRequestOnChallenge(request, challenge, context);
    return nextPolicy.Send(request, context);
  }

  return response;
}

void BearerTokenAuthenticationPolicy::AuthenticateAndAuthorizeRequest(
    Request& request,
    Context const& context) const
{
  std::lock_guard<std::mutex> lock(m_accessTokenMutex);

  // Refresh the token in 2 or less minutes before the actual expiration.
  if (std::chrono::system_clock::now() > (m_accessToken.ExpiresOn - std::chrono::minutes(2)))
  {
    m_accessToken = m_credential->GetToken(TokenRequestContext, context);
  }

  request.SetHeader("authorization", "Bearer " + m_accessToken.Token);
}

std::unique_ptr<RawResponse> BearerTokenAuthenticationPolicy::AuthorizeAndSendRequest(
    Request& request,
    NextHttpPolicy nextPolicy,
    Context const& context) const
{
  AuthenticateAndAuthorizeRequest(request, context);
  return nextPolicy.Send(request, context);
}

std::string BearerTokenAuthenticationPolicy::GetChallenge(RawResponse const& response) const
{
  static_cast<void>(response);
  return std::string();
}

// LCOV_EXCL_START
void BearerTokenAuthenticationPolicy::AuthorizeRequestOnChallenge(
    Request& request,
    std::string const& challenge,
    Context const& context) const
{
  static_cast<void>(request);
  static_cast<void>(challenge);
  static_cast<void>(context);
}
// LCOV_EXCL_STOP

namespace {
std::string const WwwAuthenticateHeaderName("WWW-Authenticate");

std::string GetAuthority(Url const& url)
{
  return url.GetPort() > 0 ? url.GetHost() + ":" + std::to_string(url.GetPort()) : url.GetHost();
}

std::shared_timed_mutex ChallengeCacheMutex;
std::map<std::string, TokenRequestContext> ChallengeCache;

} // namespace

std::unique_ptr<RawResponse> ChallengeBasedAuthenticationPolicy::AuthorizeAndSendRequest(
    Request& request,
    NextHttpPolicy nextPolicy,
    Context const& context) const
{
  if (TokenRequestContext.AuthorizationUrl.GetHost().empty())
  {
    auto const authority = GetAuthority(request.GetUrl());
    {
      std::shared_lock<std::shared_timed_mutex> challengeCacheRead(ChallengeCacheMutex);

      auto const cacheResult = ChallengeCache.find(authority);
      if (cacheResult != ChallengeCache.end())
      {
        TokenRequestContext = cacheResult->second;
      }
    }
  }

  if (!TokenRequestContext.AuthorizationUrl.GetHost().empty())
  {
    AuthenticateAndAuthorizeRequest(request, context);
    return nextPolicy.Send(request, context);
  }

  Request requestWithoutBody(request.GetMethod(), request.GetUrl(), request.ShouldBufferResponse());
  for (auto header : request.GetHeaders())
  {
    requestWithoutBody.SetHeader(header.first, header.second);
  }

  return nextPolicy.Send(requestWithoutBody, context);
}

std::string ChallengeBasedAuthenticationPolicy::GetChallenge(RawResponse const& response) const
{
  using Http::HttpStatusCode;

  if (response.GetStatusCode() == HttpStatusCode::Unauthorized)
  {
    auto const& headers = response.GetHeaders();
    auto const headerValue = headers.find(WwwAuthenticateHeaderName);

    if (headerValue != headers.end())
    {
      return headerValue->second;
    }
  }

  return std::string();
}

namespace {
TokenRequestContext ParseChallenge(std::string const& challenge);
} // namespace

void ChallengeBasedAuthenticationPolicy::AuthorizeRequestOnChallenge(
    Request& request,
    std::string const& challenge,
    Context const& context) const
{
  TokenRequestContext = ParseChallenge(challenge);
  auto const authority = GetAuthority(request.GetUrl());
  {
    std::unique_lock<std::shared_timed_mutex> challengeCacheWrite(ChallengeCacheMutex);
    ChallengeCache[authority] = TokenRequestContext;
  }

  AuthenticateAndAuthorizeRequest(request, context);
}

namespace {
int GetChallengeParameterValue(
    std::string const& challenge,
    std::string const& name1,
    std::string const& name2,
    std::string& out);

TokenRequestContext ParseChallenge(std::string const& challenge)
{
  std::string authUrlStr;
  if (GetChallengeParameterValue(challenge, "authorization=", "authorization_uri=", authUrlStr) > 0)
  {
    TokenRequestContext tokenRequestContext;

    try
    {
      Url authUrl(authUrlStr);
      auto authorizationUrlStr = authUrl.GetScheme() + "://" + GetAuthority(authUrl) + '/';

      tokenRequestContext.AuthorizationUrl = Url(authorizationUrlStr);

      auto const path = authUrl.GetPath();
      auto tenantIdEnd = path.find('/');
      if (tenantIdEnd == std::string::npos)
      {
        tenantIdEnd = path.size();
      }

      tokenRequestContext.TenantId = path.substr(0, tenantIdEnd);

      std::string scopeStr;
      auto paramNameIndex = GetChallengeParameterValue(challenge, "resource=", "scope=", scopeStr);
      if (paramNameIndex > 0)
      {
        if (paramNameIndex == 1)
        {
          scopeStr += "/.default";
        }

        tokenRequestContext.Scopes = {scopeStr};

        return tokenRequestContext;
      }
    }
    catch (std::exception const&)
    {
    }
  }

  throw AuthenticationException("Error parsing challenge response.");
}

int GetChallengeParameterValue(
    std::string const& challenge,
    std::string const& name1,
    std::string const& name2,
    std::string& out)
{
  auto whichNameWasParsed = 0;
  auto const challengeSize = challenge.size();

  auto valueStart = challenge.find(name1);
  auto valueEnd = std::string::npos;
  if (valueStart != std::string::npos)
  {
    valueStart += name1.size();
    whichNameWasParsed = 1;
  }
  else
  {
    valueStart = challenge.find(name2);
    if (valueStart != std::string::npos)
    {
      valueStart += name2.size();
      whichNameWasParsed = 2;
    }
  }

  if (valueStart == std::string::npos || valueStart >= challengeSize)
  {
    return 0;
  }

  if (challenge[valueStart] == '\"')
  {
    ++valueStart;

    if (valueStart == challengeSize)
    {
      return 0;
    }
    else
    {
      valueEnd = challenge.find('\"', valueStart);
      if (valueEnd == std::string::npos)
      {
        return 0;
      }
    }
  }
  else
  {
    auto const commaPos = challenge.find(',', valueStart);
    auto const spacePos = challenge.find(' ', valueStart);

    if (commaPos != std::string::npos)
    {
      valueEnd = (spacePos != std::string::npos) ? (std::min)(commaPos, spacePos) : commaPos;
    }
    else
    {
      valueEnd = (spacePos != std::string::npos) ? spacePos : challengeSize;
    }
  }

  auto const substrLen = valueEnd - valueStart;
  if (substrLen == 0)
  {
    return 0;
  }

  out = challenge.substr(valueStart, valueEnd - valueStart);
  return whichNameWasParsed;
}
} // namespace
