// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Keyvault Secret Attributes definition
 */

#pragma once

#include <azure/core/datetime.hpp>
#include <azure/core/nullable.hpp>

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets {

  /**
   * @brief The Secret attributes managed by the KeyVault service.
   *
   */
  struct KeyvaultSecretAttributes final
  {
    /**
     * @brief The name of the secret.
     *
     */
    Azure::Nullable<std::string> Name;
    /**
     * @brief Indicate whether the secret is enabled and useable for cryptographic operations.
     *
     */
    Azure::Nullable<bool> Enabled;

    /**
     * @brief Indicate when the secret will be valid and can be used for cryptographic operations.
     *
     */
    Azure::Nullable<Azure::DateTime> NotBefore;

    /**
     * @brief Indicate when the secret will expire and cannot be used for cryptographic operations.
     *
     */
    Azure::Nullable<Azure::DateTime> ExpiresOn;

    /**
     * @brief Indicate when the secret was created.
     *
     */
    Azure::Nullable<Azure::DateTime> CreatedOn;

    /**
     * @brief Indicate when the secret was updated.
     *
     */
    Azure::Nullable<Azure::DateTime> UpdatedOn;

    /**
     * @brief The number of days a secret is retained before being deleted for a soft delete-enabled
     * Key Vault.
     *
     */
    Azure::Nullable<int64_t> RecoverableDays;

    /**
     * @brief The recovery level currently in effect for secrets in the Key Vault.
     *
     * @remark If Purgeable, the secret can be permanently deleted by an authorized user; otherwise,
     * only the service can purge the secret at the end of the retention interval.
     *
     */
    Azure::Nullable<std::string> RecoveryLevel;

    /**
     * @brief Construct a new secret Properties object.
     *
     */
    KeyvaultSecretAttributes() = default;

    /**
     * @brief Construct a new secret Properties object.
     *
     * @param name The name of the secret.
     */
    KeyvaultSecretAttributes(std::string name) : Name(std::move(name)) {}
  };
}}}} // namespace Azure::Security::KeyVault::Secrets
