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

#include <azure/core/http/body_stream.hpp>

using namespace Azure::Core;

// Used to test virtual, default behavior of BodyStream.
class TestBodyStream : public Http::BodyStream {
  int64_t OnRead(Context const&, uint8_t*, int64_t) override { return 0; }
  int64_t Length() const override { return 0; }
};

TEST(BodyStream, Rewind)
{
  TestBodyStream tb;
  EXPECT_THROW(tb.Rewind(), std::logic_error);

  std::string testDataPath(AZURE_TEST_DATA_PATH);

#if defined(AZ_PLATFORM_POSIX)
  testDataPath.append("/fileData");
  int f = open(testDataPath.data(), O_RDONLY);
  EXPECT_GE(f, 0);
#elif defined(AZ_PLATFORM_WINDOWS)
  testDataPath.append("\\fileData");
  HANDLE f = CreateFile(
      testDataPath.data(),
      GENERIC_READ,
      FILE_SHARE_READ,
      NULL,
      OPEN_EXISTING,
      FILE_FLAG_SEQUENTIAL_SCAN,
      NULL);
  EXPECT_NE(f, INVALID_HANDLE_VALUE);
#else
#error "Unknown platform"
#endif
  auto fileBodyStream = Azure::Core::Http::FileBodyStream(f, 0, 0);
  EXPECT_NO_THROW(fileBodyStream.Rewind());

  std::vector<uint8_t> data = {1, 2, 3, 4};
  Azure::Core::Http::MemoryBodyStream ms(data);
  EXPECT_NO_THROW(ms.Rewind());
}
