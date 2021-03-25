// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Environment Credential.
 */

#pragma once

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/internal/client_options.hpp>

#include <memory>

namespace Azure { namespace Identity {

  /**
   * @brief Defines options for #Azure::Identity::EnvironmentCredential.
   */
  struct EnvironmentCredentialOptions : public Azure::Core::_internal::ClientOptions
  {
  };

  /**
   * @brief An environment credential.
   */
  class EnvironmentCredential : public Core::Credentials::TokenCredential {
    std::unique_ptr<TokenCredential> m_credentialImpl;
    EnvironmentCredentialOptions m_options;

  public:
    /**
     * Constructs an environment credential.
     *
     * @details May read from the following environment variables:
     * - AZURE_TENANT_ID
     * - AZURE_CLIENT_ID
     * - AZURE_CLIENT_SECRET
     * - AZURE_CLIENT_CERTIFICATE_PATH
     * - AZURE_USERNAME
     * - AZURE_PASSWORD
     */
    explicit EnvironmentCredential(
        EnvironmentCredentialOptions options = EnvironmentCredentialOptions());

    Core::Credentials::AccessToken GetToken(
        Core::Credentials::TokenRequestContext const& tokenRequestContext,
        Core::Context const& context) const override;
  };

}} // namespace Azure::Identity
