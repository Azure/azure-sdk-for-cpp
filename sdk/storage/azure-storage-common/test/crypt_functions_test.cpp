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
        Azure::Core::Base64Encode(Details::Sha256(ToBinaryVector(""))),
        "47DEQpj8HBSa+/TImW+5JCeuQeRkm5NMpJWZG3hSuFU=");
    EXPECT_EQ(
        Azure::Core::Base64Encode(Details::Sha256(ToBinaryVector("Hello Azure!"))),
        "Mjzwx2mqGHb9FSgjm33ShNmXYndkgvwA6tQmEiskOHg=");
  }

  TEST(CryptFunctionsTest, HmacSha256)
  {
    std::string key = "8CwtGFF1mGR4bPEP9eZ0x1fxKiQ3Ca5N";
    std::vector<uint8_t> binaryKey(key.begin(), key.end());
    EXPECT_EQ(
        Azure::Core::Base64Encode(Details::HmacSha256(ToBinaryVector(""), binaryKey)),
        "fFy2T+EuCvAgouw/vB/RAJ75z7jwTj+uiURebkFKF5M=");
    EXPECT_EQ(
        Azure::Core::Base64Encode(Details::HmacSha256(ToBinaryVector("Hello Azure!"), binaryKey)),
        "+SBESxQVhI53mSEdZJcCBpdBkaqwzfPaVYZMAf5LP3c=");
  }

  TEST(CryptFunctionsTest, Md5)
  {
    EXPECT_EQ(Azure::Core::Base64Encode(Md5::Hash("")), "1B2M2Y8AsgTpgAmY7PhCfg==");
    EXPECT_EQ(Azure::Core::Base64Encode(Md5::Hash("Hello Azure!")), "Pz8543xut4RVSbb2g52Mww==");

    auto data = RandomBuffer(static_cast<std::size_t>(16_MB));
    Md5 md5Instance;

    std::size_t length = 0;
    while (length < data.size())
    {
      std::size_t s = static_cast<std::size_t>(RandomInt(0, 4_MB));
      s = std::min(s, data.size() - length);
      md5Instance.Update(&data[length], s);
      md5Instance.Update(&data[length], 0);
      length += s;
    }
    EXPECT_EQ(md5Instance.Digest(), Md5::Hash(data.data(), data.size()));
  }

  TEST(CryptFunctionsTest, Crc64)
  {
    EXPECT_EQ(Azure::Core::Base64Encode(Crc64::Hash("")), "AAAAAAAAAAA=");
    EXPECT_EQ(Azure::Core::Base64Encode(Crc64::Hash("Hello Azure!")), "DtjZpL9/o8c=");

    auto data = RandomBuffer(static_cast<std::size_t>(16_MB));
    Crc64 crc64Instance;

    std::size_t length = 0;
    while (length < data.size())
    {
      std::size_t s = static_cast<std::size_t>(RandomInt(0, 4_MB));
      s = std::min(s, data.size() - length);
      crc64Instance.Update(&data[length], s);
      crc64Instance.Update(&data[length], 0);
      length += s;
    }
    EXPECT_EQ(crc64Instance.Digest(), Crc64::Hash(data.data(), data.size()));

    // Test concatenate
    crc64Instance = Crc64();
    std::string allData;
    while (allData.length() < 16_MB)
    {
      {
        Crc64 instance2;
        for (auto i = RandomInt(0, 5); i > 0; --i)
        {
          std::size_t s = static_cast<std::size_t>(RandomInt(0, 512_KB));
          std::string data2;
          data2.resize(s);
          RandomBuffer(&data2[0], s);
          instance2.Update(reinterpret_cast<const uint8_t*>(data2.data()), data2.length());
          allData += data2;
        }
        crc64Instance.Concatenate(instance2);
      }

      switch (RandomInt(0, 2))
      {
        case 0: {
          std::string data2;
          crc64Instance.Update(reinterpret_cast<const uint8_t*>(data2.data()), data2.length());
          break;
        }
        case 1: {
          Crc64 instance2;
          crc64Instance.Concatenate(instance2);
          break;
        }
        case 2: {
          std::size_t s = static_cast<std::size_t>(RandomInt(0, 512_KB));
          std::string data2;
          data2.resize(s);
          RandomBuffer(&data2[0], s);
          crc64Instance.Update(reinterpret_cast<const uint8_t*>(data2.data()), data2.length());
          allData += data2;
          break;
        }
        default:
          break;
      }
    }

    EXPECT_EQ(
        crc64Instance.Digest(),
        Crc64::Hash(reinterpret_cast<const uint8_t*>(allData.data()), allData.size()));
  }

}}} // namespace Azure::Storage::Test
