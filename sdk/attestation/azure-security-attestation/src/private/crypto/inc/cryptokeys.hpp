//
// Enclave Crypto: Asymmetric ciphers
//
// Copyright (c) Microsoft corporation. All rights reserved.
//

#pragma once
#include "crypto.hpp"
#include <string>

namespace Azure { namespace Security { namespace Attestation { namespace _internal {
  namespace Cryptography {

    class AsymmetricKey {

    public:
      virtual ~AsymmetricKey() {}

      enum class KeyType
      {
        RSA,
        ECDSA
      };

      virtual bool VerifySignature(
          std::vector<uint8_t> const& payload,
          std::vector<uint8_t> const& signature) const = 0;
      virtual std::vector<uint8_t> SignBuffer(std::vector<uint8_t> const& bufferToSign) const = 0;
      virtual std::string ExportPrivateKey() = 0;
      virtual std::string ExportPublicKey() = 0;
    };

}}}}} // namespace Azure::Security::Attestation::_internal::Cryptography
