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
    EXPECT_EQ(Base64Encode(Sha256("")), "47DEQpj8HBSa+/TImW+5JCeuQeRkm5NMpJWZG3hSuFU=");
    EXPECT_EQ(Base64Encode(Sha256("Hello Azure!")), "Mjzwx2mqGHb9FSgjm33ShNmXYndkgvwA6tQmEiskOHg=");
  }

  TEST(CryptFunctionsTest, HmacSha256)
  {
    std::string key = "8CwtGFF1mGR4bPEP9eZ0x1fxKiQ3Ca5N";
    EXPECT_EQ(Base64Encode(Hmac_Sha256("", key)), "fFy2T+EuCvAgouw/vB/RAJ75z7jwTj+uiURebkFKF5M=");
    EXPECT_EQ(
        Base64Encode(Hmac_Sha256("Hello Azure!", key)),
        "+SBESxQVhI53mSEdZJcCBpdBkaqwzfPaVYZMAf5LP3c=");
  }

}}} // namespace Azure::Storage::Test
