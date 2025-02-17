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
    auto encodedData = encodingStream.ReadToEnd();

    innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        encodedData.data(), encodedData.size());
    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = contentSize;
    _internal::StructuredMessageDecodingStream decodingStream(
        std::move(innerStream), decodingOptions);
    auto decodedData = decodingStream.ReadToEnd();

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
    auto encodedData = ReadToEnd(encodingStream, 4096);

    innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        encodedData.data(), encodedData.size());
    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = contentSize;
    _internal::StructuredMessageDecodingStream decodingStream(
        std::move(innerStream), decodingOptions);
    auto decodedData = ReadToEnd(decodingStream, 513);

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
    std::vector<uint8_t> encodedData = ReadToEnd(encodingStream, 7);

    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = contentSize;
    _internal::StructuredMessageDecodingStream decodingStream(
        std::make_unique<Azure::Core::IO::MemoryBodyStream>(encodedData.data(), encodedData.size()),
        decodingOptions);
    auto decodedData = ReadToEnd(decodingStream, 7);

    EXPECT_EQ(content, decodedData);

    // Large encode range
    encodingStream.Rewind();
    encodedData = ReadToEnd(encodingStream, 4096);
    _internal::StructuredMessageDecodingStream decodingStream1(
        std::make_unique<Azure::Core::IO::MemoryBodyStream>(encodedData.data(), encodedData.size()),
        decodingOptions);
    decodedData = ReadToEnd(decodingStream1, 5);

    EXPECT_EQ(content, decodedData);

    decodingStream1.Rewind();
    decodedData = ReadToEnd(decodingStream1, 6);

    EXPECT_EQ(content, decodedData);

    // Large decode range
    encodingStream.Rewind();
    encodedData = ReadToEnd(encodingStream, 8);
    _internal::StructuredMessageDecodingStream decodingStream2(
        std::make_unique<Azure::Core::IO::MemoryBodyStream>(encodedData.data(), encodedData.size()),
        decodingOptions);
    decodedData = ReadToEnd(decodingStream2, 4096);

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
    std::vector<uint8_t> encodedData = ReadToEnd(encodingStream, 4 * 1024 * 1024);

    innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        encodedData.data(), encodedData.size());
    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = contentSize;
    _internal::StructuredMessageDecodingStream decodingStream(
        std::move(innerStream), decodingOptions);
    auto decodedData = ReadToEnd(decodingStream, 4 * 1024 * 1024);

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
    std::vector<uint8_t> encodedData = ReadToEnd(encodingStream, 4096);

    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = contentSize;
    innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        encodedData.data(), encodedData.size());
    _internal::StructuredMessageDecodingStream decodingStream(
        std::move(innerStream), decodingOptions);
    auto decodedData = ReadToEnd(decodingStream, 4096);

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
    std::vector<uint8_t> encodedData = ReadToEnd(encodingStream, 7);

    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = contentSize;
    _internal::StructuredMessageDecodingStream decodingStream(
        std::make_unique<Azure::Core::IO::MemoryBodyStream>(encodedData.data(), encodedData.size()),
        decodingOptions);
    auto decodedData = ReadToEnd(decodingStream, 7);

    EXPECT_EQ(content, decodedData);

    // Large encode range
    encodingStream.Rewind();
    encodedData = ReadToEnd(encodingStream, 4096);
    _internal::StructuredMessageDecodingStream decodingStream1(
        std::make_unique<Azure::Core::IO::MemoryBodyStream>(encodedData.data(), encodedData.size()),
        decodingOptions);
    decodedData = ReadToEnd(decodingStream1, 5);

    EXPECT_EQ(content, decodedData);

    decodingStream1.Rewind();
    decodedData = ReadToEnd(decodingStream1, 6);

    EXPECT_EQ(content, decodedData);

    // Large decode range
    encodingStream.Rewind();
    encodedData = ReadToEnd(encodingStream, 8);
    _internal::StructuredMessageDecodingStream decodingStream2(
        std::make_unique<Azure::Core::IO::MemoryBodyStream>(encodedData.data(), encodedData.size()),
        decodingOptions);
    decodedData = ReadToEnd(decodingStream2, 4096);

    EXPECT_EQ(content, decodedData);
  }

}}} // namespace Azure::Storage::Test
