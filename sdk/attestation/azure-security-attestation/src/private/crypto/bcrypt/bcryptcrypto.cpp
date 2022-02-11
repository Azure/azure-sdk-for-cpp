// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX-License-Identifier: MIT
#if 0 // NOT FUNCTIONING

/**
 * @brief Attestation client model support classes and functions.
 *
 * This file contains private classes used to support public model types.
 *
 */

#include <azure/core/base64.hpp"
#include <azure/core/internal/json/json.hpp"
#include <azure/core/platform.hpp"
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "../inc/crypto.hpp"
#include "bcryptcert.hpp"
#include "bcryptkeys.hpp"

// cspell::words OpenSSL X509 OpenSSLX509

#if defined(AZ_PLATFORM_WINDOWS)
namespace Azure { namespace Security { namespace Attestation { namespace _internal {
  namespace Cryptography {

    std::unique_ptr<AsymmetricKey> Crypto::CreateRsaKey(size_t keySize)
    {
      return std::make_unique<RsaBCryptAsymmetricKey>(keySize);
    }

    std::unique_ptr<AsymmetricKey> Crypto::CreateEcdsaKey()
    {
      return std::make_unique<EcdsaBCryptAsymmetricKey>();
    }

    std::unique_ptr<AsymmetricKey> Crypto::ImportPublicKey(std::string const& pemEncodedKey)
    {
      return BCryptAsymmetricKey::ImportPublicKey(pemEncodedKey);
    }

    std::unique_ptr<AsymmetricKey> Crypto::ImportPrivateKey(std::string const& pemEncodedKey)
    {
      return BCryptAsymmetricKey::ImportPrivateKey(pemEncodedKey);
    }

    std::unique_ptr<X509Certificate> Crypto::ImportX509Certificate(
        std::string const& pemEncodedCertificate)
    {
      return BCryptX509Certificate::Import(pemEncodedCertificate);
    }

    std::unique_ptr<X509Certificate> Crypto::CreateX509CertificateForPrivateKey(
        std::unique_ptr<AsymmetricKey> const& privateKey,
        std::string const& subjectName)
    {
      return BCryptX509Certificate::CreateFromPrivateKey(privateKey, subjectName);
    }
}}}}} // namespace Azure::Security::Attestation::_internal::Cryptography
#endif // defined(AZ_PLATFORM_POSIX)
#endif // NOT FUNCTIONING
