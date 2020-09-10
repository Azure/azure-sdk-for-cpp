// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/body_stream.hpp>
#include <azure/core/http/curl/curl.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/pipeline.hpp>

#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <stdexcept>

using namespace Azure::Core::Credentials;

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

std::string const Azure::Core::Credentials::ClientSecretCredential::g_aadGlobalAuthority
    = "https://login.microsoftonline.com/";

AccessToken Azure::Core::Credentials::ClientSecretCredential::GetToken(
    Context const& context,
    std::vector<std::string> const& scopes) const
{
  static std::string const errorMsgPrefix("ClientSecretCredential::GetToken: ");
  try
  {
    Http::Url url(m_authority);
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
        = std::make_unique<Http::MemoryBodyStream>((uint8_t*)bodyString.data(), bodyString.size());

    Http::Request request(Http::HttpMethod::Post, url, bodyStream.get());
    bodyStream.release();

    request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
    request.AddHeader("Content-Length", std::to_string(bodyString.size()));

    std::shared_ptr<Http::HttpTransport> transport = std::make_unique<Http::CurlTransport>();

    std::vector<std::unique_ptr<Http::HttpPolicy>> policies;
    policies.push_back(std::make_unique<Http::RequestIdPolicy>());

    Http::RetryOptions retryOptions;
    policies.push_back(std::make_unique<Http::RetryPolicy>(retryOptions));

    policies.push_back(std::make_unique<Http::TransportPolicy>(std::move(transport)));

    Http::HttpPipeline httpPipeline(policies);

    std::shared_ptr<Http::RawResponse> response = httpPipeline.Send(context, request);

    if (!response)
    {
      throw AuthenticationException(errorMsgPrefix + "null response");
    }

    auto const statusCode = response->GetStatusCode();
    if (statusCode != Http::HttpStatusCode::Ok)
    {
      std::ostringstream errorMsg;
      errorMsg << errorMsgPrefix << "error response: "
               << static_cast<std::underlying_type<Http::HttpStatusCode>::type>(statusCode) << " "
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

Azure::Core::Credentials::EnvironmentCredential::EnvironmentCredential()
{
#ifdef _MSC_VER
#pragma warning(push)
// warning C4996: 'getenv': This function or variable may be unsafe. Consider using _dupenv_s
// instead.
#pragma warning(disable : 4996)
#endif

  auto tenantId = std::getenv("AZURE_TENANT_ID");
  auto clientId = std::getenv("AZURE_CLIENT_ID");

  auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
  auto authority = std::getenv("AZURE_AUTHORITY_HOST");

  // auto username = std::getenv("AZURE_USERNAME");
  // auto password = std::getenv("AZURE_PASSWORD");
  //
  // auto clientCertificatePath = std::getenv("AZURE_CLIENT_CERTIFICATE_PATH");

#ifdef _MSC_VER
#pragma warning(pop)
#endif

  if (tenantId != nullptr && clientId != nullptr)
  {
    if (clientSecret != nullptr)
    {
      if (authority != nullptr)
      {
        m_credentialImpl.reset(
            new ClientSecretCredential(tenantId, clientId, clientSecret, authority));
      }
      else
      {
        m_credentialImpl.reset(new ClientSecretCredential(tenantId, clientId, clientSecret));
      }
    }
    // TODO: These credential types are not implemented. Uncomment when implemented.
    // else if (username != nullptr && password != nullptr)
    //{
    //  m_credentialImpl.reset(
    //      new UsernamePasswordCredential(username, password, tenantId, clientId));
    //}
    // else if (clientCertificatePath != nullptr)
    //{
    //  m_credentialImpl.reset(
    //      new ClientCertificateCredential(tenantId, clientId, clientCertificatePath));
    //}
  }
}

AccessToken Azure::Core::Credentials::EnvironmentCredential::GetToken(
    Context const& context,
    std::vector<std::string> const& scopes) const
{
  if (!m_credentialImpl)
  {
    throw AuthenticationException("EnvironmentCredential authentication unavailable. "
                                  "Environment variables are not fully configured.");
  }

  return m_credentialImpl->GetToken(context, scopes);
}
