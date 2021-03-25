// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines a client used to perform cryptographic operations with Azure Key Vault keys.
 *
 */

#pragma once

#include <azure/keyvault/common/internal/keyvault_pipeline.hpp>

#include "azure/keyvault/keys/cryptography/cryptography_client_options.hpp"
#include "azure/keyvault/keys/cryptography/remote_cryptography_client.hpp"

#include <functional>
#include <list>
#include <memory>
#include <vector>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {

  class CryptographyClient {
  private:
    std::string m_keyId;
    std::shared_ptr<Azure::Security::KeyVault::Common::_internal::KeyVaultPipeline> m_pipeline;
    std::unique_ptr<
        Azure::Security::KeyVault::Keys::Cryptography::_detail::RemoteCryptographyClient>
        m_remoteProvider;

  public:
    /**
     * @brief Construct a new Cryptography Client object.
     *
     * @param keyId The key identifier of the #KeyVaultKey which will be used for cryptographic
     * operations.
     * @param credential The authentication method to use.
     * @param options The options to customize the client behavior.
     */
    explicit CryptographyClient(
        std::string const& keyId,
        std::shared_ptr<Core::Credentials::TokenCredential const> credential,
        CryptographyClientOptions options = CryptographyClientOptions());

    /**
     * @brief Encrypts the specified plaintext.
     * 
     * @param parameters An #EncryptParameters containing the data to encrypt and other parameters for algorithm-dependent encryption.
     * @param context A #Azure::Core::Context controlling the request lifetime.
     * @return Azure::Response<EncryptResult> 
     */
    Azure::Response<EncryptResult> Encrypt(
        EncryptParameters parameters,
        Azure::Core::Context const& context = Azure::Core::Context()) const;

    Azure::Response<DecryptResult> Decrypt(
        DecryptParameters parameters,
        Azure::Core::Context const& context = Azure::Core::Context()) const;
  };

}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography
