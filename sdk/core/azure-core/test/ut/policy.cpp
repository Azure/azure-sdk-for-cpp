// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/http/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <gtest/gtest.h>

#include <vector>

namespace {
class NoOpPolicy : public Azure::Core::Http::HttpPolicy {
public:
  std::unique_ptr<Azure::Core::Http::HttpPolicy> Clone() const override
  {
    return std::make_unique<NoOpPolicy>(*this);
  }

  std::unique_ptr<Azure::Core::Http::RawResponse> Send(
      Azure::Core::Http::Request&,
      Azure::Core::Http::NextHttpPolicy,
      Azure::Core::Context const&) const override
  {
    return nullptr;
  }
};

// A policy to test retry state
static int retryCounterState = 0;
struct TestRetryPolicySharedState : public Azure::Core::Http::HttpPolicy
{
  std::unique_ptr<HttpPolicy> Clone() const override
  {
    return std::make_unique<TestRetryPolicySharedState>(*this);
  }

  std::unique_ptr<Azure::Core::Http::RawResponse> Send(
      Azure::Core::Http::Request& request,
      Azure::Core::Http::NextHttpPolicy nextHttpPolicy,
      Azure::Core::Context const& ctx) const override
  {
    EXPECT_EQ(retryCounterState, Azure::Core::Http::RetryPolicy::GetRetryNumber(ctx));
    retryCounterState += 1;
    return nextHttpPolicy.Send(request, ctx);
  }
};

class SuccessAfter : public Azure::Core::Http::HttpPolicy {
private:
  int m_successAfter; // Always success

public:
  std::unique_ptr<Azure::Core::Http::HttpPolicy> Clone() const override
  {
    return std::make_unique<SuccessAfter>(*this);
  }

  SuccessAfter(int successAfter = 1) : m_successAfter(successAfter) {}

  std::unique_ptr<Azure::Core::Http::RawResponse> Send(
      Azure::Core::Http::Request&,
      Azure::Core::Http::NextHttpPolicy,
      Azure::Core::Context const& context) const override
  {
    auto retryNumber = Azure::Core::Http::RetryPolicy::GetRetryNumber(context);
    if (retryNumber == m_successAfter)
    {
      auto response = std::make_unique<Azure::Core::Http::RawResponse>(
          1, 1, Azure::Core::Http::HttpStatusCode::Ok, "All Fine");
      return response;
    }

    auto retryResponse = std::make_unique<Azure::Core::Http::RawResponse>(
        1, 1, Azure::Core::Http::HttpStatusCode::ServiceUnavailable, "retry please :)");
    return retryResponse;
  }
};

} // namespace

TEST(Policy, throwWhenNoTransportPolicy)
{
  // Construct pipeline without exception
  std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
  policies.push_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>("test", "test"));
  policies.push_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>("test", "test"));
  policies.push_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>("test", "test"));
  policies.push_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>("test", "test"));

  Azure::Core::Internal::Http::HttpPipeline pipeline(policies);
  Azure::Core::Http::Url url("");
  Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
  EXPECT_THROW(pipeline.Send(request, Azure::Core::GetApplicationContext()), std::invalid_argument);
}

TEST(Policy, throwWhenNoTransportPolicyMessage)
{
  // Construct pipeline without exception
  std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
  policies.push_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>("test", "test"));
  policies.push_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>("test", "test"));
  policies.push_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>("test", "test"));
  policies.push_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>("test", "test"));

  Azure::Core::Internal::Http::HttpPipeline pipeline(policies);
  Azure::Core::Http::Url url("");
  Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

  try
  {
    pipeline.Send(request, Azure::Core::GetApplicationContext());
  }
  catch (const std::invalid_argument& ex)
  {
    EXPECT_STREQ("Invalid pipeline. No transport policy found. Endless policy.", ex.what());
  }
}

TEST(Policy, ValuePolicy)
{
  using namespace Azure::Core;
  using namespace Azure::Core::Http;
  using namespace Azure::Core::Internal::Http;

  Azure::Core::Http::Internal::ValueOptions options
      = {{{"hdrkey1", "HdrVal1"}, {"hdrkey2", "HdrVal2"}},
         {{"QryKey1", "QryVal1"}, {"QryKey2", "QryVal2"}}};

  std::vector<std::unique_ptr<HttpPolicy>> policies;
  policies.emplace_back(std::make_unique<Azure::Core::Http::Internal::ValuePolicy>(options));
  policies.emplace_back(std::make_unique<NoOpPolicy>());
  HttpPipeline pipeline(policies);

  Request request(HttpMethod::Get, Url("https:://www.example.com"));

  pipeline.Send(request, GetApplicationContext());

  auto headers = request.GetHeaders();
  auto queryParams = request.GetUrl().GetQueryParameters();

  ASSERT_EQ(headers, decltype(headers)({{"hdrkey1", "HdrVal1"}, {"hdrkey2", "HdrVal2"}}));
  ASSERT_EQ(queryParams, decltype(queryParams)({{"QryKey1", "QryVal1"}, {"QryKey2", "QryVal2"}}));
}

TEST(Policy, RetryPolicyCounter)
{
  using namespace Azure::Core;
  using namespace Azure::Core::Http;
  using namespace Azure::Core::Internal::Http;

  // Check when there's no info about retry on the context
  auto initialContext = GetApplicationContext();
  EXPECT_EQ(-1, RetryPolicy::GetRetryNumber(initialContext));

  // Pipeline with retry test
  std::vector<std::unique_ptr<HttpPolicy>> policies;
  RetryOptions opt;
  policies.push_back(std::make_unique<RetryPolicy>(opt));
  policies.push_back(std::make_unique<TestRetryPolicySharedState>());
  policies.push_back(std::make_unique<SuccessAfter>());

  HttpPipeline pipeline(policies);
  Request request(HttpMethod::Get, Url("url"));
  pipeline.Send(request, initialContext);
}

TEST(Policy, RetryPolicyRetryCycle)
{
  using namespace Azure::Core;
  using namespace Azure::Core::Http;
  using namespace Azure::Core::Internal::Http;

  // Pipeline with retry test
  std::vector<std::unique_ptr<HttpPolicy>> policies;
  RetryOptions opt;
  opt.RetryDelay = std::chrono::milliseconds(10);
  policies.push_back(std::make_unique<RetryPolicy>(opt));
  policies.push_back(std::make_unique<TestRetryPolicySharedState>());
  policies.push_back(std::make_unique<SuccessAfter>(3));

  HttpPipeline pipeline(policies);
  Request request(HttpMethod::Get, Url("url"));
  pipeline.Send(request, GetApplicationContext());
}
