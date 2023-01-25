// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <gtest/gtest.h>

using namespace Azure::Core;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::_internal;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;

namespace {

class NoOpPolicy final : public HttpPolicy {
private:
  std::unique_ptr<RawResponse> Send(
      Request& request,
      NextHttpPolicy nextPolicy,
      Context const& context) const override
  {
    (void)context;
    (void)request;
    (void)nextPolicy;

    return std::unique_ptr<RawResponse>();
  }

  std::unique_ptr<HttpPolicy> Clone() const override { return std::make_unique<NoOpPolicy>(*this); }
};

} // namespace

TEST(TelemetryPolicy, telemetryString)
{

  struct
  {
    const std::string serviceName;
    const std::string serviceVersion;
    const std::string applicationId;
    const std::string expectedPrefix;
  } UserAgentTests[]
      = {{"storage-blob", "11.0.0", "", "azsdk-cpp-storage-blob/11.0.0 ("},
         {"storage-blob",
          "11.0.0",
          "AzCopy/10.0.4-Preview",
          "AzCopy/10.0.4-Preview azsdk-cpp-storage-blob/11.0.0 ("},
         {"storage-blob",
          "11.0.0",
          "AzCopy / 10.0.4-Preview ",
          "AzCopy / 10.0.4-Preview azsdk-cpp-storage-blob/11.0.0 ("},
         {"storage-blob",
          "11.0.0",
          "  01234567890123456789abcde  ",
          "01234567890123456789abcd azsdk-cpp-storage-blob/11.0.0 ("}};

  constexpr auto TelemetryHeader = "user-agent";
  constexpr auto ClosingBrace = ')';
  constexpr auto OSInfoMinLength = 10;

  for (auto const& test : UserAgentTests)
  {
    std::vector<std::unique_ptr<HttpPolicy>> policies;
    Azure::Core::_internal::ClientOptions options;
    options.Telemetry.ApplicationId = test.applicationId;
    policies.emplace_back(std::make_unique<TelemetryPolicy>(
        test.serviceName, test.serviceVersion, options.Telemetry));
    policies.emplace_back(std::make_unique<NoOpPolicy>());
    HttpPipeline pipeline(policies);

    auto request = Request(HttpMethod::Get, Url("http://microsoft.com"));
    Context context;

    pipeline.Send(request, context);
    auto const headers = request.GetHeaders();
    auto telemetryHeader = headers.find(TelemetryHeader);
    EXPECT_NE(telemetryHeader, headers.end());
    auto const actualValue = telemetryHeader->second;
    EXPECT_GE(
        actualValue.size(), test.expectedPrefix.size() + OSInfoMinLength + sizeof(ClosingBrace));
    EXPECT_EQ(actualValue[actualValue.size() - 1], ClosingBrace);

    EXPECT_EQ(actualValue.substr(0, test.expectedPrefix.size()), test.expectedPrefix);
  }
}
