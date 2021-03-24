// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines a client used to perform cryptographic operations with Azure Key Vault keys.
 *
 */

#pragma once

#include <azure/core/cryptography/key_encryption_key.hpp>

#include <azure/keyvault/common/internal/keyvault_pipeline.hpp>

#include "azure/keyvault/keys/Cryptography/cryptography_client_options.hpp"

#include <functional>
#include <list>
#include <vector>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {

  class CryptographyClient : public Azure::Core::Cryptography::IKeyEncryptionKey {
  private:
    std::string m_keyId;
    std::shared_ptr<Azure::Security::KeyVault::Common::_internal::KeyVaultPipeline> m_pipeline;

  public:
    /**
     * @brief Construct a new Cryptography Client object.
     *
     * @param keyId The key identifier of the #KeyVaultKey which will be used for cryptographic
     * operations.
     * @param credential
     * @param options
     */
    explicit CryptographyClient(
        std::string const& keyId,
        std::shared_ptr<Core::Credentials::TokenCredential const> credential,
        CryptographyClientOptions options = CryptographyClientOptions())
        : m_keyId(keyId);
  };

}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography
