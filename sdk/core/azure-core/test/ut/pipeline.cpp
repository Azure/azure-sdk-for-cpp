// Copyright (c) Microsoft Corporation. All rights reserved.
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
