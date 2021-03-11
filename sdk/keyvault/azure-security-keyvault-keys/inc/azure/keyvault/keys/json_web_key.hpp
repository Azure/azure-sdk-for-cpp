// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the JsonWebKey.
 *
 */

#pragma once

#include <azure/core/nullable.hpp>

#include "azure/keyvault/keys/key_curve_name.hpp"
#include "azure/keyvault/keys/key_operation.hpp"
#include "azure/keyvault/keys/key_type.hpp"

#include <string>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  /**
   * @brief Represents a JSON Web Key as defined in http://tools.ietf.org/html/rfc7517.
   *
   */
  struct JsonWebKey
  {
    /**
     * @brief The Identifier of the key. This is not limited to a Url.
     *
     */
    std::string Id;

    /**
     * @brief They type of the key.
     *
     */
    JsonWebKeyType KeyType;

    /**
     * @brief Construct a new Json Web Key object.
     *
     */
    JsonWebKey() = default;

    /**
     * @brief Set the Key Operations object based on a list of operations.
     *
     * @param keyOperations The list of key operations.
     */
    void SetKeyOperations(std::vector<KeyOperation> const& keyOperations)
    {
      m_keyOps = keyOperations;
    }

    /**
     * @brief Get the list of operations from the JsonWebKey.
     *
     * @return std::vector<KeyOperation> const&
     */
    std::vector<KeyOperation> const& KeyOperations() const { return m_keyOps; }

    /**
     * @brief Gets or sets the elliptic curve name.
     *
     * @remark See #KeyCurveName for possible values.
     *
     * @remark If null, the service default is used.
     *
     */
    Azure::Core::Nullable<KeyCurveName> CurveName;

  private:
    std::vector<KeyOperation> m_keyOps;
  };

}}}} // namespace Azure::Security::KeyVault::Keys
