// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "gtest/gtest.h"

#include "key_client_base_test.hpp"

#include <azure/keyvault/key_vault.hpp>
#include <azure/keyvault/keys/key_constants.hpp>

#include <string>

namespace {
template <class T>
void CheckValidResponse(
    Azure::Core::Response<T>& response,
    Azure::Core::Http::HttpStatusCode expectedCode = Azure::Core::Http::HttpStatusCode::Ok)
{
  auto rawResponse = response.ExtractRawResponse();
  EXPECT_EQ(rawResponse->GetStatusCode(), expectedCode);
}
} // namespace

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
  EXPECT_EQ(key.GetKeyType(), Azure::Security::KeyVault::Keys::KeyTypeEnum::Rsa);
}

TEST_F(KeyVaultClientTest, CreateKey)
{
  Azure::Security::KeyVault::Keys::KeyClient keyClient(m_keyVaultUrl, m_credential);
  std::string keyName("createKey");

  {
    auto keyResponse
        = keyClient.CreateKey(keyName, Azure::Security::KeyVault::Keys::KeyTypeEnum::Ec);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.ExtractValue();
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
  {
    // Now get the key
    auto keyResponse = keyClient.GetKey(keyName);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.ExtractValue();
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
}

TEST_F(KeyVaultClientTest, CreateKeyWithOptions)
{
  Azure::Security::KeyVault::Keys::KeyClient keyClient(m_keyVaultUrl, m_credential);
  std::string keyName("createKeyWithOptions");

  Azure::Security::KeyVault::Keys::CreateKeyOptions options;
  options.KeyOperations.push_back(Azure::Security::KeyVault::Keys::KeyOperation::Sign());
  options.KeyOperations.push_back(Azure::Security::KeyVault::Keys::KeyOperation::Verify());
  {
    auto keyResponse
        = keyClient.CreateKey(keyName, Azure::Security::KeyVault::Keys::KeyTypeEnum::Ec, options);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.ExtractValue();

    EXPECT_EQ(keyVaultKey.Name(), keyName);
    EXPECT_EQ(keyVaultKey.GetKeyType(), Azure::Security::KeyVault::Keys::KeyTypeEnum::Ec);
    auto& keyOperations = keyVaultKey.KeyOperations();
    uint16_t expectedSize = 2;
    EXPECT_EQ(keyOperations.size(), expectedSize);

    auto findOperation = [keyOperations](Azure::Security::KeyVault::Keys::KeyOperation op) {
      for (Azure::Security::KeyVault::Keys::KeyOperation operation : keyOperations)
      {
        if (operation.ToString() == op.ToString())
        {
          return true;
        }
      }
      return false;
    };
    EXPECT_PRED1(findOperation, Azure::Security::KeyVault::Keys::KeyOperation::Sign());
    EXPECT_PRED1(findOperation, Azure::Security::KeyVault::Keys::KeyOperation::Verify());
  }
}

TEST_F(KeyVaultClientTest, CreateKeyWithTags)
{
  Azure::Security::KeyVault::Keys::KeyClient keyClient(m_keyVaultUrl, m_credential);
  std::string keyName("myKeyWithOptionsTags");

  Azure::Security::KeyVault::Keys::CreateKeyOptions options;
  options.Tags.emplace("one", "value=1");
  options.Tags.emplace("two", "value=2");

  {
    auto keyResponse
        = keyClient.CreateKey(keyName, Azure::Security::KeyVault::Keys::KeyTypeEnum::Rsa, options);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.ExtractValue();

    EXPECT_EQ(keyVaultKey.Name(), keyName);
    EXPECT_EQ(keyVaultKey.GetKeyType(), Azure::Security::KeyVault::Keys::KeyTypeEnum::Rsa);

    auto findTag = [keyVaultKey](std::string key, std::string value) {
      // Will throw if key is not found
      auto v = keyVaultKey.Properties.Tags.at(key);
      return value == v;
    };
  }
}

// Test Key Delete.
// The test works for either soft-delete or not, but for non soft-delete, the LRO is completed as
// soon as the Operation returns.
TEST_F(KeyVaultClientTest, DeleteKey)
{
  Azure::Security::KeyVault::Keys::KeyClient keyClient(m_keyVaultUrl, m_credential);
  std::string keyName("deleteThisKey");

  {
    auto keyResponse
        = keyClient.CreateKey(keyName, Azure::Security::KeyVault::Keys::KeyTypeEnum::Ec);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.ExtractValue();
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
  {
    // Setting a timeout context to avoid this test to run up to ctest default timeout
    // The polling operation would usually complete in ~20 seconds.
    // Setting 3 min as timeout just because I like number 3. We just want to prevent test running
    // for so long if something happens and no exception is thrown (paranoid scenario)
    auto duration = std::chrono::system_clock::now() + std::chrono::minutes(3);
    auto cancelToken = Azure::Core::GetApplicationContext().WithDeadline(duration);

    auto keyResponseLRO = keyClient.StartDeleteKey(keyName);
    auto expectedStatusToken = m_keyVaultUrl + "/"
        + std::string(Azure::Security::KeyVault::Keys::Details::DeletedKeysPath) + "/" + keyName;
    EXPECT_EQ(keyResponseLRO.GetResumeToken(), expectedStatusToken);
    // poll each second until key is soft-deleted
    // Will throw and fail test if test takes more than 3 minutes (token cancelled)
    auto keyResponse = keyResponseLRO.PollUntilDone(cancelToken, std::chrono::milliseconds(1000));
  }
}
