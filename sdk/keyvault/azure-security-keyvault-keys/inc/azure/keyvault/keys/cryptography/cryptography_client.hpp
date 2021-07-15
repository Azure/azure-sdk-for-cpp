// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief A client used to perform cryptographic operations with Azure Key Vault keys.
 *
 */

#pragma once

#include <azure/core/context.hpp>
#include <azure/core/io/body_stream.hpp>

#include "azure/keyvault/keys/cryptography/cryptography_client_options.hpp"
#include "azure/keyvault/keys/cryptography/decrypt_parameters.hpp"
#include "azure/keyvault/keys/cryptography/decrypt_result.hpp"
#include "azure/keyvault/keys/cryptography/encrypt_parameters.hpp"
#include "azure/keyvault/keys/cryptography/encrypt_result.hpp"
#include "azure/keyvault/keys/cryptography/key_wrap_algorithm.hpp"
#include "azure/keyvault/keys/cryptography/sign_result.hpp"
#include "azure/keyvault/keys/cryptography/signature_algorithm.hpp"
#include "azure/keyvault/keys/cryptography/unwrap_result.hpp"
#include "azure/keyvault/keys/cryptography/verify_result.hpp"
#include "azure/keyvault/keys/cryptography/wrap_result.hpp"

#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault {

  namespace _detail {
    class KeyVaultProtocolClient;
  } // namespace _detail

  namespace Keys { namespace Cryptography {

    namespace _detail {
      class CryptographyProvider;
      class RemoteCryptographyClient;
    } // namespace _detail

    /**
     * @brief A client used to perform cryptographic operations with Azure Key Vault keys.
     *
     */
    class CryptographyClient final {
    private:
      std::shared_ptr<Azure::Security::KeyVault::_detail::KeyVaultProtocolClient> m_pipeline;
      std::string m_keyId;
      std::shared_ptr<
          Azure::Security::KeyVault::Keys::Cryptography::_detail::RemoteCryptographyClient>
          m_remoteProvider;
      std::shared_ptr<Azure::Security::KeyVault::Keys::Cryptography::_detail::CryptographyProvider>
          m_provider;

      explicit CryptographyClient(
          std::string const& keyId,
          std::shared_ptr<Core::Credentials::TokenCredential const> credential,
          CryptographyClientOptions const& options,
          bool forceRemote);

      void Initialize(std::string const& operation, Azure::Core::Context const& context);

      /**
       * @brief Gets whether this #CryptographyClient runs only local operations.
       *
       */
      bool LocalOnly() const noexcept { return m_remoteProvider == nullptr; }

    public:
      /**
       * @brief Initializes a new instance of the #CryptographyClient class.
       *
       * @param keyId The key identifier of the #KeyVaultKey which will be used for cryptographic
       * operations.
       * @param credential A #TokenCredential used to authenticate requests to the vault, like
       * DefaultAzureCredential.
       * @param options #CryptographyClientOptions for the #CryptographyClient for local or remote
       * operations on Key Vault.
       */
      explicit CryptographyClient(
          std::string const& keyId,
          std::shared_ptr<Core::Credentials::TokenCredential const> credential,
          CryptographyClientOptions options = CryptographyClientOptions())
          : CryptographyClient(keyId, credential, options, false)
      {
      }

      /**
       * @brief Destructs `%CryptographyClient`.
       *
       */
      ~CryptographyClient();

      /**
       * @brief Encrypts plaintext.
       *
       * @param parameters An #EncryptParameters containing the data to encrypt and other parameters
       * for algorithm-dependent encryption.
       * @param context A #Azure::Core::Context to cancel the operation.
       * @return An #EncryptResult containing the encrypted data along with all other information
       * needed to decrypt it. This information should be stored with the encrypted data.
       */
      EncryptResult Encrypt(
          EncryptParameters const& parameters,
          Azure::Core::Context const& context = Azure::Core::Context());

      /**
       * @brief Decrypts ciphertext.
       *
       * @param parameters A #DecryptParameters containing the data to decrypt and other parameters
       * for algorithm-dependent Decryption.
       * @param context A #Azure::Core::Context to cancel the operation.
       * @return An #DecryptResult containing the decrypted data along with all other information
       * needed to decrypt it. This information should be stored with the Decrypted data.
       */
      DecryptResult Decrypt(
          DecryptParameters const& parameters,
          Azure::Core::Context const& context = Azure::Core::Context());

      /**
       * @brief Encrypts the specified key.
       *
       * @param algorithm The #KeyWrapAlgorithm to use.
       * @param key The key to encrypt.
       * @param context A #Azure::Core::Context to cancel the operation.
       * @return The result of the wrap operation. The returned #WrapResult contains the wrapped key
       * along with all other information needed to unwrap it. This information should be stored
       * with the wrapped key.
       */
      WrapResult WrapKey(
          KeyWrapAlgorithm algorithm,
          std::vector<uint8_t> const& key,
          Azure::Core::Context const& context = Azure::Core::Context());

      /**
       * @brief Decrypts the specified encrypted key.
       *
       * @param algorithm The #KeyWrapAlgorithm to use.
       * @param encryptedKey The encrypted key.
       * @param context A #Azure::Core::Context to cancel the operation.
       * @return The result of the unwrap operation. The returned #UnwrapResult contains the key
       * along with information regarding the algorithm and key used to unwrap it.
       */
      UnwrapResult UnwrapKey(
          KeyWrapAlgorithm algorithm,
          std::vector<uint8_t> const& encryptedKey,
          Azure::Core::Context const& context = Azure::Core::Context());

      /**
       * @brief Signs the specified digest.
       *
       * @param algorithm The #SignatureAlgorithm to use.
       * @param digest The pre-hashed digest to sign. The hash algorithm used to compute the digest
       * must be compatable with the specified algorithm.
       * @param context A #Azure::Core::Context to cancel the operation.
       * @return The result of the sign operation. The returned #SignResult contains the signature
       * along with all other information needed to verify it. This information should be stored
       * with the signature.
       */
      SignResult Sign(
          SignatureAlgorithm algorithm,
          std::vector<uint8_t> const& digest,
          Azure::Core::Context const& context = Azure::Core::Context());

      /**
       * @brief Signs the specified data.
       *
       * @param algorithm The #SignatureAlgorithm to use.
       * @param data The data to sign.
       * @param context A #Azure::Core::Context to cancel the operation.
       * @return The result of the sign operation. The returned #SignResult contains the signature
       * along with all other information needed to verify it. This information should be stored
       * with the signature.
       */
      SignResult SignData(
          SignatureAlgorithm algorithm,
          Azure::Core::IO::BodyStream& data,
          Azure::Core::Context const& context = Azure::Core::Context());

      /**
       * @brief Signs the specified data.
       *
       * @param algorithm The #SignatureAlgorithm to use.
       * @param data The data to sign.
       * @param context A #Azure::Core::Context to cancel the operation.
       * @return The result of the sign operation. The returned #SignResult contains the signature
       * along with all other information needed to verify it. This information should be stored
       * with the signature.
       */
      SignResult SignData(
          SignatureAlgorithm algorithm,
          std::vector<uint8_t> const& data,
          Azure::Core::Context const& context = Azure::Core::Context());

      /**
       * @brief Verifies the specified signature.
       *
       * @param algorithm The #SignatureAlgorithm to use. This must be the same algorithm used to
       * sign the digest.
       * @param digest The pre-hashed digest corresponding to the signature. The hash algorithm used
       * to compute the digest must be compatable with the specified algorithm.
       * @param signature The signature to verify.
       * @param context A #Azure::Core::Context to cancel the operation.
       * @return The result of the verify operation. If the signature is valid the
       * #VerifyResult.IsValid property of the returned #VerifyResult will be set to true.
       */
      VerifyResult Verify(
          SignatureAlgorithm algorithm,
          std::vector<uint8_t> const& digest,
          std::vector<uint8_t> const& signature,
          Azure::Core::Context const& context = Azure::Core::Context());

      /**
       * @brief Verifies the specified signature.
       *
       * @param algorithm The #SignatureAlgorithm to use. This must be the same algorithm used to
       * sign the data.
       * @param data The data corresponding to the signature.
       * @param signature The signature to verify.
       * @param context A #Azure::Core::Context to cancel the operation.
       * @return The result of the verify operation. If the signature is valid the
       * #VerifyResult.IsValid property of the returned #VerifyResult will be set to true.
       */
      VerifyResult VerifyData(
          SignatureAlgorithm algorithm,
          Azure::Core::IO::BodyStream& data,
          std::vector<uint8_t> const& signature,
          Azure::Core::Context const& context = Azure::Core::Context());

      /**
       * @brief Verifies the specified signature.
       *
       * @param algorithm The #SignatureAlgorithm to use. This must be the same algorithm used to
       * sign the data.
       * @param data The data corresponding to the signature.
       * @param signature The signature to verify.
       * @param context A #Azure::Core::Context to cancel the operation.
       * @return The result of the verify operation. If the signature is valid the
       * #VerifyResult.IsValid property of the returned #VerifyResult will be set to true.
       */
      VerifyResult VerifyData(
          SignatureAlgorithm algorithm,
          std::vector<uint8_t> const& data,
          std::vector<uint8_t> const& signature,
          Azure::Core::Context const& context = Azure::Core::Context());
    };

  }} // namespace Keys::Cryptography
}}} // namespace Azure::Security::KeyVault
