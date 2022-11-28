// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "private/token_credential_impl.hpp"

#include <azure/core/url.hpp>

#include "private/package_version.hpp"

#include <chrono>
#include <sstream>

using namespace Azure::Identity::_detail;

TokenCredentialImpl::TokenCredentialImpl(Core::Credentials::TokenCredentialOptions const& options)
    : m_httpPipeline(options, "identity", PackageVersion::ToString(), {}, {})
{
}

std::string TokenCredentialImpl::FormatScopes(
    std::vector<std::string> const& scopes,
    bool asResource)
{
  using Azure::Core::Url;

  if (asResource && scopes.size() == 1)
  {
    auto resource = scopes[0];
    constexpr char suffix[] = "/.default";
    constexpr int suffixLen = sizeof(suffix) - 1;
    auto const resourceLen = resource.length();

    // If scopes[0] ends with '/.default', remove it.
    if (resourceLen >= suffixLen
        && resource.find(suffix, resourceLen - suffixLen) != std::string::npos)
    {
      resource = resource.substr(0, resourceLen - suffixLen);
    }

    return Url::Encode(resource);
  }

  auto scopesIter = scopes.begin();
  auto scopesStr = Azure::Core::Url::Encode(*scopesIter);

  auto const scopesEnd = scopes.end();
  for (++scopesIter; scopesIter != scopesEnd; ++scopesIter)
  {
    scopesStr += std::string(" ") + Url::Encode(*scopesIter);
  }

  return scopesStr;
}

Core::Credentials::AccessToken TokenCredentialImpl::ParseToken(
    std::string const& jsonString,
    std::string const& accessTokenPropertyName,
    std::string const& expirationPropertyName,
    bool expirationInSecondsFromNow)
{
  // TODO: use JSON parser.
  auto const jsonStringBegin = jsonString.begin();
  auto const jsonStringSize = jsonString.size();

  auto jsonStringPos = jsonString.find(':', jsonString.find(accessTokenPropertyName));
  if (jsonStringPos == std::string::npos)
  {
    throw std::runtime_error(
        std::string("Token JSON object: \'") + accessTokenPropertyName
        + "\' property was not found.");
  }

  for (; jsonStringPos < jsonStringSize; ++jsonStringPos)
  {
    auto c = jsonString[jsonStringPos];
    if (c != ':' && c != ' ' && c != '\"' && c != '\'')
    {
      break;
    }
  }

  auto const tokenBegin = jsonStringPos;
  for (; jsonStringPos < jsonStringSize; ++jsonStringPos)
  {
    auto c = jsonString[jsonStringPos];
    if (c == '\"' || c == '\'')
    {
      break;
    }
  }
  auto const tokenEnd = jsonStringPos;

  jsonStringPos = jsonString.find(':', jsonString.find(expirationPropertyName));
  if (jsonStringPos == std::string::npos)
  {
    throw std::runtime_error(std::string("Token JSON object: \'" + expirationPropertyName "\' property was not found.";
  }

  auto const accessToken = std::string(jsonStringBegin + tokenBegin, jsonStringBegin + tokenEnd);

  if (expirationInSecondsFromNow)
  {
    for (; jsonStringPos < jsonStringSize; ++jsonStringPos)
    {
      auto c = jsonString[jsonStringPos];
      if (c != ':' && c != ' ' && c != '\"' && c != '\'')
      {
        break;
      }
    }

    long long expiresInSeconds = 0;
    for (; jsonStringPos < jsonStringSize; ++jsonStringPos)
    {
      auto c = jsonString[jsonStringPos];
      if (c < '0' || c > '9')
      {
        break;
      }

      expiresInSeconds = (expiresInSeconds * 10) + (static_cast<long long>(c) - '0');
    }

    return {
        accessToken,
        std::chrono::system_clock::now() + std::chrono::seconds(expiresInSeconds),
    };
  }
  else
  {
    for (; jsonStringPos < jsonStringSize; ++jsonStringPos)
    {
      auto c = jsonString[jsonStringPos];
      if (c != ':' && c != ' ' && c != '\"' && c != '\'')
      {
        break;
      }
    }

    auto const dateTimeStampStartPos = jsonStringPos;
    for (; jsonStringPos < jsonStringSize; ++jsonStringPos)
    {
      auto c = jsonString[jsonStringPos];
      if (c == '\"' || c == '\'')
      {
        break;
      }
    }

    auto dateTimeString
        = std::string(jsonStringBegin + dateTimeStampStartPos, jsonStringBegin + jsonStringPos);

    // dateTimeString may not be in strict RFC3339 format - space may be used instead of 'T'.
    {
      auto const spacePos = dateTimeString.find(' ');
      if (spacePos != std::string::npos)
      {
        dateTimeString = dateTimeString.replace(spacePos, 1, 1, 'T');
      }
    }

    return {
        accessToken,
        Azure::DateTime::Parse(dateTimeString, Azure::DateTime::DateFormat::Rfc3339),
    };
  }
}

Azure::Core::Credentials::AccessToken TokenCredentialImpl::GetToken(
    Core::Context const& context,
    std::function<std::unique_ptr<TokenCredentialImpl::TokenRequest>()> const& createRequest,
    std::function<std::unique_ptr<TokenCredentialImpl::TokenRequest>(
        Azure::Core::Http::HttpStatusCode statusCode,
        Azure::Core::Http::RawResponse const& response)> const& shouldRetry) const
{
  using Azure::Core::Credentials::AuthenticationException;
  using Azure::Core::Http::HttpStatusCode;
  using Azure::Core::Http::RawResponse;

  static std::string const errorMsgPrefix("GetToken: ");

  try
  {
    std::unique_ptr<RawResponse> response;
    {
      auto request = createRequest();
      for (;;)
      {
        response = m_httpPipeline.Send(request->HttpRequest, context);
        if (!response)
        {
          throw AuthenticationException(errorMsgPrefix + "null response");
        }

        auto const statusCode = response->GetStatusCode();
        if (statusCode == HttpStatusCode::Ok)
        {
          break;
        }

        request = shouldRetry(statusCode, *response);
        if (request == nullptr)
        {
          std::ostringstream errorMsg;
          errorMsg << errorMsgPrefix << "error response: "
                   << static_cast<std::underlying_type<HttpStatusCode>::type>(statusCode) << " "
                   << response->GetReasonPhrase();

          throw AuthenticationException(errorMsg.str());
        }

        response.reset();
      }
    }

    auto const& responseBodyVector = response->GetBody();
    std::string responseBody(responseBodyVector.begin(), responseBodyVector.end());

    return ParseToken(responseBody, "access_token", "expires_in", true);
  }
  catch (AuthenticationException const&)
  {
    throw;
  }
  catch (std::exception const& e)
  {
    throw AuthenticationException(errorMsgPrefix + e.what());
  }
}
