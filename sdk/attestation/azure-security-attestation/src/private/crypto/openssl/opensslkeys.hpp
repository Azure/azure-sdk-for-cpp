// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Attestation client model support classes and functions.
 *
 * This file contains private classes used to support public model types.
 *
 */

#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "../inc/crypto.hpp"
#include "openssl_helpers.hpp"
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

namespace Azure { namespace Security { namespace Attestation { namespace _detail {

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
  class OpenSSLAsymmetricKey : public Cryptography::AsymmetricKey {
    friend class OpenSSLX509Certificate; // cspell: disable-line

  protected:
    openssl_evp_pkey m_pkey;

    OpenSSLAsymmetricKey() = default;
    OpenSSLAsymmetricKey(openssl_evp_pkey&& pkey) : m_pkey(std::move(pkey)) {}
    virtual bool VerifySignature(
        std::vector<uint8_t> const& payload,
        std::vector<uint8_t> const& signature) const override;
    virtual std::vector<uint8_t> SignBuffer(
        std::vector<uint8_t> const& bufferToSign) const override;
    virtual std::string ExportPrivateKey() override;
    virtual std::string ExportPublicKey() override;

    openssl_evp_pkey const& GetKey() { return m_pkey; }

  public:
    static std::unique_ptr<AsymmetricKey> ImportPublicKey(std::string const& pemEncodedKey);
    static std::unique_ptr<AsymmetricKey> ImportPrivateKey(std::string const& pemEncodedKey);
  };

  class RsaOpenSSLAsymmetricKey : public OpenSSLAsymmetricKey {
  public:
    RsaOpenSSLAsymmetricKey(size_t keySize);
    RsaOpenSSLAsymmetricKey(openssl_evp_pkey&& key) : OpenSSLAsymmetricKey(std::move(key))
    {
    }
  };

  class EcdsaOpenSSLAsymmetricKey : public OpenSSLAsymmetricKey {
  public:
    EcdsaOpenSSLAsymmetricKey();
    EcdsaOpenSSLAsymmetricKey(openssl_evp_pkey&& key)
        : OpenSSLAsymmetricKey(std::move(key))
    {
    }
  };

}}}} // namespace Azure::Security::Attestation::_detail