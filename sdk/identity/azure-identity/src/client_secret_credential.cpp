// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/client_secret_credential.hpp"

#include <azure/core/http/http.hpp>
#include <azure/core/internal/http/pipeline.hpp>

#include <chrono>
#include <sstream>

using namespace Azure::Identity;

std::string const Azure::Identity::Details::g_aadGlobalAuthority
    = "https://login.microsoftonline.com/";

Azure::Core::AccessToken ClientSecretCredential::GetToken(
    Azure::Core::Context const& context,
    Azure::Core::Http::TokenRequestOptions const& tokenRequestOptions) const
{
  using namespace Azure::Core;
  using namespace Azure::Core::Http;
  using namespace Azure::Core::Internal::Http;

  static std::string const errorMsgPrefix("ClientSecretCredential::GetToken: ");
  try
  {
    Url url(m_options.AuthorityHost);
    url.AppendPath(m_tenantId);
    url.AppendPath("oauth2/v2.0/token");

    std::ostringstream body;
    body << "grant_type=client_credentials&client_id=" << Url::Encode(m_clientId)
         << "&client_secret=" << Url::Encode(m_clientSecret);

    auto const& scopes = tokenRequestOptions.Scopes;
    if (!scopes.empty())
    {
      auto scopesIter = scopes.begin();
      body << "&scope=" << Url::Encode(*scopesIter);

      auto const scopesEnd = scopes.end();
      for (++scopesIter; scopesIter != scopesEnd; ++scopesIter)
      {
        body << " " << *scopesIter;
      }
    }

    auto const bodyString = body.str();
    auto bodyStream = std::make_unique<MemoryBodyStream>(
        reinterpret_cast<uint8_t const*>(bodyString.data()), bodyString.size());

    Request request(HttpMethod::Post, url, bodyStream.get());
    bodyStream.release();

    request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
    request.AddHeader("Content-Length", std::to_string(bodyString.size()));

    std::vector<std::unique_ptr<HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<RequestIdPolicy>());

    {
      RetryOptions retryOptions;
      policies.emplace_back(std::make_unique<RetryPolicy>(retryOptions));
    }

    policies.emplace_back(std::make_unique<TransportPolicy>(m_options.TransportPolicyOptions));

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

    auto const responseBodyBegin = responseBody.begin();

    return {
        std::string(responseBodyBegin + tokenBegin, responseBodyBegin + tokenEnd),
        std::chrono::system_clock::now() + std::chrono::seconds(expiresInSeconds),
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
