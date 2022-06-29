// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Environment Credential initializes an Azure credential from system environment variables.
 */

#pragma once

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/credentials/token_credential_options.hpp>

#include <memory>

namespace Azure { namespace Identity {
  /**
   * @brief Environment Credential initializes an Azure credential, based on the system environment
   * variables being set.
   *
   */
  class EnvironmentCredential final : public Core::Credentials::TokenCredential {
    std::unique_ptr<TokenCredential> m_credentialImpl;

  public:
    /**
     * @brief Constructs an Environment Credential.
     *
     * @param options Options for token retrieval.
     *
     * @note May read from the following environment variables:
     * - AZURE_TENANT_ID
     * - AZURE_CLIENT_ID
     * - AZURE_CLIENT_SECRET
     * - AZURE_CLIENT_CERTIFICATE_PATH
     * - AZURE_USERNAME
     * - AZURE_PASSWORD
     * - AZURE_AUTHORITY_HOST
     */
    explicit EnvironmentCredential(
        Azure::Core::Credentials::TokenCredentialOptions options
        = Azure::Core::Credentials::TokenCredentialOptions());

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
