// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/policies/policy.hpp"
#include <chrono>
#include <vector>

using Azure::Core::Context;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;

std::unique_ptr<RawResponse> ChallengeBasedAuthenticationPolicy::Send(
    Request& request,
    NextHttpPolicy nextPolicy,
    Context const& context) const
{
  request.SetHeader("authorization", "Bearer " + m_accessToken.Token);
  auto result = nextPolicy.Send(request, context);

  if (result->GetStatusCode() == HttpStatusCode::Unauthorized)
  {
    auto rawData = result->GetHeaders().find("www-authenticate");
    if (rawData != result->GetHeaders().end())
    {
      ChallengeParameters challenge(rawData->second);
      Credentials::TokenRequestContext tokenRequestContext;

      tokenRequestContext.TenantId = std::move(challenge.TenantId);
      tokenRequestContext.AuthorizationUri = std::move(challenge.AuthorizationUri);
      tokenRequestContext.Scopes = std::move(m_tokenRequestContext.Scopes);

      {
        std::lock_guard<std::mutex> lock(m_accessTokenMutex);
        m_accessToken = m_credential->GetToken(tokenRequestContext, context);
        request.SetHeader("authorization", "Bearer " + m_accessToken.Token);
      }
    }
    return nextPolicy.Send(request, context);
  }

  return result;
}

std::string ChallengeBasedAuthenticationPolicy::GetRequestAuthority(Request request)
{
  Url uri = request.GetUrl();

  std::string authority = uri.GetHost();
  if (!authority.find(":") && uri.GetPort() > 0)
  {
    // Append port for complete authority.
    authority = authority + ":" + std::to_string(uri.GetPort());
  }

  return authority;
}

ChallengeParameters::ChallengeParameters(std::string const& rawValue)
{
  if (rawValue.rfind(_detail::BearerName) == 0)
  {
    Schema = _detail::BearerName;

    auto parts = GetParts(rawValue, _detail::SpaceSeparator);
    for (size_t i = 1; i < parts.size(); i++)
    {
      ProcessFragment(parts[i]);
    }
  }
}

void ChallengeParameters::ProcessFragment(std::string const& fragment)
{
  // check that fragment is in the Key=value format
  if (fragment.rfind(_detail::EqualSeparator) != 0)
  {
    auto subParts = GetParts(fragment, _detail::EqualSeparator);

    for (size_t i = 0; i < subParts.size(); i++)
    {
      // some parts (values) have quotes around , thus clean them on quotes
      auto trimmedParts = GetParts(subParts[i], _detail::QuoteSeparator);

      if (trimmedParts.size() > 0)
      {
        // take the first piece that has been trimmed
        subParts[i] = trimmedParts[0];
      }
    }

    // authorization or authorization_uri go to authorization field
    if (subParts[0] == _detail::AuthorizationName || subParts[0] == _detail::AuthorizationUriName)
    {
      AuthorizationUri = Url(subParts[1]);
      // auth tenant is part of the authorization uri
      TenantId = AuthorizationUri.GetPath();
      AuthorizationUri.AppendPath("oauth2/v2.0/token");
    }
    // scopes are either resource or scope
    else if (subParts[0] == _detail::ResourceName)
    {
      Scopes.emplace_back(subParts[1] + _detail::DefaultSuffix);
    }
    else if (subParts[0] == _detail::ScopeName)
    {
      Scopes.emplace_back(subParts[1]);
    }
  }
}

std::vector<std::string> ChallengeParameters::GetParts(
    std::string const& inputString,
    char const& separator)
{
  std::vector<std::string> returnValue;
  std::istringstream inputStream(inputString);
  std::string token;

  while (std::getline(inputStream, token, separator))
  {
    if (!token.empty())
    {
      returnValue.emplace_back(token);
    }
  }

  return returnValue;
}
