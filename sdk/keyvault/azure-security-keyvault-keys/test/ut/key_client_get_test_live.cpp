// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "gtest/gtest.h"

#include "key_client_base_test.hpp"

#include <azure/keyvault/key_vault.hpp>
#include <azure/keyvault/keys/details/key_constants.hpp>

#include <string>

using namespace Azure::Security::KeyVault::Keys::Test;

TEST_F(KeyVaultClientTest, GetKey)
{
  Azure::Security::KeyVault::Keys::KeyClient keyClient(m_keyVaultUrl, m_credential);
  // Assuming and RS Key exists in the KeyVault Account.
  std::string keyName("testKey");

  auto keyResponse = keyClient.GetKey(keyName);
  CheckValidResponse(keyResponse);
  auto key = keyResponse.ExtractValue();

  EXPECT_EQ(key.Name(), keyName);
  EXPECT_EQ(key.GetKeyType(), Azure::Security::KeyVault::Keys::JsonWebKeyType::Rsa);
}

#include <iostream>
TEST_F(KeyVaultClientTest, GetPropertiesOfKeysOnePage)
{
  Azure::Security::KeyVault::Keys::KeyClient keyClient(m_keyVaultUrl, m_credential);

  auto keyResponse = keyClient.GetPropertiesOfKeysSinglePage();
  CheckValidResponse(keyResponse);
  for (auto const& keyProperties : keyResponse->Items)
  {
    auto keyVersions = keyClient.GetPropertiesOfKeyVersions(keyProperties.Name);
    CheckValidResponse(keyVersions);
    std::cout << std::endl
              << keyProperties.Name << " - " << keyProperties.CreatedOn.GetValue().ToString();
    std::cout << std::endl << "Versions:";
    for (auto const& keyVersion : keyVersions->Items)
    {
      std::cout << std::endl << "\t-" << keyVersion.Version;
    }
  }
}
