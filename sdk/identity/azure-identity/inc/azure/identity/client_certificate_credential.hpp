// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Client Certificate Credential and options.
 */

#pragma once

#include "azure/identity/detail/client_credential_helper.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/credentials/token_credential_options.hpp>
#include <azure/core/url.hpp>

#include <memory>
#include <string>

namespace Azure { namespace Identity {
  namespace _detail {
    class TokenCredentialImpl;
  } // namespace _detail

  /**
   * @brief Options for client certificate authentication.
   *
   */
  struct ClientCertificateCredentialOptions final : public Core::Credentials::TokenCredentialOptions
  {
  public:
    /**
     * @brief Authentication authority URL.
     * @note Default value is Azure AD global authority (https://login.microsoftonline.com/).
     *
     * @note Example of a \p authority string: "https://login.microsoftonline.us/". See national
     * clouds' Azure AD authentication endpoints:
     * https://docs.microsoft.com/azure/active-directory/develop/authentication-national-cloud.
     */
    std::string AuthorityHost = _detail::g_aadGlobalAuthority;

    /**
     * @brief Disables multi-tenant discovery feature.
     * The default value can be populated by setting the environment variable
     * `AZURE_IDENTITY_DISABLE_MULTITENANTAUTH` to `true`.
     */
    bool DisableTenantDiscovery
        = _detail::ClientCredentialHelper::IsTenantDiscoveryDisabledByDefault();
  };

  /**
   * @brief Client Certificate Credential authenticates with the Azure services using a Tenant ID,
   * Client ID and a client certificate.
   *
   */
  class ClientCertificateCredential final : public Core::Credentials::TokenCredential {
  private:
    std::unique_ptr<_detail::TokenCredentialImpl> m_tokenCredentialImpl;
    _detail::ClientCredentialHelper m_clientCredentialHelper;
    std::string m_requestBody;
    std::string m_tokenHeaderEncoded;
    std::string m_tokenPayloadStaticPart;
    void* m_pkey;

    explicit ClientCertificateCredential(
        std::string const& tenantId,
        std::string const& clientId,
        std::string const& clientCertificatePath,
        std::string const& authorityHost,
        bool disableTenantDiscovery,
        Core::Credentials::TokenCredentialOptions const& options);

  public:
    /**
     * @brief Constructs a Client Secret Credential.
     *
     * @param tenantId Tenant ID.
     * @param clientId Client ID.
     * @param clientCertificatePath Client certificate path.
     * @param options Options for token retrieval.
     */
    explicit ClientCertificateCredential(
        std::string const& tenantId,
        std::string const& clientId,
        std::string const& clientCertificatePath,
        Core::Credentials::TokenCredentialOptions const& options
        = Core::Credentials::TokenCredentialOptions());

    /**
     * @brief Constructs a Client Secret Credential.
     *
     * @param tenantId Tenant ID.
     * @param clientId Client ID.
     * @param clientCertificatePath Client certificate path.
     * @param options Options for token retrieval.
     */
    explicit ClientCertificateCredential(
        std::string const& tenantId,
        std::string const& clientId,
        std::string const& clientCertificatePath,
        ClientCertificateCredentialOptions const& options);

    /**
     * @brief Destructs `%ClientCertificateCredential`.
     *
     */
    ~ClientCertificateCredential() override;

    /**
     * @brief Gets an authentication token.
     *
     * @param tokenRequestContext A context to get the token in.
     * @param context A context to control the request lifetime.
     *
     * @throw Azure::Core::Credentials::AuthenticationException Authentication error occurred.
     */
    Core::Credentials::AccessToken GetToken(
        Core::Credentials::TokenRequestContext const& tokenRequestContext,
        Core::Context const& context) const override;
  };

}} // namespace Azure::Identity
