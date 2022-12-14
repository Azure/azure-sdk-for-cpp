//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include "key_client_base_test.hpp"
#include "private/key_serializers.hpp"
#include <azure/core/base64.hpp>
#include <azure/core/datetime.hpp>
#include <azure/keyvault/keyvault_keys.hpp>
#include <private/key_constants.hpp>

#include <string>

using namespace Azure::Security::KeyVault::Keys::Test;
using namespace Azure;
using namespace Azure::Security::KeyVault::Keys;
using namespace Azure::Security::KeyVault::Keys::_detail;

TEST_F(KeyVaultKeyClient, RotateKey)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  auto createKeyResponse = client.CreateEcKey(CreateEcKeyOptions(keyName));
  CheckValidResponse(createKeyResponse);

  std::string input
      = "{\"id\":\"https://redacted.vault.azure.net/keys/GetKeyRotationPolicy/"
        "rotationpolicy\",\"lifetimeActions\":[{\"trigger\":{\"timeAfterCreate\":\"P18M\"},"
        "\"action\":{\"type\":\"Rotate\"}},{\"trigger\":{\"timeBeforeExpiry\":\"P30D\"},\"action\":"
        "{\"type\":\"Notify\"}}],\"attributes\":{\"expiryTime\":\"P48M\",\"created\":1649797765,"
        "\"updated\":1649797765}}";

  auto policy = KeyRotationPolicySerializer::KeyRotationPolicyDeserialize(
      std::vector<uint8_t>(input.begin(), input.end()));

  auto putPolicy = client.UpdateKeyRotationPolicy(keyName, policy).Value;
  auto originalKey = client.GetKey(keyName);
  auto rotatedKey = client.RotateKey(keyName);
  EXPECT_NE(originalKey.Value.Properties.Version, rotatedKey.Value.Properties.Version);
}
TEST_F(KeyVaultKeyClient, GetKeyRotationPolicy)
{
  auto const keyName = GetTestName();
  auto const& client = GetClientForTest(keyName);

  auto createKeyResponse = client.CreateEcKey(CreateEcKeyOptions(keyName));
  CheckValidResponse(createKeyResponse);

  std::string input
      = "{\"id\":\"https://redacted.vault.azure.net/keys/GetKeyRotationPolicy/"
        "rotationpolicy\",\"lifetimeActions\":[{\"trigger\":{\"timeAfterCreate\":\"P18M\"},"
        "\"action\":{\"type\":\"Rotate\"}},{\"trigger\":{\"timeBeforeExpiry\":\"P30D\"},\"action\":"
        "{\"type\":\"Notify\"}}],\"attributes\":{\"expiryTime\":\"P48M\",\"created\":1649797765,"
        "\"updated\":1649797765}}";

  auto policy = KeyRotationPolicySerializer::KeyRotationPolicyDeserialize(
      std::vector<uint8_t>(input.begin(), input.end()));

  auto putPolicy = client.UpdateKeyRotationPolicy(keyName, policy).Value;
  auto rotationPolicy = client.GetKeyRotationPolicy(keyName).Value;

  EXPECT_EQ(rotationPolicy.Attributes.ExpiryTime.Value(), policy.Attributes.ExpiryTime.Value());
  EXPECT_NE(rotationPolicy.Id.size(), size_t(0));
  EXPECT_EQ(rotationPolicy.LifetimeActions.size(), policy.LifetimeActions.size());

  for (auto result : rotationPolicy.LifetimeActions)
  {
    bool found = false;

    for (auto original : policy.LifetimeActions)
    {
      if (result.Action == original.Action)
      {
        found = true;
        break;
      }
    }
    EXPECT_TRUE(found);
  }
}

TEST_F(KeyVaultKeyClient, GetRandomBytes_LIVEONLY_)
{
  if (m_keyVaultUrl.compare(m_keyVaultHsmUrl) != 0)
  {
    auto const keyName = GetTestName();
    CreateHsmClient();
    auto const& client = GetClientForTest(keyName);
    GetRandomBytesOptions options;
    options.Count = 4;
    auto result = client.GetRandomBytes(options);
    EXPECT_EQ(result.Value.RandomBytes.size(), size_t(options.Count));
  }
  EXPECT_TRUE(true);
}

TEST(GetRandomBytesOptions, Serialize)
{
  GetRandomBytesOptions options;
  {
    options.Count = 0;
    std::string result = GetRandomBytesSerializer::GetRandomBytesOptionsSerialize(options);
    EXPECT_EQ(result, "{\"count\":0}");
  }

  {
    options.Count = 5;
    std::string result = GetRandomBytesSerializer::GetRandomBytesOptionsSerialize(options);
    EXPECT_EQ(result, "{\"count\":5}");
  }

  {
    options.Count = -1;
    std::string result = GetRandomBytesSerializer::GetRandomBytesOptionsSerialize(options);
    EXPECT_EQ(result, "{\"count\":-1}");
  }
}

TEST(GetRandomBytesOptions, Deserialize)
{
  std::string inputString = "1234";
  auto bytes = Azure::Core::_internal::Base64Url::Base64UrlEncode(
      std::vector<uint8_t>(inputString.begin(), inputString.end()));
  std::string responseText = "{\"value\": \"" + std::string(bytes.begin(), bytes.end()) + "\" }";

  Azure::Core::Http::RawResponse rawResponse(1, 1, Azure::Core::Http::HttpStatusCode::Ok, "OK");
  rawResponse.SetBody(std::vector<uint8_t>(responseText.begin(), responseText.end()));

  auto deserialized = GetRandomBytesSerializer::GetRandomBytesResponseDeserialize(rawResponse);
  EXPECT_EQ(deserialized.size(), size_t(4));
  EXPECT_EQ(deserialized[0], uint8_t('1'));
  EXPECT_EQ(deserialized[1], uint8_t('2'));
  EXPECT_EQ(deserialized[2], uint8_t('3'));
  EXPECT_EQ(deserialized[3], uint8_t('4'));
}

TEST(GetRandomBytesOptions, DeserializeEmpty)
{
  std::string inputString = "";
  auto bytes = Azure::Core::Convert::Base64Encode(
      std::vector<uint8_t>(inputString.begin(), inputString.end()));
  std::string responseText = "{\"value\": \"" + std::string(bytes.begin(), bytes.end()) + "\" }";

  Azure::Core::Http::RawResponse rawResponse(1, 1, Azure::Core::Http::HttpStatusCode::Ok, "OK");
  rawResponse.SetBody(std::vector<uint8_t>(responseText.begin(), responseText.end()));

  auto deserialized = GetRandomBytesSerializer::GetRandomBytesResponseDeserialize(rawResponse);
  EXPECT_EQ(deserialized.size(), size_t(0));
}

TEST(KeyRotationPolicy, SerializeDeserialize1)
{
  std::string input
      = "{\"id\":\"https://redacted.vault.azure.net/keys/GetKeyRotationPolicy/"
        "rotationpolicy\",\"lifetimeActions\":[{\"trigger\":{\"timeAfterCreate\":\"P18M\"},"
        "\"action\":{\"type\":\"Rotate\"}},{\"trigger\":{\"timeBeforeExpiry\":\"P30D\"},\"action\":"
        "{\"type\":\"Notify\"}}],\"attributes\":{\"expiryTime\":\"P48M\",\"created\":1649797765,"
        "\"updated\":1649797765}}";

  auto policy = KeyRotationPolicySerializer::KeyRotationPolicyDeserialize(
      std::vector<uint8_t>(input.begin(), input.end()));

  EXPECT_EQ(policy.Id, "https://redacted.vault.azure.net/keys/GetKeyRotationPolicy/rotationpolicy");
  EXPECT_EQ(policy.Attributes.ExpiryTime.Value(), "P48M");
  EXPECT_TRUE(policy.Attributes.Created);
  EXPECT_TRUE(policy.Attributes.Updated);
  EXPECT_EQ(policy.LifetimeActions.size(), size_t(2));

  auto action0 = policy.LifetimeActions[0];
  EXPECT_EQ(action0.Action, LifetimeActionType::Rotate);
  EXPECT_EQ(action0.Trigger.TimeAfterCreate.Value(), "P18M");
  EXPECT_FALSE(action0.Trigger.TimeBeforeExpiry);

  auto action1 = policy.LifetimeActions[1];
  EXPECT_EQ(action1.Action, LifetimeActionType::Notify);
  EXPECT_EQ(action1.Trigger.TimeBeforeExpiry.Value(), "P30D");
  EXPECT_FALSE(action1.Trigger.TimeAfterCreate);

  auto serialized = KeyRotationPolicySerializer::KeyRotationPolicySerialize(policy);

  std::string serializedString
      = "{\"attributes\":{\"expiryTime\":\"P48M\"},\"lifetimeActions\":[{\"action\":{\"type\":"
        "\"rotate\"},\"trigger\":{\"timeAfterCreate\":\"P18M\"}},{\"action\":{\"type\":\"notify\"},"
        "\"trigger\":{\"timeBeforeExpiry\":\"P30D\"}}]}";

  EXPECT_EQ(serialized, serializedString);
}
