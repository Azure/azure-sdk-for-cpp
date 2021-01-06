// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/response.hpp>
#include <azure/keyvault/common/keyvault_pipeline.hpp>

#include "azure/keyvault/keys/key_client_options.hpp"
#include "azure/keyvault/keys/key_vault_key.hpp"

#include <functional>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  namespace Details {
    constexpr static const char* KeysPath = "keys";
    constexpr static const char* DeletedKeysPath = "deletedkeys";
  } // namespace Details

  /**
   * @brief The KeyClient provides synchronous methods to manage a KeyVaultKe in the Azure Key
   * Vault. The client supports creating, retrieving, updating, deleting, purging, backing up,
   * restoring, and listing the KeyVaultKey.
   */
  class KeyClient {
  protected:
    std::unique_ptr<Azure::Security::KeyVault::Common::Internal::KeyVaultPipeline> m_pipeline;

  public:
    /**
     * @brief Construct a new Key Client object
     *
     * @param vaultUrl The url address where the client will send the requests to.
     * @param credential The authentication method to use.
     * @param options The options to customize the client behavior.
     */
    explicit KeyClient(
        std::string const& vaultUrl,
        std::shared_ptr<Core::TokenCredential const> credential,
        KeyClientOptions options = KeyClientOptions());

    /**
     * @brief Optional parameters for KeyVaultClient::GetKey
     *
     */
    struct GetKeyOptions
    {
      /**
       * @brief Context for cancelling long running operations.
       */
      Azure::Core::Context context;
      /**
       * @brief Specify the key version to get.
       */
      std::string version;
    };

    /**
     * @brief Gets the public part of a stored key.
     *
     * @remark The get key operation is applicable to all key types. If the requested key is
     * symmetric, then no key is released in the response. This operation requires the keys/get
     * permission.
     *
     * @param name The name of the key.
     * @param options Optional parameters for this operation.
     * @return The Key wrapped in the Response.
     */
    Azure::Core::Response<KeyVaultKey> GetKey(
        std::string const& name,
        GetKeyOptions const& options = GetKeyOptions()) const
    {
      return m_pipeline->SendRequest<KeyVaultKey>(
          options.context,
          Azure::Core::Http::HttpMethod::Get,
          [name](Azure::Core::Http::RawResponse const& rawResponse) {
            return Details::KeyVaultKeyDeserialize(name, rawResponse);
          },
          {Details::KeysPath, name, options.version});
    }
  };

}}}} // namespace Azure::Security::KeyVault::Keys
