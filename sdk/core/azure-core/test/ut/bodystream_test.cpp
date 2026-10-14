// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

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

#include <azure/core/io/body_stream.hpp>

#include <gtest/gtest.h>

using namespace Azure::Core::IO;
using namespace Azure::Core;

// Used to test virtual, default behavior of BodyStream.
class TestBodyStream final : public BodyStream {
  size_t OnRead(uint8_t*, size_t, Context const&) override { return 0; }
  int64_t Length() const override { return 0; }
};

TEST(BodyStream, Rewind)
{
  TestBodyStream tb;

#if GTEST_HAS_DEATH_TEST
#if defined(NDEBUG)
  // Release build won't provide assert msg
  ASSERT_DEATH(tb.Rewind(), "");
#else
  ASSERT_DEATH(
      tb.Rewind(),
      "The specified BodyStream doesn't support Rewind which is required to guarantee fault ");
#endif
#endif

  std::string testDataPath(AZURE_TEST_DATA_PATH);

#if defined(AZ_PLATFORM_POSIX)
  testDataPath.append("/fileData");
#elif defined(AZ_PLATFORM_WINDOWS)
  testDataPath.append("\\fileData");
#else
#error "Unknown platform"
#endif

  Azure::Core::IO::FileBodyStream fileBodyStream(testDataPath);
  EXPECT_NO_THROW(fileBodyStream.Rewind());

  std::vector<uint8_t> data = {1, 2, 3, 4};
  Azure::Core::IO::MemoryBodyStream ms(data);
  EXPECT_NO_THROW(ms.Rewind());
}

#if GTEST_HAS_DEATH_TEST
TEST(BodyStream, BadInput)
{
  TestBodyStream tb;
  ASSERT_DEATH(tb.Read(NULL, 1), "");
  ASSERT_DEATH(tb.Read(NULL, 1, Azure::Core::Context{}), "");
  ASSERT_DEATH(tb.ReadToCount(NULL, 1), "");
  ASSERT_DEATH(tb.ReadToCount(NULL, 1, Azure::Core::Context{}), "");
}

TEST(MemoryBodyStream, BadInput) { ASSERT_DEATH(MemoryBodyStream(NULL, 1), ""); }
#endif

TEST(FileBodyStream, BadInput)
{
#if GTEST_HAS_DEATH_TEST
#if defined(NDEBUG)
  // Release build won't provide assert msg
  ASSERT_DEATH(FileBodyStream(""), "");
#else
  ASSERT_DEATH(FileBodyStream(""), "The file name must not be an empty string.");
#endif
#endif

  EXPECT_THROW(Azure::Core::IO::FileBodyStream("FileNotFound"), std::runtime_error);
}

constexpr int64_t FileSize = 1024 * 100;

TEST(FileBodyStream, Length)
{
  std::string testDataPath(AZURE_TEST_DATA_PATH);
  testDataPath.append("/fileData");

  Azure::Core::IO::FileBodyStream stream(testDataPath);
  EXPECT_EQ(stream.Length(), FileSize);

  auto readResult = stream.ReadToEnd(Azure::Core::Context{});
  EXPECT_EQ(readResult.size(), FileSize);

  stream.Rewind();
  EXPECT_EQ(stream.Length(), FileSize);
}

TEST(FileBodyStream, Read)
{
  std::string testDataPath(AZURE_TEST_DATA_PATH);
  testDataPath.append("/fileData");

  Azure::Core::IO::FileBodyStream stream(testDataPath);

  // ReadToEnd
  auto readResult = stream.ReadToEnd();
  EXPECT_EQ(readResult.size(), FileSize);

  stream.Rewind();

  readResult = stream.ReadToEnd(Azure::Core::Context{});
  EXPECT_EQ(readResult.size(), FileSize);

  stream.Rewind();

  // ReadToCount
  std::vector<uint8_t> buffer(FileSize * 2);

  size_t readSize = stream.ReadToCount(buffer.data(), 10);
  EXPECT_EQ(readSize, 10);
  EXPECT_EQ(buffer[10], 0);

  stream.Rewind();

  readSize = stream.ReadToCount(buffer.data(), 10, Azure::Core::Context{});
  EXPECT_EQ(readSize, 10);
  EXPECT_EQ(buffer[10], 0);

  stream.Rewind();

  // Read
  readSize = stream.Read(buffer.data(), buffer.size());
  EXPECT_EQ(readSize, FileSize);
  EXPECT_EQ(buffer[FileSize], 0);

  stream.Rewind();

  readSize = stream.Read(buffer.data(), buffer.size(), Azure::Core::Context{});
  EXPECT_EQ(readSize, FileSize);
  EXPECT_EQ(buffer[FileSize], 0);
}

TEST(ProgressBodyStream, Init)
{
  int64_t bytesTransferred = -1;
  std::string testDataPath(AZURE_TEST_DATA_PATH);
  testDataPath.append("/fileData");

  Azure::Core::IO::FileBodyStream stream(testDataPath);

  ProgressBodyStream progress(stream, [&bytesTransferred](int64_t bt) { bytesTransferred = bt; });

  EXPECT_EQ(bytesTransferred, -1);
  EXPECT_EQ(progress.Length(), stream.Length());
}

TEST(ProgressBodyStream, ReadChunk)
{
  int64_t bytesTransferred = -1;
  std::string testDataPath(AZURE_TEST_DATA_PATH);
  testDataPath.append("/fileData");

  Azure::Core::IO::FileBodyStream stream(testDataPath);

  ProgressBodyStream progress(stream, [&bytesTransferred](int64_t bt) { bytesTransferred = bt; });

  std::vector<uint8_t> buffer(30);

  size_t readSize = progress.ReadToCount(buffer.data(), 10);

  EXPECT_EQ(bytesTransferred, 10);
  EXPECT_EQ(progress.Length(), stream.Length());
  EXPECT_EQ(readSize, 10);
}

TEST(ProgressBodyStream, MultiWrapProgressStream)
{
  int64_t bytesTransferred = -1;
  int64_t wrapBytesTransferred = -1;
  std::string testDataPath(AZURE_TEST_DATA_PATH);
  testDataPath.append("/fileData");

  Azure::Core::IO::FileBodyStream stream(testDataPath);

  ProgressBodyStream progress(stream, [&bytesTransferred](int64_t bt) { bytesTransferred = bt; });
  ProgressBodyStream progress2(
      progress, [&wrapBytesTransferred](int64_t bt) { wrapBytesTransferred = bt; });
  std::vector<uint8_t> buffer(30);

  size_t readSize = progress2.ReadToCount(buffer.data(), 10);

  EXPECT_EQ(bytesTransferred, 10);
  EXPECT_EQ(progress.Length(), stream.Length());
  EXPECT_EQ(readSize, 10);

  EXPECT_EQ(wrapBytesTransferred, 10);
  EXPECT_EQ(progress2.Length(), stream.Length());
}

TEST(ProgressBodyStream, ReadMultipleChunks)
{
  int64_t bytesTransferred = -1;
  std::string testDataPath(AZURE_TEST_DATA_PATH);
  testDataPath.append("/fileData");

  Azure::Core::IO::FileBodyStream stream(testDataPath);

  ProgressBodyStream progress(stream, [&bytesTransferred](int64_t bt) { bytesTransferred = bt; });

  std::vector<uint8_t> buffer(10);

  for (int i = 0; i < stream.Length() / 10; i++)
  {
    size_t readSize = progress.ReadToCount(buffer.data(), 10);

    EXPECT_EQ(bytesTransferred, (i + 1) * 10);
    EXPECT_EQ(progress.Length(), stream.Length());
    EXPECT_EQ(readSize, 10);
  }
}

TEST(ProgressBodyStream, ReadMultipleChunksRewind)
{
  int64_t bytesTransferred = -1;
  std::string testDataPath(AZURE_TEST_DATA_PATH);
  testDataPath.append("/fileData");

  Azure::Core::IO::FileBodyStream stream(testDataPath);

  ProgressBodyStream progress(stream, [&bytesTransferred](int64_t bt) { bytesTransferred = bt; });

  std::vector<uint8_t> buffer(10);

  for (int i = 0; i < stream.Length() / 100; i++)
  {

    size_t readSize = progress.ReadToCount(buffer.data(), 10);

    EXPECT_EQ(bytesTransferred, (i + 1) * 10);
    EXPECT_EQ(progress.Length(), stream.Length());
    EXPECT_EQ(readSize, 10);
  }

  progress.Rewind();

  EXPECT_EQ(bytesTransferred, 0);
  EXPECT_EQ(progress.Length(), stream.Length());

  for (int i = 0; i < stream.Length() / 100; i++)
  {

    size_t readSize = progress.ReadToCount(buffer.data(), 10);

    EXPECT_EQ(bytesTransferred, (i + 1) * 10);
    EXPECT_EQ(progress.Length(), stream.Length());
    EXPECT_EQ(readSize, 10);
  }
}

TEST(MemoryBodyStream, TryReadContiguous)
{
  std::vector<uint8_t> const data = {10, 20, 30, 40, 50};
  MemoryBodyStream stream(data);

  uint8_t const* chunkPtr = nullptr;
  size_t bytesRead = 0;

  // First partial read returns exact pointer into underlying vector.
  EXPECT_TRUE(
      _detail::BodyStreamHelpers::TryReadContiguous(stream, chunkPtr, 3, bytesRead, Context{}));
  EXPECT_EQ(bytesRead, 3u);
  EXPECT_EQ(chunkPtr, data.data());

  // Second read returns remaining bytes starting at offset 3.
  EXPECT_TRUE(
      _detail::BodyStreamHelpers::TryReadContiguous(stream, chunkPtr, 3, bytesRead, Context{}));
  EXPECT_EQ(bytesRead, 2u);
  EXPECT_EQ(chunkPtr, data.data() + 3);

  // At EOF, still returns true with 0 bytesRead.
  EXPECT_TRUE(
      _detail::BodyStreamHelpers::TryReadContiguous(stream, chunkPtr, 3, bytesRead, Context{}));
  EXPECT_EQ(bytesRead, 0u);

  // Rewind resets offset for retries.
  stream.Rewind();
  EXPECT_TRUE(
      _detail::BodyStreamHelpers::TryReadContiguous(stream, chunkPtr, 10, bytesRead, Context{}));
  EXPECT_EQ(bytesRead, data.size());
  EXPECT_EQ(chunkPtr, data.data());

  // Empty stream returns true with 0 bytesRead and nullptr.
  MemoryBodyStream emptyStream(nullptr, 0);
  EXPECT_TRUE(
      _detail::BodyStreamHelpers::TryReadContiguous(
          emptyStream, chunkPtr, 10, bytesRead, Context{}));
  EXPECT_EQ(bytesRead, 0u);
  EXPECT_EQ(chunkPtr, nullptr);

  // Cancelled context throws OperationCancelledException.
  Context cancelledContext;
  cancelledContext.Cancel();
  stream.Rewind();
  EXPECT_THROW(
      _detail::BodyStreamHelpers::TryReadContiguous(
          stream, chunkPtr, 10, bytesRead, cancelledContext),
      Azure::Core::OperationCancelledException);
}

TEST(BodyStream, TryReadContiguousNonContiguous)
{
  TestBodyStream testStream;
  uint8_t const* chunkPtr = nullptr;
  size_t bytesRead = 0;
  EXPECT_FALSE(
      _detail::BodyStreamHelpers::TryReadContiguous(
          testStream, chunkPtr, 10, bytesRead, Context{}));

  std::string testDataPath(AZURE_TEST_DATA_PATH);
#if defined(AZ_PLATFORM_POSIX)
  testDataPath.append("/fileData");
#elif defined(AZ_PLATFORM_WINDOWS)
  testDataPath.append("\\fileData");
#endif
  FileBodyStream fileStream(testDataPath);
  EXPECT_FALSE(
      _detail::BodyStreamHelpers::TryReadContiguous(
          fileStream, chunkPtr, 10, bytesRead, Context{}));

  std::vector<uint8_t> const data = {1, 2, 3, 4};
  MemoryBodyStream memStream(data);
  ProgressBodyStream progressStream(memStream, [](int64_t) {});
  EXPECT_FALSE(
      _detail::BodyStreamHelpers::TryReadContiguous(
          progressStream, chunkPtr, 10, bytesRead, Context{}));
}

TEST(BodyStream, ScopedScratchBufferReuseAndReentrancy)
{
  uint8_t* firstPtr = nullptr;
  {
    _detail::BodyStreamHelpers::ScopedScratchBuffer buf1(1024);
    ASSERT_NE(buf1.Get(), nullptr);
    firstPtr = buf1.Get();
    firstPtr[0] = 0xAB;

    // Nested acquisition on the same thread must use a distinct fallback buffer.
    _detail::BodyStreamHelpers::ScopedScratchBuffer nestedBuf(1024);
    ASSERT_NE(nestedBuf.Get(), nullptr);
    EXPECT_NE(nestedBuf.Get(), firstPtr);
  }

  // Subsequent acquisition on the same thread reuses the thread-local buffer up to 64 KiB.
  {
    _detail::BodyStreamHelpers::ScopedScratchBuffer buf2(64 * 1024);
    EXPECT_EQ(buf2.Get(), firstPtr);
  }
}
