// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Utilities to be used by HTTP transport policy implementations.
 */

#pragma once

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

    /// Destructor.
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
     * @brief Construct an abstraction representing a next line in the stack sequence  of policies,
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
     * @brief Construct an HTTP transport policy.
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

  /**
   * @brief Options for the #RetryPolicy.
   */
  struct RetryOptions
  {
    /**
     * @brief Maximum number of attempts to retry.
     */
    int MaxRetries = 3;

    /**
     * @brief Mimimum amount of time between retry attempts.
     */
    std::chrono::milliseconds RetryDelay = std::chrono::seconds(4);

    /**
     * @brief Mimimum amount of time between retry attempts.
     */
    decltype(RetryDelay) MaxRetryDelay = std::chrono::minutes(2);

    /**
     * @brief HTTP status codes to retry on.
     */
    std::vector<HttpStatusCode> StatusCodes{
        HttpStatusCode::RequestTimeout,
        HttpStatusCode::InternalServerError,
        HttpStatusCode::BadGateway,
        HttpStatusCode::ServiceUnavailable,
        HttpStatusCode::GatewayTimeout,
    };
  };

  /**
   * @brief HTTP retry policy.
   */
  class RetryPolicy : public HttpPolicy {
  private:
    RetryOptions m_retryOptions;

  public:
    /**
     * Constructs HTTP retry policy with the provided #RetryOptions.
     *
     * @param options HTTP #RetryOptions.
     */
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

  /**
   * @brief HTTP Request ID policy.
   *
   * @details Applies an HTTP header with a unique ID to each HTTP request, so that each individual
   * request can be traced for troubleshooting.
   */
  class RequestIdPolicy : public HttpPolicy {
  private:
    constexpr static const char* RequestIdHeader = "x-ms-client-request-id";

  public:
    /**
     * @brief Constructs HTTP request ID policy.
     */
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

  /**
   * @brief HTTP telemetry policy.
   *
   * @details Applies an HTTP header with a component name and version to each HTTP request,
   * includes Azure SDK version information, and operating system information.
   * @remark See https://azure.github.io/azure-sdk/general_azurecore.html#telemetry-policy.
   */
  class TelemetryPolicy : public HttpPolicy {
    std::string m_telemetryId;

    static std::string const g_emptyApplicationId;

    static std::string BuildTelemetryId(
        std::string const& componentName,
        std::string const& componentVersion,
        std::string const& applicationId);

  public:
    /**
     * @brief Construct HTTP telemetry policy with component name and component version.
     *
     * @param componentName Azure SDK component name (e.g. "storage.blobs")
     * @param componentVersion Azure SDK component version (e.g. "11.0.0")
     */
    explicit TelemetryPolicy(std::string const& componentName, std::string const& componentVersion)
        : TelemetryPolicy(componentName, componentVersion, g_emptyApplicationId)
    {
    }

    /**
     * @brief Construct HTTP telemetry policy with component name, component version, and an
     * applicatin ID.
     *
     * @param componentName Azure SDK component name (e.g. "storage.blobs")
     * @param componentVersion Azure SDK component version (e.g. "11.0.0")
     * @param applicationId Customer Application ID (e.g. "AzCopy")
     */
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

  /**
   * @brief Logs every HTTP request.
   *
   * @detail Logs every HTTP request, response, or retry attempt (see #LogClassification)
   * @remark See #logging.hpp
   */
  class LoggingPolicy : public HttpPolicy {
  public:
    /**
     * @brief Constructs HTTP logging policy.
     */
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

  /**
   * @brief Log classigications being used to designate log messages from HTTP #LoggingPolicy.
   */
  class LogClassification : private Azure::Core::Logging::Details::LogClassificationProvider<
                                Azure::Core::Logging::Details::Facility::Core> {
  public:
    /// HTTP request.
    static constexpr auto const Request = Classification(1);

    /// HTTP response.
    static constexpr auto const Response = Classification(2);

    /// HTTP retry attempt.
    static constexpr auto const Retry = Classification(3);

    /// HTTP Transport adapter.
    static constexpr auto const HttpTransportAdapter = Classification(4);
  };

}}} // namespace Azure::Core::Http
