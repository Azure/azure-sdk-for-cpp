// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Azure CLI Credential usese Azure CLI to obtain an access token.
 */

#pragma once

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/credentials/token_credential_options.hpp>

#include <memory>

namespace Azure { namespace Identity {
  /**
   * @brief Enables authentication to Azure Active Directory using Azure CLI to obtain an access
   * token.
   *
   */
  class AzureCliCredential final : public Core::Credentials::TokenCredential {
  public:
    /**
     * @brief Constructs an Azure CLI Credential.
     *
     * @param options Options for token retrieval.
     */
    explicit AzureCliCredential(
        Azure::Core::Credentials::TokenCredentialOptions options
        = Azure::Core::Credentials::TokenCredentialOptions())
    {
      static_cast<void>(options);
    }

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
        Core::Context const& context) const override
    {
      static_cast<void>(tokenRequestContext);
      static_cast<void>(context);
      return Core::Credentials::AccessToken{};
    }
  };

}} // namespace Azure::Identity
