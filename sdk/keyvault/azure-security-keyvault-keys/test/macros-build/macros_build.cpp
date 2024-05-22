// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @brief Tests the ability to build when certain macros are defined outside the keyvault
 * code.
 *
 */

#include "macros_build.hpp"

#include <azure/keyvault/keys.hpp>

#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <thread>

using namespace Azure::Security::KeyVault::Keys;

int main()
{
  std::cout << "CKM_RSA_AES_KEY_WRAP : " << CKM_RSA_AES_KEY_WRAP << std::endl;
  std::cout << "RSA_AES_KEY_WRAP_256 : " << RSA_AES_KEY_WRAP_256 << std::endl;
  std::cout << "RSA_AES_KEY_WRAP_384 : " << RSA_AES_KEY_WRAP_384 << std::endl;
  std::cout << "KeyEncryptionAlgorithm::CkmRsaAesKeyWrap : "
            << KeyEncryptionAlgorithm::CkmRsaAesKeyWrap.ToString() << std::endl;
  std::cout << "KeyEncryptionAlgorithm::RsaAesKeyWrap256 : "
            << KeyEncryptionAlgorithm::RsaAesKeyWrap256.ToString() << std::endl;
  std::cout << "KeyEncryptionAlgorithm::RsaAesKeyWrap384 : "
            << KeyEncryptionAlgorithm::RsaAesKeyWrap384.ToString() << std::endl;
}
