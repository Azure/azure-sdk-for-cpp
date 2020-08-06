// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure.hpp"
#include "context.hpp"
#include "http.hpp"
#include "logging/logging.hpp"
#include "transport.hpp"
#include "uuid.hpp"

#include <chrono>
#include <utility>

namespace Azure { namespace Core { namespace Http {

  class NextHttpPolicy;

  class HttpPolicy {
  public:
    // If we get a response that goes up the stack
    // Any errors in the pipeline throws an exception
    // At the top of the pipeline we might want to turn certain responses into exceptions
    virtual std::unique_ptr<RawResponse> Send(
        Context const& context,
        Request& request,
        NextHttpPolicy policy) const = 0;
    virtual ~HttpPolicy() {}
    virtual std::unique_ptr<HttpPolicy> Clone() const = 0;

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

    std::unique_ptr<RawResponse> Send(Context const& ctx, Request& req);
  };

  class TransportPolicy : public HttpPolicy {
  private:
    std::shared_ptr<HttpTransport> m_transport;

  public:
    explicit TransportPolicy(std::shared_ptr<HttpTransport> transport)
        : m_transport(std::move(transport))
    {
    }

    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<TransportPolicy>(m_transport);
    }

    std::unique_ptr<RawResponse> Send(
        Context const& ctx,
        Request& request,
        NextHttpPolicy nextHttpPolicy) const override;
  };

  struct RetryOptions
  {
    int MaxRetries = 3;

    std::chrono::milliseconds RetryDelay = std::chrono::seconds(4);
    decltype(RetryDelay) MaxRetryDelay = std::chrono::minutes(2);

    std::vector<HttpStatusCode> StatusCodes{
        HttpStatusCode::RequestTimeout,
        HttpStatusCode::InternalServerError,
        HttpStatusCode::BadGateway,
        HttpStatusCode::ServiceUnavailable,
        HttpStatusCode::GatewayTimeout,
    };
  };

  class RetryPolicy : public HttpPolicy {
  private:
    RetryOptions m_retryOptions;

  public:
    explicit RetryPolicy(RetryOptions options) : m_retryOptions(std::move(options)) {}

    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<RetryPolicy>(*this);
    }

    std::unique_ptr<RawResponse> Send(
        Context const& ctx,
        Request& request,
        NextHttpPolicy nextHttpPolicy) const override;
  };

  class RequestIdPolicy : public HttpPolicy {
  private:
    constexpr static const char* RequestIdHeader = "x-ms-client-request-id";

  public:
    explicit RequestIdPolicy() {}

    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<RequestIdPolicy>(*this);
    }

    std::unique_ptr<RawResponse> Send(
        Context const& ctx,
        Request& request,
        NextHttpPolicy nextHttpPolicy) const override
    {
      auto uuid = UUID::CreateUUID().GetUUIDString();

      request.AddHeader(RequestIdHeader, uuid);
      return nextHttpPolicy.Send(ctx, request);
    }
  };

  class TelemetryPolicy : public HttpPolicy {
    std::string m_telemetryId;

    static std::string const g_emptyApplicationId;

    static std::string BuildTelemetryId(
        std::string const& componentName,
        std::string const& componentVersion,
        std::string const& applicationId);

  public:
    explicit TelemetryPolicy(std::string const& componentName, std::string const& componentVersion)
        : TelemetryPolicy(componentName, componentVersion, g_emptyApplicationId)
    {
    }

    explicit TelemetryPolicy(
        std::string const& componentName,
        std::string const& componentVersion,
        std::string const& applicationId)
        : m_telemetryId(BuildTelemetryId(componentName, componentVersion, applicationId))
    {
    }

    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<TelemetryPolicy>(*this);
    }

    std::unique_ptr<RawResponse> Send(
        Context const& ctx,
        Request& request,
        NextHttpPolicy nextHttpPolicy) const override;
  };

  class LoggingPolicy : public HttpPolicy {
  public:
    explicit LoggingPolicy() {}

    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<LoggingPolicy>(*this);
    }

    std::unique_ptr<RawResponse> Send(
        Context const& ctx,
        Request& request,
        NextHttpPolicy nextHttpPolicy) const override;
  };

  class LogClassification : private Azure::Core::Logging::Details::LogClassificationProvider<
                                Azure::Core::Logging::Details::Facility::Core> {
  public:
    static constexpr auto const Request = Classification(1);
    static constexpr auto const Response = Classification(2);
    static constexpr auto const Retry = Classification(3);
  };
}}} // namespace Azure::Core::Http
