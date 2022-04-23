// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "../../azure-security-attestation/src/private/crypto/inc/crypto.hpp"
#include "key_client_base_test.hpp"
#include "maasandbox.hpp"
#include "private/key_constants.hpp"
#include "private/key_serializers.hpp"
#include "gtest/gtest.h"
#include <azure/attestation.hpp>
#include <azure/core/base64.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/keyvault/keyvault_keys.hpp>
#include <private/key_constants.hpp>
#include <string>

using namespace Azure::Core::_internal;
using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Security::KeyVault::Keys::Test;

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
  options.ReleasePolicy = KeyReleasePolicy();
  options.ReleasePolicy.Value().Immutable = true;
  std::string dataStr = "release policy data";
  options.ReleasePolicy.Value().Data
      = Base64Url::Base64UrlEncode(std::vector<uint8_t>(dataStr.begin(), dataStr.end()));

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

TEST_F(KeyVaultKeyClient, CreateKeyWithRelasePolicyOptions)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  Azure::Security::KeyVault::Keys::CreateKeyOptions options;
  options.KeyOperations.push_back(Azure::Security::KeyVault::Keys::KeyOperation::Sign);
  options.KeyOperations.push_back(Azure::Security::KeyVault::Keys::KeyOperation::Verify);
  options.ReleasePolicy = KeyReleasePolicy();
  options.ReleasePolicy.Value().Immutable = false;
  std::string dataStr = "{"
                        "\"anyOf\" : [ {"
                        "\"allOf\" : [ {\"claim\" : \"claim\", \"equals\" : \"0123456789\"} ],"
                        "\"authority\" : \"https://sharedeus.eus.test.attest.azure.net/\""
                        "} ],"
                        " \"version\" : \"1.0.0\""
                        "} ";
  auto jsonParser = json::parse(dataStr);
  options.ReleasePolicy.Value().Data = jsonParser.dump();
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
        json::parse(options.ReleasePolicy.Value().Data).dump(1, ' ', true),
        json::parse(policy.Data).dump(1, ' ', true));
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

TEST_F(KeyVaultKeyClient, CreateEcHsmKey)
{
  auto const keyName = GetTestName();
  // This client requires an HSM client
  CreateHsmClient();
  auto const& client = GetClientForTest(keyName);

  {
    auto ecHsmKey = Azure::Security::KeyVault::Keys::CreateEcKeyOptions(keyName, true);
    auto keyResponse = client.CreateEcKey(ecHsmKey);
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

TEST_F(KeyVaultKeyClient, CreateRsaHsmKey)
{
  auto const keyName = GetTestName();
  // This client requires an HSM client
  CreateHsmClient();
  auto const& client = GetClientForTest(keyName);

  {
    auto rsaHsmKey = Azure::Security::KeyVault::Keys::CreateRsaKeyOptions(keyName, true);
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
  }
}

TEST_F(KeyVaultKeyClient, ReleaseKey)
{
  auto const keyName = GetTestName();
  Azure::Security::Attestation::AttestationClient attestationClient(
      "https://sharedwus.wus.attest.azure.net");

  MaaSandboxClient sandboxClient("https://maasandboxsvc.azurewebsites.net");

  auto key(Azure::Security::Attestation::_detail::Cryptography::CreateRsaKey(2048));
  std::string data = key->ExportPublicKey();

  JsonWebKey jwk;
  jwk.Id = "release-key";
  jwk.KeyType = KeyVaultKeyType::Rsa;
  jwk.SetKeyOperations({KeyOperation::Encrypt});
  jwk.E = Base64Url::Base64UrlDecode("AQAB");
  jwk.N = std::vector<uint8_t>(data.begin(), data.end());
  Azure::Core::Json::_internal::json destJson;
  Azure::Security::KeyVault::Keys::_detail::JsonWebKeySerializer::JsonWebKeySerialize(
      jwk, destJson["keys"]);

  std::string result = destJson.dump();

  auto generatedToken = sandboxClient.GenerateQuote(AttestationDataType::Binary, result);

  attestationClient.RetrieveResponseValidationCollateral();

  attestationClient.AttestOpenEnclave(
      generatedToken.Value,
      AttestOptions{AttestationData{
          std::vector<uint8_t>(result.begin(), result.end()), AttestationDataType::Binary}});

  auto const& client = GetClientForTest(keyName);

  auto ecKey = Azure::Security::KeyVault::Keys::CreateEcKeyOptions(keyName);
  ecKey.CurveName = Azure::Security::KeyVault::Keys::KeyCurveName::P384;
  auto keyResponse = client.CreateEcKey(ecKey);
  auto keyVaultKey = keyResponse.Value;
  KeyReleaseOptions relOpt;
  relOpt.Target = Base64Url::Base64UrlEncode(generatedToken.Value);
  // relOpt.Encryption =
  // Azure::Security::KeyVault::Keys::KeyEncryptionAlgorithm::RSA_AES_KEY_WRAP_256;

  // CreateHsmClient("https://gearamamhsm.managedhsm.azure.net/");
  // auto const& client2 = GetClientForTest("realeasy");
  auto result2 = client.ReleaseKey(keyVaultKey.Name(), keyVaultKey.Properties.Version, relOpt);
}
