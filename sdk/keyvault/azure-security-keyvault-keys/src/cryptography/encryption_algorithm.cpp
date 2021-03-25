// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief An algorithm used for encryption and decryption.
 */

#pragma once

#include "azure/keyvault/keys/cryptography/encryption_algorithm.hpp"
#include "azure/keyvault/keys/details/key_constants.hpp"

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {

  EncryptionAlgorithm EncryptionAlgorithm::Rsa15()
  {
    return EncryptionAlgorithm(_detail::Rsa15Value);
  }

  EncryptionAlgorithm EncryptionAlgorithm::RsaOaep()
  {
    return EncryptionAlgorithm(_detail::RsaOaepValue);
  }

  EncryptionAlgorithm EncryptionAlgorithm::RsaOaep256()
  {
    return EncryptionAlgorithm(_detail::RsaOaep256Value);
  }

  EncryptionAlgorithm EncryptionAlgorithm::A128Gcm()
  {
    return EncryptionAlgorithm(_detail::A128GcmValue);
  }

  EncryptionAlgorithm EncryptionAlgorithm::A192Gcm()
  {
    return EncryptionAlgorithm(_detail::A192GcmValue);
  }

  EncryptionAlgorithm EncryptionAlgorithm::A256Gcm()
  {
    return EncryptionAlgorithm(_detail::A256GcmValue);
  }

  EncryptionAlgorithm EncryptionAlgorithm::A128Cbc()
  {
    return EncryptionAlgorithm(_detail::A128CbcValue);
  }

  EncryptionAlgorithm EncryptionAlgorithm::A192Cbc()
  {
    return EncryptionAlgorithm(_detail::A192CbcValue);
  }

  EncryptionAlgorithm EncryptionAlgorithm::A256Cbc()
  {
    return EncryptionAlgorithm(_detail::A256CbcValue);
  }

  EncryptionAlgorithm EncryptionAlgorithm::A128CbcPad()
  {
    return EncryptionAlgorithm(_detail::A128CbcPadValue);
  }

  EncryptionAlgorithm EncryptionAlgorithm::A192CbcPad()
  {
    return EncryptionAlgorithm(_detail::A192CbcPadValue);
  }

  EncryptionAlgorithm EncryptionAlgorithm::A256CbcPad()
  {
    return EncryptionAlgorithm(_detail::A256CbcPadValue);
  }

}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography
