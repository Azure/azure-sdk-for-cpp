// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Credentials used for authentication with many (not all) Azure SDK client libraries.
 */

#pragma once

#include "azure/core/context.hpp"
#include "azure/core/datetime.hpp"

#include <exception>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace Azure { namespace Core { namespace Credentials {

  /**
   * @brief An access token is used to authenticate requests.
   *
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
   *
   */
  struct TokenRequestContext final
  {
    /**
     * @brief Authentication scopes.
     *
     */
    std::vector<std::string> Scopes;
  };

  /**
   * @brief A base type of credential that uses Azure::Core::AccessToken to authenticate requests.
   *
   */
  class TokenCredential {
  public:
    /**
     * @brief Gets an authentication token.
     *
     * @param tokenRequestContext #Azure::Core::Credentials::TokenRequestContext to get the token
     * in.
     * @param context #Azure::Core::Context so that operation can be cancelled.
     *
     * @throw Azure::Core::Credentials::AuthenticationException Authentication error occurred.
     */
    virtual AccessToken GetToken(
        TokenRequestContext const& tokenRequestContext,
        Context const& context) const = 0;

    /// Destructor.
    virtual ~TokenCredential() = default;

  protected:
    TokenCredential() {}

  private:
    TokenCredential(TokenCredential const&) = delete;
    void operator=(TokenCredential const&) = delete;
  };

  /**
   * @brief An exception that gets thrown when an authentication error occurs.
   *
   */
  class AuthenticationException final : public std::exception {
    std::string m_message;

  public:
    /**
     * @brief Constructs with message string.
     *
     * @param message Message string.
     */
    explicit AuthenticationException(std::string message) : m_message(std::move(message)) {}

    /**
     * Gets the explanatory string.
     *
     * @note See https://en.cppreference.com/w/cpp/error/exception/what.
     *
     * @return C string with explanatory information.
     */
    char const* what() const noexcept override { return m_message.c_str(); }
  };
}}} // namespace Azure::Core::Credentials
