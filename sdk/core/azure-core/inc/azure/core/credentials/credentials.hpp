// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Credentials used for authentication with many (not all) Azure SDK client libraries.
 */

#pragma once

#include "azure/core/context.hpp"
#include "azure/core/datetime.hpp"
#include "azure/core/nullable.hpp"
#include "azure/core/url.hpp"

#include <exception>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace Azure { namespace Core { namespace Credentials {

  /**
   * @brief An access token is used to authenticate requests.
   */
  struct AccessToken final
  {
    /**
     * @brief Token string.
     *
     */
    std::string Token;

    /**
     * @brief A point in time after which the token expires.
     *
     */
    DateTime ExpiresOn;
  };

  /**
   * @brief Context for getting token.
   */
  class TokenRequestContext final {
  public:
    /**
     * @brief Default constructor
     *
     */
    TokenRequestContext() = default;
    /**
     * @brief Scopes constructor
     *
     */
    TokenRequestContext(std::vector<std::string> const& scopes)
    {
      Scopes.assign(scopes.begin(), scopes.end());
    };

    /**
     * @brief Authentication scopes.
     *
     */
    std::vector<std::string> Scopes;

    /**
     * @brief Gets the "authorization" or "authorization_uri" parameter from the challenge
     * response.
     *
     */
    Azure::Nullable<Url> AuthorizationUri;

    /**
     * @brief Gets the tenant ID from <see cref="AuthorizationUri"/>.
     * end with
     *
     */
    Azure::Nullable<std::string> TenantId;
  };

  /**
   * @brief A base type of credential that uses Azure::Core::AccessToken to authenticate requests.
   */
  class TokenCredential {
  public:
    /**
     * @brief Gets an authentication token.
     *
     * @param tokenRequestContext A context to get the token in.
     * @param context A context to control the request lifetime.
     *
     * @throw Azure::Core::Credentials::AuthenticationException Authentication error occurred.
     */
    virtual AccessToken GetToken(
        TokenRequestContext const& tokenRequestContext,
        Context const& context) const = 0;

    /**
     * @brief Destructs `%TokenCredential`.
     *
     */
    virtual ~TokenCredential() = default;

  protected:
    /**
     * @brief Constructs a default instance of `%TokenCredential`.
     *
     */
    TokenCredential() {}

  private:
    /**
     * @brief `%TokenCredential` does not allow copy construction.
     *
     */
    TokenCredential(TokenCredential const&) = delete;

    /**
     * @brief `%TokenCredential` does not allow assignment.
     *
     */
    void operator=(TokenCredential const&) = delete;
  };

  /**
   * @brief An exception that gets thrown when an authentication error occurs.
   */
  class AuthenticationException final : public std::exception {
    std::string m_what;

  public:
    /**
     * @brief Constructs `%AuthenticationException` with a message string.
     *
     * @param what The explanatory string.
     */
    explicit AuthenticationException(std::string what) : m_what(std::move(what)) {}

    /**
     * Gets the explanatory string.
     *
     * @note See https://en.cppreference.com/w/cpp/error/exception/what.
     *
     * @return C string with explanatory information.
     */
    char const* what() const noexcept override { return m_what.c_str(); }
  };
}}} // namespace Azure::Core::Credentials
