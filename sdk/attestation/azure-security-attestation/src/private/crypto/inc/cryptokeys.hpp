//
// Enclave Crypto: Asymmetric ciphers
//
// Copyright (c) Microsoft corporation. All rights reserved.
//

#pragma once
#include <string>
//#include <azure/core/cryptography/hash.hpp>
//#include <azure/core/internal/cryptography/sha_hash.hpp>
#include <openssl/evp.h>
#include "crypto.hpp"

namespace Azure {
  namespace Security {
    namespace Attestation {
      namespace _private {
        namespace Cryptography {

          class AsymmetricKey {

          public:
            virtual ~AsymmetricKey() {}

            enum class KeyType
            {
              RSA,
              ECDSA
            };

            virtual bool VerifySignature() const = 0;
            virtual std::vector<uint8_t> SignBuffer(std::vector<uint8_t> const&bufferToSign) const = 0;
            virtual std::string ExportPrivateKey() = 0;
            virtual std::string ExportPublicKey() = 0;
          };

          /** @brief The Crypto class contains basic functionality to 
          */
          class Crypto {
          public:
            static std::unique_ptr<AsymmetricKey> CreateRsaKey(size_t keySizeInBytes);
            static std::unique_ptr<AsymmetricKey> ImportPublicKey(std::string const& pemEncodedString);
            static std::unique_ptr<AsymmetricKey> ImportPrivateKey(
                std::string const& pemEncodedString);

          };

}}}}} // namespace Azure::Security::Attestation::_private::Crypto
