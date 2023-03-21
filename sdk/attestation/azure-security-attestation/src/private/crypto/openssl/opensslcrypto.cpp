// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Attestation client model support classes and functions.
 *
 * This file contains private classes used to support public model types.
 *
 */

#include "../inc/crypto.hpp"
#include "opensslcert.hpp"
#include "opensslkeys.hpp"
#include <azure/core/platform.hpp>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// cspell::words OpenSSL X509 OpenSSLX509

namespace Azure { namespace Security { namespace Attestation { namespace _detail {

  std::unique_ptr<Cryptography::AsymmetricKey> Cryptography::CreateRsaKey(size_t keySize)
  {
    return std::make_unique<RsaOpenSSLAsymmetricKey>(keySize);
  }

  std::unique_ptr<Cryptography::AsymmetricKey> Cryptography::CreateEcdsaKey()
  {
    return std::make_unique<EcdsaOpenSSLAsymmetricKey>();
  }

  std::unique_ptr<Cryptography::AsymmetricKey> Cryptography::ImportPublicKey(
      std::string const& pemEncodedKey)
  {
    return OpenSSLAsymmetricKey::ImportPublicKey(pemEncodedKey);
  }

  std::unique_ptr<Cryptography::AsymmetricKey> Cryptography::ImportPrivateKey(
      std::string const& pemEncodedKey)
  {
    return OpenSSLAsymmetricKey::ImportPrivateKey(pemEncodedKey);
  }

  std::unique_ptr<Cryptography::X509Certificate> Cryptography::ImportX509Certificate(
      std::string const& pemEncodedCertificate)
  {
    return OpenSSLX509Certificate::Import(pemEncodedCertificate);
  }

  std::unique_ptr<Cryptography::X509Certificate> Cryptography::CreateX509CertificateForPrivateKey(
      std::unique_ptr<Cryptography::AsymmetricKey> const& privateKey,
      std::string const& subjectName)
  {
    return OpenSSLX509Certificate::CreateFromPrivateKey(privateKey, subjectName);
  }
}}}} // namespace Azure::Security::Attestation::_detail
