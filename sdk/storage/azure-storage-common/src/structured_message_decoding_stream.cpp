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

    auto shouldProcessFooters = [](StructuredMessageCurrentRegion region) {
      return region == StructuredMessageCurrentRegion::SegmentFooter
          || region == StructuredMessageCurrentRegion::StreamFooter;
    };

    size_t totalContentRead = 0;
    // Loop logic:
    // 1. Continue reading content until the target count is reached or all data in this stream is
    // consumed.
    // 2. If all content in a segment has been read but the footer is still required, proceed to
    // read the footer.
    while ((totalContentRead == 0 && m_currentRegion != StructuredMessageCurrentRegion::StreamEnd)
           || shouldProcessFooters(m_currentRegion))
    {
      switch (m_currentRegion)
      {
        case StructuredMessageCurrentRegion::StreamHeader: {
          AZURE_ASSERT(m_streamHeaderLength <= StreamHeaderBufferSize);
          uint8_t streamHeaderBuffer[StreamHeaderBufferSize];
          auto bytesRead = m_inner->ReadToCount(streamHeaderBuffer, m_streamHeaderLength, context);
          if (bytesRead != m_streamHeaderLength)
          {
            throw StorageException(
                "Unexpected end of stream while reading structured message stream header.");
          }
          StructuredMessageHelper::ReadStreamHeader(
              streamHeaderBuffer, m_streamHeaderLength, m_length, m_flags, m_segmentCount);
          m_streamFooterLength
              = m_flags == StructuredMessageFlags::Crc64 ? StructuredMessageHelper::Crc64Length : 0;
          m_segmentFooterLength
              = m_flags == StructuredMessageFlags::Crc64 ? StructuredMessageHelper::Crc64Length : 0;
          m_segmentFooterBuffer.resize(m_segmentFooterLength);
          m_offset += bytesRead;

          // If no segments, move to stream footer directly.
          m_currentRegion = m_segmentCount == 0 ? StructuredMessageCurrentRegion::StreamFooter
                                                : StructuredMessageCurrentRegion::SegmentHeader;
          break;
        }
        case StructuredMessageCurrentRegion::SegmentHeader: {
          auto bytesRead
              = m_inner->ReadToCount(m_segmentHeaderBuffer.data(), m_segmentHeaderLength, context);
          if (bytesRead != m_segmentHeaderLength)
          {
            throw StorageException(
                "Unexpected end of stream while reading structured message segment header.");
          }
          StructuredMessageHelper::ReadSegmentHeader(
              m_segmentHeaderBuffer.data(),
              m_segmentHeaderLength,
              m_currentSegmentNumber,
              m_currentSegmentLength);
          m_offset += bytesRead;
          m_currentRegion = StructuredMessageCurrentRegion::SegmentContent;
          m_currentSegmentOffset = 0;
          break;
        }
        case StructuredMessageCurrentRegion::SegmentContent: {
          size_t bytesToRead = std::min<size_t>(
              count - totalContentRead,
              static_cast<size_t>(m_currentSegmentLength - m_currentSegmentOffset));
          auto bytesRead = m_inner->Read(buffer + totalContentRead, bytesToRead, context);

          if (m_flags == StructuredMessageFlags::Crc64)
          {
            m_segmentCrc64Hash->Append(buffer + totalContentRead, bytesRead);
          }
          m_offset += bytesRead;
          m_currentSegmentOffset += bytesRead;
          totalContentRead += bytesRead;
          if (m_currentSegmentOffset == m_currentSegmentLength)
          {
            m_currentRegion = StructuredMessageCurrentRegion::SegmentFooter;
          }

          if (bytesRead == 0)
          {
            return totalContentRead;
          }
          break;
        }
        case StructuredMessageCurrentRegion::SegmentFooter: {
          if (m_flags == StructuredMessageFlags::Crc64)
          {
            auto bytesRead = m_inner->ReadToCount(
                m_segmentFooterBuffer.data(), m_segmentFooterLength, context);
            if (bytesRead != m_segmentFooterLength)
            {
              throw StorageException(
                  "Unexpected end of stream while reading structured message segment footer.");
            }
            // In current version, segment footer contains crc64 hash of the segment content.
            auto calculatedCrc64 = m_segmentCrc64Hash->Final();
            auto reportedCrc64 = StructuredMessageHelper::ReadCrc64(
                m_segmentFooterBuffer.data(), m_segmentFooterLength);
            if (calculatedCrc64 != reportedCrc64)
            {
              throw StorageException(
                  "Segment Compared checksums did not match. Invalid data may have been written to "
                  "the "
                  "destination. calculatedChecksum:"
                  + std::string(calculatedCrc64.begin(), calculatedCrc64.end())
                  + "reportedChecksum: " + std::string(reportedCrc64.begin(), reportedCrc64.end()));
            }
            m_offset += bytesRead;
            m_streamCrc64Hash->Concatenate(*m_segmentCrc64Hash);
            m_segmentCrc64Hash = std::make_unique<Crc64Hash>();
          }

          // Decide if more segments to read or move to stream footer.
          m_currentRegion = m_currentSegmentNumber == m_segmentCount
              ? StructuredMessageCurrentRegion::StreamFooter
              : StructuredMessageCurrentRegion::SegmentHeader;
          break;
        }
        case StructuredMessageCurrentRegion::StreamFooter: {
          if (m_flags == StructuredMessageFlags::Crc64)
          {
            AZURE_ASSERT(m_streamFooterLength <= StreamFooterBufferSize);
            uint8_t streamFooterBuffer[StreamFooterBufferSize];
            auto bytesRead
                = m_inner->ReadToCount(streamFooterBuffer, m_streamFooterLength, context);
            if (bytesRead != m_streamFooterLength)
            {
              throw StorageException(
                  "Unexpected end of stream while reading structured message stream footer.");
            }
            // in current version, stream footer contains crc64 hash of all segment content.
            auto calculatedCrc64 = m_streamCrc64Hash->Final();
            auto reportedCrc64
                = StructuredMessageHelper::ReadCrc64(streamFooterBuffer, m_streamFooterLength);
            if (calculatedCrc64 != reportedCrc64)
            {
              throw StorageException(
                  "Stream Compared checksums did not match. Invalid data may have been written to "
                  "the "
                  "destination. calculatedChecksum:"
                  + std::string(calculatedCrc64.begin(), calculatedCrc64.end())
                  + "reportedChecksum: " + std::string(reportedCrc64.begin(), reportedCrc64.end()));
            }
            m_offset += bytesRead;
          }
          m_currentRegion = StructuredMessageCurrentRegion::StreamEnd;
          break;
        }
        case StructuredMessageCurrentRegion::StreamEnd: {
          break;
        }
      }
    }

    // Validate stream integrity once all data has been consumed.
    if (m_currentRegion == StructuredMessageCurrentRegion::StreamEnd)
    {
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
    }
    return totalContentRead;
  }
}}} // namespace Azure::Storage::_internal
