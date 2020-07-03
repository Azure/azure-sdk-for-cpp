// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure.hpp"
#include "context.hpp"
#include "http.hpp"
#include "transport.hpp"
#include "uuid.hpp"

namespace Azure { namespace Core { namespace Http {

  class NextHttpPolicy;

  class HttpPolicy {
  public:
    // If we get a response that goes up the stack
    // Any errors in the pipeline throws an exception
    // At the top of the pipeline we might want to turn certain responses into exceptions
    virtual std::unique_ptr<Response> Send(
        Context& context,
        Request& request,
        NextHttpPolicy policy) const = 0;
    virtual ~HttpPolicy() {}
    virtual HttpPolicy* Clone() const = 0;

  protected:
    HttpPolicy() = default;
    HttpPolicy(const HttpPolicy& other) = default;
    HttpPolicy(HttpPolicy&& other) = default;
    HttpPolicy& operator=(const HttpPolicy& other) = default;
  };

  class NextHttpPolicy {
    const std::size_t m_index;
    const std::vector<std::unique_ptr<HttpPolicy>>* m_policies;

  public:
    explicit NextHttpPolicy(
        std::size_t index,
        const std::vector<std::unique_ptr<HttpPolicy>>* policies)
        : m_index(index), m_policies(policies)
    {
    }

    std::unique_ptr<Response> Send(Context& ctx, Request& req);
  };

  class TransportPolicy : public HttpPolicy {
  private:
    std::shared_ptr<HttpTransport> m_transport;

  public:
    explicit TransportPolicy(std::shared_ptr<HttpTransport> transport)
        : m_transport(std::move(transport))
    {
    }

    HttpPolicy* Clone() const override { return new TransportPolicy(m_transport); }

    std::unique_ptr<Response> Send(Context& ctx, Request& request, NextHttpPolicy nextHttpPolicy)
        const override
    {
      AZURE_UNREFERENCED_PARAMETER(nextHttpPolicy);
      /**
       * The transport policy is always the last policy.
       * Call the transport and return
       */
      return m_transport->Send(ctx, request);
    }
  };

  struct RetryOptions
  {
    int16_t MaxRetries = 5;
    int32_t RetryDelayMsec = 500;
  };

  class RetryPolicy : public HttpPolicy {
  private:
    RetryOptions m_retryOptions;

  public:
    explicit RetryPolicy(RetryOptions options) : m_retryOptions(options) {}

    HttpPolicy* Clone() const override { return new RetryPolicy(m_retryOptions); }

    std::unique_ptr<Response> Send(Context& ctx, Request& request, NextHttpPolicy nextHttpPolicy)
        const override
    {
      // Do real work here
      // nextPolicy->Process(ctx, message, )
      return nextHttpPolicy.Send(ctx, request);
    }
  };

  class RequestIdPolicy : public HttpPolicy {

    constexpr static const char* RequestIdHeader = "x-ms-request-id";

  public:
    explicit RequestIdPolicy() {}

    HttpPolicy* Clone() const override { return new RequestIdPolicy(); }

    std::unique_ptr<Response> Send(Context& ctx, Request& request, NextHttpPolicy nextHttpPolicy)
        const override
    {
      auto uuid = UUID().GetUUIDString();

      request.AddHeader(RequestIdHeader, uuid);
      // Do real work here
      return nextHttpPolicy.Send(ctx, request);
    }
  };

}}} // namespace Azure::Core::Http
