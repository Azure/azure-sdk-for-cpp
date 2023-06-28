// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "private/token_credential_impl.hpp"

#include "private/package_version.hpp"

#include <azure/core/internal/json/json.hpp>
#include <azure/core/url.hpp>

#include <chrono>
#include <type_traits>

using Azure::Identity::_detail::TokenCredentialImpl;

using Azure::Identity::_detail::PackageVersion;

using Azure::DateTime;
using Azure::Core::Context;
using Azure::Core::Url;
using Azure::Core::_internal::PosixTimeConverter;
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
        "expires_on");
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
[[noreturn]] void ThrowJsonPropertyError(std::string const& propertyName)
{
  throw std::runtime_error(
      std::string("Token JSON object: can't find or parse \'") + propertyName + "\' property.");
}
} // namespace

AccessToken TokenCredentialImpl::ParseToken(
    std::string const& jsonString,
    std::string const& accessTokenPropertyName,
    std::string const& expiresInPropertyName,
    std::string const& expiresOnPropertyName)
{
  auto const parsedJson = Azure::Core::Json::_internal::json::parse(jsonString);

  if (!parsedJson.contains(accessTokenPropertyName)
      || !parsedJson[accessTokenPropertyName].is_string())
  {
    ThrowJsonPropertyError(accessTokenPropertyName);
  }

  AccessToken accessToken;
  accessToken.Token = parsedJson[accessTokenPropertyName].get<std::string>();
  accessToken.ExpiresOn = std::chrono::system_clock::now();

  if (parsedJson.contains(expiresInPropertyName))
  {
    auto const& expiresIn = parsedJson[expiresInPropertyName];

    if (expiresIn.is_number_unsigned())
    {
      // 'expires_in' as number (seconds until expiration)
      accessToken.ExpiresOn
          += std::chrono::seconds(expiresIn.get<std::chrono::seconds::duration::rep>());

      return accessToken;
    }

    if (expiresIn.is_string())
    {
      try
      {
        // 'expires_in' as numeric string (seconds until expiration)
        accessToken.ExpiresOn += std::chrono::seconds(std::stoi(expiresIn.get<std::string>()));

        return accessToken;
      }
      catch (std::exception const&)
      {
        // stoi() has thrown, we may throw later.
      }
    }
  }

  if (expiresOnPropertyName.empty())
  {
    // 'expires_in' is undefined, 'expires_on' is not expected.
    ThrowJsonPropertyError(expiresInPropertyName);
  }

  if (parsedJson.contains(expiresOnPropertyName))
  {
    auto const& expiresOn = parsedJson[expiresOnPropertyName];

    if (expiresOn.is_number_unsigned())
    {
      // 'expires_on' as number (posix time representing an absolute timestamp)
      accessToken.ExpiresOn
          = PosixTimeConverter::PosixTimeToDateTime(expiresOn.get<std::int64_t>());

      return accessToken;
    }

    if (expiresOn.is_string())
    {
      auto const expiresOnAsString = expiresOn.get<std::string>();
      for (auto const parse : {
               std::function<DateTime(std::string const&)>([](auto const& s) {
                 // 'expires_on' as RFC3339 date string (absolute timestamp)
                 return DateTime::Parse(s, DateTime::DateFormat::Rfc3339);
               }),
               std::function<DateTime(std::string const&)>([](auto const& s) {
                 // 'expires_on' as numeric string (posix time representing an absolute timestamp)
                 return PosixTimeConverter::PosixTimeToDateTime(std::stoll(s));
               }),
               std::function<DateTime(std::string const&)>([](auto const& s) {
                 // 'expires_on' as RFC1123 date string (absolute timestamp)
                 return DateTime::Parse(s, DateTime::DateFormat::Rfc1123);
               }),
           })
      {
        try
        {
          accessToken.ExpiresOn = parse(expiresOnAsString);
          return accessToken;
        }
        catch (std::exception const&)
        {
          // parse() has thrown, we may throw later.
        }
      }
    }
  }

  ThrowJsonPropertyError(expiresOnPropertyName);
}
