// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// Code generated by Microsoft (R) TypeSpec Code Generator.
// Changes may cause incorrect behavior and will be lost if the code is regenerated.

#pragma once

#include "azure/keyvault/secrets/generated/key_vault_client_options.hpp"
#include "azure/keyvault/secrets/generated/models/generated_models.hpp"

#include <azure/core/datetime.hpp>
#include <azure/core/internal/extendable_enumeration.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/paged_response.hpp>

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Secrets {
        namespace Generated {
  class KeyVaultClient;

  /**
   * @brief The secret list result.
   *
   */
  class GetSecretsPagedResponse final : public Core::PagedResponse<GetSecretsPagedResponse> {
    friend class KeyVaultClient;
    friend class Core::PagedResponse<GetSecretsPagedResponse>;

  private:
    std::shared_ptr<KeyVaultClient> m_client;
    KeyVaultClientGetSecretsOptions m_options;

    void OnNextPage(Core::Context const& context);

  public:
    /// A response message containing a list of secrets in the key vault along with a link to the
    /// next page of secrets.
    Nullable<std::vector<Models::SecretItem>> Value;
  };

  /**
   * @brief The secret list result.
   *
   */
  class GetSecretVersionsPagedResponse final
      : public Core::PagedResponse<GetSecretVersionsPagedResponse> {
    friend class KeyVaultClient;
    friend class Core::PagedResponse<GetSecretVersionsPagedResponse>;

  private:
    std::shared_ptr<KeyVaultClient> m_client;
    std::string m_secretName;
    KeyVaultClientGetSecretVersionsOptions m_options;

    void OnNextPage(Core::Context const& context);

  public:
    /// A response message containing a list of secrets in the key vault along with a link to the
    /// next page of secrets.
    Nullable<std::vector<Models::SecretItem>> Value;
  };

  /**
   * @brief The deleted secret list result
   *
   */
  class GetDeletedSecretsPagedResponse final
      : public Core::PagedResponse<GetDeletedSecretsPagedResponse> {
    friend class KeyVaultClient;
    friend class Core::PagedResponse<GetDeletedSecretsPagedResponse>;

  private:
    std::shared_ptr<KeyVaultClient> m_client;
    KeyVaultClientGetDeletedSecretsOptions m_options;

    void OnNextPage(Core::Context const& context);

  public:
    /// A response message containing a list of deleted secrets in the key vault along with a link
    /// to the next page of deleted secrets.
    Nullable<std::vector<Models::DeletedSecretItem>> Value;
  };
}}}}} // namespace Azure::Security::KeyVault::Secrets::Generated
