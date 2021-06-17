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

CredentialTestHelper::TokenRequestSimulationResult CredentialTestHelper::SimulateTokenRequest(
    CredentialTestHelper::CreateCredentialCallback const& createCredential,
    Core::Credentials::TokenRequestContext const& tokenRequestContext,
    std::vector<std::pair<Core::Http::HttpStatusCode, std::string>> const& responses)
{
  using Azure::Core::Context;
  using Azure::Core::Http::HttpStatusCode;
  using Azure::Core::Http::RawResponse;
  using Azure::Core::IO::MemoryBodyStream;

  auto const nResponses = responses.size();

  TokenRequestSimulationResult result;
  {
    result.Requests.reserve(nResponses);
  }

  std::vector<std::vector<uint8_t>> responseBuffers;
  {
    responseBuffers.reserve(nResponses);
    for (auto const& response : responses)
    {
      auto const& responseStr = response.second;
      responseBuffers.emplace_back(std::vector<uint8_t>(responseStr.begin(), responseStr.end()));
    }
  }

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

        auto response = std::make_unique<RawResponse>(1, 1, responses.at(i).first, "Test");
        response->SetBodyStream(std::make_unique<MemoryBodyStream>(responseBuffers.at(i)));

        result.Response.EarliestExpiration = std::chrono::system_clock::now();

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

  result.Response.AccessToken = credential->GetToken(tokenRequestContext, Context());
  result.Response.LatestExpiration = std::chrono::system_clock::now();

  return result;
}
