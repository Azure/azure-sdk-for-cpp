// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief A remote client used to perform cryptographic operations with Azure Key Vault keys.
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

      bool SupportsOperation(Azure::Security::KeyVault::Keys::KeyOperation) override
      {
        return true;
      };

      Azure::Response<KeyVaultKey> GetKey(
          Azure::Core::Context const& context = Azure::Core::Context()) const;

      EncryptResult Encrypt(
          EncryptParameters const& parameters,
          Azure::Core::Context const& context) const override;

      Azure::Response<EncryptResult> EncryptWithResponse(
          EncryptParameters const& parameters,
          Azure::Core::Context const& context) const;

      DecryptResult Decrypt(
          DecryptParameters const& parameters,
          Azure::Core::Context const& context) const override;

      Azure::Response<DecryptResult> DecryptWithResponse(
          DecryptParameters const& parameters,
          Azure::Core::Context const& context) const;

      Azure::Response<WrapResult> WrapKeyWithResponse(
          KeyWrapAlgorithm const& algorithm,
          std::vector<uint8_t> const& key,
          Azure::Core::Context const& context) const;

      WrapResult WrapKey(
          KeyWrapAlgorithm const& algorithm,
          std::vector<uint8_t> const& key,
          Azure::Core::Context const& context) const override;

      Azure::Response<UnwrapResult> UnwrapKeyWithResponse(
          KeyWrapAlgorithm const& algorithm,
          std::vector<uint8_t> const& encryptedKey,
          Azure::Core::Context const& context) const;

      UnwrapResult UnwrapKey(
          KeyWrapAlgorithm const& algorithm,
          std::vector<uint8_t> const& encryptedKey,
          Azure::Core::Context const& context) const override;

      Azure::Response<SignResult> SignWithResponse(
          SignatureAlgorithm const& algorithm,
          std::vector<uint8_t> const& digest,
          Azure::Core::Context const& context) const;

      SignResult Sign(
          SignatureAlgorithm const& algorithm,
          std::vector<uint8_t> const& digest,
          Azure::Core::Context const& context) const override;

      Azure::Response<VerifyResult> VerifyWithResponse(
          SignatureAlgorithm const& algorithm,
          std::vector<uint8_t> const& digest,
          std::vector<uint8_t> const& signature,
          Azure::Core::Context const& context) const;

      VerifyResult Verify(
          SignatureAlgorithm const& algorithm,
          std::vector<uint8_t> const& digest,
          std::vector<uint8_t> const& signature,
          Azure::Core::Context const& context) const override;
    };
}}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography::_detail
