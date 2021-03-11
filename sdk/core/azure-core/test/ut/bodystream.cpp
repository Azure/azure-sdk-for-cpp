// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/platform.hpp>

#include <iostream>

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

using namespace Azure::Core::IO;
using namespace Azure::Core;

// Used to test virtual, default behavior of BodyStream.
class TestBodyStream : public BodyStream {
  int64_t OnRead(uint8_t*, int64_t, Context const&) override { return 0; }
  int64_t Length() const override { return 0; }
};

TEST(BodyStream, Rewind)
{
  TestBodyStream tb;
  EXPECT_THROW(tb.Rewind(), std::logic_error);

  std::string testDataPath(AZURE_TEST_DATA_PATH);

#if defined(AZ_PLATFORM_POSIX)
  testDataPath.append("/fileData");
#elif defined(AZ_PLATFORM_WINDOWS)
  testDataPath.append("\\fileData");
#else
#error "Unknown platform"
#endif

  auto fileBodyStream = Azure::Core::IO::FileBodyStream(testDataPath);
  EXPECT_NO_THROW(fileBodyStream.Rewind());

  std::vector<uint8_t> data = {1, 2, 3, 4};
  Azure::Core::IO::MemoryBodyStream ms(data);
  EXPECT_NO_THROW(ms.Rewind());
}

TEST(FileBodyStream, BadInput)
{
  EXPECT_THROW(Azure::Core::IO::FileBodyStream(""), std::runtime_error);
  EXPECT_THROW(Azure::Core::IO::FileBodyStream("FileNotFound"), std::runtime_error);
}

constexpr int64_t FileSize = 1024 * 100;

TEST(FileBodyStream, Length)
{
  std::string testDataPath(AZURE_TEST_DATA_PATH);
  testDataPath.append("/fileData");

  auto stream = Azure::Core::IO::FileBodyStream(testDataPath);
  EXPECT_EQ(stream.Length(), FileSize);

  auto readResult = Azure::Core::IO::BodyStream::ReadToEnd(
      stream, Azure::Core::Context::GetApplicationContext());
  EXPECT_EQ(readResult.size(), FileSize);

  stream.Rewind();
  EXPECT_EQ(stream.Length(), FileSize);

  stream = Azure::Core::IO::FileBodyStream(testDataPath);
  EXPECT_EQ(stream.Length(), FileSize);
}
