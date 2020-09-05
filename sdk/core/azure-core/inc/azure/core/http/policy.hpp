// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Utilities to be used by HTTP transport policy implementations.
 */

#pragma once

#include "azure/core/azure.hpp"
#include "azure/core/context.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/http/transport.hpp"
#include "azure/core/logging/logging.hpp"
#include "azure/core/uuid.hpp"

#include <chrono>
#include <utility>

namespace Azure { namespace Core { namespace Http {

  class NextHttpPolicy;

  /**
   * @brief HTTP policy.
   * An HTTP pipeline inside SDK clients is an stack sequence of HTTP policies.
   */
  class HttpPolicy {
  public:
    // If we get a response that goes up the stack
    // Any errors in the pipeline throws an exception
    // At the top of the pipeline we might want to turn certain responses into exceptions

    /**
     * @brief Apply this HTTP policy.
     *
     * @param context #Context so that operation can be canceled.
     * @param request An HTTP #Request being sent.
     * @param policy #NextHttpPolicy to invoke after this policy has been applied.
     *
     * @return An HTTP #RawResponse after this policy, and all subsequent HTTP policies in the stack
     * sequence of policies have been applied.
     */
    virtual std::unique_ptr<RawResponse> Send(
        Context const& context,
        Request& request,
        NextHttpPolicy policy) const = 0;

    /// HTTP policy destructor.
    virtual ~HttpPolicy() {}

    /**
     * @brief Creates a clone of this HTTP policy.
     * @return A clone of this HTTP policy.
     */
    virtual std::unique_ptr<HttpPolicy> Clone() const = 0;

  protected:
    HttpPolicy() = default;
    HttpPolicy(const HttpPolicy& other) = default;
    HttpPolicy(HttpPolicy&& other) = default;
    HttpPolicy& operator=(const HttpPolicy& other) = default;
  };

  // Represents the next HTTP policy in the stack sequence of policies.
  class NextHttpPolicy {
    const std::size_t m_index;
    const std::vector<std::unique_ptr<HttpPolicy>>* m_policies;

  public:
    /**
     * @brief Constructs an abstraction representing a next line in the stack sequence  of policies,
     * from the caller's perspective.
     *
     * @param index An sequential index of this policy in the stack sequence of policies.
     * @param policies A vector of unique pointers next in the line to be invoked after the current
     * policy.
     */
    explicit NextHttpPolicy(
        std::size_t index,
        const std::vector<std::unique_ptr<HttpPolicy>>* policies)
        : m_index(index), m_policies(policies)
    {
    }

    /**
     * @brief Apply this HTTP policy.
     *
     * @param context #Context so that operation can be canceled.
     * @param request An HTTP #Request being sent.
     *
     * @return An HTTP #RawResponse after this policy, and all subsequent HTTP policies in the stack
     * sequence of policies have been applied.
     */
    std::unique_ptr<RawResponse> Send(Context const& ctx, Request& req);
  };

  /**
   * @brief Applying this policy sends an HTTP request over the wire.
   * @remark This policy must be the bottom policy in the stack of the HTTP policy stack.
   */
  class TransportPolicy : public HttpPolicy {
  private:
    std::shared_ptr<HttpTransport> m_transport;

  public:
    /**
     * @brief Constructs an HTTP transport policy.
     *
     * @param transport A pointer to the #HttpTransport implementation to use when this policy gets
     * applied (#Send).
     */
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

  /// Options for the #RetryPolicy.
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
      auto uuid = Uuid::CreateUuid().GetUuidString();

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
