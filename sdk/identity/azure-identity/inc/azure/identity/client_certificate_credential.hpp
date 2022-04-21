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
  };

  /**
   * @brief Client Certificate Credential authenticates with the Azure services using a Tenant ID,
   * Client ID and a client certificate.
   *
   */
  class ClientCertificateCredential final : public Core::Credentials::TokenCredential {
  private:
    std::unique_ptr<_detail::TokenCredentialImpl> m_tokenCredentialImpl;
    Core::Url m_requestUrl;
    std::string m_requestBody;
    std::string m_tokenHeaderEncoded;
    std::string m_tokenPayloadStaticPart;
    void* m_pkey;

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
