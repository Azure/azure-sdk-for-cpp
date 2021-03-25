// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines a client used to perform cryptographic operations with Azure Key Vault keys.
 *
 */

#pragma once

#include <azure/keyvault/keys/cryptography/cryptography_provider.hpp>

#include <azure/keyvault/common/internal/keyvault_pipeline.hpp>

#include "azure/keyvault/keys/cryptography/cryptography_client_options.hpp"

#include <functional>
#include <list>
#include <vector>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {
  namespace _detail {

    class RemoteCryptographyClient
        : public Azure::Security::KeyVault::Keys::Cryptography::ICryptographyProvider {
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
    };

}}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography::_detail
