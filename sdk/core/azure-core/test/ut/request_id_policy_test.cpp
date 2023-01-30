// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>

#include <gtest/gtest.h>

using namespace Azure::Core;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;

namespace {
class NoOpPolicy final : public HttpPolicy {
public:
  std::unique_ptr<HttpPolicy> Clone() const override { return std::make_unique<NoOpPolicy>(*this); }

  std::unique_ptr<RawResponse> Send(Request&, NextHttpPolicy, Azure::Core::Context const&)
      const override
  {
    return nullptr;
  }
};

constexpr char const* RequestIdHeaderName = "x-ms-client-request-id";
} // namespace

TEST(RequestIdPolicy, Basic)
{
  Request request(HttpMethod::Get, Url("https://www.microsoft.com"));

  {
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;

    policies.emplace_back(std::make_unique<RequestIdPolicy>());
    policies.emplace_back(std::make_unique<NoOpPolicy>());

    Azure::Core::Http::_internal::HttpPipeline(policies).Send(request, Azure::Core::Context());
  }

  auto const headers = request.GetHeaders();
  auto const requestIdHeader = headers.find(RequestIdHeaderName);
  EXPECT_NE(requestIdHeader, headers.end());
  EXPECT_EQ(requestIdHeader->second.length(), 36);
}

TEST(RequestIdPolicy, Unique)
{
  std::string guid1;
  std::string guid2;

  {
    Request request(HttpMethod::Get, Url("https://www.microsoft.com"));

    {
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;

      policies.emplace_back(std::make_unique<RequestIdPolicy>());
      policies.emplace_back(std::make_unique<NoOpPolicy>());

      Azure::Core::Http::_internal::HttpPipeline(policies).Send(request, Azure::Core::Context());
    }

    auto const headers = request.GetHeaders();
    auto const requestIdHeader = headers.find(RequestIdHeaderName);
    EXPECT_NE(requestIdHeader, headers.end());

    guid1 = requestIdHeader->second;
    EXPECT_EQ(guid1.length(), 36);
  }

  {
    Request request(HttpMethod::Get, Url("https://www.microsoft.com"));

    {
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;

      policies.emplace_back(std::make_unique<RequestIdPolicy>());
      policies.emplace_back(std::make_unique<NoOpPolicy>());

      Azure::Core::Http::_internal::HttpPipeline(policies).Send(request, Azure::Core::Context());
    }

    auto const headers = request.GetHeaders();
    auto const requestIdHeader = headers.find(RequestIdHeaderName);
    EXPECT_NE(requestIdHeader, headers.end());

    guid2 = requestIdHeader->second;
    EXPECT_EQ(guid2.length(), 36);
  }

  EXPECT_NE(guid1, guid2);
}

TEST(RequestIdPolicy, NoOverwrite)
{
  Request request(HttpMethod::Get, Url("https://www.microsoft.com"));
  request.SetHeader("x-ms-client-request-id", "0123-45-67-89-abcdef");

  {
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;

    policies.emplace_back(std::make_unique<RequestIdPolicy>());
    policies.emplace_back(std::make_unique<NoOpPolicy>());

    Azure::Core::Http::_internal::HttpPipeline(policies).Send(request, Azure::Core::Context());
  }

  auto const headers = request.GetHeaders();
  auto const requestIdHeader = headers.find("x-ms-client-request-id");

  EXPECT_NE(requestIdHeader, headers.end());
  EXPECT_EQ(requestIdHeader->second, "0123-45-67-89-abcdef");
}
