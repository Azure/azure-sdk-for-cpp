// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/platform.hpp>

#include <iostream>
#include <stdio.h>

#if defined(AZ_PLATFORM_POSIX)
#include <fcntl.h>
#elif defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <gtest/gtest.h>

#include <azure/core/io/body_stream.hpp>

using namespace Azure::IO;
using namespace Azure::Core;

// Used to test virtual, default behavior of BodyStream.
class TestBodyStream : public BodyStream {
  int64_t OnRead(Context const&, uint8_t*, int64_t) override { return 0; }
  int64_t Length() const override { return 0; }
};

TEST(BodyStream, Rewind)
{
  TestBodyStream tb;
  EXPECT_THROW(tb.Rewind(), std::logic_error);

  std::string testDataPath(AZURE_TEST_DATA_PATH);
  testDataPath.append("/fileData");

  FILE* f{};
  EXPECT_EQ(fopen_s(&f, testDataPath.c_str(), "rb"), 0);
  EXPECT_NE(f, nullptr);

  auto fileBodyStream = Azure::IO::FileBodyStream(f, 0);
  EXPECT_NO_THROW(fileBodyStream.Rewind());

  std::vector<uint8_t> data = {1, 2, 3, 4};
  Azure::IO::MemoryBodyStream ms(data);
  EXPECT_NO_THROW(ms.Rewind());

  EXPECT_EQ(fclose(f), 0);
}

constexpr int64_t FileSize = 1024 * 100;

TEST(FileBodyStream, BadInput)
{
  FILE* f = NULL;
  EXPECT_THROW((Azure::IO::FileBodyStream(f, 0)), std::invalid_argument);
  EXPECT_THROW((Azure::IO::FileBodyStream(f, 0, 0)), std::invalid_argument);

  std::string testDataPath(AZURE_TEST_DATA_PATH);
  testDataPath.append("/fileData");
  EXPECT_EQ(fopen_s(&f, testDataPath.c_str(), "rb"), 0);
  EXPECT_NE(f, nullptr);

  EXPECT_THROW(Azure::IO::FileBodyStream(f, -1), std::invalid_argument);
  EXPECT_THROW(Azure::IO::FileBodyStream(f, -1, 0), std::invalid_argument);
  EXPECT_THROW(Azure::IO::FileBodyStream(f, FileSize + 1), std::invalid_argument);
  EXPECT_THROW(Azure::IO::FileBodyStream(f, FileSize + 1, 0), std::invalid_argument);
  EXPECT_THROW(Azure::IO::FileBodyStream(f, 0, -1), std::invalid_argument);
  EXPECT_THROW(Azure::IO::FileBodyStream(f, 0, FileSize + 1), std::invalid_argument);
  EXPECT_THROW(Azure::IO::FileBodyStream(f, 1, FileSize), std::invalid_argument);
}

#ifdef _MSC_VER
#pragma warning(push)
// warning C6387: 'f' could be '0': this does not adhere to the specification for the function fread
#pragma warning(disable : 6387)
#endif

TEST(FileBodyStream, Length)
{
  FILE* f = NULL;

  std::string testDataPath(AZURE_TEST_DATA_PATH);
  testDataPath.append("/fileData");
  EXPECT_EQ(fopen_s(&f, testDataPath.c_str(), "rb"), 0);
  EXPECT_NE(f, nullptr);

  auto stream = Azure::IO::FileBodyStream(f, 0);
  EXPECT_EQ(stream.Length(), FileSize);

  stream = Azure::IO::FileBodyStream(f, 0, 2);
  EXPECT_EQ(stream.Length(), 2);

  std::vector<uint8_t> data(10);
  const size_t actualRead = fread(data.data(), 1, data.size(), f);
  EXPECT_EQ(actualRead, data.size());

  stream = Azure::IO::FileBodyStream(f, 10);
  EXPECT_EQ(stream.Length(), FileSize - data.size());

  auto readResult = Azure::IO::BodyStream::ReadToEnd(Azure::Core::GetApplicationContext(), stream);
  EXPECT_EQ(readResult.size(), FileSize - data.size());

  stream.Rewind();
  stream = Azure::IO::FileBodyStream(f, 0, 10);
  EXPECT_EQ(stream.Length(), 10);

  readResult = Azure::IO::BodyStream::ReadToEnd(Azure::Core::GetApplicationContext(), stream);
  EXPECT_EQ(readResult.size(), 10);

  stream.Rewind();
  stream = Azure::IO::FileBodyStream(f, 0);
  EXPECT_EQ(stream.Length(), FileSize);

  stream.Rewind();
  stream = Azure::IO::FileBodyStream(f, 15);
  EXPECT_EQ(stream.Length(), FileSize - 15);

  EXPECT_EQ(fclose(f), 0);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

TEST(FileBodyStream, ReadAndRewind)
{
  FILE* f = NULL;

  std::string testDataPath(AZURE_TEST_DATA_PATH);
  testDataPath.append("/fileData");
  EXPECT_EQ(fopen_s(&f, testDataPath.c_str(), "rb"), 0);
  EXPECT_NE(f, nullptr);

  auto stream = Azure::IO::FileBodyStream(f, 0);
  EXPECT_EQ(stream.Length(), FileSize);

  std::vector<uint8_t> data(5);
  EXPECT_EQ(stream.Read(Azure::Core::GetApplicationContext(), data.data(), data.size()), 5);
  EXPECT_EQ(stream.Length(), FileSize);

  data.resize(100);
  EXPECT_EQ(
      Azure::IO::BodyStream::ReadToCount(
          Azure::Core::GetApplicationContext(), stream, data.data(), data.size()),
      100);
  EXPECT_EQ(stream.Length(), FileSize);

  auto result = Azure::IO::BodyStream::ReadToEnd(Azure::Core::GetApplicationContext(), stream);
  EXPECT_EQ(result.size(), FileSize - 105);
  EXPECT_EQ(stream.Length(), FileSize);

  stream.Rewind();
  EXPECT_EQ(stream.Length(), FileSize);

  data.resize(1024 * 1024);
  EXPECT_EQ(
      Azure::IO::BodyStream::ReadToCount(
          Azure::Core::GetApplicationContext(), stream, data.data(), data.size()),
      FileSize);
  EXPECT_EQ(stream.Length(), FileSize);

  stream.Rewind();
  EXPECT_EQ(stream.Length(), FileSize);

  data.resize(1024 * 1024);
  EXPECT_EQ(stream.Read(Azure::Core::GetApplicationContext(), data.data(), data.size()), FileSize);
  EXPECT_EQ(stream.Length(), FileSize);

  EXPECT_EQ(fclose(f), 0);
}
