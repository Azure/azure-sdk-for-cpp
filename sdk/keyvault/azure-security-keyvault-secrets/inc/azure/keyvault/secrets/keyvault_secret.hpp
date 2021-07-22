// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Keyvault Secret definition
 */

#pragma once
#include "keyvault_secret_properties.hpp"

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets {
  struct KeyVaultSecret final
  {
    /**
     * @brief The secret value.
     *
     */
    std::string Value;

    /**
     * @brief The secret id.
     *
     */
    std::string Id;

    KeyvaultSecretProperties Properties;

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
    bool Managed = false;

    /**
     * @brief Construct a new SecretBundle object.
     *
     */
    KeyVaultSecret() = default;

    KeyVaultSecret(std::string name) : Properties(std::move(name)) {}
  };
}}}} // namespace Azure::Security::KeyVault::Secrets
