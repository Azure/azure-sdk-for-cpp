// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <azure/core/http/pipeline.hpp>
#include <azure/core/http/policy.hpp>

#include <vector>

TEST(Logging, createPipeline)
{
  // Construct pipeline without exception
  std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
  policies.push_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>("test", "test"));

  EXPECT_NO_THROW(Azure::Core::Http::HttpPipeline pipeline(policies));
}

TEST(Logging, createEmptyPipeline)
{
  // throw invalid arg for empty policies
  std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
  EXPECT_THROW(Azure::Core::Http::HttpPipeline pipeline(policies), std::invalid_argument);
}

TEST(Logging, clonePipeline)
{
  // Construct pipeline without exception and clone
  std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
  policies.push_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>("test", "test"));

  Azure::Core::Http::HttpPipeline pipeline(policies);
  EXPECT_NO_THROW(Azure::Core::Http::HttpPipeline pipeline2(pipeline));
}

TEST(Logging, refrefPipeline)
{
  // Construct pipeline without exception
  EXPECT_NO_THROW(Azure::Core::Http::HttpPipeline pipeline(
      std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>>(1)));
}

TEST(Logging, refrefEmptyPipeline)
{
  // Construct pipeline with invalid exception with move constructor
  EXPECT_THROW(
      Azure::Core::Http::HttpPipeline pipeline(
          std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>>(0)),
      std::invalid_argument);
}
