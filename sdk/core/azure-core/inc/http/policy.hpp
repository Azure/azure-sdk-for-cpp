// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "context.hpp"
#include "http.hpp"
#include "transport.hpp"

namespace Azure { namespace Core { namespace Http {

    class NextHttpPolicy;

  class HttpPolicy {
  public:
    // If we get a response that goes up the stack
    // Any errors in the pipeline throws an exception
    // At the top of the pipeline we might want to turn certain responses into exceptions
    virtual std::unique_ptr<Response> Process(
        Context& context,
        Request& request,
        NextHttpPolicy policy) const = 0;
    virtual ~HttpPolicy() {}
    //virtual HttpPolicy* Clone() const = 0;

  protected:
    HttpPolicy() = default;
    HttpPolicy(const HttpPolicy& other) = default;
    HttpPolicy(HttpPolicy&& other) = default;
    HttpPolicy& operator=(const HttpPolicy& other) = default;
  };

  class NextHttpPolicy {
    std::size_t m_index;
    const std::vector<std::unique_ptr<HttpPolicy>>* m_policies;

  public:
    explicit NextHttpPolicy(const std::vector<std::unique_ptr<HttpPolicy>>* policies)
        : m_index(0), m_policies(policies)
    {
    }

    explicit NextHttpPolicy(
        std::size_t index,
        const std::vector<std::unique_ptr<HttpPolicy>>* policies)
        : m_index(index), m_policies(policies)
    {
    }

    std::unique_ptr<Response> ProcessNext(Context& ctx, Request& req);
  };

  struct RetryOptions
  {
    int16_t MaxRetries = 5;
    int32_t RetryDelayMsec = 500;
  };

  class TransportPolicy : public HttpPolicy {
  private:
    std::unique_ptr<Transport> m_transport;

  public:
    explicit TransportPolicy(std::unique_ptr<Transport> transport)
        : m_transport(std::move(transport))
    {
    }

    std::unique_ptr<Response> Process(Context& ctx, Request& request, NextHttpPolicy nextHttpPolicy)
        const override
    {
      (void)nextHttpPolicy;
      /**
       * The transport policy is always the last policy.
       * Call the transport and return
      */
      return m_transport->Send(ctx, request);
    }
  };


  class RetryPolicy : public HttpPolicy {
  private:
    RetryOptions m_retryOptions;

  public:
    explicit RetryPolicy(RetryOptions options)
        : m_retryOptions(options)
    {
    }

    std::unique_ptr<Response> Process(Context& ctx, Request& request, NextHttpPolicy nextHttpPolicy)
        const override
    {
      // Do real work here
      //nextPolicy->Process(ctx, message, )
      return nextHttpPolicy.ProcessNext(ctx, request);
    }
  };

  struct RequestIdOptions
  {
  private:
    void* Reserved;
  };

  class RequestIdPolicy : public HttpPolicy {

  public:
    explicit RequestIdPolicy(){}

    std::unique_ptr<Response> Process(Context& ctx, Request& request, NextHttpPolicy nextHttpPolicy)
        const override
    {
      // Do real work here
      return nextHttpPolicy.ProcessNext(ctx, request);
    }
  };

}}} // namespace Azure::Core::Http
