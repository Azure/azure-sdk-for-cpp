// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/internal/cryptography/rsa_cryptography_provider.hpp"

#include <memory>
#include <string>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {
  namespace _detail {

    EncryptResult RsaCryptographyProvider::Encrypt(
        EncryptParameters const& parameters,
        Azure::Core::Context const&) const
    {
      EncryptResult result;
      result.Algorithm = parameters.Algorithm;
      return result;
    }

    DecryptResult RsaCryptographyProvider::Decrypt(
        DecryptParameters const& parameters,
        Azure::Core::Context const&) const
    {
      DecryptResult result;
      result.Algorithm = parameters.Algorithm;
      return result;
    }

    WrapResult RsaCryptographyProvider::WrapKey(
        KeyWrapAlgorithm const& algorithm,
        std::vector<uint8_t> const& key,
        Azure::Core::Context const&) const
    {
      WrapResult result;
      (void)key;
      result.Algorithm = algorithm;
      return result;
    }

    UnwrapResult RsaCryptographyProvider::UnwrapKey(
        KeyWrapAlgorithm const& algorithm,
        std::vector<uint8_t> const& key,
        Azure::Core::Context const&) const
    {
      UnwrapResult result;
      (void)key;
      result.Algorithm = algorithm;
      return result;
    }

    SignResult RsaCryptographyProvider::Sign(
        SignatureAlgorithm const& algorithm,
        std::vector<uint8_t> const& digest,
        Azure::Core::Context const&) const
    {
      SignResult result;
      (void)digest;
      result.Algorithm = algorithm;
      return result;
    }

    VerifyResult RsaCryptographyProvider::Verify(
        SignatureAlgorithm const& algorithm,
        std::vector<uint8_t> const& digest,
        std::vector<uint8_t> const& signature,
        Azure::Core::Context const&) const
    {
      VerifyResult result;
      (void)digest;
      (void)signature;
      result.Algorithm = algorithm;
      return result;
    }

}}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography::_detail
