//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <gtest/gtest.h>

#include <vector>

TEST(Pipeline, createPipeline)
{
  // Construct pipeline without exception
  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;
  policies.push_back(
      std::make_unique<Azure::Core::Http::Policies::_internal::TelemetryPolicy>("test", "test"));

  EXPECT_NO_THROW(Azure::Core::Http::_internal::HttpPipeline pipeline(policies));
}

TEST(Pipeline, createEmptyPipeline)
{
  // throw invalid arg for empty policies
  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;
  EXPECT_THROW(
      Azure::Core::Http::_internal::HttpPipeline pipeline(policies), std::invalid_argument);
}

TEST(Pipeline, clonePipeline)
{
  // Construct pipeline without exception and clone
  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;
  policies.push_back(
      std::make_unique<Azure::Core::Http::Policies::_internal::TelemetryPolicy>("test", "test"));

  Azure::Core::Http::_internal::HttpPipeline pipeline(policies);
  EXPECT_NO_THROW(Azure::Core::Http::_internal::HttpPipeline pipeline2(pipeline));
}

TEST(Pipeline, refrefPipeline)
{
  // Construct pipeline without exception
  EXPECT_NO_THROW(Azure::Core::Http::_internal::HttpPipeline pipeline(
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>>(1)));
}

TEST(Pipeline, refrefEmptyPipeline)
{
  // Construct pipeline with invalid exception with move constructor
  EXPECT_THROW(
      Azure::Core::Http::_internal::HttpPipeline pipeline(
          std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>>(0)),
      std::invalid_argument);
}

TEST(Pipeline, attestationConstructor)
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif // Construct pipeline without exception
  EXPECT_NO_THROW(Azure::Core::Http::_internal::HttpPipeline pipeline(
      Azure::Core::_internal::ClientOptions(),
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>>(0),
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>>(0)));
#ifdef _MSC_VER
#pragma warning(pop)
#elif defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif // _MSC_VER
}

TEST(Pipeline, AdditionalPolicies)
{
  class TestPolicy : public Azure::Core::Http::Policies::HttpPolicy {
    int* m_cloneCount;

  public:
    TestPolicy(int* cloneCount) : m_cloneCount(cloneCount) {}

    std::unique_ptr<HttpPolicy> Clone() const override
    {
      ++(*m_cloneCount);
      return std::make_unique<TestPolicy>(*this);
    }

    std::unique_ptr<Azure::Core::Http::RawResponse> Send(
        Azure::Core::Http::Request& request,
        Azure::Core::Http::Policies::NextHttpPolicy nextPolicy,
        Azure::Core::Context const& context) const override
    {
      return nextPolicy.Send(request, context);
    }
  };

  int perCallPolicyCloneCount = 0;
  int perCallClientPolicyCloneCount = 0;
  int perRetryPolicyCloneCount = 0;
  int perRetryClientPolicyCloneCount = 0;

  auto options = Azure::Core::_internal::ClientOptions();

  using PolicyVector = std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>>;
  PolicyVector perCallPolicies;
  PolicyVector perRetryPolicies;

  {
    struct InitHelper
    {
      PolicyVector* Policies;
      int* Counter;
    };

    std::vector<InitHelper> const initializations = {
        {&perCallPolicies, &perCallPolicyCloneCount},
        {&options.PerOperationPolicies, &perCallClientPolicyCloneCount},
        {&perRetryPolicies, &perRetryPolicyCloneCount},
        {&options.PerRetryPolicies, &perRetryClientPolicyCloneCount},
    };

    const int size = static_cast<int>(initializations.size());
    for (int i = 0; i < size; ++i)
    {
      for (int j = 0; j < i + 2; ++j)
      {
        initializations[i].Policies->emplace_back(
            std::make_unique<TestPolicy>(initializations[i].Counter));
      }
    }
  }

  EXPECT_NO_THROW(static_cast<void>(Azure::Core::Http::_internal::HttpPipeline(
      options, "Test", "1.0.0", std::move(perRetryPolicies), std::move(perCallPolicies))));

  EXPECT_EQ(perCallPolicyCloneCount, 2);
  EXPECT_EQ(perCallClientPolicyCloneCount, 3);
  EXPECT_EQ(perRetryPolicyCloneCount, 4);
  EXPECT_EQ(perRetryClientPolicyCloneCount, 5);
}
