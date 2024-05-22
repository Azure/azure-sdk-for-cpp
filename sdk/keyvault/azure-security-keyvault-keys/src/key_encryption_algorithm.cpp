// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/keyvault/keys/key_client_models.hpp"
#include "private/key_constants.hpp"

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {
// Disable deprecation warning
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
      const KeyEncryptionAlgorithm KeyEncryptionAlgorithm::CKM_RSA_AES_KEY_WRAP(
          _detail::CKM_RSA_AES_KEY_WRAP_Value);

      const KeyEncryptionAlgorithm KeyEncryptionAlgorithm::RSA_AES_KEY_WRAP_256(
          _detail::RSA_AES_KEY_WRAP_256_Value);

      const KeyEncryptionAlgorithm KeyEncryptionAlgorithm::RSA_AES_KEY_WRAP_384(
          _detail::RSA_AES_KEY_WRAP_384_Value);
#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif // _MSC_VER
      const KeyEncryptionAlgorithm KeyEncryptionAlgorithm::CkmRsaAesKeyWrap(
          _detail::CKM_RSA_AES_KEY_WRAP_Value);

      const KeyEncryptionAlgorithm KeyEncryptionAlgorithm::RsaAesKeyWrap256(
          _detail::RSA_AES_KEY_WRAP_256_Value);

      const KeyEncryptionAlgorithm KeyEncryptionAlgorithm::RsaAesKeyWrap384(
          _detail::RSA_AES_KEY_WRAP_384_Value);
}}}} // namespace Azure::Security::KeyVault::Keys
