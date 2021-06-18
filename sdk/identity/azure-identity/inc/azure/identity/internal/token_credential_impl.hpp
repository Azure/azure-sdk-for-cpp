// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Most common implementation part for a Token Credential.
 */

#pragma once

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/credentials/token_credential_options.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/http_status_code.hpp>
#include <azure/core/http/raw_response.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/io/body_stream.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Azure { namespace Identity { namespace _detail {
  /**
   * @brief Implements `GetToken()`, requiring deriving classes to implement `GetRequest()`.
   *
   */
  class TokenCredentialImpl : public Core::Credentials::TokenCredential {
  protected:
    /**
     * @brief Holds `#Azure::Core::Http::Request` and all the associated resources for the HTTP
     * request body, so that the lifetime for all the resources needed for the request aligns with
     * its lifetime, and so that instances of this class can easily be returned from a function.
     *
     */
    class TokenRequest final {
    private:
      std::unique_ptr<std::string> m_body;
      std::unique_ptr<Core::IO::MemoryBodyStream> m_memoryBodyStream;

    public:
      /**
       * @brief HTTP request.
       *
       */
      Core::Http::Request HttpRequest;

      /**
       * @brief Constructs `%TokenRequest` from HTTP request components.
       *
       * @param httpMethod HTTP method for the `HttpRequest`.
       * @param url URL for the `HttpRequest`.
       * @param body Body for the `HttpRequest`.
       */
      explicit TokenRequest(Core::Http::HttpMethod httpMethod, Core::Url url, std::string body)
          : m_body(new std::string(std::move(body))),
            m_memoryBodyStream(new Core::IO::MemoryBodyStream(
                reinterpret_cast<uint8_t const*>(m_body->data()),
                m_body->size())),
            HttpRequest(std::move(httpMethod), std::move(url), m_memoryBodyStream.get())
      {
        HttpRequest.SetHeader("Content-Type", "application/x-www-form-urlencoded");
        HttpRequest.SetHeader("Content-Length", std::to_string(m_body->size()));
      }

      /**
       * @brief Constructs `%TokenRequest` from HTTP request.
       * @param httpRequest HTTP request to initialize `HttpRequest` with.
       */
      explicit TokenRequest(Core::Http::Request httpRequest) : HttpRequest(std::move(httpRequest))
      {
      }
    };

  private:
    Core::Http::_internal::HttpPipeline m_httpPipeline;

    virtual std::unique_ptr<TokenRequest> CreateRequest(
        Core::Credentials::TokenRequestContext const& tokenRequestContext) const = 0;

    virtual std::unique_ptr<TokenRequest> ShouldRetry(
        Core::Http::HttpStatusCode statusCode,
        Core::Http::RawResponse const& response,
        Core::Credentials::TokenRequestContext const& tokenRequestContext) const
    {
      static_cast<void>(statusCode);
      static_cast<void>(response);
      static_cast<void>(tokenRequestContext);

      return nullptr;
    }

  protected:
    /**
     * @brief Constructs `%TokenCredentialImpl`.
     *
     */
    explicit TokenCredentialImpl(Core::Credentials::TokenCredentialOptions const& options);

    /**
     * @brief Formats authentication scopes so that they can be used in Identity requests.
     *
     * @param scopes Authentication scopes.
     * @param asResource `true` if \p scopes need to be formatted as a resource.
     *
     * @return A string representing scopes so that it can be used in Identity request.
     *
     * @note Does not check for \p scopes being empty.
     */
    static std::string FormatScopes(std::vector<std::string> const& scopes, bool asResource);

  public:
    /**
     * @brief Gets an authentication token.
     *
     * @param tokenRequestContext A context to get the token in.
     * @param context A context to control the request lifetime.
     *
     * @throw Azure::Core::Credentials::AuthenticationException Authentication error occurred.
     *
     * @note Invokes `GetRequest()` to get the request to send.
     */
    Core::Credentials::AccessToken GetToken(
        Core::Credentials::TokenRequestContext const& tokenRequestContext,
        Core::Context const& context) const final;
  };

}}} // namespace Azure::Identity::_detail
