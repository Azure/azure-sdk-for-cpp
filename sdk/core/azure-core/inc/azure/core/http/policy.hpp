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
#include "azure/core/uuid.hpp"

#include <chrono>
#include <cstddef>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

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
     * @param context #Azure::Core::Context so that operation can be cancelled.
     * @param request An #Azure::Core::Http::Request being sent.
     * @param policy #Azure::Core::Http::NextHttpPolicy to invoke after this policy has been
     * applied.
     *
     * @return An #Azure::Core::Http::RawResponse after this policy, and all subsequent HTTP
     * policies in the stack sequence of policies have been applied.
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

  /**
   * @brief Represents the next HTTP policy in the stack sequence of policies.
   *
   */
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
     * @param context #Azure::Core::Context so that operation can be cancelled.
     * @param request An #Azure::Core::Http::Request being sent.
     *
     * @return An #Azure::Core::Http::RawResponse after this policy, and all subsequent HTTP
     * policies in the stack sequence of policies have been applied.
     */
    std::unique_ptr<RawResponse> Send(Context const& context, Request& request);
  };

  /**
   * @brief The options for the #Azure::Core::Http::TransportPolicy.
   *
   */
  struct TransportPolicyOptions
  {
    /**
     * @brief Set the #Azure::Core::Http::HttpTransport that the transport policy will use to send
     * and receive requests and responses over the wire.
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
     * @param options #Azure::Core::Http::TransportPolicyOptions.
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
   * @brief Options for the #Azure::Core::Http::RetryPolicy.
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
     * Constructs HTTP retry policy with the provided #Azure::Core::Http::RetryOptions.
     *
     * @param options #Azure::Core::Http::RetryOptions.
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
      auto uuid = Uuid::CreateUuid().ToString();

      request.AddHeader(RequestIdHeader, uuid);
      return nextHttpPolicy.Send(ctx, request);
    }
  };

  /**
   * @brief The options for the #Azure::Core::Http::TelemetryPolicy
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
   * @brief Defines options for getting token.
   */
  struct TokenRequestOptions
  {
    /**
     * @brief Authentication scopes.
     */
    std::vector<std::string> Scopes;
  };

  /**
   * @brief Bearer Token authentication policy.
   */
  class BearerTokenAuthenticationPolicy : public HttpPolicy {
  private:
    std::shared_ptr<TokenCredential const> const m_credential;
    TokenRequestOptions m_tokenRequestOptions;

    mutable AccessToken m_accessToken;
    mutable std::mutex m_accessTokenMutex;

    BearerTokenAuthenticationPolicy(BearerTokenAuthenticationPolicy const&) = delete;
    void operator=(BearerTokenAuthenticationPolicy const&) = delete;

  public:
    /**
     * @brief Construct a Bearer Token authentication policy.
     *
     * @param credential A #Azure::Core::TokenCredential to use with this policy.
     * @param tokenRequestOptions #Azure::Core::Http::TokenRequestOptions.
     */
    explicit BearerTokenAuthenticationPolicy(
        std::shared_ptr<TokenCredential const> credential,
        TokenRequestOptions tokenRequestOptions)
        : m_credential(std::move(credential)), m_tokenRequestOptions(std::move(tokenRequestOptions))
    {
    }

    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<BearerTokenAuthenticationPolicy>(m_credential, m_tokenRequestOptions);
    }

    std::unique_ptr<RawResponse> Send(
        Context const& context,
        Request& request,
        NextHttpPolicy policy) const override;
  };

  /**
   * @brief Logs every HTTP request.
   *
   * @details Logs every HTTP request, response, or retry attempt.
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

  namespace Internal {
    /**
     * @brief #Azure::Core::Http::Internal::ValuePolicy options.
     */
    struct ValuePolicyOptions
    {
      std::map<std::string, std::string> HeaderValues;
      std::map<std::string, std::string> QueryValues;
    };

    /**
     * @brief Value policy.
     *
     * @details Applies key-value pair values to each HTTP request (either HTTP headers or query
     * parameters).
     */
    class ValuePolicy : public HttpPolicy {
    private:
      ValuePolicyOptions m_options;

    public:
      /**
       * @brief Construct a #Azure::Core::Http::Internal::ValuePolicy with the
       * #Azure::Core::Http::Internal::ValuePolicyOptions provided.
       * @param options #Azure::Core::Http::Internal::ValuePolicyOptions.
       */
      explicit ValuePolicy(ValuePolicyOptions options) : m_options(std::move(options)) {}

      std::unique_ptr<HttpPolicy> Clone() const override
      {
        return std::make_unique<ValuePolicy>(*this);
      }

      std::unique_ptr<RawResponse> Send(
          Context const& ctx,
          Request& request,
          NextHttpPolicy nextHttpPolicy) const override
      {
        for (auto const& hdrPair : m_options.HeaderValues)
        {
          request.AddHeader(hdrPair.first, hdrPair.second);
        }

        {
          auto& url = request.GetUrl();
          for (auto const& qryPair : m_options.QueryValues)
          {
            url.AppendQueryParameter(qryPair.first, qryPair.second);
          }
        }

        return nextHttpPolicy.Send(ctx, request);
      }
    };
  } // namespace Internal
}}} // namespace Azure::Core::Http
