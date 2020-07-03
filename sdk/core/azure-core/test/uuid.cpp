// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <uuid.hpp>
#include <string>
#include <vector>

using namespace Azure::Core;

TEST(UUID, Basic)
{
  auto uuid = UUID();
  EXPECT_TRUE(uuid.GetUUIDString().length() == 36);
}
