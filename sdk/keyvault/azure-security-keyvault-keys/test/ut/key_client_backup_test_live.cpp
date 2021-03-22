// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "gtest/gtest.h"

#include "key_client_base_test.hpp"

#include <azure/core/base64.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/keyvault/key_vault.hpp>
#include <azure/keyvault/keys/details/key_constants.hpp>

#include <string>

using namespace Azure::Security::KeyVault::Keys::Test;
using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Core::Json::_internal;

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
    auto& rawResponse = backUpResponse.GetRawResponse();
    auto& payload = rawResponse.GetBody();
    auto jsonParser = json::parse(payload);
    auto originalBackUpValue = jsonParser["value"].get<std::string>();
    // auto& decodedValue = *backUpResponse;
    auto decodedValue = Azure::Core::Convert::Base64Decode(originalBackUpValue);
    auto encodedValue = Azure::Core::Convert::Base64Encode(decodedValue);
    EXPECT_EQ(originalBackUpValue, encodedValue);
  }
}
