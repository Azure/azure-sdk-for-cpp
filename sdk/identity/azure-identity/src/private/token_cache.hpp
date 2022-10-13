// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Token cache.
 */

#pragma once

#include <azure/core/credentials/credentials.hpp>

#include <functional>
#include <string>

namespace Azure { namespace Identity { namespace _detail {
  /**
   * @brief Implements common tasks such as token parsing.
   *
   */
  class TokenCache {
  private:
    TokenCache() = delete;
    ~TokenCache() = delete;

  public:
    /**
     * @brief Attempts to get token from cache, and if not found, gets the token using the function
     * provided, caches it, and returns its value.
     *
     * @param tenantId Azure Tenant ID.
     * @param clientId Azure Client ID.
     * @param authorityHost Authentication authority URL.
     * @param scopes Authentication scopes.
     *
     * @return Authentication token.
     *
     * @throw Azure::Core::Credentials::AuthenticationException Authentication error occurred.
     */
    static Core::Credentials::AccessToken GetToken(
        std::string const& tenantId,
        std::string const& clientId,
        std::string const& authorityHost,
        std::string const& scopes,
        std::function<Core::Credentials::AccessToken()> const& getNewToken);
  };
}}} // namespace Azure::Identity::_detail
