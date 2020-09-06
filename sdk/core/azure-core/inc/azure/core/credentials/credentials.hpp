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
#include <stdexcept>
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
   * @brief This class is used by Azure SDK clients to authenticate with the Azure service using a
   * tenant ID, client ID and client secret.
   */
  class ClientSecretCredential : public TokenCredential {
  private:
    static std::string const g_aadGlobalAuthority;

    std::string m_tenantId;
    std::string m_clientId;
    std::string m_clientSecret;
    std::string m_authority;

  public:
    /**
     * @brief Construct a Client Secret credential.
     *
     * @param tenantId Tenant ID.
     * @param clientId Client ID.
     * @param clientSecret Client Secret.
     * @param authority Authentication authority URL to set. If omitted, initializes credential with
     * default authority (Azure AD global authority - "https://login.microsoftonline.com/").
     *
     * @note Example of a \p authority string: "https://login.microsoftonline.us/". See national
     * clouds' Azure AD authentication endpoints:
     * https://docs.microsoft.com/en-us/azure/active-directory/develop/authentication-national-cloud.
     */
    explicit ClientSecretCredential(
        std::string tenantId,
        std::string clientId,
        std::string clientSecret,
        std::string authority = g_aadGlobalAuthority)
        : m_tenantId(std::move(tenantId)), m_clientId(std::move(clientId)),
          m_clientSecret(std::move(clientSecret)), m_authority(std::move(authority))
    {
    }

    AccessToken GetToken(Context const& context, std::vector<std::string> const& scopes)
        const override;
  };

  /**
   * @brief An exception that gets thrown when authentication error occurs.
   */
  class AuthenticationException : public std::runtime_error {
  public:
    explicit AuthenticationException(std::string const& msg) : std::runtime_error(msg) {}
  };

  /**
   * @brief An environment credential.
   */
  class EnvironmentCredential : public TokenCredential {
    std::unique_ptr<TokenCredential> m_credentialImpl;

  public:
    /**
     * Constructs an environment credential.
     */
    explicit EnvironmentCredential();

    AccessToken GetToken(Context const& context, std::vector<std::string> const& scopes)
        const override;
  };

}}} // namespace Azure::Core::Credentials
