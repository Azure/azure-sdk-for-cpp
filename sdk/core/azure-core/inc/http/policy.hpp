// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "context.hpp"
#include "http.hpp"

namespace Azure
{
namespace Core
{
namespace Http
{

class HttpPolicy
{
public:
  // If we get a response that goes up the stack
  // Any errors in the pipeline throws an exception
  // At the top of the pipeline we might want to turn certain responses into exceptions
  virtual Response Process(Context& context, Request& request) const = 0;
  virtual ~HttpPolicy() {}
};

class HttpTransport : public HttpPolicy
{
  Response Process(Context& context, Request& request) const override
  {
    context.CancelWhen();
    request.getHeaders();

    return Response(200, "OK\n");
  }
};

struct RetryPolicyOptions
{

private:
  int16_t max_retries;
  int32_t retry_delay_msec;

public:
  RetryPolicyOptions()
  {
    max_retries = 5;
    retry_delay_msec = 500;
  };
};

class RetryPolicy : public HttpPolicy
{
private:
  std::unique_ptr<HttpPolicy> nextPolicy_;
  RetryPolicyOptions retryPolicyOptions_;

public:
  explicit RetryPolicy(std::unique_ptr<HttpPolicy> nextPolicy, RetryPolicyOptions options)
  {
    // Ensure this is a copy
    //  Assert
    retryPolicyOptions_ = options;
    nextPolicy_ = std::move(nextPolicy);
  }

  Response Process(Context& ctx, Request& message) const override
  {
    // Do real work here

    return nextPolicy_->Process(ctx, message);
  }
};

struct RequestIdPolicyOptions
{
public:
  RequestIdPolicyOptions(){
    // Set some values
  };
};

class RequestIdPolicy : public HttpPolicy
{
private:
  std::unique_ptr<HttpPolicy> nextPolicy;

public:
  explicit RequestIdPolicy(std::unique_ptr<HttpPolicy> nextPolicy)
      : nextPolicy(std::move(nextPolicy)){}

  Response Process(Context& ctx, Request& request) const override
  {
    // Do real work here

    return nextPolicy->Process(ctx, request);
  }
};

} // namespace Http
} // namespace Core
} // namespace Azure
