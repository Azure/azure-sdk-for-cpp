// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "test_base.hpp"

#include <azure/storage/common/internal/structured_message_decoding_stream.hpp>
#include <azure/storage/common/internal/structured_message_encoding_stream.hpp>

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

}}} // namespace Azure::Storage::Test
