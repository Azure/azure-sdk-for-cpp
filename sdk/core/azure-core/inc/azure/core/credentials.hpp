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
    DateTime ExpiresOn;
  };

  namespace Http {
    class TransportPolicy;
  }

  /**
   * @brief Defines parameters for getting token.
   */
  struct GetTokenOptions
  {
    std::vector<std::string> Scopes;
    std::unique_ptr<Azure::Core::Http::TransportPolicy> TransportPolicy;
  };

  /**
   * @brief Token credential.
   */
  class TokenCredential {
  public:
    /**
     * @brief Get an authentication token.
     *
     * @param context #Context so that operation can be cancelled.
     * @param options Options to get the token.
     */
    virtual AccessToken GetToken(Context const& context, GetTokenOptions const& options) const = 0;

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
}} // namespace Azure::Core
