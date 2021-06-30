// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the JsonWebKey.
 *
 */

#pragma once

#include <azure/core/internal/json/json.hpp>
#include <azure/core/nullable.hpp>

#include "azure/keyvault/keys/key_curve_name.hpp"
#include "azure/keyvault/keys/key_operation_type.hpp"
#include "azure/keyvault/keys/key_type.hpp"

#include <string>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  /**
   * @brief Represents a JSON Web Key as defined in http://tools.ietf.org/html/rfc7517.
   *
   */
  class JsonWebKey final {
  public:
    /**
     * @brief The Identifier of the key. This is not limited to a Url.
     *
     */
    std::string Id;

    /**
     * @brief They type of the key.
     *
     */
    KeyVaultKeyType KeyType;

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
    void SetKeyOperations(std::vector<KeyOperationType> const& keyOperations)
    {
      m_keyOps = keyOperations;
    }

    /**
     * @brief Get the list of operations from the JsonWebKey.
     *
     * @return std::vector<KeyOperation> const&
     */
    std::vector<KeyOperationType> const& KeyOperations() const { return m_keyOps; }

    /**
     * @brief Gets or sets the elliptic curve name.
     *
     * @remark See #KeyCurveName for possible values.
     *
     * @remark If null, the service default is used.
     *
     */
    Azure::Nullable<KeyCurveName> CurveName;

    /**** RSA fields ****/

    /// The RSA modulus.
    std::vector<uint8_t> N;
    /// The RSA public exponent.
    std::vector<uint8_t> E;
    /// The RSA private key parameter.
    std::vector<uint8_t> DP;
    /// The RSA private key parameter.
    std::vector<uint8_t> DQ;
    /// The RSA private key parameter.
    std::vector<uint8_t> QI;
    /// The RSA secret prime.
    std::vector<uint8_t> P;
    /// The RSA secret prime.
    std::vector<uint8_t> Q;

    /// The RSA private exponent or EC private key.
    std::vector<uint8_t> D;

    /// Gets the symmetric key.
    std::vector<uint8_t> K;
    /// Gets the protected key used with "Bring Your Own Key".
    std::vector<uint8_t> T;
    /// Gets the X coordinate of the elliptic curve point.
    std::vector<uint8_t> X;
    /// Gets the Y coordinate for the elliptic curve point.
    std::vector<uint8_t> Y;

    bool HasPrivateKey() const
    {
      if (KeyType == KeyVaultKeyType::Rsa || KeyType == KeyVaultKeyType::Ec
          || KeyType == KeyVaultKeyType::RsaHsm || KeyType == KeyVaultKeyType::EcHsm)
      {
        return D.size() > 0;
      }

      if (KeyType == KeyVaultKeyType::Oct)
      {
        return K.size() > 0;
      }

      return false;
    }

    bool SupportsOperation(KeyOperationType operation) const
    {
      for (auto supportedOperation : m_keyOps)
      {
        if (operation == supportedOperation)
        {
          return true;
        }
      }
      return false;
    }

  private:
    std::vector<KeyOperationType> m_keyOps;
  };

}}}} // namespace Azure::Security::KeyVault::Keys
