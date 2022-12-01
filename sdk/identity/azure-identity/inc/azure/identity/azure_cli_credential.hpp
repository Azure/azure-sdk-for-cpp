// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Azure CLI Credential uses Azure CLI to obtain an access token.
 */

#pragma once

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/credentials/token_credential_options.hpp>

#include <azure/core/datetime.hpp>

#include <chrono>
#include <string>

namespace Azure { namespace Identity {
  /**
   * @brief Options for configuring the #Azure::Identity::AzureCliCredential.
   */
  struct AzureCliCredentialOptions final : public Core::Credentials::TokenCredentialOptions
  {
  public:
    /**
     * @brief The ID of the tenant to which the credential will authenticate by default. If not
     * specified, the credential will authenticate to any requested tenant, and will default to the
     * tenant provided to the 'az login' command.
     */
    std::string TenantId;

    /**
     * @brief The CLI process timeout.
     */
    DateTime::duration CliProcessTimeout
        = std::chrono::seconds(13); // Value was taken from .NET SDK.
  };

  /**
   * @brief Enables authentication to Azure Active Directory using Azure CLI to obtain an access
   * token.
   */
  class AzureCliCredential
#if !defined(TESTING_BUILD)
      final
#endif
      : public Core::Credentials::TokenCredential {
  protected:
    std::string m_tenantId;
    std::chrono::steady_clock::duration m_cliProcessTimeout;

  private:
    explicit AzureCliCredential(
        std::string tenantId,
        DateTime::duration const& cliProcessTimeout,
        Core::Credentials::TokenCredentialOptions const& options);

  public:
    /**
     * @brief Constructs an Azure CLI Credential.
     *
     * @param options Options for token retrieval.
     */
    explicit AzureCliCredential(AzureCliCredentialOptions const& options = {});

    /**
     * @brief Constructs an Azure CLI Credential.
     *
     * @param options Options for token retrieval.
     */
    explicit AzureCliCredential(Core::Credentials::TokenCredentialOptions const& options);

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

  protected:
    virtual std::string GetAzCommand(std::string const& resource, std::string const& tenantId)
        const;
  };

}} // namespace Azure::Identity
