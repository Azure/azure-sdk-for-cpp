// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <credentials/credentials.hpp>
#include <http/curl/curl.hpp>
#include <http/http.hpp>
#include <http/pipeline.hpp>
#include <http/stream.hpp>
#include <sstream>
#include <stdexcept>

using namespace Azure::Core::Credentials;

AccessToken ClientSecretCredential::GetToken(
    Azure::Core::Context& context,
    std::vector<std::string> const& scopes) const
{
  static std::string const errorMsgPrefix("ClientSecretCredential::GetToken: ");
  try
  {
    std::ostringstream url;
    url << "https://login.microsoftonline.com/" << m_tenantId << "/oauth2/v2.0/token";

    std::ostringstream body;
    body << "grant_type=client_credentials&client_id=" << m_clientId
         << "&client_secret=" << m_clientSecret;

    if (!scopes.empty())
    {
      auto scopesIter = scopes.begin();
      body << "&scope=" << *scopesIter;

      auto const scopesEnd = scopes.end();
      for (++scopesIter; scopesIter != scopesEnd; ++scopesIter)
      {
        body << " " << *scopesIter;
      }
    }

    auto const bodyString = body.str();
    std::vector<uint8_t> bodyVec;
    bodyVec.reserve(bodyString.size());
    for (auto c : bodyString)
    {
      bodyVec.push_back(static_cast<uint8_t>(c));
    }

    auto const bodyStream = std::make_unique<Http::BodyStream>(new Http::MemoryBodyStream(bodyVec));

    Http::Request request(Http::HttpMethod::Get, url.str(), bodyStream.get());
    bodyStream.release();

    std::shared_ptr<Http::HttpTransport> transport = std::make_unique<Http::CurlTransport>();

    std::vector<std::unique_ptr<Http::HttpPolicy>> policies;
    policies.push_back(std::make_unique<Http::RequestIdPolicy>());

    Http::RetryOptions retryOptions;
    policies.push_back(std::make_unique<Http::RetryPolicy>(retryOptions));

    policies.push_back(std::make_unique<Http::TransportPolicy>(std::move(transport)));

    Http::HttpPipeline httpPipeline(policies);

    std::shared_ptr<Http::Response> response = httpPipeline.Send(context, request);

    if (!response)
    {
      throw AuthenticationException(errorMsgPrefix + "null response");
    }

    auto const statusCode = response->GetStatusCode();
    if (statusCode != Azure::Core::HttpStatusCode::Ok)
    {
      std::ostringstream errorMsg;
      errorMsg << errorMsgPrefix << "error response: "
               << static_cast<typename std::underlying_type<Http::HttpStatusCode>::type>(statusCode)
               << " " << response->GetReasonPhrase();

      throw AuthenticationException(errorMsg.str());
    }

    auto const bodyVector = response->GetBodyBuffer();
    std::string responseBody(bodyVector.begin(), bodyVector.end());

    // TODO: use JSON parser.
    auto const responseBodySize = responseBody.size();

    auto responseBodyPos = responseBody.find(':', responseBody.find("expires_in"));
    for (; responseBodyPos < responseBodySize; ++responseBodyPos)
    {
      auto c = responseBody[responseBodyPos];
      if (c != ' ' && c != '\"' && c != '\'')
      {
        break;
      }
    }

    long long expiresInSeconds = 0;
    for (; responseBodyPos < responseBodySize; ++responseBodyPos)
    {
      auto c = responseBody[responseBodyPos];
      if (c <= '0' || c >= '9')
      {
        break;
      }

      expiresInSeconds = (expiresInSeconds * 10) + (c - '0');
    }

    responseBodyPos = responseBody.find(':', responseBody.find("access_token"));
    for (; responseBodyPos < responseBodySize; ++responseBodyPos)
    {
      auto c = responseBody[responseBodyPos];
      if (c != ' ' && c != '\"' && c != '\'')
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
        std::string(
            std::advance(responseBodyBegin, tokenBegin), std::advance(responseBodyBegin, tokenEnd)),
        std::chrono::system_clock::now()
            + std::chrono::seconds(expiresInSeconds < 0 ? 0 : expiresInSeconds)};
  }
  catch (AuthenticationException const&)
  {
    throw;
  }
  catch (std::exception const& e)
  {
    throw new AuthenticationException(e.what());
  }
  catch (...)
  {
    throw new AuthenticationException("unknown error");
  }
}
