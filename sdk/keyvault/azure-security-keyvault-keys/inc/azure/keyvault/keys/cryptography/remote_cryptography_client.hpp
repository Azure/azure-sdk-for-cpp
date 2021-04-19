// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief A client used to perform cryptographic operations with Azure Key Vault keys.
 *
 */

#pragma once

#include <azure/core/response.hpp>
#include <azure/core/url.hpp>

#include <azure/keyvault/common/internal/keyvault_pipeline.hpp>

#include "azure/keyvault/keys/cryptography/cryptography_client_options.hpp"
#include "azure/keyvault/keys/cryptography/cryptography_provider.hpp"
#include "azure/keyvault/keys/cryptography/encrypt_parameters.hpp"
#include "azure/keyvault/keys/cryptography/encrypt_result.hpp"
#include "azure/keyvault/keys/key_vault_key.hpp"

#include <memory>
#include <string>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {
  namespace _detail {

    struct RemoteCryptographyClient
        : public Azure::Security::KeyVault::Keys::Cryptography::_detail::CryptographyProvider
    {

      std::shared_ptr<Azure::Security::KeyVault::_internal::KeyVaultPipeline> Pipeline;
      Azure::Core::Url KeyId;

      explicit RemoteCryptographyClient(
          std::string const& keyId,
          std::shared_ptr<Core::Credentials::TokenCredential const> credential,
          CryptographyClientOptions options = CryptographyClientOptions());

      bool CanRemote() const override { return true; }

      bool SupportsOperation(Azure::Security::KeyVault::Keys::KeyOperation) { return true; };

      Azure::Response<KeyVaultKey> GetKey(
          Azure::Core::Context const& context = Azure::Core::Context()) const;

      EncryptResult Encrypt(
          EncryptParameters const& parameters,
          Azure::Core::Context const& context) const override;
    };
}}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography::_detail
