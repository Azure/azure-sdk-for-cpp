// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Keyvault Secret Attributes definition
 */

#pragma once

#include <azure/core/datetime.hpp>
#include <azure/core/nullable.hpp>
#include <azure/core/url.hpp>
#include <unordered_map>

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets {

  /**
   * @brief The Secret attributes managed by the KeyVault service.
   *
   */
  struct SecretProperties final
  {
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
     * @brief The content type of the secret.
     *
     */
    Azure::Nullable<std::string> ContentType;

    /**
     * @brief  If this is a secret backing a KV certificate, then this field specifies the
     * corresponding key backing the KV certificate.
     *
     */
    Azure::Nullable<std::string> KeyId;

    /**
     * @brief Application specific metadata in the form of key-value pairs.
     *
     */
    std::unordered_map<std::string, std::string> Tags;

    /**
     * @brief True if the secret's lifetime is managed by key vault. If this is a secret
     * backing a certificate, then managed will be true.
     *
     */
    bool Managed;

    /**
     * @brief The secret id.
     *
     */
    std::string Id;

    /**
     * @brief The name of the secret.
     *
     */
    std::string Name;

    /**
     * @brief The vault url of the secret.
     *
     */
    std::string VaultUrl;

    /**
     * @brief The version of the secret.
     *
     */
    std::string Version;

    /**
     * @brief Construct a new secret Properties object.
     *
     */
    SecretProperties() = default;

    /**
     * @brief Construct a new secret Properties object.
     *
     */
    SecretProperties(std::string const& name) : Name(name)
    {
      if (Name.empty())
      {
        throw std::invalid_argument("Name cannot be empty");
      }
    }

    /**
     * @brief Construct a new secret Properties object.
     *
     */
    static SecretProperties CreateFromURL(std::string const& url);
  };
}}}} // namespace Azure::Security::KeyVault::Secrets