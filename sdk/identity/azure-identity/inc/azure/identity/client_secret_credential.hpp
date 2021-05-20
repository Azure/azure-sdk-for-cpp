// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Client Secret Credential and options.
 */

#pragma once

#include "azure/identity/dll_import_export.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/credentials/token_credential_options.hpp>
#include <azure/core/http/policies/policy.hpp>

#include <string>
#include <utility>

namespace Azure { namespace Identity {
  namespace _detail {
    AZ_IDENTITY_DLLEXPORT extern std::string const g_aadGlobalAuthority;
  }

  /**
   * @brief Options for token authentication.
   *
   */
  struct ClientSecretCredentialOptions final
      : public Azure::Core::Credentials::TokenCredentialOptions
  {
  public:
    /**
     * @brief Authentication authority URL.
     * @note Default value is Azure AD global authority (https://login.microsoftonline.com/).
     *
     * @note Example of a \p authority string: "https://login.microsoftonline.us/". See national
     * clouds' Azure AD authentication endpoints:
     * https://docs.microsoft.com/en-us/azure/active-directory/develop/authentication-national-cloud.
     */
    std::string AuthorityHost = _detail::g_aadGlobalAuthority;
  };

  /**
   * @brief Client Secret Credential authenticates with the Azure services using a Tenant ID, Client
   * ID and a client secret.
   */
  class ClientSecretCredential final : public Core::Credentials::TokenCredential {
  private:
    std::string m_tenantId;
    std::string m_clientId;
    std::string m_clientSecret;
    ClientSecretCredentialOptions m_options;

  public:
    /**
     * @brief Constructs a Client Secret Credential.
     *
     * @param tenantId Tenant ID.
     * @param clientId Client ID.
     * @param clientSecret Client secret.
     * @param options #Azure::Identity::ClientSecretCredentialOptions.
     */
    explicit ClientSecretCredential(
        std::string tenantId,
        std::string clientId,
        std::string clientSecret,
        ClientSecretCredentialOptions options)
        : m_tenantId(std::move(tenantId)), m_clientId(std::move(clientId)),
          m_clientSecret(std::move(clientSecret)), m_options(std::move(options))
    {
    }

    /**
     * @brief Constructs a Client Secret Credential.
     *
     * @param tenantId Tenant ID.
     * @param clientId Client ID.
     * @param clientSecret Client Secret.
     * @param options #Azure::Core::Credentials::TokenCredentialOptions.
     */
    explicit ClientSecretCredential(
        std::string tenantId,
        std::string clientId,
        std::string clientSecret,
        Azure::Core::Credentials::TokenCredentialOptions const& options
        = Azure::Core::Credentials::TokenCredentialOptions())
        : m_tenantId(std::move(tenantId)), m_clientId(std::move(clientId)),
          m_clientSecret(std::move(clientSecret))
    {
      static_cast<Azure::Core::Credentials::TokenCredentialOptions&>(m_options) = options;
    }

    /**
     * @brief Gets an authentication token.
     *
     * @param tokenRequestContext #Azure::Core::Credentials::TokenRequestContext to get the token
     * in.
     * @contextParameter
     *
     * @throw Azure::Core::Credentials::AuthenticationException Authentication error occurred.
     */
    Core::Credentials::AccessToken GetToken(
        Core::Credentials::TokenRequestContext const& tokenRequestContext,
        Core::Context const& context) const override;
  };

}} // namespace Azure::Identity
