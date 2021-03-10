// Copyright (c) Microsoft Corporation. All rights reserved.
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

  TEST(CryptFunctionsTest, Sha256)
  {
    EXPECT_EQ(
        Azure::Core::Base64Encode(_detail::Sha256(ToBinaryVector(""))),
        "47DEQpj8HBSa+/TImW+5JCeuQeRkm5NMpJWZG3hSuFU=");
    EXPECT_EQ(
        Azure::Core::Base64Encode(_detail::Sha256(ToBinaryVector("Hello Azure!"))),
        "Mjzwx2mqGHb9FSgjm33ShNmXYndkgvwA6tQmEiskOHg=");
  }

  TEST(CryptFunctionsTest, HmacSha256)
  {
    std::string key = "8CwtGFF1mGR4bPEP9eZ0x1fxKiQ3Ca5N";
    std::vector<uint8_t> binaryKey(key.begin(), key.end());
    EXPECT_EQ(
        Azure::Core::Base64Encode(_detail::HmacSha256(ToBinaryVector(""), binaryKey)),
        "fFy2T+EuCvAgouw/vB/RAJ75z7jwTj+uiURebkFKF5M=");
    EXPECT_EQ(
        Azure::Core::Base64Encode(_detail::HmacSha256(ToBinaryVector("Hello Azure!"), binaryKey)),
        "+SBESxQVhI53mSEdZJcCBpdBkaqwzfPaVYZMAf5LP3c=");
  }

  static std::vector<uint8_t> ComputeHash(const std::string& data)
  {
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data.data());
    Crc64Hash instance;
    return instance.Final(ptr, data.length());
  }

  TEST(CryptFunctionsTest, Crc64Hash_Basic)
  {
    Crc64Hash crc64empty;
    EXPECT_EQ(Azure::Core::Base64Encode(crc64empty.Final()), "AAAAAAAAAAA=");

    EXPECT_EQ(Azure::Core::Base64Encode(ComputeHash("")), "AAAAAAAAAAA=");
    EXPECT_EQ(Azure::Core::Base64Encode(ComputeHash("Hello Azure!")), "DtjZpL9/o8c=");

    auto data = RandomBuffer(static_cast<std::size_t>(16_MB));
    {
      Crc64Hash crc64Single;
      Crc64Hash crc64Streaming;

      std::size_t length = 0;
      while (length < data.size())
      {
        std::size_t s = static_cast<std::size_t>(RandomInt(0, 4_MB));
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
          std::size_t s = static_cast<std::size_t>(RandomInt(0, 512_KB));
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
          std::size_t s = static_cast<std::size_t>(RandomInt(0, 512_KB));
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

  TEST(CryptFunctionsTest, Crc64Hash_ExpectThrow)
  {
    std::string data = "";
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data.data());
    Crc64Hash instance;

    EXPECT_THROW(instance.Final(nullptr, 1), std::invalid_argument);
    EXPECT_THROW(instance.Append(nullptr, 1), std::invalid_argument);

    EXPECT_EQ(Azure::Core::Base64Encode(instance.Final(ptr, data.length())), "AAAAAAAAAAA=");
    EXPECT_THROW(instance.Final(), std::runtime_error);
    EXPECT_THROW(instance.Final(ptr, data.length()), std::runtime_error);
    EXPECT_THROW(instance.Append(ptr, data.length()), std::runtime_error);
  }

  TEST(CryptFunctionsTest, Crc64Hash_CtorDtor)
  {
    {
      Crc64Hash instance;
    }
  }

}}} // namespace Azure::Storage::Test
