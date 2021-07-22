// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Keyvault Secret Parameters definition
 */

#pragma once

#include "keyvault_secret_attributes.hpp"

#include <azure/core/datetime.hpp>
#include <azure/core/nullable.hpp>

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets {

  /**
   * @brief The Secret Set parameters.
   *
   */
  struct KeyVaultSecretSetParameters final 
  {
    /**
     * @brief The value of the secret.
     */
    std::string Value;

    /**
     * @brief The value of the secret.
     */
    Azure::Nullable<std::string> ContentType;

    /**
     * @brief The value of the secret.
     */
    Azure::Nullable<KeyvaultSecretAttributes> Attributes;

    /**
     * @brief Application specific metadata in the form of key-value pairs.
     */
    Azure::Nullable<std::unordered_map<std::string, std::string>> Tags;

    /**
     * @brief Default constructor.
     */
    KeyVaultSecretSetParameters() = default;
    
    /**
     * @brief Constructor.
     * 
     * @param value Value to be set for the secret.
     */
    KeyVaultSecretSetParameters(std::string value) : Value(value) {}
  };
}}}} // namespace Azure::Security::KeyVault::Secrets