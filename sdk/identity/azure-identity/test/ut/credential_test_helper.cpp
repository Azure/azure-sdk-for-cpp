// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "credential_test_helper.hpp"

#include <azure/core/internal/environment.hpp>

#include <stdlib.h>
#include <type_traits>

namespace {
class TestTransport final : public Azure::Core::Http::HttpTransport {
public:
  using SendCallback = std::function<std::unique_ptr<Azure::Core::Http::RawResponse>(
      Azure::Core::Http::Request& request,
      Azure::Core::Context const& context)>;

private:
  SendCallback m_sendCallback;

public:
  TestTransport(SendCallback send) : m_sendCallback(send) {}

  std::unique_ptr<Azure::Core::Http::RawResponse> Send(
      Azure::Core::Http::Request& request,
      Azure::Core::Context const& context) override
  {
    return m_sendCallback(request, context);
  }
};
} // namespace

using namespace Azure::Identity::Test::_detail;

void CredentialTestHelper::EnvironmentOverride::SetVariables(
    std::map<std::string, std::string> const& vars)
{
  for (auto var : vars)
  {
    Azure::Core::_internal::Environment::SetVariable(var.first.c_str(), var.second.c_str());
  }
}

CredentialTestHelper::EnvironmentOverride::EnvironmentOverride(
    std::map<std::string, std::string> const& environment)
{
  for (auto var : environment)
  {
    m_originalEnv[var.first] = Azure::Core::_internal::Environment::GetVariable(var.first.c_str());
  }

  SetVariables(environment);
}

namespace {
Azure::Core::Credentials::AccessToken GetToken(
    Azure::Core::Credentials::TokenCredential const& credential,
    Azure::Core::Credentials::TokenRequestContext const& tokenRequestContext,
    Azure::Core::Context const& context)
{
  return credential.GetToken(tokenRequestContext, context);
}
} // namespace

CredentialTestHelper::GetTokenCallback const CredentialTestHelper::DefaultGetToken(GetToken);

CredentialTestHelper::TokenRequestSimulationResult CredentialTestHelper::SimulateTokenRequest(
    CredentialTestHelper::CreateCredentialCallback const& createCredential,
    std::vector<decltype(Azure::Core::Credentials::TokenRequestContext::Scopes)> const&
        tokenRequestContextScopes,
    std::vector<TokenRequestSimulationServerResponse> const& responses,
    GetTokenCallback getToken)
{
  using Azure::Core::Context;
  using Azure::Core::Http::HttpStatusCode;
  using Azure::Core::Http::RawResponse;
  using Azure::Core::IO::MemoryBodyStream;

  auto const nResponses = responses.size();
  auto const nRequestTimes = tokenRequestContextScopes.size();

  TokenRequestSimulationResult result;
  {
    result.Requests.reserve(nResponses);
    result.Responses.reserve(nRequestTimes);
  }

  std::vector<std::vector<uint8_t>> responseBuffers;
  {
    responseBuffers.reserve(nResponses);
    for (auto const& response : responses)
    {
      auto const& responseStr = response.Body;
      responseBuffers.emplace_back(std::vector<uint8_t>(responseStr.begin(), responseStr.end()));
    }
  }

  std::chrono::system_clock::time_point earliestExpiration = std::chrono::system_clock::now();
  std::vector<TestTransport::SendCallback> callbacks;
  {
    for (std::remove_cv<decltype(nResponses)>::type i = 0; i < nResponses; ++i)
    {
      callbacks.emplace_back([&, i](auto request, auto context) {
        auto const bodyVec = request.GetBodyStream()->ReadToEnd(context);

        result.Requests.push_back(
            {request.GetMethod(),
             request.GetUrl().GetAbsoluteUrl(),
             request.GetHeaders(),
             std::string(bodyVec.begin(), bodyVec.end())});

        auto const& serverResponse = responses.at(i);

        auto response = std::make_unique<RawResponse>(1, 1, serverResponse.StatusCode, "Test");
        response->SetBodyStream(std::make_unique<MemoryBodyStream>(responseBuffers.at(i)));

        for (auto const& header : serverResponse.Headers)
        {
          response->SetHeader(header.first, header.second);
        }

        earliestExpiration = std::chrono::system_clock::now();

        return response;
      });
    }
  }

  auto const credential
      = createCredential(std::make_shared<TestTransport>([&](auto request, auto context) {
          auto const callback = callbacks.at(0);
          callbacks.erase(callbacks.begin());
          return callback(request, context);
        }));

  for (std::remove_cv<decltype(nRequestTimes)>::type i = 0; i < nRequestTimes; ++i)
  {
    TokenRequestSimulationResult::ResponseInfo response{};

    Azure::Core::Credentials::TokenRequestContext tokenRequestContext;
    tokenRequestContext.Scopes = tokenRequestContextScopes.at(i);
    tokenRequestContext.MinimumExpiration = std::chrono::hours(1000000);

    response.AccessToken = getToken(*credential, tokenRequestContext, Context());
    response.EarliestExpiration = earliestExpiration;
    response.LatestExpiration = std::chrono::system_clock::now();

    result.Responses.emplace_back(response);
  }

  return result;
}
