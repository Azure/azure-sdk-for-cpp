// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Utilities to be used by HTTP transport policy implementations.
 */

#pragma once

#include "azure/core/case_insensitive_containers.hpp"
#include "azure/core/context.hpp"
#include "azure/core/credentials/credentials.hpp"
#include "azure/core/dll_import_export.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/http/transport.hpp"
#include "azure/core/uuid.hpp"

#include <chrono>
#include <cstddef>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace Azure { namespace Core { namespace Http { namespace Policies {

  namespace _detail {
    std::shared_ptr<HttpTransport> GetTransportAdapter();
    AZ_CORE_DLLEXPORT extern Azure::Core::CaseInsensitiveSet const g_defaultAllowedHttpHeaders;
  } // namespace _detail

  /**
   * @brief Telemetry options.
   *
   */
  struct TelemetryOptions final
  {
    /**
     * @brief The Application ID is the last part of the user agent for telemetry.
     *
     * @remark This option allows an end-user to create an SDK client and report telemetry with a
     * specific ID for it. The default is an empty string.
     *
     */
    std::string ApplicationId;
  };

  /**
   * @brief Retry options.
   */
  struct RetryOptions final
  {
    /**
     * @brief Maximum number of attempts to retry.
     */
    int32_t MaxRetries = 3;

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
    std::set<HttpStatusCode> StatusCodes{
        HttpStatusCode::RequestTimeout,
        HttpStatusCode::InternalServerError,
        HttpStatusCode::BadGateway,
        HttpStatusCode::ServiceUnavailable,
        HttpStatusCode::GatewayTimeout,
    };
  };

  /**
   * @brief Log options.
   */
  struct LogOptions final
  {
    /**
     * @brief HTTP query parameters that are allowed to be logged.
     */
    std::set<std::string> AllowedHttpQueryParameters;

    /**
     * @brief HTTP headers that are allowed to be logged.
     */
    Azure::Core::CaseInsensitiveSet AllowedHttpHeaders = _detail::g_defaultAllowedHttpHeaders;
  };

  /**
   * @brief Transport options.
   *
   */
  struct TransportOptions final
  {
    /**
     * @brief Set the #Azure::Core::Http::HttpTransport that the transport policy will use to send
     * and receive requests and responses over the wire.
     *
     * @remark When no option is set, the default transport adapter on non-Windows platforms is
     * the curl transport adapter and winhttp transport adapter on Windows.
     *
     * @remark When using a custom transport adapter, the implementation for
     * `AzureSdkGetCustomHttpTransport` must be linked in the end-user application.
     *
     */
    std::shared_ptr<HttpTransport> Transport = _detail::GetTransportAdapter();
  };

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
     * @param policy #Azure::Core::Http::Policies::NextHttpPolicy to invoke after this
     * policy has been applied.
     *
     * @return An #Azure::Core::Http::RawResponse after this policy, and all subsequent HTTP
     * policies in the stack sequence of policies have been applied.
     */
    virtual std::unique_ptr<RawResponse> Send(
        Request& request,
        NextHttpPolicy policy,
        Context const& context) const = 0;

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
  class NextHttpPolicy final {
    const std::size_t m_index;
    const std::vector<std::unique_ptr<HttpPolicy>>& m_policies;

  public:
    /**
     * @brief Construct an abstraction representing a next line in the stack sequence  of
     * policies, from the caller's perspective.
     *
     * @param index An sequential index of this policy in the stack sequence of policies.
     * @param policies A vector of unique pointers next in the line to be invoked after the
     * current policy.
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
     * @param request An #Azure::Core::Http::Request being sent.
     * @param context #Azure::Core::Context so that operation can be cancelled.
     *
     * @return An #Azure::Core::Http::RawResponse after this policy, and all subsequent HTTP
     * policies in the stack sequence of policies have been applied.
     */
    std::unique_ptr<RawResponse> Send(Request& request, Context const& context);
  };

  namespace _internal {

    /**
     * @brief Applying this policy sends an HTTP request over the wire.
     * @remark This policy must be the bottom policy in the stack of the HTTP policy stack.
     */
    class TransportPolicy final : public HttpPolicy {
    private:
      TransportOptions m_options;

    public:
      /**
       * @brief Construct an HTTP transport policy.
       *
       * @param options #Azure::Core::Http::Policies::TransportOptions.
       */
      explicit TransportPolicy(TransportOptions options = TransportOptions())
          : m_options(std::move(options))
      {
      }

      std::unique_ptr<HttpPolicy> Clone() const override
      {
        return std::make_unique<TransportPolicy>(*this);
      }

      std::unique_ptr<RawResponse> Send(
          Request& request,
          NextHttpPolicy nextHttpPolicy,
          Context const& ctx) const override;
    };

    /**
     * @brief HTTP retry policy.
     */
    class RetryPolicy
#if !defined(TESTING_BUILD)
        final
#endif
        : public HttpPolicy {
    private:
      RetryOptions m_retryOptions;

    public:
      /**
       * Constructs HTTP retry policy with the provided #Azure::Core::Http::Policies::RetryOptions.
       *
       * @param options #Azure::Core::Http::Policies::RetryOptions.
       */
      explicit RetryPolicy(RetryOptions options) : m_retryOptions(std::move(options)) {}

      std::unique_ptr<HttpPolicy> Clone() const override
      {
        return std::make_unique<RetryPolicy>(*this);
      }

      std::unique_ptr<RawResponse> Send(
          Request& request,
          NextHttpPolicy nextHttpPolicy,
          Context const& ctx) const final;

      /**
       * @brief Get the Retry Count from the context.
       *
       * @remark The sentinel `-1` is returned if there is no information in the \p Context about
       * #RetryPolicy is trying to send a request. Then `0` is returned for the first try of sending
       * a request by the #RetryPolicy. Any subsequent retry will be referenced with a number
       * greater than 0.
       *
       * @param context The context used to call send request.
       * @return A positive number indicating the current intent to send the request.
       */
      static int32_t GetRetryCount(Context const& context);

    protected:
      virtual bool ShouldRetryOnTransportFailure(
          RetryOptions const& retryOptions,
          int32_t attempt,
          std::chrono::milliseconds& retryAfter,
          double jitterFactor = -1) const;

      virtual bool ShouldRetryOnResponse(
          RawResponse const& response,
          RetryOptions const& retryOptions,
          int32_t attempt,
          std::chrono::milliseconds& retryAfter,
          double jitterFactor = -1) const;
    };

    /**
     * @brief HTTP Request ID policy.
     *
     * @details Applies an HTTP header with a unique ID to each HTTP request, so that each
     * individual request can be traced for troubleshooting.
     */
    class RequestIdPolicy final : public HttpPolicy {
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
          Request& request,
          NextHttpPolicy nextHttpPolicy,
          Context const& ctx) const override
      {
        auto uuid = Uuid::CreateUuid().ToString();

        request.SetHeader(RequestIdHeader, uuid);
        return nextHttpPolicy.Send(request, ctx);
      }
    };

    /**
     * @brief HTTP telemetry policy.
     *
     * @details Applies an HTTP header with a component name and version to each HTTP request,
     * includes Azure SDK version information, and operating system information.
     * @remark See https://azure.github.io/azure-sdk/general_azurecore.html#telemetry-policy.
     */
    class TelemetryPolicy final : public HttpPolicy {
    private:
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
          TelemetryOptions options = TelemetryOptions())
          : m_telemetryId(BuildTelemetryId(componentName, componentVersion, options.ApplicationId))
      {
      }

      std::unique_ptr<HttpPolicy> Clone() const override
      {
        return std::make_unique<TelemetryPolicy>(*this);
      }

      std::unique_ptr<RawResponse> Send(
          Request& request,
          NextHttpPolicy nextHttpPolicy,
          Context const& ctx) const override;
    };

    /**
     * @brief Bearer Token authentication policy.
     */
    class BearerTokenAuthenticationPolicy final : public HttpPolicy {
    private:
      std::shared_ptr<Credentials::TokenCredential const> const m_credential;
      Credentials::TokenRequestContext m_tokenRequestContext;

      mutable Credentials::AccessToken m_accessToken;
      mutable std::mutex m_accessTokenMutex;

      BearerTokenAuthenticationPolicy(BearerTokenAuthenticationPolicy const&) = delete;
      void operator=(BearerTokenAuthenticationPolicy const&) = delete;

    public:
      /**
       * @brief Construct a Bearer Token authentication policy.
       *
       * @param credential An #Azure::Core::TokenCredential to use with this policy.
       * @param tokenRequestContext #Azure::Core::Credentials::TokenRequestContext.
       */
      explicit BearerTokenAuthenticationPolicy(
          std::shared_ptr<Credentials::TokenCredential const> credential,
          Credentials::TokenRequestContext tokenRequestContext)
          : m_credential(std::move(credential)),
            m_tokenRequestContext(std::move(tokenRequestContext))
      {
      }

      std::unique_ptr<HttpPolicy> Clone() const override
      {
        return std::make_unique<BearerTokenAuthenticationPolicy>(
            m_credential, m_tokenRequestContext);
      }

      std::unique_ptr<RawResponse> Send(
          Request& request,
          NextHttpPolicy policy,
          Context const& context) const override;
    };

    /**
     * @brief Logs every HTTP request.
     *
     * @details Logs every HTTP request and response.
     * @remark See Azure::Core::Diagnostics::Logger.
     */
    class LogPolicy final : public HttpPolicy {
      LogOptions m_options;

    public:
      /**
       * @brief Constructs HTTP logging policy.
       */
      explicit LogPolicy(LogOptions options) : m_options(std::move(options)) {}

      std::unique_ptr<HttpPolicy> Clone() const override
      {
        return std::make_unique<LogPolicy>(*this);
      }

      std::unique_ptr<RawResponse> Send(
          Request& request,
          NextHttpPolicy nextHttpPolicy,
          Context const& ctx) const override;
    };
  } // namespace _internal
}}}} // namespace Azure::Core::Http::Policies
