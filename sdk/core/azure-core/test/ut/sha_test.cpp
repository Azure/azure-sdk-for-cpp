// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include "azure/core/internal/cryptography/sha_hash.hpp"

using namespace Azure::Core::Cryptography::_internal;

// cspell: words ABCDE FGHIJ
TEST(SHA, SHA1Test)
{
  {
    Sha1Hash sha;
    Sha1Hash sha2;
    uint8_t data[] = "A";
    auto shaResult = sha.Final(data, sizeof(data));
    auto shaResult2 = sha2.Final(data, sizeof(data));
    EXPECT_EQ(shaResult, shaResult2);
    for (size_t i = 0; i != shaResult.size(); i++)
      printf("%02x", shaResult[i]);
  }
  {
    Sha1Hash sha;
    Sha1Hash sha2;
    std::string data1 = "ABCDE";
    std::string data2 = "FGHIJ";
    sha.Append(reinterpret_cast<const uint8_t*>(data1.data()), data1.size());
    auto shaResult = sha.Final(reinterpret_cast<const uint8_t*>(data2.data()), data2.size());
    auto shaResult2 = sha2.Final(
        reinterpret_cast<const uint8_t*>((data1 + data2).data()), data1.size() + data2.size());
    EXPECT_EQ(shaResult, shaResult2);
    for (size_t i = 0; i != shaResult.size(); i++)
      printf("%02x", shaResult[i]);
  }
}

TEST(SHA, SHA256Test)
{
  {

    Sha256Hash sha;
    Sha256Hash sha2;
    uint8_t data[] = "A";
    auto shaResult = sha.Final(data, sizeof(data));
    auto shaResult2 = sha2.Final(data, sizeof(data));
    EXPECT_EQ(shaResult, shaResult2);
    for (size_t i = 0; i != shaResult.size(); i++)
      printf("%02x", shaResult[i]);
  }
  {
    Sha256Hash sha;
    Sha256Hash sha2;
    std::string data1 = "ABCDE";
    std::string data2 = "FGHIJ";
    sha.Append(reinterpret_cast<const uint8_t*>(data1.data()), data1.size());
    auto shaResult = sha.Final(reinterpret_cast<const uint8_t*>(data2.data()), data2.size());
    auto shaResult2 = sha2.Final(
        reinterpret_cast<const uint8_t*>((data1 + data2).data()), data1.size() + data2.size());
    EXPECT_EQ(shaResult, shaResult2);
    for (size_t i = 0; i != shaResult.size(); i++)
      printf("%02x", shaResult[i]);
  }
}

TEST(SHA, SHA384Test)
{
  {

    Sha384Hash sha;
    Sha384Hash sha2;
    uint8_t data[] = "A";
    auto shaResult = sha.Final(data, sizeof(data));
    auto shaResult2 = sha2.Final(data, sizeof(data));
    EXPECT_EQ(shaResult, shaResult2);
    for (size_t i = 0; i != shaResult.size(); i++)
      printf("%02x", shaResult[i]);
  }
  {
    Sha384Hash sha;
    Sha384Hash sha2;
    std::string data1 = "ABCDE";
    std::string data2 = "FGHIJ";
    sha.Append(reinterpret_cast<const uint8_t*>(data1.data()), data1.size());
    auto shaResult = sha.Final(reinterpret_cast<const uint8_t*>(data2.data()), data2.size());
    auto shaResult2 = sha2.Final(
        reinterpret_cast<const uint8_t*>((data1 + data2).data()), data1.size() + data2.size());
    EXPECT_EQ(shaResult, shaResult2);
    for (size_t i = 0; i != shaResult.size(); i++)
      printf("%02x", shaResult[i]);
  }
}

TEST(SHA, SHA512Test)
{
  {

    Sha512Hash sha;
    Sha512Hash sha2;
    uint8_t data[] = "A";
    auto shaResult = sha.Final(data, sizeof(data));
    auto shaResult2 = sha2.Final(data, sizeof(data));
    EXPECT_EQ(shaResult, shaResult2);
    for (size_t i = 0; i != shaResult.size(); i++)
      printf("%02x", shaResult[i]);
  }
  {
    Sha512Hash sha;
    Sha512Hash sha2;
    std::string data1 = "ABCDE";
    std::string data2 = "FGHIJ";
    sha.Append(reinterpret_cast<const uint8_t*>(data1.data()), data1.size());
    auto shaResult = sha.Final(reinterpret_cast<const uint8_t*>(data2.data()), data2.size());
    auto shaResult2 = sha2.Final(
        reinterpret_cast<const uint8_t*>((data1 + data2).data()), data1.size() + data2.size());
    EXPECT_EQ(shaResult, shaResult2);
    for (size_t i = 0; i != shaResult.size(); i++)
      printf("%02x", shaResult[i]);
  }
}