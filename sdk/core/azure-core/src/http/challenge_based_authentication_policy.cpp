// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/policies/policy.hpp"
#include <chrono>
#include <vector>
#include <sstream>

using Azure::Core::Context;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;
using namespace Azure::Core::Http::Policies::_detail;

std::unique_ptr<RawResponse> ChallengeBasedAuthenticationPolicy::Send(
    Request& request,
    NextHttpPolicy nextPolicy,
    Context const& context) const
{
  request.SetHeader("authorization", "Bearer " + m_accessToken.Token);
  auto result = nextPolicy.Send(request, context);
  auto const& headers = result->GetHeaders();
  auto const& rawDataHeader = headers.find("www-authenticate");
  // Only re-run when Unauthorized AND www-authenticate header in the response
  if (result->GetStatusCode() == HttpStatusCode::Unauthorized && rawDataHeader != headers.end())
  {
    ChallengeParameters challenge(rawDataHeader->second);
    Credentials::TokenRequestContext tokenRequestContext;

    tokenRequestContext.TenantId = std::move(challenge.TenantId);
    tokenRequestContext.AuthorizationUri = std::move(challenge.AuthorizationUri);
    tokenRequestContext.Scopes = std::move(challenge.Scopes);

    {
      std::lock_guard<std::mutex> lock(m_accessTokenMutex);
      m_accessToken = m_credential->GetToken(tokenRequestContext, context);
      request.SetHeader("authorization", "Bearer " + m_accessToken.Token);
    }
    return nextPolicy.Send(request, context);
  }

  // Did not re-send on any other case
  return result;
}
/*
 * the raw value ( the value of the "www-authenticate" header is in the following format
 * "Bearer authorization/authorization_uri=[value] resource/scope=[value]";
 */
ChallengeParameters::ChallengeParameters(std::string const& headerValue)
{
  if (headerValue.rfind(_detail::BearerName) == 0)
  {
    Schema = _detail::BearerName;

    std::vector<std::string> const parts = GetParts(headerValue, _detail::SpaceSeparator);
    for (size_t i = 1; i < parts.size(); i++)
    {
      ProcessFragment(parts[i]);
    }
  }
}

void ChallengeParameters::ProcessFragment(std::string const& fragment)
{
  // check that fragment is in the Key=value format
  if (fragment.rfind(_detail::EqualSeparator) != std::string::npos)
  {
    auto subParts = GetParts(fragment, _detail::EqualSeparator);

    for (size_t i = 0; i < subParts.size(); i++)
    {
      // some parts (values) have quotes around , thus clean them on quotes
      auto finalParts = GetParts(subParts[i], _detail::QuoteSeparator);

      if (finalParts.size() > 0)
      {
        // take the first piece that has been trimmed
        subParts[i] = finalParts[0];
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
  size_t startPosition = 0;
  size_t endPosition = 0;
  while (endPosition != std::string::npos)
  {
    endPosition = inputString.find(separator, startPosition);
    if (endPosition != std::string::npos)
    {
      if (endPosition != startPosition)
      {
        returnValue.emplace_back(inputString.substr(startPosition, endPosition - startPosition));
      }
    }
    else
    {
      returnValue.emplace_back(
          inputString.substr(startPosition, inputString.length() - startPosition));
    }
    startPosition = endPosition + 1;
  }
  return returnValue;
}
