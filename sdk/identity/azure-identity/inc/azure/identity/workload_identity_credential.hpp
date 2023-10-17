// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Workload Identity Credential and options.
 */

#pragma once

#include "azure/identity/detail/client_credential_core.hpp"
#include "azure/identity/detail/token_cache.hpp"

#include <azure/core/credentials/token_credential_options.hpp>

#include <string>
#include <vector>

namespace Azure { namespace Identity {
  namespace _detail {
    class TokenCredentialImpl;
  } // namespace _detail

  /**
   * @brief Options for workload identity credential.
   *
   */
  struct WorkloadIdentityCredentialOptions final : public Core::Credentials::TokenCredentialOptions
  {
    /**
     * @brief The TenantID of the service principal. Defaults to the value of the environment
     * variable AZURE_TENANT_ID.
     */
    std::string TenantId;

    /**
     * @brief The ClientID of the service principal. Defaults to the value of the environment
     * variable AZURE_CLIENT_ID.
     */
    std::string ClientId;

    /**
     * @brief Authentication authority URL.
     * @note Defaults to the value of the environment variable 'AZURE_AUTHORITY_HOST'. If that's not
     * set, the default value is Microsoft Entra global authority
     * (https://login.microsoftonline.com/).
     *
     * @note Example of an authority host string: "https://login.microsoftonline.us/". See national
     * clouds' Microsoft Entra authentication endpoints:
     * https://learn.microsoft.com/azure/active-directory/develop/authentication-national-cloud.
     */
    std::string AuthorityHost;

    /**
     * @brief The path of a file containing a Kubernetes service account token. Defaults to the
     * value of the environment variable AZURE_FEDERATED_TOKEN_FILE.
     */
    std::string TokenFilePath;

    /**
     * @brief For multi-tenant applications, specifies additional tenants for which the credential
     * may acquire tokens. Add the wildcard value `"*"` to allow the credential to acquire tokens
     * for any tenant in which the application is installed.
     */
    std::vector<std::string> AdditionallyAllowedTenants;
  };

  /**
   * @brief Workload Identity Credential supports Azure workload identity authentication on
   * Kubernetes and other hosts supporting workload identity. See the Azure Kubernetes Service
   * documentation at https://learn.microsoft.com/azure/aks/workload-identity-overview for more
   * information.
   *
   */
  class WorkloadIdentityCredential final : public Core::Credentials::TokenCredential {
  private:
    _detail::TokenCache m_tokenCache;
    _detail::ClientCredentialCore m_clientCredentialCore;
    std::unique_ptr<_detail::TokenCredentialImpl> m_tokenCredentialImpl;
    std::string m_requestBody;
    std::string m_tokenFilePath;

  public:
    /**
     * @brief Constructs a Workload Identity Credential.
     *
     * @param options Options for token retrieval.
     */
    explicit WorkloadIdentityCredential(
        Core::Credentials::TokenCredentialOptions const& options
        = Core::Credentials::TokenCredentialOptions());

    /**
     * @brief Constructs a Workload Identity Credential.
     *
     * @param options Options for token retrieval.
     */
    explicit WorkloadIdentityCredential(WorkloadIdentityCredentialOptions const& options);

    /**
     * @brief Destructs `%WorkloadIdentityCredential`.
     *
     */
    ~WorkloadIdentityCredential() override;

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
