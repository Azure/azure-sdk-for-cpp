//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Keyvault Secret definition
 */

#pragma once
#include "azure/keyvault/secrets/keyvault_secret_properties.hpp"

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets {
  struct KeyVaultSecret
  {
    /**
     * @brief The name of the secret.
     *
     */
    std::string Name;

    /**
     * @brief The secret value.
     *
     */
    Azure::Nullable<std::string> Value;

    /**
     * @brief The secret id.
     *
     */
    std::string Id;

    /**
     * @brief The secret Properties bundle.
     *
     */
    SecretProperties Properties;

    /**
     * @brief Construct a new Secret object.
     *
     */
    KeyVaultSecret() = default;

    /**
     * @brief Construct a new Secret object.
     *
     * @param name The name of the secret.
     * @param value The name of the secret.
     */
    KeyVaultSecret(std::string const& name, std::string const& value)
        : Name(name), Value(value), Properties(name)
    {
      if (Name.empty())
      {
        throw std::invalid_argument("Name cannot be empty");
      }

      if (Value.HasValue() == false || Value.Value().empty())
      {
        throw std::invalid_argument("Value cannot be empty");
      }
    }

  private:
    KeyVaultSecret(std::string name) : Name(std::move(name))
    {
      if (Name.empty())
      {
        throw std::invalid_argument("Name cannot be empty");
      }
    }

    friend struct DeletedSecret;
  };

}}}} // namespace Azure::Security::KeyVault::Secrets
