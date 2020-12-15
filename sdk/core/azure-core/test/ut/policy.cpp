// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>
#include <azure/core/http/pipeline.hpp>
#include <azure/core/http/policy.hpp>

#include <vector>

TEST(Policy, throwWhenNoTransportPolicy)
{
  // Construct pipeline without exception
  std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
  policies.push_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>("test", "test"));
  policies.push_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>("test", "test"));
  policies.push_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>("test", "test"));
  policies.push_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>("test", "test"));

  Azure::Core::Http::HttpPipeline pipeline(policies);
  Azure::Core::Http::Url url("");
  Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
  EXPECT_THROW(pipeline.Send(Azure::Core::GetApplicationContext(), request), std::invalid_argument);
}

TEST(Policy, throwWhenNoTransportPolicyMessage)
{
  // Construct pipeline without exception
  std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
  policies.push_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>("test", "test"));
  policies.push_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>("test", "test"));
  policies.push_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>("test", "test"));
  policies.push_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>("test", "test"));

  Azure::Core::Http::HttpPipeline pipeline(policies);
  Azure::Core::Http::Url url("");
  Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);

  try
  {
    pipeline.Send(Azure::Core::GetApplicationContext(), request);
  }
  catch (std::invalid_argument& err)
  {
    EXPECT_STREQ("Invalid pipeline. No transport policy found. Endless policy.", err.what());
  }
}
