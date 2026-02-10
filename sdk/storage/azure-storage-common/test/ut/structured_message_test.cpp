// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "test_base.hpp"

#include <azure/storage/common/internal/reliable_stream.hpp>
#include <azure/storage/common/internal/structured_message_decoding_stream.hpp>
#include <azure/storage/common/internal/structured_message_encoding_stream.hpp>
#include <azure/storage/common/storage_exception.hpp>

namespace Azure { namespace Storage { namespace Test {

  class StructuredMessageTest : public StorageTest {
  };

  // ==================== Helper Functions ====================

  /**
   * @brief Reads all remaining data from a body stream in fixed-size chunks.
   * @param stream The body stream to read from.
   * @param chunkSize The size of each read chunk.
   * @return A vector containing all bytes read from the stream.
   */
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

  /**
   * @brief Encodes content into a structured message.
   * @param content The raw content bytes to encode.
   * @param flags The structured message flags (e.g., Crc64 or None).
   * @param maxSegmentLength The maximum segment length for encoding.
   * @return A vector containing the encoded structured message bytes.
   */
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

  /**
   * @brief Creates a decoding stream from pre-encoded structured message data.
   * @param encodedData The encoded structured message bytes.
   * @param contentLength The expected decoded content length.
   * @return A StructuredMessageDecodingStream ready for reading.
   */
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

  // ==================== Helper Classes ====================

  /**
   * @brief A body stream that owns its data buffer.
   * @details Used in retry lambdas where the encoded data must outlive the lambda call.
   */
  class OwningMemoryBodyStream final : public Azure::Core::IO::BodyStream {
    std::vector<uint8_t> m_data;
    size_t m_offset = 0;
    size_t OnRead(uint8_t* buffer, size_t count, Azure::Core::Context const&) override
    {
      size_t toRead = (std::min)(count, m_data.size() - m_offset);
      if (toRead > 0)
      {
        std::copy(m_data.data() + m_offset, m_data.data() + m_offset + toRead, buffer);
      }
      m_offset += toRead;
      return toRead;
    }

  public:
    /**
     * @brief Constructs an OwningMemoryBodyStream that takes ownership of the given data.
     * @param data The data buffer to own.
     */
    explicit OwningMemoryBodyStream(std::vector<uint8_t> data) : m_data(std::move(data)) {}
    int64_t Length() const override { return static_cast<int64_t>(m_data.size()); }
    void Rewind() override { m_offset = 0; }
  };

  /**
   * @brief A body stream that simulates a network failure after delivering a specified
   * number of raw bytes from the inner stream.
   */
  class FailingBodyStream final : public Azure::Core::IO::BodyStream {
    std::unique_ptr<Azure::Core::IO::BodyStream> m_inner;
    size_t m_failAfterBytes;
    size_t m_bytesDelivered = 0;
    size_t OnRead(uint8_t* buffer, size_t count, Azure::Core::Context const& context) override
    {
      if (m_bytesDelivered >= m_failAfterBytes)
      {
        throw std::runtime_error("Simulated network failure");
      }
      size_t maxRead = (std::min)(count, m_failAfterBytes - m_bytesDelivered);
      auto bytesRead = m_inner->Read(buffer, maxRead, context);
      m_bytesDelivered += bytesRead;
      return bytesRead;
    }

  public:
    /**
     * @brief Constructs a FailingBodyStream.
     * @param inner The inner body stream to read from before failure.
     * @param failAfterBytes The number of bytes to deliver before throwing.
     */
    FailingBodyStream(std::unique_ptr<Azure::Core::IO::BodyStream> inner, size_t failAfterBytes)
        : m_inner(std::move(inner)), m_failAfterBytes(failAfterBytes)
    {
    }
    int64_t Length() const override { return m_inner->Length(); }
    void Rewind() override
    {
      m_inner->Rewind();
      m_bytesDelivered = 0;
    }
  };

  /**
   * @brief Creates a DecodingStream wrapping an OwningMemoryBodyStream for retry scenarios.
   * @details Encodes the remaining content (from @p retryOffset) as a fresh structured message,
   * then wraps it in a DecodingStream — mirroring what blob_client/share_file_client Download()
   * does on retry.
   * @param content The original full content.
   * @param retryOffset The byte offset into @p content where the retry should resume.
   * @param flags The structured message flags to use for encoding.
   * @param maxSegmentLength The maximum segment length for encoding.
   * @return A unique_ptr to a BodyStream representing the decoded remaining content.
   */
  std::unique_ptr<Azure::Core::IO::BodyStream> CreateRetryDecodingStream(
      const std::vector<uint8_t>& content,
      int64_t retryOffset,
      _internal::StructuredMessageFlags flags,
      int64_t maxSegmentLength)
  {
    std::vector<uint8_t> remainingContent(
        content.begin() + static_cast<ptrdiff_t>(retryOffset), content.end());
    size_t remainingSize = remainingContent.size();
    auto remainingEncoded = EncodeContent(remainingContent, flags, maxSegmentLength);
    auto owningStream = std::make_unique<OwningMemoryBodyStream>(std::move(remainingEncoded));
    _internal::StructuredMessageDecodingStreamOptions opts;
    opts.ContentLength = static_cast<int64_t>(remainingSize);
    return std::make_unique<_internal::StructuredMessageDecodingStream>(
        std::move(owningStream), opts);
  }

  // ==================== Basic Round-Trip Tests ====================

  /// @brief Test encoding and decoding empty content.
  TEST_F(StructuredMessageTest, EmptyContent)
  {
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

  /// @brief Test with minimal (single-byte) content.
  TEST_F(StructuredMessageTest, SingleByteContent)
  {
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

  /// @brief Test basic encode-then-decode round-trip with CRC64 and multiple segments.
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

  /// @brief Test with large content (16MB+) to ensure scalability.
  TEST_F(StructuredMessageTest, VeryLargeContent)
  {
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

  // ==================== Segment Size & Boundary Tests ====================

  /// @brief Test content that exactly fills one segment.
  TEST_F(StructuredMessageTest, ExactlyOneSegment)
  {
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

  /// @brief Test content that is one byte over the segment boundary.
  TEST_F(StructuredMessageTest, ContentOneByteOverSegment)
  {
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

  /// @brief Test reading exactly at segment boundaries.
  TEST_F(StructuredMessageTest, SegmentBoundaryReads)
  {
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

  /// @brief Test with the default max segment length (4MB).
  TEST_F(StructuredMessageTest, MaxSegmentLengthBoundaries)
  {
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

  /// @brief Test with a very large segment length (10MB).
  TEST_F(StructuredMessageTest, LargeSegmentLength)
  {
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

  /// @brief Stress test with very small segments (10 bytes) and large content (10KB).
  TEST_F(StructuredMessageTest, VerySmallSegmentWithLargeContent)
  {
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

  // ==================== Read Chunk Size Variation Tests ====================

  /// @brief Test encode and decode with very small read chunks (33-byte segment, varying chunks).
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

  /// @brief Test small (7-byte) read chunks for both encoding and decoding.
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

  /// @brief Test large (4MB) read chunks with content slightly over 4MB.
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

  /// @brief Test reading with various odd-sized chunks (137 encode, 193 decode).
  TEST_F(StructuredMessageTest, ReadInOddSizedChunks)
  {
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

  /// @brief Test with different chunk sizes for encoding (8192) and decoding (11).
  TEST_F(StructuredMessageTest, MixedChunkSizesEncodeAndDecode)
  {
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

  /// @brief Reading one byte at a time should still produce correct output.
  TEST_F(StructuredMessageTest, SingleByteReads)
  {
    const size_t contentSize = 300;
    auto content = RandomBuffer(contentSize);
    auto encodedData = EncodeContent(content, _internal::StructuredMessageFlags::Crc64, 128);
    auto decodingStream = CreateDecodingStream(encodedData, contentSize);

    std::vector<uint8_t> decodedData;
    EXPECT_NO_THROW(decodedData = ReadToEnd(decodingStream, 1));

    EXPECT_EQ(content, decodedData);
  }

  // ==================== Flag Variation Tests (CRC64 vs None) ====================

  /// @brief Test encode/decode without CRC64 flags.
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

  /// @brief Test without CRC64 using small read chunks.
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

  /// @brief Test without CRC64 across multiple content sizes.
  TEST_F(StructuredMessageTest, NoCrc64WithVaryingSizes)
  {
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
      EXPECT_NO_THROW(decodedData = ReadToEnd(decodingStream, 4096));

      EXPECT_EQ(content, decodedData);
    }
  }

  /// @brief Test both CRC64 and None flags with the same content.
  TEST_F(StructuredMessageTest, AlternatingFlagsModes)
  {
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

  // ==================== Stream State & Behavior Tests ====================

  /// @brief Test that Length() returns correct values for encoding and decoding streams.
  TEST_F(StructuredMessageTest, LengthProperty)
  {
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

  /// @brief Test that multiple rewind operations produce identical encoded output.
  TEST_F(StructuredMessageTest, MultipleRewinds)
  {
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

  /// @brief Test rewinding after partial reads on both encoding and decoding streams.
  TEST_F(StructuredMessageTest, RewindAfterPartialRead)
  {
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

  /// @brief After all data is consumed, subsequent Read() calls should return 0.
  TEST_F(StructuredMessageTest, ReadAfterStreamEnd)
  {
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

  /// @brief Read() with count=0 should return 0 without advancing state.
  TEST_F(StructuredMessageTest, ReadWithZeroCount)
  {
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

  // ==================== Single Read() Semantics Tests ====================

  /**
   * @brief Test that a single Read() call returns at most one segment's worth of content,
   * even if the buffer is much larger.
   */
  TEST_F(StructuredMessageTest, SingleReadReturnsAtMostOneSegment)
  {
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

  /**
   * @brief Same as SingleReadReturnsAtMostOneSegment but without CRC64, to verify the
   * loop condition applies regardless of flags.
   */
  TEST_F(StructuredMessageTest, SingleReadReturnsAtMostOneSegment_NoCrc64)
  {
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

  /**
   * @brief Verify that calling Read() repeatedly with a large buffer correctly accumulates
   * all content one segment at a time.
   */
  TEST_F(StructuredMessageTest, SequentialSingleReadsAccumulateCorrectly)
  {
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

  /**
   * @brief When buffer is exactly 2x segment size, each Read() still returns at most
   * one segment.
   */
  TEST_F(StructuredMessageTest, BufferExactlyDoubleSegmentSize)
  {
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

  // ==================== Error Handling Tests ====================

  /// @brief Verify that CRC64 corruption in segment content is detected during decoding.
  TEST_F(StructuredMessageTest, Crc64CorruptionDetected)
  {
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

  // ==================== ReliableStream Integration Tests ====================

  /**
   * @brief Basic composition: DecodingStream wrapped in ReliableStream with no failures.
   * @details Mirrors the blob_client/share_file_client Download() stream chain.
   */
  TEST_F(StructuredMessageTest, ReliableStreamWithDecodingStream_NoFailure)
  {
    const size_t contentSize = 2 * 1024 + 512;
    auto content = RandomBuffer(contentSize);
    auto encodedData = EncodeContent(content, _internal::StructuredMessageFlags::Crc64, 1024);

    auto innerStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        encodedData.data(), encodedData.size());
    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = static_cast<int64_t>(contentSize);
    auto decodingStream = std::make_unique<_internal::StructuredMessageDecodingStream>(
        std::move(innerStream), decodingOptions);

    _internal::ReliableStreamOptions reliableOptions;
    reliableOptions.MaxRetryRequests = 3;
    auto retryFunction
        = [](int64_t, Azure::Core::Context const&) -> std::unique_ptr<Azure::Core::IO::BodyStream> {
      EXPECT_TRUE(false) << "Retry should not be called when there are no failures";
      return nullptr;
    };
    _internal::ReliableStream reliableStream(
        std::move(decodingStream), reliableOptions, std::move(retryFunction));

    auto decodedData = ReadToEnd(reliableStream, 4096);
    EXPECT_EQ(content, decodedData);
  }

  /**
   * @brief ReliableStream retries when the inner transport fails mid-read.
   * @details The reconnector produces a fresh DecodingStream for the remaining content.
   */
  TEST_F(StructuredMessageTest, ReliableStreamWithDecodingStream_RetryOnFailure)
  {
    const size_t contentSize = 4 * 1024 + 512;
    auto content = RandomBuffer(contentSize);
    const int64_t maxSegmentLength = 1024;
    auto encodedData
        = EncodeContent(content, _internal::StructuredMessageFlags::Crc64, maxSegmentLength);

    // Fail after delivering half the raw encoded bytes
    size_t failAfterRawBytes = encodedData.size() / 2;
    auto failingStream = std::make_unique<FailingBodyStream>(
        std::make_unique<Azure::Core::IO::MemoryBodyStream>(encodedData.data(), encodedData.size()),
        failAfterRawBytes);

    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = static_cast<int64_t>(contentSize);
    auto decodingStream = std::make_unique<_internal::StructuredMessageDecodingStream>(
        std::move(failingStream), decodingOptions);

    _internal::ReliableStreamOptions reliableOptions;
    reliableOptions.MaxRetryRequests = 3;
    int retryCount = 0;
    auto retryFunction
        = [&content, &retryCount, maxSegmentLength](
              int64_t retryOffset,
              Azure::Core::Context const&) -> std::unique_ptr<Azure::Core::IO::BodyStream> {
      retryCount++;
      return CreateRetryDecodingStream(
          content, retryOffset, _internal::StructuredMessageFlags::Crc64, maxSegmentLength);
    };

    _internal::ReliableStream reliableStream(
        std::move(decodingStream), reliableOptions, std::move(retryFunction));

    auto decodedData = ReadToEnd(reliableStream, 4096);
    EXPECT_EQ(content, decodedData);
    EXPECT_GT(retryCount, 0);
  }

  /**
   * @brief Tests the ReadToCount pattern used by DownloadTo() buffer overload:
   *   stream->ReadToCount(buffer, length, context)
   */
  TEST_F(StructuredMessageTest, ReliableStreamWithDecodingStream_RetryWithReadToCount)
  {
    const size_t contentSize = 4 * 1024 + 512;
    auto content = RandomBuffer(contentSize);
    const int64_t maxSegmentLength = 1024;
    auto encodedData
        = EncodeContent(content, _internal::StructuredMessageFlags::Crc64, maxSegmentLength);

    size_t failAfterRawBytes = encodedData.size() / 3;
    auto failingStream = std::make_unique<FailingBodyStream>(
        std::make_unique<Azure::Core::IO::MemoryBodyStream>(encodedData.data(), encodedData.size()),
        failAfterRawBytes);

    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = static_cast<int64_t>(contentSize);
    auto decodingStream = std::make_unique<_internal::StructuredMessageDecodingStream>(
        std::move(failingStream), decodingOptions);

    _internal::ReliableStreamOptions reliableOptions;
    reliableOptions.MaxRetryRequests = 3;
    auto retryFunction
        = [&content, maxSegmentLength](
              int64_t retryOffset,
              Azure::Core::Context const&) -> std::unique_ptr<Azure::Core::IO::BodyStream> {
      return CreateRetryDecodingStream(
          content, retryOffset, _internal::StructuredMessageFlags::Crc64, maxSegmentLength);
    };

    _internal::ReliableStream reliableStream(
        std::move(decodingStream), reliableOptions, std::move(retryFunction));

    // Use ReadToCount like DownloadTo() does
    std::vector<uint8_t> buffer(contentSize);
    Azure::Core::Context context;
    size_t bytesRead = reliableStream.ReadToCount(buffer.data(), contentSize, context);
    EXPECT_EQ(bytesRead, contentSize);
    EXPECT_EQ(std::vector<uint8_t>(buffer.begin(), buffer.begin() + bytesRead), content);
  }

  /// @brief Failure before any raw bytes are delivered — retryOffset should be 0.
  TEST_F(StructuredMessageTest, ReliableStreamWithDecodingStream_FailureAtStart)
  {
    const size_t contentSize = 2 * 1024;
    auto content = RandomBuffer(contentSize);
    const int64_t maxSegmentLength = 1024;
    auto encodedData
        = EncodeContent(content, _internal::StructuredMessageFlags::Crc64, maxSegmentLength);

    // Fail immediately (0 raw bytes delivered)
    auto failingStream = std::make_unique<FailingBodyStream>(
        std::make_unique<Azure::Core::IO::MemoryBodyStream>(encodedData.data(), encodedData.size()),
        0);

    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = static_cast<int64_t>(contentSize);
    auto decodingStream = std::make_unique<_internal::StructuredMessageDecodingStream>(
        std::move(failingStream), decodingOptions);

    _internal::ReliableStreamOptions reliableOptions;
    reliableOptions.MaxRetryRequests = 3;
    int retryCount = 0;
    auto retryFunction
        = [&content, &retryCount, maxSegmentLength](
              int64_t retryOffset,
              Azure::Core::Context const&) -> std::unique_ptr<Azure::Core::IO::BodyStream> {
      retryCount++;
      EXPECT_EQ(retryOffset, 0);
      return CreateRetryDecodingStream(
          content, retryOffset, _internal::StructuredMessageFlags::Crc64, maxSegmentLength);
    };

    _internal::ReliableStream reliableStream(
        std::move(decodingStream), reliableOptions, std::move(retryFunction));

    auto decodedData = ReadToEnd(reliableStream, 4096);
    EXPECT_EQ(content, decodedData);
    EXPECT_GT(retryCount, 0);
  }

  /// @brief Same retry pattern but without CRC64 flags.
  TEST_F(StructuredMessageTest, ReliableStreamWithDecodingStream_NoCrc64)
  {
    const size_t contentSize = 3 * 1024 + 256;
    auto content = RandomBuffer(contentSize);
    const int64_t maxSegmentLength = 1024;
    auto encodedData
        = EncodeContent(content, _internal::StructuredMessageFlags::None, maxSegmentLength);

    size_t failAfterRawBytes = encodedData.size() / 2;
    auto failingStream = std::make_unique<FailingBodyStream>(
        std::make_unique<Azure::Core::IO::MemoryBodyStream>(encodedData.data(), encodedData.size()),
        failAfterRawBytes);

    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = static_cast<int64_t>(contentSize);
    auto decodingStream = std::make_unique<_internal::StructuredMessageDecodingStream>(
        std::move(failingStream), decodingOptions);

    _internal::ReliableStreamOptions reliableOptions;
    reliableOptions.MaxRetryRequests = 3;
    auto retryFunction
        = [&content, maxSegmentLength](
              int64_t retryOffset,
              Azure::Core::Context const&) -> std::unique_ptr<Azure::Core::IO::BodyStream> {
      return CreateRetryDecodingStream(
          content, retryOffset, _internal::StructuredMessageFlags::None, maxSegmentLength);
    };

    _internal::ReliableStream reliableStream(
        std::move(decodingStream), reliableOptions, std::move(retryFunction));

    auto decodedData = ReadToEnd(reliableStream, 4096);
    EXPECT_EQ(content, decodedData);
  }

  /// @brief Small read chunks exercise segment boundary handling during retry.
  TEST_F(StructuredMessageTest, ReliableStreamWithDecodingStream_SmallReadChunks)
  {
    const size_t contentSize = 2 * 1024 + 512;
    auto content = RandomBuffer(contentSize);
    const int64_t maxSegmentLength = 1024;
    auto encodedData
        = EncodeContent(content, _internal::StructuredMessageFlags::Crc64, maxSegmentLength);

    size_t failAfterRawBytes = encodedData.size() / 4;
    auto failingStream = std::make_unique<FailingBodyStream>(
        std::make_unique<Azure::Core::IO::MemoryBodyStream>(encodedData.data(), encodedData.size()),
        failAfterRawBytes);

    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = static_cast<int64_t>(contentSize);
    auto decodingStream = std::make_unique<_internal::StructuredMessageDecodingStream>(
        std::move(failingStream), decodingOptions);

    _internal::ReliableStreamOptions reliableOptions;
    reliableOptions.MaxRetryRequests = 3;
    auto retryFunction
        = [&content, maxSegmentLength](
              int64_t retryOffset,
              Azure::Core::Context const&) -> std::unique_ptr<Azure::Core::IO::BodyStream> {
      return CreateRetryDecodingStream(
          content, retryOffset, _internal::StructuredMessageFlags::Crc64, maxSegmentLength);
    };

    _internal::ReliableStream reliableStream(
        std::move(decodingStream), reliableOptions, std::move(retryFunction));

    // Read with very small chunks to stress boundary handling
    auto decodedData = ReadToEnd(reliableStream, 7);
    EXPECT_EQ(content, decodedData);
  }

  /**
   * @brief Exercises multiple retries within a single OnRead() call.
   * @details Initial stream fails mid-read, first 2 reconnector calls return
   * immediately-failing streams, and the 3rd reconnector returns a working stream.
   *
   * OnRead intent trace (MaxRetryRequests=5):
   *   - intent=1: initial DecodingStream(FailingBodyStream) fails  -> catch
   *   - intent=2: reconnector @#1 (fails immediately)              -> catch
   *   - intent=3: reconnector @#2 (fails immediately)              -> catch
   *   - intent=4: reconnector @#3 (working stream)                 -> succeeds
   */
  TEST_F(StructuredMessageTest, ReliableStreamWithDecodingStream_ThreeRetries)
  {
    const size_t contentSize = 4 * 1024 + 512;
    auto content = RandomBuffer(contentSize);
    const int64_t maxSegmentLength = 1024;
    auto encodedData
        = EncodeContent(content, _internal::StructuredMessageFlags::Crc64, maxSegmentLength);

    // Initial stream delivers half the raw encoded bytes, then fails
    size_t failAfterRawBytes = encodedData.size() / 2;
    auto failingStream = std::make_unique<FailingBodyStream>(
        std::make_unique<Azure::Core::IO::MemoryBodyStream>(encodedData.data(), encodedData.size()),
        failAfterRawBytes);

    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = static_cast<int64_t>(contentSize);
    auto decodingStream = std::make_unique<_internal::StructuredMessageDecodingStream>(
        std::move(failingStream), decodingOptions);

    // MaxRetryRequests must be > 3 so the 3rd reconnector call (intent=4) is allowed
    _internal::ReliableStreamOptions reliableOptions;
    reliableOptions.MaxRetryRequests = 5;
    int retryCount = 0;
    auto retryFunction
        = [&content, &retryCount, maxSegmentLength](
              int64_t retryOffset,
              Azure::Core::Context const&) -> std::unique_ptr<Azure::Core::IO::BodyStream> {
      retryCount++;
      if (retryCount <= 2)
      {
        // First 2 retries return a stream that fails immediately
        return std::make_unique<FailingBodyStream>(
            std::make_unique<OwningMemoryBodyStream>(std::vector<uint8_t>()), 0);
      }
      // 3rd retry returns a working stream
      return CreateRetryDecodingStream(
          content, retryOffset, _internal::StructuredMessageFlags::Crc64, maxSegmentLength);
    };

    _internal::ReliableStream reliableStream(
        std::move(decodingStream), reliableOptions, std::move(retryFunction));

    auto decodedData = ReadToEnd(reliableStream, 4096);
    EXPECT_EQ(content, decodedData);
    EXPECT_EQ(retryCount, 3);
  }

  /**
   * @brief Large 4MB content with multiple 1MB segments.
   * @details The transport fails mid-download and the reconnector provides a fresh
   * DecodingStream for the remaining content.
   */
  TEST_F(StructuredMessageTest, ReliableStreamWithDecodingStream_4MBContent)
  {
    const size_t contentSize = 4 * 1024 * 1024;
    auto content = RandomBuffer(contentSize);
    const int64_t maxSegmentLength = 1 * 1024 * 1024; // 1MB segments  4 segments
    auto encodedData
        = EncodeContent(content, _internal::StructuredMessageFlags::Crc64, maxSegmentLength);

    // Fail roughly halfway through the raw encoded bytes
    size_t failAfterRawBytes = encodedData.size() / 2;
    auto failingStream = std::make_unique<FailingBodyStream>(
        std::make_unique<Azure::Core::IO::MemoryBodyStream>(encodedData.data(), encodedData.size()),
        failAfterRawBytes);

    _internal::StructuredMessageDecodingStreamOptions decodingOptions;
    decodingOptions.ContentLength = static_cast<int64_t>(contentSize);
    auto decodingStream = std::make_unique<_internal::StructuredMessageDecodingStream>(
        std::move(failingStream), decodingOptions);

    _internal::ReliableStreamOptions reliableOptions;
    reliableOptions.MaxRetryRequests = 3;
    int retryCount = 0;
    auto retryFunction
        = [&content, &retryCount, maxSegmentLength](
              int64_t retryOffset,
              Azure::Core::Context const&) -> std::unique_ptr<Azure::Core::IO::BodyStream> {
      retryCount++;
      return CreateRetryDecodingStream(
          content, retryOffset, _internal::StructuredMessageFlags::Crc64, maxSegmentLength);
    };

    _internal::ReliableStream reliableStream(
        std::move(decodingStream), reliableOptions, std::move(retryFunction));

    // Use 1MB read chunks matching the segment size
    auto decodedData = ReadToEnd(reliableStream, 1024 * 1024);
    EXPECT_EQ(content, decodedData);
    EXPECT_GT(retryCount, 0);
  }
}}} // namespace Azure::Storage::Test