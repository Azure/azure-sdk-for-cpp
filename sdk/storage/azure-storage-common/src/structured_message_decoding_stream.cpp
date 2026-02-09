// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/common/internal/structured_message_decoding_stream.hpp"

#include "azure/storage/common/crypt.hpp"
#include "azure/storage/common/storage_exception.hpp"

#include <azure/core/http/http.hpp>

using Azure::Core::Context;
using Azure::Core::IO::BodyStream;

namespace Azure { namespace Storage { namespace _internal {

  namespace {
    // The buffer sizes may change with different structured message versions. Please ensure they
    // are larger than the largest possible header/footer length for any supported version.
    constexpr size_t StreamHeaderBufferSize = StructuredMessageHelper::StreamHeaderLength;
    constexpr size_t StreamFooterBufferSize = StructuredMessageHelper::Crc64Length;
  } // namespace

  size_t StructuredMessageDecodingStream::OnRead(
      uint8_t* buffer,
      size_t count,
      Context const& context)
  {
    if (count == 0 || m_currentRegion == StructuredMessageCurrentRegion::StreamEnd)
    {
      return 0;
    }

    // Reads exactly 'count' bytes from the inner stream, or throws.
    auto readInnerStreamExact
        = [this, &context](uint8_t* buffer, size_t count, const char* regionName) {
            if (m_inner->ReadToCount(buffer, count, context) != count)
            {
              throw StorageException(
                  std::string("Unexpected end of stream while reading structured message ")
                  + regionName + ".");
            }
          };

    // Finalizes and validates a CRC64 checksum against a calculated hash, or throws.
    auto finalizeAndValidateCrc64
        = [](Crc64Hash& hash, uint8_t const* buffer, size_t bufferSize, const char* regionName) {
            auto calculated = hash.Final();
            auto reported = StructuredMessageHelper::ReadCrc64(buffer, bufferSize);
            if (calculated != reported)
            {
              throw StorageException(
                  std::string(regionName)
                  + " checksum mismatch. Invalid data may have been written to the destination.");
            }
          };

    // Read stream header and advance to the first segment (or stream footer if empty).
    if (m_currentRegion == StructuredMessageCurrentRegion::StreamHeader)
    {
      AZURE_ASSERT(m_streamHeaderLength <= StreamHeaderBufferSize);
      uint8_t streamHeaderBuffer[StreamHeaderBufferSize];
      readInnerStreamExact(streamHeaderBuffer, m_streamHeaderLength, "stream header");

      StructuredMessageHelper::ReadStreamHeader(
          streamHeaderBuffer, m_streamHeaderLength, m_version, m_length, m_flags, m_segmentCount);
      if (m_version != StructuredMessageHelper::StructuredMessageVersion)
      {
        throw StorageException(
            "Unsupported structured message version: " + std::to_string(m_version)
            + ". Expected version: "
            + std::to_string(StructuredMessageHelper::StructuredMessageVersion) + ".");
      }
      m_streamFooterLength
          = m_flags == StructuredMessageFlags::Crc64 ? StructuredMessageHelper::Crc64Length : 0;
      m_segmentFooterLength
          = m_flags == StructuredMessageFlags::Crc64 ? StructuredMessageHelper::Crc64Length : 0;
      m_segmentFooterBuffer.resize(m_segmentFooterLength);
      m_offset += m_streamHeaderLength;

      m_currentRegion = m_segmentCount == 0 ? StructuredMessageCurrentRegion::StreamFooter
                                            : StructuredMessageCurrentRegion::SegmentHeader;
    }

    // Read segment header and advance to segment content.
    if (m_currentRegion == StructuredMessageCurrentRegion::SegmentHeader)
    {
      readInnerStreamExact(m_segmentHeaderBuffer.data(), m_segmentHeaderLength, "segment header");

      StructuredMessageHelper::ReadSegmentHeader(
          m_segmentHeaderBuffer.data(),
          m_segmentHeaderLength,
          m_currentSegmentNumber,
          m_currentSegmentLength);
      m_offset += m_segmentHeaderLength;
      m_currentSegmentOffset = 0;

      m_currentRegion = StructuredMessageCurrentRegion::SegmentContent;
    }

    // Read segment content. This is the only region that produces output for the caller.
    size_t totalContentRead = 0;
    if (m_currentRegion == StructuredMessageCurrentRegion::SegmentContent)
    {
      size_t bytesToRead = std::min<size_t>(
          count, static_cast<size_t>(m_currentSegmentLength - m_currentSegmentOffset));
      auto bytesRead = m_inner->Read(buffer, bytesToRead, context);

      if (m_flags == StructuredMessageFlags::Crc64)
      {
        m_segmentCrc64Hash->Append(buffer, bytesRead);
      }
      m_offset += bytesRead;
      m_currentSegmentOffset += bytesRead;
      totalContentRead = bytesRead;

      // Advance to footer once all segment content has been consumed.
      if (m_currentSegmentOffset == m_currentSegmentLength)
      {
        m_currentRegion = StructuredMessageCurrentRegion::SegmentFooter;
      }
    }

    // Read and validate segment footer. Advance to next segment or stream footer.
    if (m_currentRegion == StructuredMessageCurrentRegion::SegmentFooter)
    {
      if (m_flags == StructuredMessageFlags::Crc64)
      {
        readInnerStreamExact(m_segmentFooterBuffer.data(), m_segmentFooterLength, "segment footer");
        finalizeAndValidateCrc64(
            *m_segmentCrc64Hash, m_segmentFooterBuffer.data(), m_segmentFooterLength, "Segment");
        m_offset += m_segmentFooterLength;
        m_streamCrc64Hash->Concatenate(*m_segmentCrc64Hash);
        m_segmentCrc64Hash = std::make_unique<Crc64Hash>();
      }

      m_currentRegion = m_currentSegmentNumber == m_segmentCount
          ? StructuredMessageCurrentRegion::StreamFooter
          : StructuredMessageCurrentRegion::SegmentHeader;
    }

    // Read and validate stream footer. Mark stream as complete.
    if (m_currentRegion == StructuredMessageCurrentRegion::StreamFooter)
    {
      if (m_flags == StructuredMessageFlags::Crc64)
      {
        AZURE_ASSERT(m_streamFooterLength <= StreamFooterBufferSize);
        uint8_t streamFooterBuffer[StreamFooterBufferSize];
        readInnerStreamExact(streamFooterBuffer, m_streamFooterLength, "stream footer");
        finalizeAndValidateCrc64(
            *m_streamCrc64Hash, streamFooterBuffer, m_streamFooterLength, "Stream");
        m_offset += m_streamFooterLength;
      }

      // Validate stream integrity before marking complete.
      if (m_currentSegmentNumber != m_segmentCount)
      {
        throw StorageException(
            "Structured message stream ended before all segments were read. Expected "
            + std::to_string(m_segmentCount) + " segments, but read "
            + std::to_string(m_currentSegmentNumber) + ".");
      }
      if (static_cast<uint64_t>(m_offset) != m_length)
      {
        throw StorageException(
            "Structured message length mismatch. Total bytes read was " + std::to_string(m_offset)
            + " bytes, but stream header declared " + std::to_string(m_length) + " bytes.");
      }

      m_currentRegion = StructuredMessageCurrentRegion::StreamEnd;
    }

    return totalContentRead;
  }
}}} // namespace Azure::Storage::_internal
