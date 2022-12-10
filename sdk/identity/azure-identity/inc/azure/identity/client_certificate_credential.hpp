// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Client Certificate Credential and options.
 */

#pragma once

#include "azure/identity/detail/client_credential_core.hpp"
#include "azure/identity/detail/token_cache.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/credentials/token_credential_options.hpp>
#include <azure/core/internal/unique_handle.hpp>
#include <azure/core/url.hpp>

#include <memory>
#include <string>

namespace Azure { namespace Identity {
  namespace _detail {
    class TokenCredentialImpl;

    void FreePkeyIfNotNull(void* pkey);

    template <typename> struct UniquePkeyHelper;
    template <> struct UniquePkeyHelper<void*>
    {
      static void FreePkey(void* pkey) { FreePkeyIfNotNull(pkey); }

      using type = Azure::Core::_internal::BasicUniqueHandle<void, FreePkey>;
    };

    using UniquePkeyHandle = Azure::Core::_internal::UniqueHandle<void*, UniquePkeyHelper>;
  } // namespace _detail

  /**
   * @brief Options for client certificate authentication.
   *
   */
  struct ClientCertificateCredentialOptions final : public Core::Credentials::TokenCredentialOptions
  {
    /**
     * @brief Authentication authority URL.
     * @note Default value is Azure AD global authority (https://login.microsoftonline.com/).
     *
     * @note Example of a \p authority string: "https://login.microsoftonline.us/". See national
     * clouds' Azure AD authentication endpoints:
     * https://docs.microsoft.com/azure/active-directory/develop/authentication-national-cloud.
     */
    std::string AuthorityHost = _detail::ClientCredentialCore::AadGlobalAuthority;
  };

  /**
   * @brief Client Certificate Credential authenticates with the Azure services using a Tenant ID,
   * Client ID and a client certificate.
   *
   */
  class ClientCertificateCredential final : public Core::Credentials::TokenCredential {
  private:
    _detail::TokenCache m_tokenCache;
    _detail::ClientCredentialCore m_clientCredentialCore;
    std::unique_ptr<_detail::TokenCredentialImpl> m_tokenCredentialImpl;
    std::string m_requestBody;
    std::string m_tokenPayloadStaticPart;
    std::string m_tokenHeaderEncoded;
    _detail::UniquePkeyHandle m_pkey;

    explicit ClientCertificateCredential(
        std::string tenantId,
        std::string const& clientId,
        std::string const& clientCertificatePath,
        std::string const& authorityHost,
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
        std::string tenantId,
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
        std::string tenantId,
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
