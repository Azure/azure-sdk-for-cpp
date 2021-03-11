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
    DateTime ExpiresOn;
  };

  /**
   * @brief Defines context for getting token.
   */
  struct TokenRequestContext
  {
    /**
     * @brief Authentication scopes.
     */
    std::vector<std::string> Scopes;
  };

  /**
   * @brief Token credential.
   */
  class TokenCredential {
  public:
    /**
     * @brief Get an authentication token.
     *
     * @param tokenRequestContext Context to get the token in.
     * @param context #Azure::Core::Context so that operation can be cancelled.
     *
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
   * @brief An exception that gets thrown when authentication error occurs.
   */
  class AuthenticationException : public std::exception {
    std::string m_message;

  public:
    /**
     * @brief Construct with message string.
     *
     * @param message Message string.
     */
    explicit AuthenticationException(std::string message) : m_message(std::move(message)) {}

    /**
     * Get the explanatory string.
     *
     * @return C string with explanatory information.
     */
    char const* what() const noexcept override { return m_message.c_str(); }
  };
}}} // namespace Azure::Core::Credentials
