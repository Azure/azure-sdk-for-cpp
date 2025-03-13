// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// Code generated by Microsoft (R) TypeSpec Code Generator.
// Changes may cause incorrect behavior and will be lost if the code is regenerated.

#pragma once

#include "azure/keyvault/secrets/generated/models/generated_models.hpp"

#include <azure/core/internal/client_options.hpp>
#include <azure/core/nullable.hpp>

#include <cstdint>
#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets { namespace _detail {
  /**
   * @brief KeyVaultClient options.
   *
   */
  struct KeyVaultClientOptions final : public Core::_internal::ClientOptions
  {
    /// The API version to use for this operation.
    std::string ApiVersion = "7.6-preview.2";
  };

  /**
   * @brief GetSecrets operation options.
   *
   */
  struct KeyVaultClientGetSecretsOptions final
  {
    /// The URL to fetch the next page of results.
    std::string NextPageToken;

    /// Maximum number of results to return in a page. If not specified the service will return up
    /// to 25 results.
    Nullable<std::int32_t> Maxresults;
  };

  /**
   * @brief GetSecretVersions operation options.
   *
   */
  struct KeyVaultClientGetSecretVersionsOptions final
  {
    /// The URL to fetch the next page of results.
    std::string NextPageToken;

    /// Maximum number of results to return in a page. If not specified the service will return up
    /// to 25 results.
    Nullable<std::int32_t> Maxresults;
  };

  /**
   * @brief GetDeletedSecrets operation options.
   *
   */
  struct KeyVaultClientGetDeletedSecretsOptions final
  {
    /// The URL to fetch the next page of results.
    std::string NextPageToken;

    /// Maximum number of results to return in a page. If not specified the service will return up
    /// to 25 results.
    Nullable<std::int32_t> Maxresults;
  };
}}}}} // namespace Azure::Security::KeyVault::Secrets::_detail
