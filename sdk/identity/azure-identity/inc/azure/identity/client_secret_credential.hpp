// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Client Secret Credential.
 */

#pragma once

#include "azure/identity/dll_import_export.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/client_options.hpp>

#include <string>
#include <utility>

namespace Azure { namespace Identity {
  namespace _detail {
    AZ_IDENTITY_DLLEXPORT extern std::string const g_aadGlobalAuthority;
  }

  /**
   * @brief Defines options for token authentication.
   */
  struct ClientSecretCredentialOptions : public Azure::Core::_internal::ClientOptions
  {
  public:
    /**
     * @brief Authentication authority URL.
     * @details Default value is Azure AD global authority -
     * "https://login.microsoftonline.com/".
     *
     * @note Example of a \p authority string: "https://login.microsoftonline.us/". See national
     * clouds' Azure AD authentication endpoints:
     * https://docs.microsoft.com/en-us/azure/active-directory/develop/authentication-national-cloud.
     */
    std::string AuthorityHost = _detail::g_aadGlobalAuthority;
  };

  /**
   * @brief This class is used by Azure SDK clients to authenticate with the Azure service using a
   * tenant ID, client ID and client secret.
   */
  class ClientSecretCredential : public Core::Credentials::TokenCredential {
  private:
    std::string m_tenantId;
    std::string m_clientId;
    std::string m_clientSecret;
    ClientSecretCredentialOptions m_options;

  public:
    /**
     * @brief Construct a Client Secret credential.
     *
     * @param tenantId Tenant ID.
     * @param clientId Client ID.
     * @param clientSecret Client Secret.
     * @param options #Azure::Identity::ClientSecretCredentialOptions.
     */
    explicit ClientSecretCredential(
        std::string tenantId,
        std::string clientId,
        std::string clientSecret,
        ClientSecretCredentialOptions options = ClientSecretCredentialOptions())
        : m_tenantId(std::move(tenantId)), m_clientId(std::move(clientId)),
          m_clientSecret(std::move(clientSecret)), m_options(std::move(options))
    {
    }

    Core::Credentials::AccessToken GetToken(
        Core::Credentials::TokenRequestContext const& tokenRequestContext,
        Core::Context const& context) const override;
  };

}} // namespace Azure::Identity
