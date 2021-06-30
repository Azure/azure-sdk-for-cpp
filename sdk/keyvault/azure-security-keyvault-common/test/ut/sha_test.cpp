// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include "azure/keyvault/common/internal/sha.hpp"

using namespace Azure::Security::KeyVault::_internal;

TEST(SHA, SHA256Test)
{
  SHA256 sha;
  SHA256 sha2;
  uint8_t data[] = "A";
  auto shaResult = sha.Final(data, sizeof(data));
  auto shaResult2 = sha2.Final(data, sizeof(data));
  EXPECT_EQ(shaResult, shaResult2);
  for (size_t i = 0; i != shaResult.size(); i++)
    printf("%02x", shaResult[i]);
}
