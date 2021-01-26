// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/http/pipeline.hpp>
#include <azure/core/http/policy.hpp>
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
      Azure::Core::Context const&,
      Azure::Core::Http::Request&,
      Azure::Core::Http::NextHttpPolicy) const override
  {
    return nullptr;
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

TEST(Policy, ValuePolicy)
{
  using namespace Azure::Core;
  using namespace Azure::Core::Http;

  ValuePolicyOptions options;
  options.AddHeaderValues({{"HdrKey1", "HdrVal1"}, {"HdrKey2", "HdrVal0"}});
  options.AddQueryValues({{"QryKey1", "QryVal0"}, {"QryKey2", "QryVal2"}});

  options.AddHeaderValues({{"HdrKey2", "HdrVal2"}, {"HdrKey3", "HdrVal3"}});
  options.AddQueryValues({{"QryKey1", "QryVal1"}, {"QryKey3", "QryVal3"}});

  std::vector<std::unique_ptr<HttpPolicy>> policies;
  policies.emplace_back(std::make_unique<ValuePolicy>(options));
  policies.emplace_back(std::make_unique<NoOpPolicy>());
  HttpPipeline pipeline(policies);

  Request request(HttpMethod::Get, Url("https:://www.example.com"));

  pipeline.Send(GetApplicationContext(), request);

  auto headers = request.GetHeaders();
  auto queryParams = request.GetUrl().GetQueryParameters();

  ASSERT_EQ(
      headers,
      decltype(headers)({{"hdrkey1", "HdrVal1"}, {"hdrkey2", "HdrVal2"}, {"hdrkey3", "HdrVal3"}}));

  ASSERT_EQ(
      queryParams,
      decltype(queryParams)(
          {{"QryKey1", "QryVal1"}, {"QryKey2", "QryVal2"}, {"QryKey3", "QryVal3"}}));
}
