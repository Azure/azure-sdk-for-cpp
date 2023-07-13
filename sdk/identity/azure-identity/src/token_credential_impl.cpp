// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "private/token_credential_impl.hpp"

#include "private/identity_log.hpp"
#include "private/package_version.hpp"

#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/strings.hpp>
#include <azure/core/url.hpp>

#include <chrono>
#include <limits>
#include <map>

using Azure::Identity::_detail::TokenCredentialImpl;

using Azure::Identity::_detail::IdentityLog;
using Azure::Identity::_detail::PackageVersion;

using Azure::DateTime;
using Azure::Core::Context;
using Azure::Core::Url;
using Azure::Core::_internal::PosixTimeConverter;
using Azure::Core::_internal::StringExtensions;
using Azure::Core::Credentials::AccessToken;
using Azure::Core::Credentials::AuthenticationException;
using Azure::Core::Credentials::TokenCredentialOptions;
using Azure::Core::Http::HttpStatusCode;
using Azure::Core::Http::RawResponse;
using Azure::Core::Json::_internal::json;

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
std::string const ParseTokenLogPrefix = "TokenCredentialImpl::ParseToken(): ";

constexpr std::int64_t MaxExpirationInSeconds = 2147483647; // int32 max (68+ years)
constexpr std::int64_t MaxPosixTimestamp = 253402300799; // 9999-12-31T23:59:59

std::int64_t ParseNumericExpiration(
    std::string const& numericString,
    std::int64_t maxValue,
    std::int64_t minValue = 0)
{
  auto const asNumber = std::stoll(numericString);
  return (asNumber >= minValue && asNumber <= maxValue && std::to_string(asNumber) == numericString)
      ? static_cast<std::int64_t>(asNumber)
      : throw std::exception();
}

std::string TokenAsDiagnosticString(
    json const& jsonObject,
    std::string const& accessTokenPropertyName,
    std::string const& expiresInPropertyName,
    std::string const& expiresOnPropertyName);

[[noreturn]] void ThrowJsonPropertyError(
    std::string const& failedPropertyName,
    json const& jsonObject,
    std::string const& accessTokenPropertyName,
    std::string const& expiresInPropertyName,
    std::string const& expiresOnPropertyName)
{
  if (IdentityLog::ShouldWrite(IdentityLog::Level::Verbose))
  {
    IdentityLog::Write(
        IdentityLog::Level::Verbose,
        ParseTokenLogPrefix
            + TokenAsDiagnosticString(
                jsonObject, accessTokenPropertyName, expiresInPropertyName, expiresOnPropertyName));
  }

  throw std::runtime_error(
      "Token JSON object: can't find or parse \'" + failedPropertyName
      + "\' property.\nSee Azure::Core::Diagnostics::Logger for details"
        " (https://aka.ms/azsdk/cpp/identity/troubleshooting).");
}
} // namespace

AccessToken TokenCredentialImpl::ParseToken(
    std::string const& jsonString,
    std::string const& accessTokenPropertyName,
    std::string const& expiresInPropertyName,
    std::string const& expiresOnPropertyName)
{
  json parsedJson;
  try
  {
    parsedJson = Azure::Core::Json::_internal::json::parse(jsonString);
  }
  catch (json::exception const&)
  {
    IdentityLog::Write(
        IdentityLog::Level::Verbose,
        ParseTokenLogPrefix + "Cannot parse the string '" + jsonString + "' as JSON.");

    throw;
  }

  if (!parsedJson.contains(accessTokenPropertyName)
      || !parsedJson[accessTokenPropertyName].is_string())
  {
    ThrowJsonPropertyError(
        accessTokenPropertyName,
        parsedJson,
        accessTokenPropertyName,
        expiresInPropertyName,
        expiresOnPropertyName);
  }

  AccessToken accessToken;
  accessToken.Token = parsedJson[accessTokenPropertyName].get<std::string>();
  accessToken.ExpiresOn = std::chrono::system_clock::now();

  if (parsedJson.contains(expiresInPropertyName))
  {
    auto const& expiresIn = parsedJson[expiresInPropertyName];

    if (expiresIn.is_number_unsigned())
    {
      try
      {
        // 'expires_in' as number (seconds until expiration)
        auto const value = expiresIn.get<std::int64_t>();
        if (value <= MaxExpirationInSeconds)
        {
          static_assert(
              MaxExpirationInSeconds <= std::numeric_limits<std::int32_t>::max(),
              "Can safely cast to int32");

          accessToken.ExpiresOn += std::chrono::seconds(static_cast<std::int32_t>(value));
          return accessToken;
        }
      }
      catch (std::exception const&)
      {
        // expiresIn.get<std::int64_t>() has thrown, we may throw later.
      }
    }

    if (expiresIn.is_string())
    {
      try
      {
        // 'expires_in' as numeric string (seconds until expiration)
        static_assert(
            MaxExpirationInSeconds <= std::numeric_limits<std::int32_t>::max(),
            "Can safely cast to int32");

        accessToken.ExpiresOn += std::chrono::seconds(static_cast<std::int32_t>(
            ParseNumericExpiration(expiresIn.get<std::string>(), MaxExpirationInSeconds)));

        return accessToken;
      }
      catch (std::exception const&)
      {
        // ParseNumericExpiration() has thrown, we may throw later.
      }
    }
  }

  if (expiresOnPropertyName.empty())
  {
    // 'expires_in' is undefined, 'expires_on' is not expected.
    ThrowJsonPropertyError(
        expiresInPropertyName,
        parsedJson,
        accessTokenPropertyName,
        expiresInPropertyName,
        expiresOnPropertyName);
  }

  if (parsedJson.contains(expiresOnPropertyName))
  {
    auto const& expiresOn = parsedJson[expiresOnPropertyName];

    if (expiresOn.is_number_unsigned())
    {
      try
      {
        // 'expires_on' as number (posix time representing an absolute timestamp)
        auto const value = expiresOn.get<std::int64_t>();
        if (value <= MaxPosixTimestamp)
        {
          accessToken.ExpiresOn = PosixTimeConverter::PosixTimeToDateTime(value);
          return accessToken;
        }
      }
      catch (std::exception const&)
      {
        // expiresIn.get<std::int64_t>() has thrown, we may throw later.
      }
    }

    if (expiresOn.is_string())
    {
      auto const expiresOnAsString = expiresOn.get<std::string>();
      for (auto const& parse : {
               std::function<DateTime(std::string const&)>([](auto const& s) {
                 // 'expires_on' as RFC3339 date string (absolute timestamp)
                 return DateTime::Parse(s, DateTime::DateFormat::Rfc3339);
               }),
               std::function<DateTime(std::string const&)>([](auto const& s) {
                 // 'expires_on' as numeric string (posix time representing an absolute timestamp)
                 return PosixTimeConverter::PosixTimeToDateTime(
                     ParseNumericExpiration(s, MaxPosixTimestamp));
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

  ThrowJsonPropertyError(
      expiresOnPropertyName,
      parsedJson,
      accessTokenPropertyName,
      expiresInPropertyName,
      expiresOnPropertyName);
}

namespace {
std::string PrintSanitizedJsonObject(json const& jsonObject, bool printString, int depth = 0)
{
  if (jsonObject.is_null() || jsonObject.is_boolean() || jsonObject.is_number()
      || (printString && jsonObject.is_string()))
  {
    return jsonObject.dump();
  }

  if (jsonObject.is_string())
  {
    auto const stringValue = jsonObject.get<std::string>();
    for (auto const& parse : {
             std::function<std::string()>([&]() -> std::string {
               return (StringExtensions::LocaleInvariantCaseInsensitiveEqual(stringValue, "null")
                       || StringExtensions::LocaleInvariantCaseInsensitiveEqual(stringValue, "true")
                       || StringExtensions::LocaleInvariantCaseInsensitiveEqual(
                           stringValue, "false"))
                   ? stringValue
                   : std::string{};
             }),
             std::function<std::string()>([&]() -> std::string {
               return DateTime::Parse(stringValue, DateTime::DateFormat::Rfc3339)
                   .ToString(DateTime::DateFormat::Rfc3339);
             }),
             std::function<std::string()>([&]() -> std::string {
               return std::to_string(ParseNumericExpiration(
                   stringValue,
                   std::numeric_limits<std::int64_t>::max(),
                   std::numeric_limits<std::int64_t>::min()));
             }),
             std::function<std::string()>([&]() -> std::string {
               return DateTime::Parse(stringValue, DateTime::DateFormat::Rfc1123)
                   .ToString(DateTime::DateFormat::Rfc1123);
             }),
         })
    {
      try
      {
        auto const parsedValueAsString = parse();
        if (!parsedValueAsString.empty())
        {
          return '"' + parsedValueAsString + '"';
        }
      }
      catch (std::exception const&)
      {
      }
    }

    return "string.length=" + std::to_string(stringValue.length());
  }

  if (jsonObject.is_array())
  {
    return "[...]";
  }

  if (jsonObject.is_object())
  {
    if (depth == 0)
    {
      std::string objectValue{"{"};
      char const* delimiter = "";
      for (auto const& property : jsonObject.items())
      {
        objectValue += delimiter;
        objectValue += '\'' + property.key() + "': ";
        objectValue += PrintSanitizedJsonObject(property.value(), false, depth + 1);
        delimiter = ", ";
      }
      return objectValue + '}';
    }
    else
    {
      return "{...}";
    }
  }

  return "?"; // LCOV_EXCL_LINE
}

std::string TokenAsDiagnosticString(
    json const& jsonObject,
    std::string const& accessTokenPropertyName,
    std::string const& expiresInPropertyName,
    std::string const& expiresOnPropertyName)
{
  std::string result = "Please report an issue with the following details:\nToken JSON";

  if (!jsonObject.is_object())
  {
    result += " is not an object (" + PrintSanitizedJsonObject(jsonObject, false) + ")";
  }
  else
  {
    result += ": Access token property ('" + accessTokenPropertyName + "'): ";
    if (!jsonObject.contains(accessTokenPropertyName))
    {
      result += "undefined";
    }
    else
    {
      auto const& accessTokenProperty = jsonObject[accessTokenPropertyName];
      result += accessTokenProperty.is_string()
          ? ("string.length=" + std::to_string(accessTokenProperty.get<std::string>().length()))
          : PrintSanitizedJsonObject(accessTokenProperty, false);
    }

    for (auto const& p : {
             std::pair<char const*, std::string const*>{"relative", &expiresInPropertyName},
             std::pair<char const*, std::string const*>{"absolute", &expiresOnPropertyName},
         })
    {
      result += ", ";
      result += p.first;
      result += " expiration property ('" + *p.second + "'): ";

      if (!jsonObject.contains(*p.second))
      {
        result += "undefined";
      }
      else
      {
        result += PrintSanitizedJsonObject(jsonObject[*p.second], true);
      }
    }

    std::map<std::string, json> otherProperties;
    for (auto const& property : jsonObject.items())
    {
      if (property.key() != accessTokenPropertyName && property.key() != expiresInPropertyName
          && property.key() != expiresOnPropertyName)
      {
        otherProperties[property.key()] = property.value();
      }
    }

    result += ", ";
    if (otherProperties.empty())
    {
      result += "and there are no other properties";
    }
    else
    {
      result += "other properties";
      const char* delimiter = ": ";
      for (auto const& property : otherProperties)
      {
        result += delimiter;
        result += "'" + property.first + "': " + PrintSanitizedJsonObject(property.second, false);
        delimiter = ", ";
      }
    }
  }

  return result + '.';
}
} // namespace
