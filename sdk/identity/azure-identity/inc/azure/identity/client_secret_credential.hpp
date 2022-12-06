// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Client Secret Credential and options.
 */

#pragma once

#include "azure/identity/detail/token_cache.hpp"
#include "azure/identity/dll_import_export.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/credentials/token_credential_options.hpp>
#include <azure/core/url.hpp>

#include <memory>
#include <string>

namespace Azure { namespace Identity {
  namespace _detail {
    class TokenCredentialImpl;
    AZ_IDENTITY_DLLEXPORT extern std::string const g_aadGlobalAuthority;
  } // namespace _detail

  /**
   * @brief Options for token authentication.
   *
   */
  struct ClientSecretCredentialOptions final : public Core::Credentials::TokenCredentialOptions
  {
    /**
     * @brief Authentication authority URL.
     * @note Default value is Azure AD global authority (https://login.microsoftonline.com/).
     *
     * @note Example of a \p authority string: "https://login.microsoftonline.us/". See national
     * clouds' Azure AD authentication endpoints:
     * https://docs.microsoft.com/azure/active-directory/develop/authentication-national-cloud.
     */
    std::string AuthorityHost = _detail::g_aadGlobalAuthority;
  };

  /**
   * @brief Client Secret Credential authenticates with the Azure services using a Tenant ID, Client
   * ID and a client secret.
   *
   */
  class ClientSecretCredential final : public Core::Credentials::TokenCredential {
  private:
    _detail::TokenCache m_tokenCache;
    std::unique_ptr<_detail::TokenCredentialImpl> m_tokenCredentialImpl;
    Core::Url m_requestUrl;
    std::string m_requestBody;

    std::string m_tenantId;
    std::string m_clientId;
    std::string m_authorityHost;

    bool m_isAdfs;

    ClientSecretCredential(
        std::string tenantId,
        std::string clientId,
        std::string const& clientSecret,
        std::string authorityHost,
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
        std::string tenantId,
        std::string clientId,
        std::string const& clientSecret,
        ClientSecretCredentialOptions const& options);

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
        std::string const& clientSecret,
        Core::Credentials::TokenCredentialOptions const& options
        = Core::Credentials::TokenCredentialOptions());

    /**
     * @brief Destructs `%ClientSecretCredential`.
     *
     */
    ~ClientSecretCredential() override;

    /**
     * @brief Gets an authentication token.
     *
     * @param tokenRequestContext A context to get the token in.
     * @param context A context to control the request lifetime.
     *
     * @return Authentication token.
     *
     * @throw Azure::Core::Credentials::AuthenticationException Authentication error occurred.
     */
    Core::Credentials::AccessToken GetToken(
        Core::Credentials::TokenRequestContext const& tokenRequestContext,
        Core::Context const& context) const override;
  };

}} // namespace Azure::Identity
