// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include "key_client_base_test.hpp"

#include <azure/keyvault/key_vault.hpp>

using namespace Azure::Security::KeyVault::Keys::Test;

TEST_F(KeyVaultClientTest, SendRequestDefault)
{
  Azure::Security::KeyVault::Keys::KeyClient keyClient(m_keyVaultUrl, m_credential);

  auto r = keyClient.GetKey("testKey");
  auto t = r.ExtractValue();
  auto rr = r.ExtractRawResponse();

  EXPECT_EQ(t.Name(), "testKey");
}
