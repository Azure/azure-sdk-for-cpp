// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/client_secret_credential.hpp"

#include <azure/core/http/curl/curl.hpp>
#include <azure/core/http/pipeline.hpp>

#include <iomanip>
#include <sstream>

using namespace Azure::Identity;

namespace {
std::string UrlEncode(std::string const& s)
{
  std::ostringstream encoded;
  encoded << std::hex;

  for (auto c : s)
  {
    if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
        || (c == '-' || c == '.' || c == '_' || c == '~'))
    {
      encoded << c;
    }
    else
    {
      encoded << std::uppercase;
      encoded << '%' << std::setw(2) << int((unsigned char)c);
      encoded << std::nouppercase;
    }
  }

  return encoded.str();
}
} // namespace

std::string const ClientSecretCredential::g_aadGlobalAuthority
    = "https://login.microsoftonline.com/";

Azure::Core::AccessToken ClientSecretCredential::GetToken(
    Azure::Core::Context const& context,
    std::vector<std::string> const& scopes) const
{
  using namespace Azure::Core;
  using namespace Azure::Core::Http;

  static std::string const errorMsgPrefix("ClientSecretCredential::GetToken: ");
  try
  {
    Url url(m_authority);
    url.AppendPath(m_tenantId);
    url.AppendPath("oauth2/v2.0/token");

    std::ostringstream body;
    // TODO: Use encoding from Http::Url::Encode once it becomes public
    body << "grant_type=client_credentials&client_id=" << UrlEncode(m_clientId)
         << "&client_secret=" << UrlEncode(m_clientSecret);

    if (!scopes.empty())
    {
      auto scopesIter = scopes.begin();
      body << "&scope=" << UrlEncode(*scopesIter);

      auto const scopesEnd = scopes.end();
      for (++scopesIter; scopesIter != scopesEnd; ++scopesIter)
      {
        body << " " << *scopesIter;
      }
    }

    auto const bodyString = body.str();
    auto bodyStream
        = std::make_unique<MemoryBodyStream>((uint8_t*)bodyString.data(), bodyString.size());

    Request request(HttpMethod::Post, url, bodyStream.get());
    bodyStream.release();

    request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
    request.AddHeader("Content-Length", std::to_string(bodyString.size()));

    std::shared_ptr<HttpTransport> transport = std::make_unique<CurlTransport>();

    std::vector<std::unique_ptr<HttpPolicy>> policies;
    policies.push_back(std::make_unique<RequestIdPolicy>());

    RetryOptions retryOptions;
    policies.push_back(std::make_unique<RetryPolicy>(retryOptions));

    policies.push_back(std::make_unique<TransportPolicy>(std::move(transport)));

    HttpPipeline httpPipeline(policies);

    std::shared_ptr<RawResponse> response = httpPipeline.Send(context, request);

    if (!response)
    {
      throw AuthenticationException(errorMsgPrefix + "null response");
    }

    auto const statusCode = response->GetStatusCode();
    if (statusCode != HttpStatusCode::Ok)
    {
      std::ostringstream errorMsg;
      errorMsg << errorMsgPrefix << "error response: "
               << static_cast<std::underlying_type<HttpStatusCode>::type>(statusCode) << " "
               << response->GetReasonPhrase();

      throw AuthenticationException(errorMsg.str());
    }

    auto const& responseBodyVector = response->GetBody();
    std::string responseBody(responseBodyVector.begin(), responseBodyVector.end());

    // TODO: use JSON parser.
    auto const responseBodySize = responseBody.size();

    static std::string const jsonExpiresIn = "expires_in";
    static std::string const jsonAccessToken = "access_token";

    auto responseBodyPos = responseBody.find(':', responseBody.find(jsonExpiresIn));
    if (responseBodyPos == std::string::npos)
    {
      std::ostringstream errorMsg;
      errorMsg << errorMsgPrefix << "response json: \'" << jsonExpiresIn << "\' not found.";

      throw AuthenticationException(errorMsg.str());
    }

    for (; responseBodyPos < responseBodySize; ++responseBodyPos)
    {
      auto c = responseBody[responseBodyPos];
      if (c != ':' && c != ' ' && c != '\"' && c != '\'')
      {
        break;
      }
    }

    long long expiresInSeconds = 0;
    for (; responseBodyPos < responseBodySize; ++responseBodyPos)
    {
      auto c = responseBody[responseBodyPos];
      if (c < '0' || c > '9')
      {
        break;
      }

      expiresInSeconds = (expiresInSeconds * 10) + (static_cast<long long>(c) - '0');
    }

    responseBodyPos = responseBody.find(':', responseBody.find(jsonAccessToken));
    if (responseBodyPos == std::string::npos)
    {
      std::ostringstream errorMsg;
      errorMsg << errorMsgPrefix << "response json: \'" << jsonAccessToken << "\' not found.";

      throw AuthenticationException(errorMsg.str());
    }

    for (; responseBodyPos < responseBodySize; ++responseBodyPos)
    {
      auto c = responseBody[responseBodyPos];
      if (c != ':' && c != ' ' && c != '\"' && c != '\'')
      {
        break;
      }
    }

    auto const tokenBegin = responseBodyPos;
    for (; responseBodyPos < responseBodySize; ++responseBodyPos)
    {
      auto c = responseBody[responseBodyPos];
      if (c == '\"' || c == '\'')
      {
        break;
      }
    }
    auto const tokenEnd = responseBodyPos;

    expiresInSeconds -= 2 * 60;
    auto const responseBodyBegin = responseBody.begin();

    return {
        std::string(responseBodyBegin + tokenBegin, responseBodyBegin + tokenEnd),
        std::chrono::system_clock::now()
            + std::chrono::seconds(expiresInSeconds < 0 ? 0 : expiresInSeconds),
    };
  }
  catch (AuthenticationException const&)
  {
    throw;
  }
  catch (std::exception const& e)
  {
    throw AuthenticationException(e.what());
  }
  catch (...)
  {
    throw AuthenticationException("unknown error");
  }
}
