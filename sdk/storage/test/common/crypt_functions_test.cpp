// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "common/crypt.hpp"
#include "test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  TEST(CryptFunctionsTest, Base64)
  {
    for (std::size_t len : {0, 10, 100, 1000, 10000})
    {
      std::string data;
      data.resize(len);
      RandomBuffer(&data[0], data.length());
      EXPECT_EQ(Base64Decode(Base64Encode(data)), data);
    }
  }

  TEST(CryptFunctionsTest, Sha256)
  {
    EXPECT_EQ(Base64Encode(Details::Sha256("")), "47DEQpj8HBSa+/TImW+5JCeuQeRkm5NMpJWZG3hSuFU=");
    EXPECT_EQ(
        Base64Encode(Details::Sha256("Hello Azure!")),
        "Mjzwx2mqGHb9FSgjm33ShNmXYndkgvwA6tQmEiskOHg=");
  }

  TEST(CryptFunctionsTest, HmacSha256)
  {
    std::string key = "8CwtGFF1mGR4bPEP9eZ0x1fxKiQ3Ca5N";
    EXPECT_EQ(
        Base64Encode(Details::Hmac_Sha256("", key)),
        "fFy2T+EuCvAgouw/vB/RAJ75z7jwTj+uiURebkFKF5M=");
    EXPECT_EQ(
        Base64Encode(Details::Hmac_Sha256("Hello Azure!", key)),
        "+SBESxQVhI53mSEdZJcCBpdBkaqwzfPaVYZMAf5LP3c=");
  }

  TEST(CryptFunctionsTest, Md5)
  {
    EXPECT_EQ(Base64Encode(Md5::Hash("")), "1B2M2Y8AsgTpgAmY7PhCfg==");
    EXPECT_EQ(Base64Encode(Md5::Hash("Hello Azure!")), "Pz8543xut4RVSbb2g52Mww==");

    auto data = RandomBuffer(16_MB);
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

}}} // namespace Azure::Storage::Test
