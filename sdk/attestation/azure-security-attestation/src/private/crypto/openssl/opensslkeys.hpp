// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Attestation client model support classes and functions.
 *
 * This file contains private classes used to support public model types.
 *
 */

#pragma once

#include <azure/core/internal/json/json.hpp>

#include <azure/core/base64.hpp>
#include <memory>
#include <string>
#include <vector>
#include <type_traits>
#include <utility>

#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include "..\inc\crypto.hpp"
#include "openssl_helpers.hpp"

namespace Azure { namespace Security { namespace Attestation { namespace _private {
  namespace Cryptography {

    class OpenSSLAsymmetricKey : public AsymmetricKey {
    protected:
      _details::openssl_evp_pkey m_pkey;

      OpenSSLAsymmetricKey() = default;
      OpenSSLAsymmetricKey(_details::openssl_evp_pkey&& pkey) : m_pkey(std::move(pkey)) {}
      virtual bool VerifySignature() const override = 0;
      virtual std::vector<uint8_t> SignBuffer(std::vector<uint8_t> const& bufferToSign) const override = 0;
      virtual std::string ExportPrivateKey() override;
      virtual std::string ExportPublicKey() override;

    public:
      static std::unique_ptr<AsymmetricKey> ImportPublicKey(std::string const& pemEncodedKey);
      static std::unique_ptr<AsymmetricKey> ImportPrivateKey(std::string const& pemEncodedKey);
    };

    class RsaOpenSSLAsymmetricKey: public OpenSSLAsymmetricKey {
    public:
      RsaOpenSSLAsymmetricKey(size_t keySize);
      RsaOpenSSLAsymmetricKey(_details::openssl_evp_pkey&& key)
          : OpenSSLAsymmetricKey(std::move(key))
      {
      }
      virtual bool VerifySignature() const override;
      virtual std::vector<uint8_t> SignBuffer(std::vector<uint8_t> const& bufferToSign) const override;
    };


}}}}} // namespace Azure::Security::Attestation::_private::Cryptography