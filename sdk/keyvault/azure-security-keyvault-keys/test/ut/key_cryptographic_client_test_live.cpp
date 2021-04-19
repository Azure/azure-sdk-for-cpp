// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "gtest/gtest.h"

#include "key_client_base_test.hpp"

#include <azure/keyvault/key_vault_keys.hpp>

#include <string>
#include <vector>

using namespace Azure::Security::KeyVault::Keys::Test;
using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Security::KeyVault::Keys::Cryptography;

TEST_F(KeyVaultClientTest, RemoteEncrypt)
{
  KeyClient keyClient(m_keyVaultUrl, m_credential);
  std::string keyName(GetUniqueName());

  CreateRsaKeyOptions rsaKeyOptions(keyName);
  rsaKeyOptions.KeySize = 2048;
  auto rsaKey = keyClient.CreateRsaKey(rsaKeyOptions).Value;

  // init crypto client from key id. The remote client will get the key and try to create a local
  // crypto client.
  CryptographyClient cryptoClient(rsaKey.Id(), m_credential);

  {
    uint8_t plaintextSource[] = "A single block of plaintext";
    std::vector<uint8_t> plaintext(std::begin(plaintextSource), std::end(plaintextSource));

    auto encryptResult = cryptoClient.Encrypt(EncryptionAlgorithm::RsaOaep, plaintext);
    EXPECT_EQ(encryptResult.Algorithm.ToString(), EncryptionAlgorithm::RsaOaep.ToString());
    EXPECT_EQ(encryptResult.KeyId, rsaKey.Id());
    EXPECT_TRUE(encryptResult.Ciphertext.size() > 0);
  }
}
