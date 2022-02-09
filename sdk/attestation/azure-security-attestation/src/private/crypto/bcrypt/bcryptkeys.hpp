// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#if 0 // NOT FUNCTIONAL

/**
 * @brief Attestation client model support classes and functions.
 *
 * This file contains private classes used to support public model types.
 *
 */

#pragma once

#include <Windows.h>
#include <bcrypt.h>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "../inc/crypto.hpp"
#include "bcrypt_helpers.hpp"
#include <wil/resource.h>
// cspell: words PCERT PCRYPT hcryptprov hcryptkey

namespace Azure { namespace Security { namespace Attestation { namespace _internal {
  namespace Cryptography {

    /** Represents an Asymmetric Key.
     *
     * There are several operations that can be performed with an Asymmetric key.
     *
     * If the key is a full key (either created with Crypto::CreateRsaKey or Crypto::CreateEcdsKey
     * or imported with ImportPrivateKey) then the SignBuffer API is available to allow signing an
     * arbitrary buffer. This returns the signature of the buffer.
     *
     * If the key is a public key (created by ImportPublicKey), then the VerifySignature API can be
     * used to verify an signed buffer.
     */
    class BCryptAsymmetricKey : public AsymmetricKey {
      friend class BCryptX509Certificate; // cspell: disable-line

    protected:
      wil::unique_hcryptkey m_key;
      wil::unique_hcryptprov m_cryptProvider;

      BCryptAsymmetricKey() = default;
      BCryptAsymmetricKey(wil::unique_hcryptprov&& provider, wil::unique_hcryptkey&& pkey)
          : m_key(std::move(pkey)), m_cryptProvider(std::move(provider))
      {
      }
      virtual bool VerifySignature(
          std::vector<uint8_t> const& payload,
          std::vector<uint8_t> const& signature) const override;
      virtual std::vector<uint8_t> SignBuffer(
          std::vector<uint8_t> const& bufferToSign) const override;
      virtual std::string ExportPrivateKey() override;
      virtual std::string ExportPublicKey() override;

    public:
      static std::unique_ptr<AsymmetricKey> ImportPublicKey(std::string const& pemEncodedKey);
      static std::unique_ptr<AsymmetricKey> ImportPrivateKey(std::string const& pemEncodedKey);
    };

    class RsaBCryptAsymmetricKey : public BCryptAsymmetricKey {
    public:
      RsaBCryptAsymmetricKey(size_t keySize);
      RsaBCryptAsymmetricKey(wil::unique_hcryptprov&& provider, wil::unique_hcryptkey&& key)
          : BCryptAsymmetricKey(std::move(provider), std::move(key))
      {
      }
    };

    class EcdsaBCryptAsymmetricKey : public BCryptAsymmetricKey {
    public:
      EcdsaBCryptAsymmetricKey();
      EcdsaBCryptAsymmetricKey(wil::unique_hcryptprov&& provider, wil::unique_hcryptkey&& key)
          : BCryptAsymmetricKey(std::move(provider), std::move(key))
      {
      }
    };

}}}}} // namespace Azure::Security::Attestation::_private::Cryptography
#endif // NOT FUNCTIONAL
