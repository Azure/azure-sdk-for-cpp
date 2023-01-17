// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Most common implementation part for a Token Credential.
 */

#pragma once

#include <azure/core/credentials/token_credential_options.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/http_status_code.hpp>
#include <azure/core/http/raw_response.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/io/body_stream.hpp>

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Azure { namespace Identity { namespace _detail {
  /**
   * @brief Implements common tasks such as token parsing.
   *
   */
  class TokenCredentialImpl {
  private:
    Core::Http::_internal::HttpPipeline m_httpPipeline;

  public:
    /**
     * @brief Destructs `%TokenCredentialImpl`.
     *
     */
    virtual ~TokenCredentialImpl() = default;

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
     * @param urlEncode `true` if the result needs to be URL-encoded.
     *
     * @return A string representing scopes so that it can be used in Identity request.
     *
     * @note Does not check for \p scopes being empty.
     */
    static std::string FormatScopes(
        std::vector<std::string> const& scopes,
        bool asResource,
        bool urlEncode = true);

    /**
     * @brief Parses JSON that contains access token and its expiration.
     *
     * @param jsonString String with a JSON object to parse.
     * @param accessTokenPropertyName Name of a property in the JSON object that represents access
     * token.
     * @param expiresInPropertyName Name of a property in the JSON object that represents token
     * expiration in number of seconds from now.
     * @param expiresOnPropertyName Name of a property in the JSON object that represents token
     * expiration as absolute date-time stamp. Can be empty, in which case no attempt to parse it is
     * made.
     *
     * @return A successfully parsed access token.
     *
     * @throw `std::exception` if there was a problem parsing the token.
     */
    static Core::Credentials::AccessToken ParseToken(
        std::string const& jsonString,
        std::string const& accessTokenPropertyName,
        std::string const& expiresInPropertyName,
        std::string const& expiresOnPropertyName);

    /**
     * @brief Holds `#Azure::Core::Http::Request` and all the associated resources for the HTTP
     * request body, so that the lifetime for all the resources needed for the request aligns
     * with its lifetime, and so that instances of this class can easily be returned from a
     * function.
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

    /**
     * @brief Gets an authentication token.
     *
     * @param context A context to control the request lifetime.
     * @param createRequest A function to create a token request.
     * @param shouldRetry A function to determine whether a response should be retried with
     * another request.
     *
     * @throw Azure::Core::Credentials::AuthenticationException Authentication error occurred.
     */
    Core::Credentials::AccessToken GetToken(
        Core::Context const& context,
        std::function<std::unique_ptr<TokenRequest>()> const& createRequest,
        std::function<std::unique_ptr<TokenRequest>(
            Core::Http::HttpStatusCode statusCode,
            Core::Http::RawResponse const& response)> const& shouldRetry
        = [](auto const, auto const&) { return nullptr; }) const;
  };
}}} // namespace Azure::Identity::_detail
