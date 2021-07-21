// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include <azure/keyvault/secrets/secret_client.hpp>

using namespace Azure::Security::KeyVault::Secrets;

TEST(SecretClient, Basic)
{
  SecretClient secretClient;

  EXPECT_FALSE(secretClient.ClientVersion().empty());
}
