// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "gtest/gtest.h"
#include "key_client_base_test.hpp"
#include "private/key_serializers.hpp"

#include <azure/core/base64.hpp>
#include <azure/core/datetime.hpp>
#include <azure/keyvault/keyvault_keys.hpp>

#include <string>

#include <private/key_constants.hpp>

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

  Azure::Security::KeyVault::Keys::KeyRotationPolicy policy;
  policy.Attributes.ExpiryTime = "P48M";

  LifetimeActionType action1 = LifetimeActionType::Rotate;
  LifetimeActionsTrigger trigger1;
  trigger1.TimeAfterCreate = "P18M";

  LifetimeActionsTrigger trigger2;
  trigger2.TimeBeforeExpiry = "P30D";
  LifetimeActionType action2 = LifetimeActionType::Notify;

  policy.LifetimeActions.push_back(LifetimeActionsType{trigger1, action1});
  policy.LifetimeActions.push_back(LifetimeActionsType{trigger2, action2});

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

  Azure::Security::KeyVault::Keys::KeyRotationPolicy policy;
  policy.Attributes.ExpiryTime = "P48M";

  LifetimeActionType action1 = LifetimeActionType::Rotate;
  LifetimeActionsTrigger trigger1;
  trigger1.TimeAfterCreate = "P18M";

  LifetimeActionsTrigger trigger2;
  trigger2.TimeBeforeExpiry = "P30D";
  LifetimeActionType action2 = LifetimeActionType::Notify;

  policy.LifetimeActions.push_back(LifetimeActionsType{trigger1, action1});
  policy.LifetimeActions.push_back(LifetimeActionsType{trigger2, action2});
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
