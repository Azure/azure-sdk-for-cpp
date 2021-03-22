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
using namespace Azure::Security::KeyVault::Keys;

TEST_F(KeyVaultClientTest, BackupKey)
{
  KeyClient keyClient(m_keyVaultUrl, m_credential);
  std::string keyName = GetUniqueName();

  {
    auto keyResponse = keyClient.CreateKey(keyName, JsonWebKeyType::Ec);
    CheckValidResponse(keyResponse);
  }
  {
    // backup
    auto backUpResponse = keyClient.BackupKey(keyName);
    CheckValidResponse(backUpResponse);
  }
}
