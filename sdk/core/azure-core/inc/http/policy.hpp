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
    virtual ~HttpPolicy() {}

    std::unique_ptr<Response> NextPolicy();


  protected:
    HttpPolicy() = default;
    HttpPolicy(const HttpPolicy& other) = default;
    HttpPolicy(HttpPolicy&& other) = default;
    HttpPolicy& operator=(const HttpPolicy& other) = default;
  };

  struct RetryOptions
  {
    int16_t MaxRetries = 5;
    int32_t RetryDelayMsec = 500;
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
      //nextPolicy->Process(ctx, message, )
      return m_nextPolicy->Process(ctx, message);
    }
  };

  struct RequestIdOptions
  {
  private:
    void* Reserved;
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
