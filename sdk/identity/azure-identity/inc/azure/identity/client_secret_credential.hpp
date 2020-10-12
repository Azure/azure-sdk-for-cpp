// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Client Secret Credential.
 */

#pragma once

#include <azure/core/credentials.hpp>

#include <string>
#include <utility>

namespace Azure { namespace Identity {

  /**
   * @brief This class is used by Azure SDK clients to authenticate with the Azure service using a
   * tenant ID, client ID and client secret.
   */
  class ClientSecretCredential : public Core::TokenCredential {
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

    Core::AccessToken GetToken(Core::Context const& context, std::vector<std::string> const& scopes)
        const override;
  };

}} // namespace Azure::Identity
