// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "private/token_credential_impl.hpp"

#include <azure/core/url.hpp>

#include "private/package_version.hpp"

#include <chrono>
#include <type_traits>

using Azure::Identity::_detail::TokenCredentialImpl;

using Azure::Identity::_detail::PackageVersion;

using Azure::Core::Context;
using Azure::Core::Url;
using Azure::Core::Credentials::AccessToken;
using Azure::Core::Credentials::AuthenticationException;
using Azure::Core::Credentials::TokenCredentialOptions;
using Azure::Core::Http::HttpStatusCode;
using Azure::Core::Http::RawResponse;

TokenCredentialImpl::TokenCredentialImpl(TokenCredentialOptions const& options)
    : m_httpPipeline(options, "identity", PackageVersion::ToString(), {}, {})
{
}

namespace {
std::string OptionalUrlEncode(std::string const& value, bool doEncode)
{
  return doEncode ? Url::Encode(value) : value;
}
} // namespace

std::string TokenCredentialImpl::FormatScopes(
    std::vector<std::string> const& scopes,
    bool asResource,
    bool urlEncode)
{
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

    return OptionalUrlEncode(resource, urlEncode);
  }

  std::string scopesStr;
  {
    auto scopesIter = scopes.begin();
    auto const scopesEnd = scopes.end();

    if (scopesIter != scopesEnd) // LCOV_EXCL_LINE
    {
      auto const scope = *scopesIter;
      scopesStr += OptionalUrlEncode(scope, urlEncode);
    }

    for (++scopesIter; scopesIter != scopesEnd; ++scopesIter)
    {
      auto const Separator = std::string(" "); // Element separator never gets URL-encoded

      auto const scope = *scopesIter;
      scopesStr += Separator + OptionalUrlEncode(scope, urlEncode);
    }
  }

  return scopesStr;
}

AccessToken TokenCredentialImpl::GetToken(
    Context const& context,
    std::function<std::unique_ptr<TokenCredentialImpl::TokenRequest>()> const& createRequest,
    std::function<std::unique_ptr<TokenCredentialImpl::TokenRequest>(
        HttpStatusCode statusCode,
        RawResponse const& response)> const& shouldRetry) const
{
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
          throw std::runtime_error("null response");
        }

        auto const statusCode = response->GetStatusCode();
        if (statusCode == HttpStatusCode::Ok)
        {
          break;
        }

        request = shouldRetry(statusCode, *response);
        if (request == nullptr)
        {
          throw std::runtime_error(
              std::string("error response: ")
              + std::to_string(
                  static_cast<std::underlying_type<decltype(statusCode)>::type>(statusCode))
              + " " + response->GetReasonPhrase());
        }

        response.reset();
      }
    }

    auto const& responseBodyVector = response->GetBody();

    return ParseToken(
        std::string(responseBodyVector.begin(), responseBodyVector.end()),
        "access_token",
        "expires_in",
        std::string());
  }
  catch (AuthenticationException const&)
  {
    throw;
  }
  catch (std::exception const& e)
  {
    throw AuthenticationException(std::string("GetToken(): ") + e.what());
  }
}

namespace {
[[noreturn]] void ThrowMissingJsonPropertyError(std::string const& propertyName)
{
  throw std::runtime_error(
      std::string("Token JSON object: \'") + propertyName + "\' property was not found.");
}

bool GetPropertyValueAsInt64(
    std::string const& jsonString,
    std::string const& propertyName,
    std::string& outValue);

bool GetPropertyValueAsString(
    std::string const& jsonString,
    std::string const& propertyName,
    std::string& outValue);
} // namespace

AccessToken TokenCredentialImpl::ParseToken(
    std::string const& jsonString,
    std::string const& accessTokenPropertyName,
    std::string const& expiresInPropertyName,
    std::string const& expiresOnPropertyName)
{
  // TODO: use JSON parser.
  AccessToken accessToken;
  if (!GetPropertyValueAsString(jsonString, accessTokenPropertyName, accessToken.Token))
  {
    ThrowMissingJsonPropertyError(accessTokenPropertyName);
  }

  int64_t expiresIn = 0;
  if (GetPropertyValueAsInt64(jsonString, expiresInPropertyName, accessToken.Token))
  {
    accessToken.ExpiresOn = std::chrono::system_clock::now() + std::chrono::seconds(expiresIn);
    return accessToken;
  }

  if (expiresOnPropertyName.empty())
  {
    ThrowMissingJsonPropertyError(expiresInPropertyName);
  }

  std::string expiresOn;
  if (!GetPropertyValueAsString(jsonString, expiresOnPropertyName, expiresOn))
  {
    ThrowMissingJsonPropertyError(expiresInPropertyName + "\' or \'" + expiresOnPropertyName);
  }

  {
    auto const spacePos = expiresOn.find(' ');
    if (spacePos != std::string::npos)
    {
      expiresOn = expiresOn.replace(spacePos, 1, 1, 'T');
    }
  }

  accessToken.ExpiresOn = Azure::DateTime::Parse(expiresOn, Azure::DateTime::DateFormat::Rfc3339);
  return accessToken;
}

namespace {
std::string::size_type GetPropertyValueStart(
    std::string const& jsonString,
    std::string const& propertyName);

bool GetPropertyValueAsInt64(
    std::string const& jsonString,
    std::string const& propertyName,
    std::string& outValue)
{
  auto const valueStartPos = GetPropertyValueStart(jsonString, propertyName);
  if (valueStartPos == std::string::npos)
  {
    return false;
  }

  int64_t value = 0;
  {
    auto const size = jsonString.size();
    for (auto pos = valueStartPos; pos < size; ++pos)
    {
      auto c = jsonString[pos];
      if (c < '0' || c > '9')
      {
        break;
      }

      value = (value * 10) + (static_cast<int64_t>(c) - '0');
    }
  }

  outValue = value;

  return true;
}

std::string::size_type GetPropertyValueEnd(std::string const& str, std::string::size_type startPos);

bool GetPropertyValueAsString(
    std::string const& jsonString,
    std::string const& propertyName,
    std::string& outValue)
{
  auto const valueStartPos = GetPropertyValueStart(jsonString, propertyName);
  if (valueStartPos == std::string::npos)
  {
    return false;
  }
  auto const jsonStringBegin = jsonString.begin();
  outValue = std::string(
      jsonStringBegin + valueStartPos,
      jsonStringBegin + GetPropertyValueEnd(jsonString, valueStartPos));

  return true;
}

std::string::size_type GetPropertyValueStart(
    std::string const& jsonString,
    std::string const& propertyName)
{
  auto const propertyNameStart = jsonString.find(':', jsonString.find(propertyName));
  if (propertyNameStart == std::string::npos)
  {
    return std::string::npos;
  }

  auto pos = propertyNameStart + propertyName.size();
  {
    auto const jsonStringSize = jsonString.size();
    for (; pos < jsonStringSize; ++pos)
    {
      auto c = jsonString[pos];
      if (c != ':' && c != ' ' && c != '\"' && c != '\'')
      {
        break;
      }
    }
  }

  return pos;
}

std::string::size_type GetPropertyValueEnd(std::string const& str, std::string::size_type startPos)
{
  auto pos = startPos;
  {
    auto const strSize = str.size();
    for (; pos < strSize; ++pos)
    {
      auto c = str[pos];
      if (c == '\"' || c == '\'')
      {
        break;
      }
    }
  }

  return pos;
}
} // namespace
