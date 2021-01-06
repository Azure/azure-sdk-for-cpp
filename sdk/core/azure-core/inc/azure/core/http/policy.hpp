// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Utilities to be used by HTTP transport policy implementations.
 */

#pragma once

#include "azure/core/context.hpp"
#include "azure/core/credentials.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/http/transport.hpp"
#include "azure/core/logging/logging.hpp"
#include "azure/core/uuid.hpp"

// Need to include the transport adapters offered by the SDK here so anyone can use them by just
// including policy.hpp
#include "azure/core/http/curl/curl.hpp"
#include "azure/core/http/winhttp/win_http_client.hpp"

#include <chrono>
#include <utility>

namespace Azure { namespace Core { namespace Http {

  namespace Details {
    std::shared_ptr<HttpTransport> GetTransportAdapter();
  }

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
     * @param context #Context so that operation can be cancelled.
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
    const std::vector<std::unique_ptr<HttpPolicy>>& m_policies;

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
        const std::vector<std::unique_ptr<HttpPolicy>>& policies)
        : m_index(index), m_policies(policies)
    {
    }

    /**
     * @brief Apply this HTTP policy.
     *
     * @param context #Context so that operation can be cancelled.
     * @param request An HTTP #Request being sent.
     *
     * @return An HTTP #RawResponse after this policy, and all subsequent HTTP policies in the stack
     * sequence of policies have been applied.
     */
    std::unique_ptr<RawResponse> Send(Context const& ctx, Request& req);
  };

  /**
   * @brief The options for the #TransportPolicy.
   *
   */
  struct TransportPolicyOptions
  {
    /**
     * @brief Set the #HttpTransport that the transport policy will use to send and receive requests
     * and responses over the wire.
     *
     * @remark When no option is set, the default transport adapter on non-Windows platforms is the
     * curl transport adapter and winhttp transport adapter on Windows.
     *
     * @remark When using a custom transport adapter, the implementation for
     * `AzureSdkGetCustomHttpTransport` must be linked in the end-user application.
     *
     */
    std::shared_ptr<HttpTransport> Transport = Details::GetTransportAdapter();
  };

  /**
   * @brief Applying this policy sends an HTTP request over the wire.
   * @remark This policy must be the bottom policy in the stack of the HTTP policy stack.
   */
  class TransportPolicy : public HttpPolicy {
  private:
    TransportPolicyOptions m_options;

  public:
    /**
     * @brief Construct an HTTP transport policy.
     *
     * @param transport A pointer to the #HttpTransport implementation to use when this policy gets
     * applied (#Send).
     */
    explicit TransportPolicy(TransportPolicyOptions options = TransportPolicyOptions())
        : m_options(std::move(options))
    {
    }

    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<TransportPolicy>(*this);
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
   * @brief The options for the #TelemetryPolicy
   *
   */
  struct TelemetryPolicyOptions
  {
    /**
     * @brief The Application id is the last part of the user agent for telemetry.
     *
     * @remark This option allows an end-user to create an SDK client and report telemetry with a
     * specific ID for it. The default is an empty string.
     *
     */
    std::string ApplicationId;
  };

  /**
   * @brief HTTP telemetry policy.
   *
   * @details Applies an HTTP header with a component name and version to each HTTP request,
   * includes Azure SDK version information, and operating system information.
   * @remark See https://azure.github.io/azure-sdk/general_azurecore.html#telemetry-policy.
   */
  class TelemetryPolicy : public HttpPolicy {
    std::string const m_telemetryId;

    static std::string BuildTelemetryId(
        std::string const& componentName,
        std::string const& componentVersion,
        std::string const& applicationId);

  public:
    /**
     * @brief Construct HTTP telemetry policy.
     *
     * @param componentName Azure SDK component name (e.g. "storage.blobs").
     * @param componentVersion Azure SDK component version (e.g. "11.0.0").
     * @param options The optional parameters for the policy (e.g. "AzCopy")
     */
    explicit TelemetryPolicy(
        std::string const& componentName,
        std::string const& componentVersion,
        TelemetryPolicyOptions options = TelemetryPolicyOptions())
        : m_telemetryId(BuildTelemetryId(componentName, componentVersion, options.ApplicationId))
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
   * @brief Bearer Token authentication policy.
   */
  class BearerTokenAuthenticationPolicy : public HttpPolicy {
  private:
    std::shared_ptr<TokenCredential const> const m_credential;
    std::vector<std::string> m_scopes;

    mutable AccessToken m_accessToken;
    mutable std::mutex m_accessTokenMutex;

    BearerTokenAuthenticationPolicy(BearerTokenAuthenticationPolicy const&) = delete;
    void operator=(BearerTokenAuthenticationPolicy const&) = delete;

  public:
    /**
     * @brief Construct a Bearer Token authentication policy with single authentication scope.
     *
     * @param credential A #TokenCredential to use with this policy.
     * @param scope Authentication scope.
     */
    explicit BearerTokenAuthenticationPolicy(
        std::shared_ptr<TokenCredential const> credential,
        std::string scope)
        : m_credential(std::move(credential))
    {
      m_scopes.emplace_back(std::move(scope));
    }

    /**
     * @brief Construct a Bearer Token authentication policy with multiple authentication scopes.
     *
     * @param credential A #TokenCredential to use with this policy.
     * @param scopes A vector of authentication scopes.
     */
    explicit BearerTokenAuthenticationPolicy(
        std::shared_ptr<TokenCredential const> credential,
        std::vector<std::string> scopes)
        : m_credential(std::move(credential)), m_scopes(std::move(scopes))
    {
    }

    /**
     * @brief Construct a Bearer Token authentication policy with multiple authentication scopes.
     *
     * @tparam A type of scopes sequence iterator.
     *
     * @param credential A #TokenCredential to use with this policy.
     * @param scopesBegin An iterator pointing to begin of the sequence of scopes to use.
     * @param scopesEnd An iterator pointing to an element after the last element in sequence of
     * scopes to use.
     */
    template <typename ScopesIterator>
    explicit BearerTokenAuthenticationPolicy(
        std::shared_ptr<TokenCredential const> credential,
        ScopesIterator const& scopesBegin,
        ScopesIterator const& scopesEnd)
        : m_credential(std::move(credential)), m_scopes(scopesBegin, scopesEnd)
    {
    }

    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<BearerTokenAuthenticationPolicy>(m_credential, m_scopes);
    }

    std::unique_ptr<RawResponse> Send(
        Context const& context,
        Request& request,
        NextHttpPolicy policy) const override;
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
