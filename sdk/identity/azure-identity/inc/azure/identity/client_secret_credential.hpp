// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Client Secret Credential and options.
 */

#pragma once

#include "azure/identity/dll_import_export.hpp"
#include "azure/identity/internal/token_credential_impl.hpp"

#include <azure/core/credentials/token_credential_options.hpp>
#include <azure/core/url.hpp>

#include <string>

namespace Azure { namespace Identity {
  namespace _detail {
    AZ_IDENTITY_DLLEXPORT extern std::string const g_aadGlobalAuthority;
  }

  /**
   * @brief Options for token authentication.
   *
   */
  struct ClientSecretCredentialOptions final : public Core::Credentials::TokenCredentialOptions
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
   *
   */
  class ClientSecretCredential final : public _detail::TokenCredentialImpl {
  private:
    Core::Url m_requestUrl;
    std::string m_requestBody;
    bool m_isAdfs;

    std::unique_ptr<TokenRequest> CreateRequest(
        Core::Credentials::TokenRequestContext const& tokenRequestContext) const final;

    ClientSecretCredential(
        std::string const& tenantId,
        std::string const& clientId,
        std::string const& clientSecret,
        std::string const& authorityHost,
        Core::Credentials::TokenCredentialOptions const& options);

  public:
    /**
     * @brief Constructs a Client Secret Credential.
     *
     * @param tenantId Tenant ID.
     * @param clientId Client ID.
     * @param clientSecret Client secret.
     * @param options Options for token retrieval.
     */
    explicit ClientSecretCredential(
        std::string const& tenantId,
        std::string const& clientId,
        std::string const& clientSecret,
        ClientSecretCredentialOptions const& options)
        : ClientSecretCredential(tenantId, clientId, clientSecret, options.AuthorityHost, options)
    {
    }

    /**
     * @brief Constructs a Client Secret Credential.
     *
     * @param tenantId Tenant ID.
     * @param clientId Client ID.
     * @param clientSecret Client Secret.
     * @param options Options for token retrieval.
     */
    explicit ClientSecretCredential(
        std::string tenantId,
        std::string clientId,
        std::string clientSecret,
        Core::Credentials::TokenCredentialOptions const& options
        = Core::Credentials::TokenCredentialOptions())
        : ClientSecretCredential(
            tenantId,
            clientId,
            clientSecret,
            _detail::g_aadGlobalAuthority,
            options)
    {
    }
  };

}} // namespace Azure::Identity
