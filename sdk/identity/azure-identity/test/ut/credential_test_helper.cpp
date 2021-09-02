// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "credential_test_helper.hpp"

#include "private/environment.hpp"

#include <azure/core/platform.hpp>

#include <stdlib.h>
#include <type_traits>

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif

#include <windows.h>
#endif

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

bool const CredentialTestHelper::EnvironmentOverride::IsEnvironmentAvailable(
#if !defined(WINAPI_PARTITION_DESKTOP) \
    || WINAPI_PARTITION_DESKTOP // See azure/core/platform.hpp for explanation.
    true
#else
    false
#endif
);

void CredentialTestHelper::EnvironmentOverride::SetVariables(
    std::map<std::string, std::string> const& vars)
{
#if !defined(WINAPI_PARTITION_DESKTOP) \
    || WINAPI_PARTITION_DESKTOP // See azure/core/platform.hpp for explanation.
  for (auto var : vars)
  {
    auto const& name = var.first;
    auto const& value = var.second;

#if defined(_MSC_VER)
    static_cast<void>(_putenv((name + "=" + value).c_str()));
#else
    if (value.empty())
    {
      static_cast<void>(unsetenv(name.c_str()));
    }
    else
    {
      static_cast<void>(setenv(name.c_str(), value.c_str(), 1));
    }
#endif
  }
#endif
}

CredentialTestHelper::EnvironmentOverride::EnvironmentOverride(
    std::map<std::string, std::string> const& environment)
{
  for (auto var : environment)
  {
    m_originalEnv[var.first] = Identity::_detail::Environment::GetVariable(var.first.c_str());
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
    std::vector<Core::Credentials::TokenRequestContext> const& tokenRequestContexts,
    std::vector<TokenRequestSimulationServerResponse> const& responses,
    GetTokenCallback getToken)
{
  using Azure::Core::Context;
  using Azure::Core::Http::HttpStatusCode;
  using Azure::Core::Http::RawResponse;
  using Azure::Core::IO::MemoryBodyStream;

  auto const nResponses = responses.size();
  auto const nRequestTimes = tokenRequestContexts.size();

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

    response.AccessToken = getToken(*credential, tokenRequestContexts.at(i), Context());
    response.EarliestExpiration = earliestExpiration;
    response.LatestExpiration = std::chrono::system_clock::now();

    result.Responses.emplace_back(response);
  }

  return result;
}
