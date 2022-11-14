//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/uuid.hpp>
#include <gtest/gtest.h>
#include <set>
#include <string>

using namespace Azure::Core;

TEST(Uuid, Basic)
{
  auto uuid = Uuid::CreateUuid();
  EXPECT_TRUE(uuid.ToString().length() == 36);
}

TEST(Uuid, Randomness)
{
  const int size = 100000;
  std::set<std::string> uuids;
  for (int i = 0; i < size; i++)
  {
    auto ret = uuids.insert(Uuid::CreateUuid().ToString());
    // If the value already exists in the set then the insert will fail
    // ret.second == false means the insert failed.
    EXPECT_TRUE(ret.second);
  }
  EXPECT_TRUE(uuids.size() == size);
}

TEST(Uuid, separatorPosition)
{
  auto uuidKey = Uuid::CreateUuid().ToString();
  // validate expected format '8-4-4-4-12'
  EXPECT_PRED5(
      [](std::string const&, char pos1, char pos2, char pos3, char pos4) {
        return pos1 == pos2 && pos1 == pos3 && pos1 == pos4 && pos1 == '-';
      },
      uuidKey,
      uuidKey[8],
      uuidKey[13],
      uuidKey[18],
      uuidKey[23]);
}

TEST(Uuid, validChars)
{
  auto uuidKey = Uuid::CreateUuid().ToString();
  // validate valid chars and separators count
  EXPECT_PRED2(
      [](std::string const& uuidKey, int expectedSeparators) {
        int separatorsCount = 0;
        for (size_t index = 0; index < uuidKey.size(); index++)
        {
          if (uuidKey[index] == '-')
          {
            separatorsCount++;
            continue;
          }
          else if (!((uuidKey[index] >= '0' && uuidKey[index] <= '9')
                     || (uuidKey[index] >= 'a' && uuidKey[index] <= 'f')
                     || (uuidKey[index] >= 'A' && uuidKey[index] <= 'F')))
          {
            // invalid char found
            return false;
          }
        }
        return separatorsCount == expectedSeparators;
      },
      uuidKey,
      4);
}
