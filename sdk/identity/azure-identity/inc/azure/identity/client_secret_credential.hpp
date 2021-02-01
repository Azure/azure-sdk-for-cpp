// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Client Secret Credential.
 */

#pragma once

#include "azure/identity/dll_import_export.hpp"

#include <azure/core/credentials.hpp>
#include <azure/core/http/policy.hpp>

#include <string>
#include <utility>

namespace Azure { namespace Identity {

  /**
   * @brief Defines options for token authentication.
   */
  struct TokenCredentialOptions
  {
  private:
    AZ_IDENTITY_DLLEXPORT static std::string const g_aadGlobalAuthority;

  public:
    /**
     * @brief Authentication authority URL.
     * @detail Default value is Azure AD global authority -
     * "https://login.microsoftonline.com/".
     *
     * @note Example of a \p authority string: "https://login.microsoftonline.us/". See national
     * clouds' Azure AD authentication endpoints:
     * https://docs.microsoft.com/en-us/azure/active-directory/develop/authentication-national-cloud.
     */
    std::string AuthorityHost = g_aadGlobalAuthority;

    /**
     * @brief #TransportPolicyOptions for authentication HTTP pipeline.
     */
    Azure::Core::Http::TransportPolicyOptions TransportPolicyOptions;
  };

  /**
   * @brief This class is used by Azure SDK clients to authenticate with the Azure service using a
   * tenant ID, client ID and client secret.
   */
  class ClientSecretCredential : public Core::TokenCredential {
  private:
    std::string m_tenantId;
    std::string m_clientId;
    std::string m_clientSecret;
    TokenCredentialOptions m_options;

  public:
    /**
     * @brief Construct a Client Secret credential.
     *
     * @param tenantId Tenant ID.
     * @param clientId Client ID.
     * @param clientSecret Client Secret.
     * @param options #TokenCredentialOptions.
     */
    explicit ClientSecretCredential(
        std::string tenantId,
        std::string clientId,
        std::string clientSecret,
        TokenCredentialOptions options = TokenCredentialOptions())
        : m_tenantId(std::move(tenantId)), m_clientId(std::move(clientId)),
          m_clientSecret(std::move(clientSecret)), m_options(std::move(options))
    {
    }

    Core::AccessToken GetToken(
        Core::Context const& context,
        Core::Http::GetTokenOptions const& options) const override;
  };

}} // namespace Azure::Identity
