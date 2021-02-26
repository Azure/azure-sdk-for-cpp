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
      Azure::Core::Context const&,
      Azure::Core::Http::Request&,
      std::vector<std::unique_ptr<HttpPolicy>>::const_iterator) const override
  {
    return nullptr;
  }
};
} // namespace

TEST(Policy, ValuePolicy)
{
  using namespace Azure::Core;
  using namespace Azure::Core::Http;
  using namespace Azure::Core::Internal::Http;

  Azure::Core::Http::Internal::ValuePolicyOptions options
      = {{{"hdrkey1", "HdrVal1"}, {"hdrkey2", "HdrVal2"}},
         {{"QryKey1", "QryVal1"}, {"QryKey2", "QryVal2"}}};

  std::vector<std::unique_ptr<HttpPolicy>> policies;
  policies.emplace_back(std::make_unique<Azure::Core::Http::Internal::ValuePolicy>(options));
  policies.emplace_back(std::make_unique<NoOpPolicy>());
  HttpPipeline pipeline(policies);

  Request request(HttpMethod::Get, Url("https:://www.example.com"));

  pipeline.Send(GetApplicationContext(), request);

  auto headers = request.GetHeaders();
  auto queryParams = request.GetUrl().GetQueryParameters();

  ASSERT_EQ(headers, decltype(headers)({{"hdrkey1", "HdrVal1"}, {"hdrkey2", "HdrVal2"}}));
  ASSERT_EQ(queryParams, decltype(queryParams)({{"QryKey1", "QryVal1"}, {"QryKey2", "QryVal2"}}));
}
