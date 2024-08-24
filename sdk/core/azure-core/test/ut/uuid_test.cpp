// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/uuid.hpp>

#include <set>
#include <string>

#include <gtest/gtest.h>

using namespace Azure::Core;

TEST(Uuid, Basic)
{
  auto uuid = Uuid::CreateUuid();
  EXPECT_EQ(uuid.ToString().length(), 36);
}

TEST(Uuid, Roundtrip)
{
  std::array<uint8_t, 16U> uuidArray
      = {97, 126, 195, 45, 41, 178, 70, 23, 142, 131, 221, 245, 20, 45, 215, 15};

  auto uuid = Uuid::CreateFromArray(uuidArray);
  std::string uuidString = uuid.ToString();
  std::string expectedString = "617ec32d-29b2-4617-8e83-ddf5142dd70f";
  EXPECT_EQ(expectedString, uuidString);

  auto roundTrip = uuid.AsArray();
  EXPECT_EQ(uuidArray, roundTrip);
}

TEST(Uuid, Transparent)
{
  auto uuid1 = Uuid::CreateUuid();
  auto arrayUuid1(uuid1.AsArray());
  auto uuid2 = Azure::Core::Uuid::CreateFromArray(arrayUuid1);
  EXPECT_EQ(uuid1.ToString(), uuid2.ToString());

  // Repeated calls of ToString() to validate the same values are returned, whether it is cached or
  // not.
  EXPECT_EQ(uuid1.ToString(), uuid2.ToString());
  EXPECT_EQ(uuid1.ToString(), uuid1.ToString());
  EXPECT_EQ(uuid2.ToString(), uuid2.ToString());
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
  EXPECT_EQ(uuids.size(), size);
}

TEST(Uuid, Rfc4122Conforming)
{
  const int size = 100;
  for (int i = 0; i < size; i++)
  {
    auto uuid = Uuid::CreateUuid();
    auto uuidStr = uuid.ToString();
    auto version = uuidStr[14];
    EXPECT_EQ(version, '4'); // Version 4: Pseudo-random number

    // The variant field consists of a variable number of the most significant bits of octet 8 of
    // the UUID.
    // https://www.rfc-editor.org/rfc/rfc4122.html#section-4.1.1
    // The high bits of the variant need to be of the form 10xx, which means they can only be either
    // 8, 9, A|a, B|b. The 0-7 values are reserved for backward compatibility. The C|c, D|d values
    // are reserved for Microsoft, and the E|e, F|f values are reserved for future use.
    auto variant = uuidStr[19];

    // The test is written this way to improve logging IF it was to fail, so we can see the value
    // of the incorrect variant.
    EXPECT_TRUE(
        (variant == '8' || variant == '9' || variant == 'A' || variant == 'B' || variant == 'a'
         || variant == 'b'))
        << variant << " is not one of the expected values of 8, 9, A, B, a, b";
  }
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

TEST(Uuid, nilAndDefault)
{
  Uuid uuid;
  ASSERT_TRUE(uuid.IsNil());
  ASSERT_EQ(uuid.ToString(), "00000000-0000-0000-0000-000000000000");
  ASSERT_EQ(uuid, Uuid{});
  ASSERT_EQ(uuid.AsArray(), Uuid::ValueArray({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}));
}

TEST(Uuid, parse)
{
  Uuid uuid1 = Uuid::Parse("00112233-4455-6677-8899-aAbBcCdDeEfF");

  ASSERT_FALSE(uuid1.IsNil());
  ASSERT_EQ(uuid1.ToString(), "00112233-4455-6677-8899-aabbccddeeff");
  ASSERT_NE(uuid1, Uuid{});
  ASSERT_EQ(
      uuid1.AsArray(),
      Uuid::ValueArray(
          {0x00,
           0x11,
           0x22,
           0x33,
           0x44,
           0x55,
           0x66,
           0x77,
           0x88,
           0x99,
           0xAA,
           0xBB,
           0xCC,
           0xDD,
           0xEE,
           0xFF}));

  // Empty string
  ASSERT_THROW(Uuid::Parse(""), std::invalid_argument);

  // Special characters - make sure we're not treating them as byte array
  ASSERT_THROW(Uuid::Parse("\a\a\a\a\a\a\a\a\a\a\a\a\a\a\a\a"), std::invalid_argument);

  // Spaces before, after, and both.
  ASSERT_THROW(Uuid::Parse("00000000-0000-0000-0000-000000000000 "), std::invalid_argument);
  ASSERT_THROW(Uuid::Parse(" 00000000-0000-0000-0000-000000000000"), std::invalid_argument);
  ASSERT_THROW(Uuid::Parse("00000000-0000-0000-0000-00000000000"), std::invalid_argument);

  // Valid characters, but in places where dashes should be
  ASSERT_THROW(Uuid::Parse("00000000a0000-0000-0000-000000000000"), std::invalid_argument);
  ASSERT_THROW(Uuid::Parse("00000000-0000a0000-0000-000000000000"), std::invalid_argument);
  ASSERT_THROW(Uuid::Parse("00000000-0000-0000a0000-000000000000"), std::invalid_argument);
  ASSERT_THROW(Uuid::Parse("00000000-0000-0000-0000a000000000000"), std::invalid_argument);

  // Another ToString() formats
  // (https://learn.microsoft.com/dotnet/api/system.guid.tostring?view=net-8.0)
  ASSERT_THROW(Uuid::Parse("00000000000000000000000000000000"), std::invalid_argument);
  ASSERT_THROW(Uuid::Parse("{00000000-0000-0000-0000-000000000000}"), std::invalid_argument);
  ASSERT_THROW(Uuid::Parse("(00000000-0000-0000-0000-000000000000)"), std::invalid_argument);
  ASSERT_THROW(
      Uuid::Parse("{0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}"),
      std::invalid_argument);

  // Correct length, invalid characters
  ASSERT_THROW(Uuid::Parse("o000000000-0000-0000-0000-000000000000"), std::invalid_argument);
  ASSERT_THROW(Uuid::Parse("0000000000-0000-0000-0000-00000000000o"), std::invalid_argument);

  // Incorrect length, incorrect caracters
  ASSERT_THROW(Uuid::Parse("00000000-0000-0000-0000-0000000000G"), std::invalid_argument);

  // Less dashes
  ASSERT_THROW(Uuid::Parse("00000000-000000000000000000000000"), std::invalid_argument);
  ASSERT_THROW(Uuid::Parse("00000000-0000-00000000000000000000"), std::invalid_argument);
  ASSERT_THROW(Uuid::Parse("00000000-0000-0000-0000000000000000"), std::invalid_argument);

  // Just a string of text
  ASSERT_THROW(Uuid::Parse("The quick brown fox jumps over the lazy dog."), std::invalid_argument);
}
