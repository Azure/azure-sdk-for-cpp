// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Default Azure Credential.
 */

#pragma once

#include <azure/core/credentials/token_credential_options.hpp>

#include <azure/identity/chained_token_credential.hpp>

#include <memory>

namespace Azure { namespace Identity {

  /**
   * @brief Default Azure Credential combines multiple credentials that depend on the setup
   * environment and require no parameters into a single chain. If the environment is set up
   * sufficiently for at least one of such credentials to work, `DefaultAzureCredential` will work
   * as well.
   *
   * @details This credential is using the #ChainedTokenCredential of 3 credentials in the order:
   * #EnvironmentCredential, #AzureCliCredential, and #ManagedIdentityCredential. Even though the
   * credentials being used and their order is documented, it may be changed in the future versions
   * of the SDK, potentially bringing breaking changes in its behavior.
   *
   * @note This credential is intended to be used at the early stages of development, to allow the
   * developer some time to work with the other aspects of the SDK, and later to replace this
   * credential with the exact credential that is the best fit for the application. It is not
   * intended to be used in a production environment.
   *
   */
  class DefaultAzureCredential final : public Core::Credentials::TokenCredential {
  public:
    /**
     * @brief Constructs `%DefaultAzureCredential`.
     *
     */
    explicit DefaultAzureCredential()
        : DefaultAzureCredential(Core::Credentials::TokenCredentialOptions{}){};

    /**
     * @brief Constructs `%DefaultAzureCredential`.
     *
     * @param options Generic Token Credential Options.
     */
    explicit DefaultAzureCredential(Core::Credentials::TokenCredentialOptions const& options);

    /**
     * @brief Destructs `%DefaultAzureCredential`.
     *
     */
    ~DefaultAzureCredential() override;

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

    /**
     * @brief Gets the name of the credential.
     *
     */
    std::string GetCredentialName() const override;

  private:
    std::shared_ptr<ChainedTokenCredential> m_credentials;
  };

}} // namespace Azure::Identity
