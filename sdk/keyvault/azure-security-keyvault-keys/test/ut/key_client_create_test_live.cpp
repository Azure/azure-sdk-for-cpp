// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "key_client_base_test.hpp"

#include "private/key_constants.hpp"
#include "private/key_serializers.hpp"
#include "test_consts.hpp"
#include "gtest/gtest.h"
#include <azure/attestation.hpp>
#include <azure/attestation/attestation_client_options.hpp>
#include <azure/core/base64.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/keyvault/keys.hpp>
#include <private/key_constants.hpp>
#include <string>

using namespace Azure::Core::_internal;
using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Security::KeyVault::Keys::Test;
using namespace Azure::Security::Attestation;
using namespace Azure::Core::Http;
using namespace Azure::Core::Json::_internal;
using namespace Azure::Security::KeyVault::Keys::Cryptography;

TEST_F(KeyVaultKeyClient, CreateKey)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  {
    auto keyResponse
        = client.CreateKey(keyName, Azure::Security::KeyVault::Keys::KeyVaultKeyType::Ec);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
  {
    // Now get the key
    auto keyResponse = client.GetKey(keyName);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
}

TEST_F(KeyVaultKeyClient, CreateKeyWithOptions)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  Azure::Security::KeyVault::Keys::CreateKeyOptions options;
  options.KeyOperations.push_back(Azure::Security::KeyVault::Keys::KeyOperation::Sign);
  options.KeyOperations.push_back(Azure::Security::KeyVault::Keys::KeyOperation::Verify);

  {
    auto keyResponse
        = client.CreateKey(keyName, Azure::Security::KeyVault::Keys::KeyVaultKeyType::Ec, options);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;

    EXPECT_EQ(keyVaultKey.Name(), keyName);
    EXPECT_EQ(
        keyVaultKey.GetKeyType().ToString(),
        Azure::Security::KeyVault::Keys::KeyVaultKeyType::Ec.ToString());
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
    EXPECT_PRED1(findOperation, Azure::Security::KeyVault::Keys::KeyOperation::Sign);
    EXPECT_PRED1(findOperation, Azure::Security::KeyVault::Keys::KeyOperation::Verify);
  }
}

TEST_F(KeyVaultKeyClient, CreateKeyWithTags)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  Azure::Security::KeyVault::Keys::CreateKeyOptions options;
  options.Tags.emplace("one", "value=1");
  options.Tags.emplace("two", "value=2");

  {
    auto keyResponse
        = client.CreateKey(keyName, Azure::Security::KeyVault::Keys::KeyVaultKeyType::Rsa, options);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;

    EXPECT_EQ(keyVaultKey.Name(), keyName);
    EXPECT_EQ(keyVaultKey.GetKeyType(), Azure::Security::KeyVault::Keys::KeyVaultKeyType::Rsa);

    auto findTag = [keyVaultKey](std::string key, std::string value) {
      // Will throw if key is not found
      auto v = keyVaultKey.Properties.Tags.at(key);
      return value == v;
    };
  }
}

/********************************* Create key overloads  *********************************/
TEST_F(KeyVaultKeyClient, CreateEcKey)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  {
    auto ecKey = Azure::Security::KeyVault::Keys::CreateEcKeyOptions(keyName);
    auto keyResponse = client.CreateEcKey(ecKey);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
  {
    // Now get the key
    auto keyResponse = client.GetKey(keyName);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
}


/********************************* Create key overloads  *********************************/
TEST_F(KeyVaultKeyClient, CreateOkpKey)
{
  auto const keyName = GetTestName();
  // This client requires an HSM client
  CreateHsmClient();
  auto const& client = GetClientForTest(keyName);

  {
    auto okpKey = Azure::Security::KeyVault::Keys::CreateOkpKeyOptions(keyName);
    auto keyResponse = client.CreateOkpKey(okpKey);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
  {
    // Now get the key
    auto keyResponse = client.GetKey(keyName);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
}

TEST_F(KeyVaultKeyClient, CreateOkpHSMKey)
{
  auto const keyName = GetTestName();
  // This client requires an HSM client
  CreateHsmClient();
  auto const& client = GetClientForTest(keyName);

  {
    auto okpKey = Azure::Security::KeyVault::Keys::CreateOkpKeyOptions(keyName,true);
    auto keyResponse = client.CreateOkpKey(okpKey);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
    EXPECT_EQ(keyVaultKey.GetKeyType(), KeyVaultKeyType::OkpHsm);
    EXPECT_EQ(keyVaultKey.Key.CurveName.Value(), KeyCurveName::Ed25519);
  }
  {
    // Now get the key
    auto keyResponse = client.GetKey(keyName);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
}
TEST_F(KeyVaultKeyClient, CreateEcKeyWithCurve)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  {
    auto ecKey = Azure::Security::KeyVault::Keys::CreateEcKeyOptions(keyName);
    ecKey.CurveName = Azure::Security::KeyVault::Keys::KeyCurveName::P384;
    auto keyResponse = client.CreateEcKey(ecKey);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
    EXPECT_EQ(ecKey.CurveName->ToString(), keyVaultKey.Key.CurveName->ToString());
  }
  {
    // Now get the key
    auto keyResponse = client.GetKey(keyName);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
    EXPECT_TRUE(keyVaultKey.Key.CurveName.HasValue());
    EXPECT_EQ(
        keyVaultKey.Key.CurveName.Value().ToString(),
        Azure::Security::KeyVault::Keys::KeyCurveName::P384.ToString());
  }
}

TEST_F(KeyVaultKeyClient, CreateRsaKey)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  {
    auto rsaKey = Azure::Security::KeyVault::Keys::CreateRsaKeyOptions(keyName, false);
    auto keyResponse = client.CreateRsaKey(rsaKey);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
  {
    // Now get the key
    auto keyResponse = client.GetKey(keyName);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
}

// No tests for octKey since the server does not support it.
// FOR THIS TEST TO WORK MAKE SURE YOU ACTUALLY HAVE A VALID HSM VALUE FOR AZURE_KEYVAULT_HSM_URL
TEST_F(KeyVaultKeyClient, CreateEcHsmKey)
{
  auto const keyName = GetTestName();
  // This client requires an HSM client
  CreateHsmClient();
  auto const& client = GetClientForTest(keyName);

  {
    auto ecHsmKey = Azure::Security::KeyVault::Keys::CreateEcKeyOptions(keyName, true);
    ecHsmKey.Enabled = true;
    ecHsmKey.KeyOperations = {KeyOperation::Sign};
    auto keyResponse = client.CreateEcKey(ecHsmKey);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
    EXPECT_TRUE(keyVaultKey.Properties.Enabled.Value());
  }
  {
    // Now get the key
    auto keyResponse = client.GetKey(keyName);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
    EXPECT_FALSE(keyResponse.Value.Properties.ReleasePolicy.HasValue());
    EXPECT_TRUE(keyVaultKey.Properties.Enabled.Value());
  }
}
// FOR THIS TEST TO WORK MAKE SURE YOU ACTUALLY HAVE A VALID HSM VALUE FOR AZURE_KEYVAULT_HSM_URL
TEST_F(KeyVaultKeyClient, CreateRsaHsmKey)
{
  auto const keyName = GetTestName();
  // This client requires an HSM client
  CreateHsmClient();
  auto const& client = GetClientForTest(keyName);

  {
    auto rsaHsmKey = Azure::Security::KeyVault::Keys::CreateRsaKeyOptions(keyName, true);
    rsaHsmKey.Enabled = true;
    rsaHsmKey.KeyOperations = {KeyOperation::Sign};
    auto keyResponse = client.CreateRsaKey(rsaHsmKey);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
  }
  {
    // Now get the key
    auto keyResponse = client.GetKey(keyName);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;
    EXPECT_EQ(keyVaultKey.Name(), keyName);
    EXPECT_FALSE(keyResponse.Value.Properties.ReleasePolicy.HasValue());
    EXPECT_TRUE(keyVaultKey.Properties.Enabled.Value());
  }
}

std::string BinaryToHexString(std::vector<uint8_t> const& src)
{
  static constexpr char hexMap[]
      = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
  std::string output(static_cast<size_t>(src.size()) * 2, '\0');
  const uint8_t* input = src.data();

  for (size_t i = 0; i < src.size(); i++)
  {
    output[2 * i] = hexMap[(input[i] & 0xF0) >> 4];
    output[2 * i + 1] = hexMap[input[i] & 0x0F];
  }

  return output;
}
// the authority https://sharedeus.eus.test.attest.azure.net/ is not available anymore. 
// this test shoudl not be anymore since we cannot guarantee the dependencies are available. 
TEST_F(KeyVaultKeyClient, CreateKeyWithReleasePolicyOptions_DISABLED)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  Azure::Security::KeyVault::Keys::CreateKeyOptions options;
  options.KeyOperations.push_back(Azure::Security::KeyVault::Keys::KeyOperation::Sign);
  options.KeyOperations.push_back(Azure::Security::KeyVault::Keys::KeyOperation::Verify);
  options.ReleasePolicy = KeyReleasePolicy();
  options.ReleasePolicy.Value().Immutable = false;
  std::string dataStr = R"JSON({
  "anyOf":[
    {
      "allOf":[
        {
          "claim":"claim",
          "equals":"0123456789"
        }
      ],
      "authority":"https://sharedeus.eus.test.attest.azure.net/"
    }
  ],
  "version":"1.0.0"
})JSON";
  auto jsonParser = json::parse(dataStr);
  auto parsedJson = jsonParser.dump();
  options.ReleasePolicy.Value().EncodedPolicy
      = Base64Url::Base64UrlEncode(std::vector<uint8_t>(parsedJson.begin(), parsedJson.end()));
  options.Exportable = true;
  {
    auto keyResponse = client.CreateKey(
        keyName, Azure::Security::KeyVault::Keys::KeyVaultKeyType::EcHsm, options);
    CheckValidResponse(keyResponse);
    auto keyVaultKey = keyResponse.Value;

    EXPECT_EQ(keyVaultKey.Name(), keyName);
    EXPECT_EQ(
        keyVaultKey.GetKeyType().ToString(),
        Azure::Security::KeyVault::Keys::KeyVaultKeyType::EcHsm.ToString());
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
    EXPECT_PRED1(findOperation, Azure::Security::KeyVault::Keys::KeyOperation::Sign);
    EXPECT_PRED1(findOperation, Azure::Security::KeyVault::Keys::KeyOperation::Verify);
    EXPECT_TRUE(keyResponse.Value.Properties.Exportable.HasValue());
    EXPECT_TRUE(keyResponse.Value.Properties.Exportable.Value());
    EXPECT_TRUE(keyResponse.Value.Properties.ReleasePolicy.HasValue());
    auto policy = keyResponse.Value.Properties.ReleasePolicy.Value();
    EXPECT_TRUE(policy.ContentType.HasValue());
    EXPECT_EQ(
        policy.ContentType.Value(),
        Azure::Security::KeyVault::Keys::_detail::ContentTypeDefaultValue);
    EXPECT_FALSE(policy.Immutable);

    EXPECT_EQ(
        json::parse(Base64Url::Base64UrlDecode(options.ReleasePolicy.Value().EncodedPolicy))
            .dump(1, ' ', true),
        json::parse(Base64Url::Base64UrlDecode(policy.EncodedPolicy)).dump(1, ' ', true));
  }
}
