// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "context.hpp"
#include "http.hpp"

namespace Azure { namespace Core { namespace Http {

  class HttpPolicy {
  public:
    // If we get a response that goes up the stack
    // Any errors in the pipeline throws an exception
    // At the top of the pipeline we might want to turn certain responses into exceptions
    virtual Response Process(Context& context, Request& request) const = 0;
    virtual ~HttpPolicy()
    {
    }

  protected:
    HttpPolicy() = default;
    HttpPolicy(const HttpPolicy& other) = default;
    HttpPolicy(HttpPolicy&& other) = default;
    HttpPolicy& operator=(const HttpPolicy& other) = default;
  };

  class HttpTransport : public HttpPolicy {
    Response Process(Context& context, Request& request) const override
    {
      context.CancelWhen();
      request.getHeaders();

      return Response(200, "OK\n");
    }
  };

  struct RetryOptions
  {

  private:
    int16_t m_maxRetries;
    int32_t m_retryDelayMsec;

  public:
    RetryOptions() : m_maxRetries(5), m_retryDelayMsec(500)
    {
    }
  };

  class RetryPolicy : public HttpPolicy {
  private:
    std::unique_ptr<HttpPolicy> m_nextPolicy;
    RetryOptions m_retryOptions;

  public:
    explicit RetryPolicy(std::unique_ptr<HttpPolicy> nextPolicy, RetryOptions options)
        : m_nextPolicy(std::move(nextPolicy)), m_retryOptions(options)
    {
    }

    Response Process(Context& ctx, Request& message) const override
    {
      // Do real work here

      return m_nextPolicy->Process(ctx, message);
    }
  };

  struct RequestIdPolicyOptions
  {
  public:
    RequestIdPolicyOptions(){
        // Set some values
    };
  };

  class RequestIdPolicy : public HttpPolicy {
  private:
    std::unique_ptr<HttpPolicy> m_nextPolicy;

  public:
    explicit RequestIdPolicy(std::unique_ptr<HttpPolicy> nextPolicy)
        : m_nextPolicy(std::move(nextPolicy))
    {
    }

    Response Process(Context& ctx, Request& request) const override
    {
      // Do real work here

      return m_nextPolicy->Process(ctx, request);
    }
  };

}}} // namespace Azure::Core::Http
