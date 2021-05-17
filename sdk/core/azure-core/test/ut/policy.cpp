// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <gtest/gtest.h>

#include <vector>

namespace {
class NoOpPolicy final : public Azure::Core::Http::Policies::HttpPolicy {
public:
  std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy> Clone() const override
  {
    return std::make_unique<NoOpPolicy>(*this);
  }

  std::unique_ptr<Azure::Core::Http::RawResponse> Send(
      Azure::Core::Http::Request&,
      Azure::Core::Http::Policies::NextHttpPolicy,
      Azure::Core::Context const&) const override
  {
    return nullptr;
  }
};

// A policy to test retry state
static int retryCounterState = 0;
struct TestRetryPolicySharedState final : public Azure::Core::Http::Policies::HttpPolicy
{
  std::unique_ptr<HttpPolicy> Clone() const override
  {
    return std::make_unique<TestRetryPolicySharedState>(*this);
  }

  std::unique_ptr<Azure::Core::Http::RawResponse> Send(
      Azure::Core::Http::Request& request,
      Azure::Core::Http::Policies::NextHttpPolicy nextPolicy,
      Azure::Core::Context const& context) const override
  {
    EXPECT_EQ(
        retryCounterState,
        Azure::Core::Http::Policies::_internal::RetryPolicy::GetRetryCount(context));
    retryCounterState += 1;
    return nextPolicy.Send(request, context);
  }
};

Azure::Core::Context::Key const TheKey;

struct TestContextTreeIntegrity final : public Azure::Core::Http::Policies::HttpPolicy
{
  std::unique_ptr<HttpPolicy> Clone() const override
  {
    return std::make_unique<TestContextTreeIntegrity>(*this);
  }

  std::unique_ptr<Azure::Core::Http::RawResponse> Send(
      Azure::Core::Http::Request& request,
      Azure::Core::Http::Policies::NextHttpPolicy nextPolicy,
      Azure::Core::Context const& context) const override
  {
    std::string valueHolder;
    EXPECT_TRUE(context.TryGetValue<std::string>(TheKey, valueHolder));
    EXPECT_EQ("TheValue", valueHolder);
    return nextPolicy.Send(request, context);
  }
};

class SuccessAfter final : public Azure::Core::Http::Policies::HttpPolicy {
private:
  int m_successAfter; // Always success

public:
  std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy> Clone() const override
  {
    return std::make_unique<SuccessAfter>(*this);
  }

  SuccessAfter(int successAfter = 1) : m_successAfter(successAfter) {}

  std::unique_ptr<Azure::Core::Http::RawResponse> Send(
      Azure::Core::Http::Request&,
      Azure::Core::Http::Policies::NextHttpPolicy,
      Azure::Core::Context const& context) const override
  {
    auto retryNumber = Azure::Core::Http::Policies::_internal::RetryPolicy::GetRetryCount(context);
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
  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;
  policies.push_back(
      std::make_unique<Azure::Core::Http::Policies::_internal::TelemetryPolicy>("test", "test"));
  policies.push_back(
      std::make_unique<Azure::Core::Http::Policies::_internal::TelemetryPolicy>("test", "test"));
  policies.push_back(
      std::make_unique<Azure::Core::Http::Policies::_internal::TelemetryPolicy>("test", "test"));
  policies.push_back(
      std::make_unique<Azure::Core::Http::Policies::_internal::TelemetryPolicy>("test", "test"));

  Azure::Core::Http::_internal::HttpPipeline pipeline(policies);
  Azure::Core::Url url("");
  Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
  EXPECT_THROW(
      pipeline.Send(request, Azure::Core::Context::ApplicationContext), std::invalid_argument);
}

TEST(Policy, throwWhenNoTransportPolicyMessage)
{
  // Construct pipeline without exception
  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;
  policies.push_back(
      std::make_unique<Azure::Core::Http::Policies::_internal::TelemetryPolicy>("test", "test"));
  policies.push_back(
      std::make_unique<Azure::Core::Http::Policies::_internal::TelemetryPolicy>("test", "test"));
  policies.push_back(
      std::make_unique<Azure::Core::Http::Policies::_internal::TelemetryPolicy>("test", "test"));
  policies.push_back(
      std::make_unique<Azure::Core::Http::Policies::_internal::TelemetryPolicy>("test", "test"));

  Azure::Core::Http::_internal::HttpPipeline pipeline(policies);
  Azure::Core::Url url("");
  Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

  try
  {
    pipeline.Send(request, Azure::Core::Context::ApplicationContext);
  }
  catch (const std::invalid_argument& ex)
  {
    EXPECT_STREQ("Invalid pipeline. No transport policy found. Endless policy.", ex.what());
  }
}

TEST(Policy, RetryPolicyCounter)
{
  using namespace Azure::Core;
  using namespace Azure::Core::Http;
  using namespace Azure::Core::Http::_internal;
  using namespace Azure::Core::Http::Policies;
  using namespace Azure::Core::Http::Policies::_internal;
  // Clean the validation global state
  retryCounterState = 0;

  // Check when there's no info about retry on the context
  auto initialContext = Context::ApplicationContext;
  EXPECT_EQ(-1, RetryPolicy::GetRetryCount(initialContext));

  // Pipeline with retry test
  std::vector<std::unique_ptr<HttpPolicy>> policies;
  RetryOptions opt;
  // Make retry policy not to take too much time for this test
  opt.RetryDelay = std::chrono::milliseconds(10);
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
  using namespace Azure::Core::Http::_internal;
  using namespace Azure::Core::Http::Policies;
  using namespace Azure::Core::Http::Policies::_internal;
  // Clean the validation global state
  retryCounterState = 0;

  // Pipeline with retry test
  std::vector<std::unique_ptr<HttpPolicy>> policies;
  RetryOptions opt;
  opt.RetryDelay = std::chrono::milliseconds(10);
  policies.push_back(std::make_unique<RetryPolicy>(opt));
  policies.push_back(std::make_unique<TestRetryPolicySharedState>());
  policies.push_back(std::make_unique<SuccessAfter>(3));

  HttpPipeline pipeline(policies);
  Request request(HttpMethod::Get, Url("url"));
  pipeline.Send(request, Context::ApplicationContext);
}

// Makes sure that the context tree is not corrupted/broken by some policy
TEST(Policy, RetryPolicyKeepContext)
{
  using namespace Azure::Core;
  using namespace Azure::Core::Http;
  using namespace Azure::Core::Http::_internal;
  using namespace Azure::Core::Http::Policies;
  using namespace Azure::Core::Http::Policies::_internal;
  // Clean the validation global state
  retryCounterState = 0;

  // Pipeline with retry test
  std::vector<std::unique_ptr<HttpPolicy>> policies;
  RetryOptions opt;
  opt.RetryDelay = std::chrono::milliseconds(10);
  policies.push_back(std::make_unique<RetryPolicy>(opt));
  policies.push_back(std::make_unique<TestRetryPolicySharedState>());
  policies.push_back(std::make_unique<TestContextTreeIntegrity>());
  policies.push_back(std::make_unique<SuccessAfter>(3));

  HttpPipeline pipeline(policies);
  Request request(HttpMethod::Get, Url("url"));
  auto withValueContext = Context::ApplicationContext.WithValue(TheKey, std::string("TheValue"));
  pipeline.Send(request, withValueContext);
}
