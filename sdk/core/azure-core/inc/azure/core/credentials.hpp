// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Credentials used for authentication with many (not all) Azure SDK client libraries.
 */

#pragma once

#include "azure/core/context.hpp"

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
}} // namespace Azure::Core
