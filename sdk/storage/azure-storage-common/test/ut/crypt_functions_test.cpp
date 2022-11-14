//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <cstring>

#include <azure/storage/common/crypt.hpp>

#include "test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  std::vector<uint8_t> ToBinaryVector(const char* text)
  {
    const uint8_t* start = reinterpret_cast<const uint8_t*>(text);
    return std::vector<uint8_t>(start, start + strlen(text));
  }

  TEST_F(CryptFunctionsTest, HmacSha256)
  {
    std::string key = "8CwtGFF1mGR4bPEP9eZ0x1fxKiQ3Ca5N";
    std::vector<uint8_t> binaryKey(key.begin(), key.end());
    EXPECT_EQ(
        Azure::Core::Convert::Base64Encode(_internal::HmacSha256(ToBinaryVector(""), binaryKey)),
        "fFy2T+EuCvAgouw/vB/RAJ75z7jwTj+uiURebkFKF5M=");
    EXPECT_EQ(
        Azure::Core::Convert::Base64Encode(
            _internal::HmacSha256(ToBinaryVector("Hello Azure!"), binaryKey)),
        "+SBESxQVhI53mSEdZJcCBpdBkaqwzfPaVYZMAf5LP3c=");
  }

  static std::vector<uint8_t> ComputeHash(const std::string& data)
  {
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data.data());
    Crc64Hash instance;
    return instance.Final(ptr, data.length());
  }

  TEST_F(CryptFunctionsTest, Crc64Hash_Basic)
  {
    Crc64Hash crc64empty;
    EXPECT_EQ(Azure::Core::Convert::Base64Encode(crc64empty.Final()), "AAAAAAAAAAA=");

    EXPECT_EQ(Azure::Core::Convert::Base64Encode(ComputeHash("")), "AAAAAAAAAAA=");
    EXPECT_EQ(Azure::Core::Convert::Base64Encode(ComputeHash("Hello Azure!")), "DtjZpL9/o8c=");

    auto data = RandomBuffer(static_cast<size_t>(16_MB));
    {
      Crc64Hash crc64Single;
      Crc64Hash crc64Streaming;

      size_t length = 0;
      while (length < data.size())
      {
        size_t s = static_cast<size_t>(RandomInt(0, 4_MB));
        s = std::min(s, data.size() - length);
        crc64Streaming.Append(&data[length], s);
        crc64Streaming.Append(&data[length], 0);
        length += s;
      }
      EXPECT_EQ(crc64Streaming.Final(), crc64Single.Final(data.data(), data.size()));
    }

    // Test concatenate
    Crc64Hash crc64Single;
    Crc64Hash crc64Streaming;
    std::string allData;
    while (allData.length() < 16_MB)
    {
      {
        Crc64Hash instance2;
        for (auto i = RandomInt(0, 5); i > 0; --i)
        {
          size_t s = static_cast<size_t>(RandomInt(0, 512_KB));
          std::string data2;
          data2.resize(s);
          RandomBuffer(&data2[0], s);
          instance2.Append(reinterpret_cast<const uint8_t*>(data2.data()), data2.length());
          allData += data2;
        }
        crc64Streaming.Concatenate(instance2);
      }

      switch (RandomInt(0, 2))
      {
        case 0: {
          std::string data2;
          crc64Streaming.Append(reinterpret_cast<const uint8_t*>(data2.data()), data2.length());
          break;
        }
        case 1: {
          Crc64Hash instance2;
          crc64Streaming.Concatenate(instance2);
          break;
        }
        case 2: {
          size_t s = static_cast<size_t>(RandomInt(0, 512_KB));
          std::string data2;
          data2.resize(s);
          RandomBuffer(&data2[0], s);
          crc64Streaming.Append(reinterpret_cast<const uint8_t*>(data2.data()), data2.length());
          allData += data2;
          break;
        }
        default:
          break;
      }
    }

    EXPECT_EQ(
        crc64Streaming.Final(),
        crc64Single.Final(reinterpret_cast<const uint8_t*>(allData.data()), allData.size()));
  }

  TEST_F(CryptFunctionsTest, Crc64Hash_ExpectThrow)
  {
    std::string data = "";
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data.data());
    Crc64Hash instance;

#if GTEST_HAS_DEATH_TEST
    ASSERT_DEATH(instance.Final(nullptr, 1), "");
    ASSERT_DEATH(instance.Append(nullptr, 1), "");
#endif

    EXPECT_EQ(
        Azure::Core::Convert::Base64Encode(instance.Final(ptr, data.length())), "AAAAAAAAAAA=");

#if GTEST_HAS_DEATH_TEST
#if defined(NDEBUG)
    // Release build won't provide assert msg
    ASSERT_DEATH(instance.Final(), "");
    ASSERT_DEATH(instance.Final(ptr, data.length()), "");
    ASSERT_DEATH(instance.Append(ptr, data.length()), "");
#else
    ASSERT_DEATH(instance.Final(), "Cannot call Final");
    ASSERT_DEATH(instance.Final(ptr, data.length()), "Cannot call Final");
    ASSERT_DEATH(instance.Append(ptr, data.length()), "Cannot call Append after calling Final");
#endif
#endif
  }

  TEST_F(CryptFunctionsTest, Crc64Hash_CtorDtor)
  {
    {
      Crc64Hash instance;
    }
  }

}}} // namespace Azure::Storage::Test
