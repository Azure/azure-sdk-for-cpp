// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Credentials used for authentication with many (not all) Azure SDK client libraries.
 */

#pragma once

#include <azure/core/context.hpp>
#include <azure/core/http/policy.hpp>

#include <chrono>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace Azure { namespace Core {

  /**
   * @brief Represents an access token.
   */
  struct AccessToken
  {
    /**
     * @brief Token string.
     */
    std::string Token;

    /**
     * @brief Token expiration.
     */
    std::chrono::system_clock::time_point ExpiresOn;
  };

  /**
   * @brief Token credential.
   */
  class TokenCredential {
  public:
    /**
     * @brief Get an authentication token.
     *
     * @param context #Context so that operation can be canceled.
     * @param scopes Authentication scopes.
     */
    virtual AccessToken GetToken(Context const& context, std::vector<std::string> const& scopes)
        const = 0;

    /// Destructor.
    virtual ~TokenCredential() = default;

  protected:
    TokenCredential() {}

  private:
    TokenCredential(TokenCredential const&) = delete;
    void operator=(TokenCredential const&) = delete;
  };

  /**
   * @brief An exception that gets thrown when authentication error occurs.
   */
  class AuthenticationException : public std::runtime_error {
  public:
    /**
     * @brief Construct with message string.
     *
     * @param msg Message string.
     */
    explicit AuthenticationException(std::string const& msg) : std::runtime_error(msg) {}
  };

  /**
   * @brief Bearer Token authentication policy.
   */
  class BearerTokenAuthenticationPolicy : public Http::HttpPolicy {
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

    std::unique_ptr<Http::RawResponse> Send(
        Context const& context,
        Http::Request& request,
        Http::NextHttpPolicy policy) const override;
  };

}} // namespace Azure::Core
