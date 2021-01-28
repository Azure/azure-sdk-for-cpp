// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "gtest/gtest.h"

#include "key_client_base_test.hpp"

#include <azure/keyvault/key_vault.hpp>

#include <string>

using namespace Azure::Security::KeyVault::Keys::Test;

TEST_F(KeyVaultClientTest, GetKey)
{
  Azure::Security::KeyVault::Keys::KeyClient keyClient(m_keyVaultUrl, m_credential);
  // Assuming and RS Key exists in the KeyVault Account.
  std::string keyName("testKey");

  auto r = keyClient.GetKey(keyName);
  auto key = r.ExtractValue();

  EXPECT_EQ(key.Name(), keyName);
  EXPECT_EQ(key.GetKeyType(), Azure::Security::KeyVault::Keys::KeyTypeEnum::Rsa);
}
