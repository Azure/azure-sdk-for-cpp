// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/context.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/http/transport.hpp>
#include <azure/core/internal/client_options.hpp>
#include <gtest/gtest.h>

#include <memory>
#include <vector>

using namespace Azure::Core;
using namespace Azure::Core::_internal;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;

struct FakeTransport final : public HttpTransport
{
  std::unique_ptr<RawResponse> Send(Request& request, Context const& context) override
  {
    (void)request;
    (void)context;
    return nullptr;
  }
};

struct PerCallPolicy final : public HttpPolicy
{
  std::unique_ptr<RawResponse> Send(
      Request& request,
      NextHttpPolicy nextPolicy,
      Context const& context) const override
  {
    (void)request;
    (void)nextPolicy;
    (void)context;
    return std::make_unique<RawResponse>(3, 3, HttpStatusCode::Gone, "IamAPerCallPolicy");
  }

  std::unique_ptr<HttpPolicy> Clone() const override
  {
    return std::make_unique<PerCallPolicy>(*this);
  }
};

struct PerRetryPolicy final : public HttpPolicy
{
  std::unique_ptr<RawResponse> Send(
      Request& request,
      NextHttpPolicy nextPolicy,
      Context const& context) const override
  {
    (void)request;
    (void)nextPolicy;
    (void)context;
    return std::make_unique<RawResponse>(6, 6, HttpStatusCode::ResetContent, "IamAPerRetryPolicy");
  }

  std::unique_ptr<HttpPolicy> Clone() const override
  {
    return std::make_unique<PerRetryPolicy>(*this);
  }
};

TEST(ClientOptions, copyWithOperator)
{
  // client Options defines its own copy constructor which clones policies
  ClientOptions options;
  options.Retry.MaxRetries = 1;
  options.Telemetry.ApplicationId = "pleaseCopyMe";
  options.Transport.Transport = std::make_shared<FakeTransport>();
  options.PerOperationPolicies.emplace_back(std::make_unique<PerCallPolicy>());
  options.PerRetryPolicies.emplace_back(std::make_unique<PerRetryPolicy>());

  // Now Copy
  auto copyOptions = options;

  // Compare
  EXPECT_EQ(1, copyOptions.Retry.MaxRetries);
  EXPECT_EQ(std::string("pleaseCopyMe"), copyOptions.Telemetry.ApplicationId);
  Request r(HttpMethod::Get, Url(""));
  auto result = copyOptions.Transport.Transport->Send(r, Context::ApplicationContext);
  EXPECT_EQ(nullptr, result);

  EXPECT_EQ(1, copyOptions.PerOperationPolicies.size());
  result = copyOptions.PerOperationPolicies[0]->Send(
      r, NextHttpPolicy(0, {}), Context::ApplicationContext);
  EXPECT_EQ(std::string("IamAPerCallPolicy"), result->GetReasonPhrase());

  EXPECT_EQ(1, copyOptions.PerRetryPolicies.size());
  result = copyOptions.PerRetryPolicies[0]->Send(
      r, NextHttpPolicy(0, {}), Context::ApplicationContext);
  EXPECT_EQ(std::string("IamAPerRetryPolicy"), result->GetReasonPhrase());
}

TEST(ClientOptions, copyWithConstructor)
{
  // client Options defines its own copy constructor which clones policies
  ClientOptions options;
  options.Retry.MaxRetries = 1;
  options.Telemetry.ApplicationId = "pleaseCopyMe";
  options.Transport.Transport = std::make_shared<FakeTransport>();
  options.PerOperationPolicies.emplace_back(std::make_unique<PerCallPolicy>());
  options.PerRetryPolicies.emplace_back(std::make_unique<PerRetryPolicy>());

  // Now Copy
  ClientOptions copyOptions(options);

  // Compare
  EXPECT_EQ(1, copyOptions.Retry.MaxRetries);
  EXPECT_EQ(std::string("pleaseCopyMe"), copyOptions.Telemetry.ApplicationId);
  Request r(HttpMethod::Get, Url(""));
  auto result = copyOptions.Transport.Transport->Send(r, Context::ApplicationContext);
  EXPECT_EQ(nullptr, result);

  EXPECT_EQ(1, copyOptions.PerOperationPolicies.size());
  result = copyOptions.PerOperationPolicies[0]->Send(
      r, NextHttpPolicy(0, {}), Context::ApplicationContext);
  EXPECT_EQ(std::string("IamAPerCallPolicy"), result->GetReasonPhrase());

  EXPECT_EQ(1, copyOptions.PerRetryPolicies.size());
  result = copyOptions.PerRetryPolicies[0]->Send(
      r, NextHttpPolicy(0, {}), Context::ApplicationContext);
  EXPECT_EQ(std::string("IamAPerRetryPolicy"), result->GetReasonPhrase());
}

TEST(ClientOptions, copyDerivedClassConstructor)
{
  struct ServiceClientOptions final : ClientOptions
  {
    std::string ApiVersion;
  };

  // client Options defines its own copy constructor which clones policies
  ServiceClientOptions options;
  options.ApiVersion = "I am not real!";
  options.Retry.MaxRetries = 1;
  options.Telemetry.ApplicationId = "pleaseCopyMe";
  options.Transport.Transport = std::make_shared<FakeTransport>();
  options.PerOperationPolicies.emplace_back(std::make_unique<PerCallPolicy>());
  options.PerRetryPolicies.emplace_back(std::make_unique<PerRetryPolicy>());

  // Now Copy
  ServiceClientOptions copyOptions(options);

  // Compare
  EXPECT_EQ("I am not real!", copyOptions.ApiVersion);
  EXPECT_EQ(1, copyOptions.Retry.MaxRetries);
  EXPECT_EQ(std::string("pleaseCopyMe"), copyOptions.Telemetry.ApplicationId);
  Request r(HttpMethod::Get, Url(""));
  auto result = copyOptions.Transport.Transport->Send(r, Context::ApplicationContext);
  EXPECT_EQ(nullptr, result);

  EXPECT_EQ(1, copyOptions.PerOperationPolicies.size());
  result = copyOptions.PerOperationPolicies[0]->Send(
      r, NextHttpPolicy(0, {}), Context::ApplicationContext);
  EXPECT_EQ(std::string("IamAPerCallPolicy"), result->GetReasonPhrase());

  EXPECT_EQ(1, copyOptions.PerRetryPolicies.size());
  result = copyOptions.PerRetryPolicies[0]->Send(
      r, NextHttpPolicy(0, {}), Context::ApplicationContext);
  EXPECT_EQ(std::string("IamAPerRetryPolicy"), result->GetReasonPhrase());
}

TEST(ClientOptions, copyDerivedClassOperator)
{
  struct ServiceClientOptions final : ClientOptions
  {
    std::string ApiVersion;
  };

  // client Options defines its own copy constructor which clones policies
  ServiceClientOptions options;
  options.ApiVersion = "I am not real!";
  options.Retry.MaxRetries = 1;
  options.Telemetry.ApplicationId = "pleaseCopyMe";
  options.Transport.Transport = std::make_shared<FakeTransport>();
  options.PerOperationPolicies.emplace_back(std::make_unique<PerCallPolicy>());
  options.PerRetryPolicies.emplace_back(std::make_unique<PerRetryPolicy>());

  // Now Copy
  auto copyOptions = options;

  // Compare
  EXPECT_EQ("I am not real!", copyOptions.ApiVersion);
  EXPECT_EQ(1, copyOptions.Retry.MaxRetries);
  EXPECT_EQ(std::string("pleaseCopyMe"), copyOptions.Telemetry.ApplicationId);
  Request r(HttpMethod::Get, Url(""));
  auto result = copyOptions.Transport.Transport->Send(r, Context::ApplicationContext);
  EXPECT_EQ(nullptr, result);

  EXPECT_EQ(1, copyOptions.PerOperationPolicies.size());
  result = copyOptions.PerOperationPolicies[0]->Send(
      r, NextHttpPolicy(0, {}), Context::ApplicationContext);
  EXPECT_EQ(std::string("IamAPerCallPolicy"), result->GetReasonPhrase());

  EXPECT_EQ(1, copyOptions.PerRetryPolicies.size());
  result = copyOptions.PerRetryPolicies[0]->Send(
      r, NextHttpPolicy(0, {}), Context::ApplicationContext);
  EXPECT_EQ(std::string("IamAPerRetryPolicy"), result->GetReasonPhrase());
}

TEST(ClientOptions, moveConstruct)
{
  struct ServiceClientOptions final : ClientOptions
  {
    std::string ApiVersion;
  };

  // client Options defines its own copy constructor which clones policies
  ServiceClientOptions options;
  options.ApiVersion = "I am not real!";
  options.Retry.MaxRetries = 1;
  options.Telemetry.ApplicationId = "pleaseCopyMe";
  options.Transport.Transport = std::make_shared<FakeTransport>();
  options.PerOperationPolicies.emplace_back(std::make_unique<PerCallPolicy>());
  options.PerRetryPolicies.emplace_back(std::make_unique<PerRetryPolicy>());

  // Now move
  ServiceClientOptions copyOptions(std::move(options));

  // Compare
  EXPECT_EQ("I am not real!", copyOptions.ApiVersion);
  EXPECT_EQ(1, copyOptions.Retry.MaxRetries);
  EXPECT_EQ(std::string("pleaseCopyMe"), copyOptions.Telemetry.ApplicationId);
  Request r(HttpMethod::Get, Url(""));
  auto result = copyOptions.Transport.Transport->Send(r, Context::ApplicationContext);
  EXPECT_EQ(nullptr, result);

  EXPECT_EQ(1, copyOptions.PerOperationPolicies.size());
  result = copyOptions.PerOperationPolicies[0]->Send(
      r, NextHttpPolicy(0, {}), Context::ApplicationContext);
  EXPECT_EQ(std::string("IamAPerCallPolicy"), result->GetReasonPhrase());

  EXPECT_EQ(1, copyOptions.PerRetryPolicies.size());
  result = copyOptions.PerRetryPolicies[0]->Send(
      r, NextHttpPolicy(0, {}), Context::ApplicationContext);
  EXPECT_EQ(std::string("IamAPerRetryPolicy"), result->GetReasonPhrase());
}
