// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the Key Vault Key.
 *
 */

#pragma once

#include "azure/keyvault/keys/json_web_key.hpp"
#include "azure/keyvault/keys/key_operation.hpp"
#include "azure/keyvault/keys/key_properties.hpp"

#include <azure/core/http/http.hpp>
#include <azure/core/internal/json/json.hpp>

#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  /**
   * @brief A key resource and its properties.
   *
   */
  struct KeyVaultKey
  {
    /**
     * @brief The cryptographic key, the key type, and the operations you can perform using the key.
     *
     */
    JsonWebKey Key;

    /**
     * @brief The additional properties.
     *
     */
    KeyProperties Properties;

    /**
     * @brief Construct an empty Key.
     *
     */
    KeyVaultKey() = default;

    /**
     * @brief Construct a new Key Vault Key object.
     *
     * @param name The name of the key.
     */
    KeyVaultKey(std::string name) : Properties(std::move(name)) {}

    /**
     * @brief Get the Key identifier.
     *
     * @return The key id.
     */
    std::string const& Id() const { return Key.Id; }

    /**
     * @brief Gets the name of the Key.
     *
     * @return The name of the key.
     */
    std::string const& Name() const { return Properties.Name; }

    /**
     * @brief Get the Key Type.
     *
     * @return The type of the key.
     */
    JsonWebKeyType const& GetKeyType() const { return Key.KeyType; }

    /**
     * @brief Gets the operations you can perform using the key.
     *
     * @return A vector with the supported operations for the key.
     */
    std::vector<KeyOperation> const& KeyOperations() const { return Key.KeyOperations(); }
  };

  /***********************  Deserializer / Serializer ******************************/
  namespace _detail {
    // Creates a new key based on a name and an http raw response.
    KeyVaultKey KeyVaultKeyDeserialize(
        std::string const& name,
        Azure::Core::Http::RawResponse const& rawResponse);

    // Create from http raw response only.
    KeyVaultKey KeyVaultKeyDeserialize(Azure::Core::Http::RawResponse const& rawResponse);

    // Updates a Key based on an Http raw response.
    void KeyVaultKeyDeserialize(
        KeyVaultKey& key,
        Azure::Core::Http::RawResponse const& rawResponse);

    // Create from json node directly. Used from listKeys
    void KeyVaultKeyDeserialize(KeyVaultKey& key, Azure::Core::Json::_internal::json const& json);

    void static inline ParseKeyUrl(KeyProperties& keyProperties, std::string const& url)
    {
      Azure::Core::Url kid(url);
      keyProperties.Id = url;
      keyProperties.VaultUrl = kid.GetUrlAuthorityWithScheme();
      auto const& path = kid.GetPath();
      //  path is in the form of `verb/keyName{/keyVersion}`
      auto const separatorChar = '/';
      auto pathEnd = path.end();
      auto start = path.begin();
      start = std::find(start, pathEnd, separatorChar);
      start += 1;
      auto separator = std::find(start, pathEnd, separatorChar);
      if (separator != pathEnd)
      {
        keyProperties.Name = std::string(start, separator);
        start = separator + 1;
        keyProperties.Version = std::string(start, pathEnd);
      }
      else
      {
        // Nothing but the name+
        keyProperties.Name = std::string(start, pathEnd);
      }
    }
  } // namespace _detail

}}}} // namespace Azure::Security::KeyVault::Keys
