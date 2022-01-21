// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once
#include <memory>
#include <list>
#include <ctime>
#include <azure/core/internal/json/json.hpp>

#include "cryptokeys.hpp"
#include "cryptocert.hpp"

          /** @brief The Crypto class contains basic functionality to
 */
namespace Azure { namespace Security { namespace Attestation { namespace _private {
  namespace Cryptography {
class Crypto {
public:
  static std::unique_ptr<AsymmetricKey> CreateRsaKey(size_t keySizeInBytes);
  static std::unique_ptr<AsymmetricKey> CreateEcdsaKey();
  static std::unique_ptr<AsymmetricKey> ImportPublicKey(std::string const& pemEncodedString);
  static std::unique_ptr<AsymmetricKey> ImportPrivateKey(std::string const& pemEncodedString);

  static std::unique_ptr<X509Certificate> CreateX509CertificateForPrivateKey(
      std::unique_ptr<AsymmetricKey> const& key,
      std::string const& certificateSubject);
  static std::unique_ptr<X509Certificate> ImportX509Certificate(
      std::string const& pemEncodedCertificate);
};
}}}}} // namespace Azure::Security::Attestation::_private::Cryptography
