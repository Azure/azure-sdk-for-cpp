// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "test_base.hpp"

#include <azure/storage/common/internal/structured_message_decoding_stream.hpp>
#include <azure/storage/common/internal/structured_message_encoding_stream.hpp>
#include <azure/storage/common/storage_exception.hpp>

namespace Azure { namespace Storage { namespace Test {

  class StructuredMessageTest : public StorageTest {
  };

  std::vector<uint8_t> ReadToEnd(Azure::Core::IO::BodyStream& stream, const size_t chunkSize)
  {
    auto buffer = std::vector<uint8_t>();

    for (auto chunkNumber = 0;; chunkNumber++)
    {
      buffer.resize((static_cast<decltype(buffer)::size_type>(chunkNumber) + 1) * chunkSize);
      size_t readBytes = stream.ReadToCount(buffer.data() + (chunkNumber * chunkSize), chunkSize);

      if (readBytes < chunkSize)
      {
        buffer.resize(static_cast<size_t>((chunkNumber * chunkSize) + readBytes));
        return buffer;
      }
    }
  }

  // Helper to encode content into a structured message
  std::vector<uint8_t> EncodeContent(
      std::vector<uint8_t> const& content,
      _internal::StructuredMessageFlags flags,
      int64_t maxSegmentLength)
  {
    auto innerStream
        = std::make_unique<Azure::Core::IO::MemoryBodyStream>(content.data(), content.size());
    _internal::StructuredMessageEncodingStreamOptions encodingOptions;
    encodingOptions.Flags = flags;
    encodingOptions.MaxSegmentLength = maxSegmentLength;
    _internal::StructuredMessageEncodingStream encodingStream(innerStream.get(), encodingOptions);
    return encodingStream.ReadToEnd();
  }

  // Helper to create a decoding stream from encoded data
  _internal::StructuredMessageDecodingStream CreateDecodingStream(
      std::vector<uint8_t> const& encodedData,
      size_t contentLength)
  {
    auto innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        encodedData.data(), encodedData.size());
    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = static_cast<int64_t>(contentLength);
    return _internal::StructuredMessageDecodingStream(std::move(innerStream), decodingOptions);
  }

  TEST_F(StructuredMessageTest, BasicFunction)
  {
    const size_t contentSize = 2 * 1024 + 512;
    auto content = RandomBuffer(contentSize);
    auto innerStream
        = std::make_unique<Azure::Core::IO::MemoryBodyStream>(content.data(), content.size());

    _internal::StructuredMessageEncodingStreamOptions encodingOptions;
    encodingOptions.Flags = _internal::StructuredMessageFlags::Crc64;
    encodingOptions.MaxSegmentLength = 1024;
    _internal::StructuredMessageEncodingStream encodingStream(innerStream.get(), encodingOptions);
    std::vector<uint8_t> encodedData;
    EXPECT_NO_THROW(encodedData = encodingStream.ReadToEnd());

    innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        encodedData.data(), encodedData.size());
    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = contentSize;
    _internal::StructuredMessageDecodingStream decodingStream(
        std::move(innerStream), decodingOptions);
    std::vector<uint8_t> decodedData;
    EXPECT_NO_THROW(decodedData = decodingStream.ReadToEnd());

    EXPECT_EQ(content, decodedData);
  }

  TEST_F(StructuredMessageTest, SmallSegment)
  {
    const size_t contentSize = 2 * 1024 * 1024 + 5122;
    auto content = RandomBuffer(contentSize);
    auto innerStream
        = std::make_unique<Azure::Core::IO::MemoryBodyStream>(content.data(), content.size());

    _internal::StructuredMessageEncodingStreamOptions encodingOptions;
    encodingOptions.Flags = _internal::StructuredMessageFlags::Crc64;
    encodingOptions.MaxSegmentLength = 33;
    _internal::StructuredMessageEncodingStream encodingStream(innerStream.get(), encodingOptions);
    std::vector<uint8_t> encodedData;
    EXPECT_NO_THROW(encodedData = ReadToEnd(encodingStream, 4096));

    innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        encodedData.data(), encodedData.size());
    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = contentSize;
    _internal::StructuredMessageDecodingStream decodingStream(
        std::move(innerStream), decodingOptions);
    std::vector<uint8_t> decodedData;
    EXPECT_NO_THROW(decodedData = ReadToEnd(decodingStream, 513));

    EXPECT_EQ(content, decodedData);
  }

  TEST_F(StructuredMessageTest, ReadSmallRange)
  {
    const size_t contentSize = 2 * 1024 + 512;
    auto content = RandomBuffer(contentSize);
    auto innerStream
        = std::make_unique<Azure::Core::IO::MemoryBodyStream>(content.data(), content.size());

    _internal::StructuredMessageEncodingStreamOptions encodingOptions;
    encodingOptions.Flags = _internal::StructuredMessageFlags::Crc64;
    encodingOptions.MaxSegmentLength = 1024;
    _internal::StructuredMessageEncodingStream encodingStream(innerStream.get(), encodingOptions);
    std::vector<uint8_t> encodedData;
    EXPECT_NO_THROW(encodedData = ReadToEnd(encodingStream, 7));

    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = contentSize;
    _internal::StructuredMessageDecodingStream decodingStream(
        std::make_unique<Azure::Core::IO::MemoryBodyStream>(encodedData.data(), encodedData.size()),
        decodingOptions);
    std::vector<uint8_t> decodedData;
    EXPECT_NO_THROW(decodedData = ReadToEnd(decodingStream, 7));

    EXPECT_EQ(content, decodedData);

    // Large encode range
    EXPECT_NO_THROW(encodingStream.Rewind());
    EXPECT_NO_THROW(encodedData = ReadToEnd(encodingStream, 4096));
    _internal::StructuredMessageDecodingStream decodingStream1(
        std::make_unique<Azure::Core::IO::MemoryBodyStream>(encodedData.data(), encodedData.size()),
        decodingOptions);
    EXPECT_NO_THROW(decodedData = ReadToEnd(decodingStream1, 5));

    EXPECT_EQ(content, decodedData);

    EXPECT_NO_THROW(decodingStream1.Rewind());
    EXPECT_NO_THROW(decodedData = ReadToEnd(decodingStream1, 6));

    EXPECT_EQ(content, decodedData);

    // Large decode range
    EXPECT_NO_THROW(encodingStream.Rewind());
    EXPECT_NO_THROW(encodedData = ReadToEnd(encodingStream, 8));
    _internal::StructuredMessageDecodingStream decodingStream2(
        std::make_unique<Azure::Core::IO::MemoryBodyStream>(encodedData.data(), encodedData.size()),
        decodingOptions);
    EXPECT_NO_THROW(decodedData = ReadToEnd(decodingStream2, 4096));

    EXPECT_EQ(content, decodedData);
  }

  TEST_F(StructuredMessageTest, ReadBigRange)
  {
    const size_t contentSize = 4 * 1024 * 1024 + 2 * 1024 + 512 + 3;
    auto content = RandomBuffer(contentSize);
    auto innerStream
        = std::make_unique<Azure::Core::IO::MemoryBodyStream>(content.data(), content.size());

    _internal::StructuredMessageEncodingStreamOptions encodingOptions;
    encodingOptions.Flags = _internal::StructuredMessageFlags::Crc64;
    _internal::StructuredMessageEncodingStream encodingStream(innerStream.get(), encodingOptions);
    std::vector<uint8_t> encodedData;
    EXPECT_NO_THROW(encodedData = ReadToEnd(encodingStream, 4 * 1024 * 1024));

    innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        encodedData.data(), encodedData.size());
    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = contentSize;
    _internal::StructuredMessageDecodingStream decodingStream(
        std::move(innerStream), decodingOptions);
    std::vector<uint8_t> decodedData;
    EXPECT_NO_THROW(decodedData = ReadToEnd(decodingStream, 4 * 1024 * 1024));

    EXPECT_EQ(content, decodedData);
  }

  TEST_F(StructuredMessageTest, NotCrc64)
  {
    const size_t contentSize = 2 * 1024 + 512;
    auto content = RandomBuffer(contentSize);
    auto innerStream
        = std::make_unique<Azure::Core::IO::MemoryBodyStream>(content.data(), content.size());

    _internal::StructuredMessageEncodingStreamOptions encodingOptions;
    encodingOptions.Flags = _internal::StructuredMessageFlags::None;
    encodingOptions.MaxSegmentLength = 1024;
    _internal::StructuredMessageEncodingStream encodingStream(innerStream.get(), encodingOptions);
    std::vector<uint8_t> encodedData;
    EXPECT_NO_THROW(encodedData = ReadToEnd(encodingStream, 4096));

    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = contentSize;
    innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        encodedData.data(), encodedData.size());
    _internal::StructuredMessageDecodingStream decodingStream(
        std::move(innerStream), decodingOptions);
    std::vector<uint8_t> decodedData;
    EXPECT_NO_THROW(decodedData = ReadToEnd(decodingStream, 4096));

    EXPECT_EQ(content, decodedData);
  }

  TEST_F(StructuredMessageTest, NotCrc64SmallRange)
  {
    const size_t contentSize = 2 * 1024 + 512;
    auto content = RandomBuffer(contentSize);
    auto innerStream
        = std::make_unique<Azure::Core::IO::MemoryBodyStream>(content.data(), content.size());

    _internal::StructuredMessageEncodingStreamOptions encodingOptions;
    encodingOptions.Flags = _internal::StructuredMessageFlags::None;
    encodingOptions.MaxSegmentLength = 1024;
    _internal::StructuredMessageEncodingStream encodingStream(innerStream.get(), encodingOptions);
    std::vector<uint8_t> encodedData;
    EXPECT_NO_THROW(encodedData = ReadToEnd(encodingStream, 7));

    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = contentSize;
    _internal::StructuredMessageDecodingStream decodingStream(
        std::make_unique<Azure::Core::IO::MemoryBodyStream>(encodedData.data(), encodedData.size()),
        decodingOptions);
    std::vector<uint8_t> decodedData;
    EXPECT_NO_THROW(decodedData = ReadToEnd(decodingStream, 7));

    EXPECT_EQ(content, decodedData);

    // Large encode range
    EXPECT_NO_THROW(encodingStream.Rewind());
    EXPECT_NO_THROW(encodedData = ReadToEnd(encodingStream, 4096));
    _internal::StructuredMessageDecodingStream decodingStream1(
        std::make_unique<Azure::Core::IO::MemoryBodyStream>(encodedData.data(), encodedData.size()),
        decodingOptions);
    EXPECT_NO_THROW(decodedData = ReadToEnd(decodingStream1, 5));

    EXPECT_EQ(content, decodedData);

    EXPECT_NO_THROW(decodingStream1.Rewind());
    EXPECT_NO_THROW(decodedData = ReadToEnd(decodingStream1, 6));

    EXPECT_EQ(content, decodedData);

    // Large decode range
    EXPECT_NO_THROW(encodingStream.Rewind());
    EXPECT_NO_THROW(encodedData = ReadToEnd(encodingStream, 8));
    _internal::StructuredMessageDecodingStream decodingStream2(
        std::make_unique<Azure::Core::IO::MemoryBodyStream>(encodedData.data(), encodedData.size()),
        decodingOptions);
    EXPECT_NO_THROW(decodedData = ReadToEnd(decodingStream2, 4096));

    EXPECT_EQ(content, decodedData);
  }

  TEST_F(StructuredMessageTest, EmptyContent)
  {
    // Test encoding and decoding empty content
    const size_t contentSize = 0;
    std::vector<uint8_t> content;
    auto innerStream
        = std::make_unique<Azure::Core::IO::MemoryBodyStream>(content.data(), content.size());

    _internal::StructuredMessageEncodingStreamOptions encodingOptions;
    encodingOptions.Flags = _internal::StructuredMessageFlags::Crc64;
    encodingOptions.MaxSegmentLength = 1024;
    _internal::StructuredMessageEncodingStream encodingStream(innerStream.get(), encodingOptions);
    std::vector<uint8_t> encodedData;
    EXPECT_NO_THROW(encodedData = encodingStream.ReadToEnd());

    // Verify encoded data has headers/footers but no content segments
    EXPECT_GT(encodedData.size(), 0);

    innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        encodedData.data(), encodedData.size());
    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = contentSize;
    _internal::StructuredMessageDecodingStream decodingStream(
        std::move(innerStream), decodingOptions);
    std::vector<uint8_t> decodedData;
    EXPECT_NO_THROW(decodedData = decodingStream.ReadToEnd());

    EXPECT_EQ(content, decodedData);
    EXPECT_EQ(decodedData.size(), 0);
  }

  TEST_F(StructuredMessageTest, SingleByteContent)
  {
    // Test with minimal content
    const size_t contentSize = 1;
    auto content = RandomBuffer(contentSize);
    auto innerStream
        = std::make_unique<Azure::Core::IO::MemoryBodyStream>(content.data(), content.size());

    _internal::StructuredMessageEncodingStreamOptions encodingOptions;
    encodingOptions.Flags = _internal::StructuredMessageFlags::Crc64;
    encodingOptions.MaxSegmentLength = 1024;
    _internal::StructuredMessageEncodingStream encodingStream(innerStream.get(), encodingOptions);
    std::vector<uint8_t> encodedData;
    EXPECT_NO_THROW(encodedData = encodingStream.ReadToEnd());

    innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        encodedData.data(), encodedData.size());
    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = contentSize;
    _internal::StructuredMessageDecodingStream decodingStream(
        std::move(innerStream), decodingOptions);
    std::vector<uint8_t> decodedData;
    EXPECT_NO_THROW(decodedData = decodingStream.ReadToEnd());

    EXPECT_EQ(content, decodedData);
  }

  TEST_F(StructuredMessageTest, ExactlyOneSegment)
  {
    // Test content that exactly fills one segment
    const size_t contentSize = 1024;
    auto content = RandomBuffer(contentSize);
    auto innerStream
        = std::make_unique<Azure::Core::IO::MemoryBodyStream>(content.data(), content.size());

    _internal::StructuredMessageEncodingStreamOptions encodingOptions;
    encodingOptions.Flags = _internal::StructuredMessageFlags::Crc64;
    encodingOptions.MaxSegmentLength = 1024;
    _internal::StructuredMessageEncodingStream encodingStream(innerStream.get(), encodingOptions);
    std::vector<uint8_t> encodedData;
    EXPECT_NO_THROW(encodedData = encodingStream.ReadToEnd());

    innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        encodedData.data(), encodedData.size());
    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = contentSize;
    _internal::StructuredMessageDecodingStream decodingStream(
        std::move(innerStream), decodingOptions);
    std::vector<uint8_t> decodedData;
    EXPECT_NO_THROW(decodedData = decodingStream.ReadToEnd());

    EXPECT_EQ(content, decodedData);
  }

  TEST_F(StructuredMessageTest, ContentOneByteOverSegment)
  {
    // Test content that's one byte over segment boundary
    const size_t contentSize = 1025;
    auto content = RandomBuffer(contentSize);
    auto innerStream
        = std::make_unique<Azure::Core::IO::MemoryBodyStream>(content.data(), content.size());

    _internal::StructuredMessageEncodingStreamOptions encodingOptions;
    encodingOptions.Flags = _internal::StructuredMessageFlags::Crc64;
    encodingOptions.MaxSegmentLength = 1024;
    _internal::StructuredMessageEncodingStream encodingStream(innerStream.get(), encodingOptions);
    std::vector<uint8_t> encodedData;
    EXPECT_NO_THROW(encodedData = encodingStream.ReadToEnd());

    innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        encodedData.data(), encodedData.size());
    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = contentSize;
    _internal::StructuredMessageDecodingStream decodingStream(
        std::move(innerStream), decodingOptions);
    std::vector<uint8_t> decodedData;
    EXPECT_NO_THROW(decodedData = decodingStream.ReadToEnd());

    EXPECT_EQ(content, decodedData);
  }

  TEST_F(StructuredMessageTest, MaxSegmentLengthBoundaries)
  {
    // Test with default max segment length (4MB)
    const size_t contentSize = 4 * 1024 * 1024;
    auto content = RandomBuffer(contentSize);
    auto innerStream
        = std::make_unique<Azure::Core::IO::MemoryBodyStream>(content.data(), content.size());

    _internal::StructuredMessageEncodingStreamOptions encodingOptions;
    encodingOptions.Flags = _internal::StructuredMessageFlags::Crc64;
    // Use default MaxSegmentLength
    _internal::StructuredMessageEncodingStream encodingStream(innerStream.get(), encodingOptions);
    std::vector<uint8_t> encodedData;
    EXPECT_NO_THROW(encodedData = encodingStream.ReadToEnd());

    innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        encodedData.data(), encodedData.size());
    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = contentSize;
    _internal::StructuredMessageDecodingStream decodingStream(
        std::move(innerStream), decodingOptions);
    std::vector<uint8_t> decodedData;
    EXPECT_NO_THROW(decodedData = decodingStream.ReadToEnd());

    EXPECT_EQ(content, decodedData);
  }

  TEST_F(StructuredMessageTest, LargeSegmentLength)
  {
    // Test with very large segment length
    const size_t contentSize = 8 * 1024 * 1024 + 123;
    auto content = RandomBuffer(contentSize);
    auto innerStream
        = std::make_unique<Azure::Core::IO::MemoryBodyStream>(content.data(), content.size());

    _internal::StructuredMessageEncodingStreamOptions encodingOptions;
    encodingOptions.Flags = _internal::StructuredMessageFlags::Crc64;
    encodingOptions.MaxSegmentLength = 10 * 1024 * 1024;
    _internal::StructuredMessageEncodingStream encodingStream(innerStream.get(), encodingOptions);
    std::vector<uint8_t> encodedData;
    EXPECT_NO_THROW(encodedData = encodingStream.ReadToEnd());

    innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        encodedData.data(), encodedData.size());
    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = contentSize;
    _internal::StructuredMessageDecodingStream decodingStream(
        std::move(innerStream), decodingOptions);
    std::vector<uint8_t> decodedData;
    EXPECT_NO_THROW(decodedData = decodingStream.ReadToEnd());

    EXPECT_EQ(content, decodedData);
  }

  TEST_F(StructuredMessageTest, MultipleRewinds)
  {
    // Test multiple rewind operations
    const size_t contentSize = 1024 + 512;
    auto content = RandomBuffer(contentSize);
    auto innerStream
        = std::make_unique<Azure::Core::IO::MemoryBodyStream>(content.data(), content.size());

    _internal::StructuredMessageEncodingStreamOptions encodingOptions;
    encodingOptions.Flags = _internal::StructuredMessageFlags::Crc64;
    encodingOptions.MaxSegmentLength = 1024;
    _internal::StructuredMessageEncodingStream encodingStream(innerStream.get(), encodingOptions);

    // First read
    std::vector<uint8_t> encodedData1;
    EXPECT_NO_THROW(encodedData1 = encodingStream.ReadToEnd());

    // Rewind and read again
    EXPECT_NO_THROW(encodingStream.Rewind());
    std::vector<uint8_t> encodedData2;
    EXPECT_NO_THROW(encodedData2 = encodingStream.ReadToEnd());

    // Rewind and read third time
    EXPECT_NO_THROW(encodingStream.Rewind());
    std::vector<uint8_t> encodedData3;
    EXPECT_NO_THROW(encodedData3 = encodingStream.ReadToEnd());

    EXPECT_EQ(encodedData1, encodedData2);
    EXPECT_EQ(encodedData2, encodedData3);

    // Decode and verify
    innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        encodedData1.data(), encodedData1.size());
    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = contentSize;
    _internal::StructuredMessageDecodingStream decodingStream(
        std::move(innerStream), decodingOptions);
    std::vector<uint8_t> decodedData;
    EXPECT_NO_THROW(decodedData = decodingStream.ReadToEnd());

    EXPECT_EQ(content, decodedData);
  }

  TEST_F(StructuredMessageTest, ReadInOddSizedChunks)
  {
    // Test reading with various odd-sized chunks
    const size_t contentSize = 3 * 1024 + 777;
    auto content = RandomBuffer(contentSize);
    auto innerStream
        = std::make_unique<Azure::Core::IO::MemoryBodyStream>(content.data(), content.size());

    _internal::StructuredMessageEncodingStreamOptions encodingOptions;
    encodingOptions.Flags = _internal::StructuredMessageFlags::Crc64;
    encodingOptions.MaxSegmentLength = 1024;
    _internal::StructuredMessageEncodingStream encodingStream(innerStream.get(), encodingOptions);
    std::vector<uint8_t> encodedData;
    EXPECT_NO_THROW(encodedData = ReadToEnd(encodingStream, 137));

    innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        encodedData.data(), encodedData.size());
    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = contentSize;
    _internal::StructuredMessageDecodingStream decodingStream(
        std::move(innerStream), decodingOptions);
    std::vector<uint8_t> decodedData;
    EXPECT_NO_THROW(decodedData = ReadToEnd(decodingStream, 193));

    EXPECT_EQ(content, decodedData);
  }

  TEST_F(StructuredMessageTest, LengthProperty)
  {
    // Test that Length() returns correct values
    const size_t contentSize = 2 * 1024 + 512;
    auto content = RandomBuffer(contentSize);
    auto innerStream
        = std::make_unique<Azure::Core::IO::MemoryBodyStream>(content.data(), content.size());

    _internal::StructuredMessageEncodingStreamOptions encodingOptions;
    encodingOptions.Flags = _internal::StructuredMessageFlags::Crc64;
    encodingOptions.MaxSegmentLength = 1024;
    _internal::StructuredMessageEncodingStream encodingStream(innerStream.get(), encodingOptions);

    // Encoded length should be greater than content length (due to headers/footers)
    EXPECT_GT(encodingStream.Length(), static_cast<int64_t>(contentSize));

    std::vector<uint8_t> encodedData;
    EXPECT_NO_THROW(encodedData = encodingStream.ReadToEnd());
    EXPECT_EQ(encodedData.size(), static_cast<size_t>(encodingStream.Length()));

    innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        encodedData.data(), encodedData.size());
    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = contentSize;
    _internal::StructuredMessageDecodingStream decodingStream(
        std::move(innerStream), decodingOptions);

    // Decoded length should match original content length
    EXPECT_EQ(decodingStream.Length(), static_cast<int64_t>(contentSize));
  }

  TEST_F(StructuredMessageTest, VeryLargeContent)
  {
    // Test with large content to ensure scalability
    const size_t contentSize = 16 * 1024 * 1024 + 1234;
    auto content = RandomBuffer(contentSize);
    auto innerStream
        = std::make_unique<Azure::Core::IO::MemoryBodyStream>(content.data(), content.size());

    _internal::StructuredMessageEncodingStreamOptions encodingOptions;
    encodingOptions.Flags = _internal::StructuredMessageFlags::Crc64;
    encodingOptions.MaxSegmentLength = 2 * 1024 * 1024;
    _internal::StructuredMessageEncodingStream encodingStream(innerStream.get(), encodingOptions);
    std::vector<uint8_t> encodedData;
    EXPECT_NO_THROW(encodedData = ReadToEnd(encodingStream, 1024 * 1024));

    innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        encodedData.data(), encodedData.size());
    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = contentSize;
    _internal::StructuredMessageDecodingStream decodingStream(
        std::move(innerStream), decodingOptions);
    std::vector<uint8_t> decodedData;
    EXPECT_NO_THROW(decodedData = ReadToEnd(decodingStream, 1024 * 1024));

    EXPECT_EQ(content, decodedData);
  }

  TEST_F(StructuredMessageTest, MixedChunkSizesEncodeAndDecode)
  {
    // Test with different chunk sizes for encoding and decoding
    const size_t contentSize = 5 * 1024 + 321;
    auto content = RandomBuffer(contentSize);
    auto innerStream
        = std::make_unique<Azure::Core::IO::MemoryBodyStream>(content.data(), content.size());

    _internal::StructuredMessageEncodingStreamOptions encodingOptions;
    encodingOptions.Flags = _internal::StructuredMessageFlags::Crc64;
    encodingOptions.MaxSegmentLength = 1024;
    _internal::StructuredMessageEncodingStream encodingStream(innerStream.get(), encodingOptions);

    // Read with large chunks
    std::vector<uint8_t> encodedData;
    EXPECT_NO_THROW(encodedData = ReadToEnd(encodingStream, 8192));

    // Decode with small chunks
    innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        encodedData.data(), encodedData.size());
    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = contentSize;
    _internal::StructuredMessageDecodingStream decodingStream(
        std::move(innerStream), decodingOptions);
    std::vector<uint8_t> decodedData;
    EXPECT_NO_THROW(decodedData = ReadToEnd(decodingStream, 11));

    EXPECT_EQ(content, decodedData);
  }

  TEST_F(StructuredMessageTest, NoCrc64WithVaryingSizes)
  {
    // Test without CRC64 across multiple sizes
    std::vector<size_t> testSizes = {1, 128, 1024, 1025, 4096, 1024 * 1024};

    for (auto contentSize : testSizes)
    {
      auto content = RandomBuffer(contentSize);
      auto innerStream
          = std::make_unique<Azure::Core::IO::MemoryBodyStream>(content.data(), content.size());

      _internal::StructuredMessageEncodingStreamOptions encodingOptions;
      encodingOptions.Flags = _internal::StructuredMessageFlags::None;
      encodingOptions.MaxSegmentLength = 1024;
      _internal::StructuredMessageEncodingStream encodingStream(innerStream.get(), encodingOptions);
      std::vector<uint8_t> encodedData;
      EXPECT_NO_THROW(encodedData = encodingStream.ReadToEnd());

      innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
          encodedData.data(), encodedData.size());
      _internal::StructuredMessageDecodingStreamOptions decodingOptions;
      decodingOptions.ContentLength = contentSize;
      _internal::StructuredMessageDecodingStream decodingStream(
          std::move(innerStream), decodingOptions);
      std::vector<uint8_t> decodedData;
      EXPECT_NO_THROW(decodedData = decodingStream.ReadToEnd());

      EXPECT_EQ(content, decodedData);
    }
  }

  TEST_F(StructuredMessageTest, SegmentBoundaryReads)
  {
    // Test reading exactly at segment boundaries
    const size_t segmentSize = 512;
    const size_t contentSize = segmentSize * 5;
    auto content = RandomBuffer(contentSize);
    auto innerStream
        = std::make_unique<Azure::Core::IO::MemoryBodyStream>(content.data(), content.size());

    _internal::StructuredMessageEncodingStreamOptions encodingOptions;
    encodingOptions.Flags = _internal::StructuredMessageFlags::Crc64;
    encodingOptions.MaxSegmentLength = segmentSize;
    _internal::StructuredMessageEncodingStream encodingStream(innerStream.get(), encodingOptions);
    std::vector<uint8_t> encodedData;
    EXPECT_NO_THROW(encodedData = encodingStream.ReadToEnd());

    innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        encodedData.data(), encodedData.size());
    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = contentSize;
    _internal::StructuredMessageDecodingStream decodingStream(
        std::move(innerStream), decodingOptions);

    // Read exactly segment-sized chunks
    std::vector<uint8_t> decodedData;
    EXPECT_NO_THROW(decodedData = ReadToEnd(decodingStream, segmentSize));

    EXPECT_EQ(content, decodedData);
  }

  TEST_F(StructuredMessageTest, RewindAfterPartialRead)
  {
    // Test rewinding after partial reads
    const size_t contentSize = 3 * 1024;
    auto content = RandomBuffer(contentSize);
    auto innerStream
        = std::make_unique<Azure::Core::IO::MemoryBodyStream>(content.data(), content.size());

    _internal::StructuredMessageEncodingStreamOptions encodingOptions;
    encodingOptions.Flags = _internal::StructuredMessageFlags::Crc64;
    encodingOptions.MaxSegmentLength = 1024;
    _internal::StructuredMessageEncodingStream encodingStream(innerStream.get(), encodingOptions);

    // Partial read
    std::vector<uint8_t> partialBuffer(512);
    EXPECT_NO_THROW(encodingStream.Read(partialBuffer.data(), partialBuffer.size()));

    // Rewind
    EXPECT_NO_THROW(encodingStream.Rewind());

    // Full read
    std::vector<uint8_t> encodedData;
    EXPECT_NO_THROW(encodedData = encodingStream.ReadToEnd());

    innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        encodedData.data(), encodedData.size());
    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = contentSize;
    _internal::StructuredMessageDecodingStream decodingStream(
        std::move(innerStream), decodingOptions);

    // Partial decode read
    std::vector<uint8_t> partialDecodeBuffer(256);
    EXPECT_NO_THROW(decodingStream.Read(partialDecodeBuffer.data(), partialDecodeBuffer.size()));

    // Rewind decode stream
    EXPECT_NO_THROW(decodingStream.Rewind());

    // Full decode read
    std::vector<uint8_t> decodedData;
    EXPECT_NO_THROW(decodedData = decodingStream.ReadToEnd());

    EXPECT_EQ(content, decodedData);
  }

  TEST_F(StructuredMessageTest, VerySmallSegmentWithLargeContent)
  {
    // Test with very small segments and large content (stress test)
    const size_t contentSize = 10 * 1024;
    auto content = RandomBuffer(contentSize);
    auto innerStream
        = std::make_unique<Azure::Core::IO::MemoryBodyStream>(content.data(), content.size());

    _internal::StructuredMessageEncodingStreamOptions encodingOptions;
    encodingOptions.Flags = _internal::StructuredMessageFlags::Crc64;
    encodingOptions.MaxSegmentLength = 10; // Very small segments
    _internal::StructuredMessageEncodingStream encodingStream(innerStream.get(), encodingOptions);
    std::vector<uint8_t> encodedData;
    EXPECT_NO_THROW(encodedData = ReadToEnd(encodingStream, 128));

    innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        encodedData.data(), encodedData.size());
    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = contentSize;
    _internal::StructuredMessageDecodingStream decodingStream(
        std::move(innerStream), decodingOptions);
    std::vector<uint8_t> decodedData;
    EXPECT_NO_THROW(decodedData = ReadToEnd(decodingStream, 97));

    EXPECT_EQ(content, decodedData);
  }

  TEST_F(StructuredMessageTest, AlternatingFlagsModes)
  {
    // Test both CRC64 and None flags with same content
    const size_t contentSize = 2048;
    auto content = RandomBuffer(contentSize);

    // Test with CRC64
    {
      auto innerStream
          = std::make_unique<Azure::Core::IO::MemoryBodyStream>(content.data(), content.size());
      _internal::StructuredMessageEncodingStreamOptions encodingOptions;
      encodingOptions.Flags = _internal::StructuredMessageFlags::Crc64;
      encodingOptions.MaxSegmentLength = 1024;
      _internal::StructuredMessageEncodingStream encodingStream(innerStream.get(), encodingOptions);
      std::vector<uint8_t> encodedData;
      EXPECT_NO_THROW(encodedData = encodingStream.ReadToEnd());

      innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
          encodedData.data(), encodedData.size());
      _internal::StructuredMessageDecodingStreamOptions decodingOptions;
      decodingOptions.ContentLength = contentSize;
      _internal::StructuredMessageDecodingStream decodingStream(
          std::move(innerStream), decodingOptions);
      std::vector<uint8_t> decodedData;
      EXPECT_NO_THROW(decodedData = decodingStream.ReadToEnd());

      EXPECT_EQ(content, decodedData);
    }

    // Test without CRC64
    {
      auto innerStream
          = std::make_unique<Azure::Core::IO::MemoryBodyStream>(content.data(), content.size());
      _internal::StructuredMessageEncodingStreamOptions encodingOptions;
      encodingOptions.Flags = _internal::StructuredMessageFlags::None;
      encodingOptions.MaxSegmentLength = 1024;
      _internal::StructuredMessageEncodingStream encodingStream(innerStream.get(), encodingOptions);
      std::vector<uint8_t> encodedData;
      EXPECT_NO_THROW(encodedData = encodingStream.ReadToEnd());

      innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
          encodedData.data(), encodedData.size());
      _internal::StructuredMessageDecodingStreamOptions decodingOptions;
      decodingOptions.ContentLength = contentSize;
      _internal::StructuredMessageDecodingStream decodingStream(
          std::move(innerStream), decodingOptions);
      std::vector<uint8_t> decodedData;
      EXPECT_NO_THROW(decodedData = decodingStream.ReadToEnd());

      EXPECT_EQ(content, decodedData);
    }
  }

  TEST_F(StructuredMessageTest, SingleReadReturnsAtMostOneSegment)
  {
    // With the totalContentRead == 0 loop condition, a single Read() call should return
    // at most one segment's worth of content, even if the buffer is much larger.
    const size_t segmentSize = 256;
    const size_t contentSize = segmentSize * 4; // 4 full segments
    auto content = RandomBuffer(contentSize);
    auto encodedData
        = EncodeContent(content, _internal::StructuredMessageFlags::Crc64, segmentSize);
    auto decodingStream = CreateDecodingStream(encodedData, contentSize);

    // Read with buffer much larger than segment size
    std::vector<uint8_t> readBuffer(contentSize);
    auto bytesRead = decodingStream.Read(readBuffer.data(), readBuffer.size());

    // Should return at most one segment, not the full buffer
    EXPECT_LE(bytesRead, segmentSize);
    EXPECT_GT(bytesRead, static_cast<size_t>(0));

    // But ReadToCount/ReadToEnd should still assemble the full content via multiple Read calls
    decodingStream.Rewind();
    auto decodedData = decodingStream.ReadToEnd();
    EXPECT_EQ(content, decodedData);
  }

  TEST_F(StructuredMessageTest, SingleReadReturnsAtMostOneSegment_NoCrc64)
  {
    // Same test without CRC64 to verify the loop condition applies regardless of flags
    const size_t segmentSize = 256;
    const size_t contentSize = segmentSize * 4;
    auto content = RandomBuffer(contentSize);
    auto encodedData = EncodeContent(content, _internal::StructuredMessageFlags::None, segmentSize);
    auto decodingStream = CreateDecodingStream(encodedData, contentSize);

    std::vector<uint8_t> readBuffer(contentSize);
    auto bytesRead = decodingStream.Read(readBuffer.data(), readBuffer.size());

    EXPECT_LE(bytesRead, segmentSize);
    EXPECT_GT(bytesRead, static_cast<size_t>(0));

    decodingStream.Rewind();
    auto decodedData = decodingStream.ReadToEnd();
    EXPECT_EQ(content, decodedData);
  }

  TEST_F(StructuredMessageTest, SequentialSingleReadsAccumulateCorrectly)
  {
    // Verify that calling Read() repeatedly with a large buffer correctly accumulates
    // all content one segment at a time.
    const size_t segmentSize = 128;
    const size_t contentSize = segmentSize * 5 + 37; // 5 full segments + partial
    auto content = RandomBuffer(contentSize);
    auto encodedData
        = EncodeContent(content, _internal::StructuredMessageFlags::Crc64, segmentSize);
    auto decodingStream = CreateDecodingStream(encodedData, contentSize);

    std::vector<uint8_t> accumulated;
    std::vector<uint8_t> readBuffer(contentSize);
    size_t readCount = 0;

    while (true)
    {
      auto bytesRead = decodingStream.Read(readBuffer.data(), readBuffer.size());
      if (bytesRead == 0)
        break;
      // Each Read should return at most one segment's worth
      EXPECT_LE(bytesRead, segmentSize);
      accumulated.insert(accumulated.end(), readBuffer.begin(), readBuffer.begin() + bytesRead);
      readCount++;
    }

    EXPECT_EQ(accumulated, content);
    // Should have taken at least (contentSize / segmentSize) reads (one per segment)
    EXPECT_GE(readCount, (contentSize + segmentSize - 1) / segmentSize);
  }

  TEST_F(StructuredMessageTest, ReadAfterStreamEnd)
  {
    // After all data is consumed, subsequent Read() calls should return 0.
    const size_t contentSize = 512;
    auto content = RandomBuffer(contentSize);
    auto encodedData = EncodeContent(content, _internal::StructuredMessageFlags::Crc64, 1024);
    auto decodingStream = CreateDecodingStream(encodedData, contentSize);

    auto decodedData = decodingStream.ReadToEnd();
    EXPECT_EQ(content, decodedData);

    // Reading after completion should return 0
    std::vector<uint8_t> extraBuffer(256);
    EXPECT_EQ(decodingStream.Read(extraBuffer.data(), extraBuffer.size()), static_cast<size_t>(0));
    EXPECT_EQ(decodingStream.Read(extraBuffer.data(), extraBuffer.size()), static_cast<size_t>(0));
  }

  TEST_F(StructuredMessageTest, ReadWithZeroCount)
  {
    // Read() with count=0 should return 0 without advancing state.
    const size_t contentSize = 512;
    auto content = RandomBuffer(contentSize);
    auto encodedData = EncodeContent(content, _internal::StructuredMessageFlags::Crc64, 1024);
    auto decodingStream = CreateDecodingStream(encodedData, contentSize);

    std::vector<uint8_t> buffer(1);
    EXPECT_EQ(decodingStream.Read(buffer.data(), 0), static_cast<size_t>(0));

    // Stream should still work normally after zero-count read
    auto decodedData = decodingStream.ReadToEnd();
    EXPECT_EQ(content, decodedData);
  }

  TEST_F(StructuredMessageTest, Crc64CorruptionDetected)
  {
    // Verify that CRC64 corruption in segment content is detected during decoding.
    const size_t contentSize = 2048;
    auto content = RandomBuffer(contentSize);
    auto encodedData = EncodeContent(content, _internal::StructuredMessageFlags::Crc64, 1024);

    // Corrupt a byte in the first segment's content area (after stream header + segment header)
    // Stream header = 13 bytes, segment header = 10 bytes, so content starts at offset 23
    size_t corruptOffset = 23 + 100; // somewhere within first segment content
    ASSERT_LT(corruptOffset, encodedData.size());
    encodedData[corruptOffset] ^= 0xFF;

    auto decodingStream = CreateDecodingStream(encodedData, contentSize);
    EXPECT_THROW(decodingStream.ReadToEnd(), StorageException);
  }

  TEST_F(StructuredMessageTest, SingleByteReads)
  {
    // Reading one byte at a time should still produce correct output.
    const size_t contentSize = 300;
    auto content = RandomBuffer(contentSize);
    auto encodedData = EncodeContent(content, _internal::StructuredMessageFlags::Crc64, 128);
    auto decodingStream = CreateDecodingStream(encodedData, contentSize);

    std::vector<uint8_t> decodedData;
    EXPECT_NO_THROW(decodedData = ReadToEnd(decodingStream, 1));

    EXPECT_EQ(content, decodedData);
  }

  TEST_F(StructuredMessageTest, BufferExactlyDoubleSegmentSize)
  {
    // When buffer is exactly 2x segment size, each Read still returns at most one segment.
    const size_t segmentSize = 512;
    const size_t contentSize = segmentSize * 3;
    auto content = RandomBuffer(contentSize);
    auto encodedData
        = EncodeContent(content, _internal::StructuredMessageFlags::Crc64, segmentSize);
    auto decodingStream = CreateDecodingStream(encodedData, contentSize);

    std::vector<uint8_t> readBuffer(segmentSize * 2);
    auto bytesRead = decodingStream.Read(readBuffer.data(), readBuffer.size());

    // First Read should return exactly one segment (512), not two (1024)
    EXPECT_EQ(bytesRead, segmentSize);

    // Verify content correctness via second read and comparison
    std::vector<uint8_t> firstSegment(readBuffer.begin(), readBuffer.begin() + bytesRead);
    EXPECT_EQ(firstSegment, std::vector<uint8_t>(content.begin(), content.begin() + segmentSize));
  }
}}} // namespace Azure::Storage::Test
