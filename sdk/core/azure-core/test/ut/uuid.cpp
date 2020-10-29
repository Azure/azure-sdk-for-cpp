// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <azure/core/uuid.hpp>
#include <string>
#include <set>

using namespace Azure::Core;

TEST(Uuid, Basic)
{
  auto uuid = Uuid::CreateUuid();
  EXPECT_TRUE(uuid.GetUuidString().length() == 36);
}

TEST(Uuid, Randomness) 
{ 
  const int Size = 100000;
  std::set<std::string> uuids;
  for (int i = 0; i < Size; i++)
  {
    auto ret = uuids.insert(Uuid::CreateUuid().GetUuidString());
    //If the value already exists in the set then the insert will fail
    //  ret.second == false means the insert failed.
    EXPECT_TRUE(ret.second);
  }
  EXPECT_TRUE(uuids.size() == Size);
}
