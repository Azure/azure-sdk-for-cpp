// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <uuid.hpp>
#include <string>
#include <vector>

using namespace Azure::Core;

TEST(Uuid, Basic)
{
  auto uuid = Uuid::CreateUuid();
  EXPECT_TRUE(uuid.GetUuidString().length() == 36);
}
