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

    auto decryptResult = cryptoClient.Decrypt(encryptResult.Algorithm, encryptResult.Ciphertext);
    EXPECT_EQ(decryptResult.Algorithm.ToString(), encryptResult.Algorithm.ToString());
    EXPECT_EQ(decryptResult.Plaintext, plaintext);
    EXPECT_EQ(decryptResult.KeyId, encryptResult.KeyId);
  }
}

TEST_F(KeyVaultClientTest, RemoteWrap)
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

    auto wrapResult = cryptoClient.WrapKey(KeyWrapAlgorithm::RsaOaep256, plaintext);
    EXPECT_EQ(wrapResult.Algorithm.ToString(), KeyWrapAlgorithm::RsaOaep256.ToString());
    EXPECT_EQ(wrapResult.KeyId, rsaKey.Id());
    EXPECT_TRUE(wrapResult.EncryptedKey.size() > 0);

    auto unwrapResult = cryptoClient.UnwrapKey(wrapResult.Algorithm, wrapResult.EncryptedKey);
    EXPECT_EQ(unwrapResult.Algorithm.ToString(), wrapResult.Algorithm.ToString());
    EXPECT_EQ(unwrapResult.Key, plaintext);
    EXPECT_EQ(unwrapResult.KeyId, wrapResult.KeyId);
  }
}
