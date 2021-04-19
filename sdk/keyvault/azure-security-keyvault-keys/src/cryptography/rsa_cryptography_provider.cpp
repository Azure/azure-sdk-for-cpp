// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/cryptography/rsa_cryptography_provider.hpp"

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
        Azure::Core::Context const& context) const
    {
      (void)parameters;
      (void)context;
      return EncryptResult();
    }
}}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography::_detail
