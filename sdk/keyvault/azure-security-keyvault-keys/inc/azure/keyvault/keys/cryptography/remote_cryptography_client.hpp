// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines a client used to perform cryptographic operations with Azure Key Vault keys.
 *
 */

#pragma once

#include <azure/core/response.hpp>

#include <azure/keyvault/common/internal/keyvault_pipeline.hpp>

#include "azure/keyvault/keys/cryptography/cryptography_client_options.hpp"
#include "azure/keyvault/keys/cryptography/decrypt_parameters.hpp"
#include "azure/keyvault/keys/cryptography/decrypt_result.hpp"
#include "azure/keyvault/keys/cryptography/encrypt_parameters.hpp"
#include "azure/keyvault/keys/cryptography/encrypt_result.hpp"

#include <functional>
#include <list>
#include <vector>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {
  namespace _detail {

    class RemoteCryptographyClient {
    private:
      std::string m_keyId;
      std::shared_ptr<Azure::Security::KeyVault::Common::_internal::KeyVaultPipeline> m_pipeline;

    public:
      /**
       * @brief Construct a new Cryptography Client object.
       *
       * @param keyId The key identifier of the #KeyVaultKey which will be used for cryptographic
       * operations.
       * @param credential The authentication method to use.
       * @param options The options to customize the client behavior.
       */
      explicit RemoteCryptographyClient(
          std::string const& keyId,
          std::shared_ptr<Core::Credentials::TokenCredential const> credential,
          CryptographyClientOptions options = CryptographyClientOptions());

      Azure::Response<EncryptResult> Encrypt(
          EncryptParameters parameters,
          Azure::Core::Context const& context = Azure::Core::Context()) const;

      Azure::Response<DecryptResult> Decrypt(
          DecryptParameters parameters,
          Azure::Core::Context const& context = Azure::Core::Context()) const;
    };

}}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography::_detail
