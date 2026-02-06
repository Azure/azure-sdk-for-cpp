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
    inline bool NeedsFooter(StructuredMessageCurrentRegion region) noexcept
    {
      switch (region)
      {
        case StructuredMessageCurrentRegion::SegmentFooter:
        case StructuredMessageCurrentRegion::StreamFooter:
          return true;
        default:
          return false;
      }
    }
  } // namespace

  size_t StructuredMessageDecodingStream::OnRead(
      uint8_t* buffer,
      size_t count,
      Context const& context)
  {
    size_t totalReadContent = 0;
    while (
        (totalReadContent < count && m_currentRegion != StructuredMessageCurrentRegion::Completed)
        || NeedsFooter(m_currentRegion))
    {
      switch (m_currentRegion)
      {
        case StructuredMessageCurrentRegion::StreamHeader: {
          std::vector<uint8_t> streamHeaderBuffer(m_streamHeaderLength);
          auto bytesRead
              = m_inner->ReadToCount(streamHeaderBuffer.data(), m_streamHeaderLength, context);
          if (bytesRead != m_streamHeaderLength)
          {
            throw StorageException(
                "Unexpected end of stream while reading structured message stream header.");
          }
          StructuredMessageHelper::ReadStreamHeader(
              streamHeaderBuffer.data(), m_streamHeaderLength, m_length, m_flags, m_segmentCount);
          m_streamFooterLength
              = m_flags == StructuredMessageFlags::Crc64 ? StructuredMessageHelper::Crc64Length : 0;
          m_segmentFooterLength
              = m_flags == StructuredMessageFlags::Crc64 ? StructuredMessageHelper::Crc64Length : 0;
          m_segmentFooterBuffer.resize(m_segmentFooterLength);
          m_offset += bytesRead;
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
          break;
        }
        case StructuredMessageCurrentRegion::SegmentContent: {
          size_t bytesToRead = std::min<size_t>(
              count - totalReadContent,
              static_cast<size_t>(m_currentSegmentLength - m_currentSegmentOffset));
          auto bytesRead
              = m_inner->Read(buffer + totalReadContent, static_cast<size_t>(bytesToRead), context);

          if (m_flags == StructuredMessageFlags::Crc64)
          {
            m_segmentCrc64Hash->Append(buffer + totalReadContent, bytesRead);
          }
          m_offset += bytesRead;
          m_currentSegmentOffset += bytesRead;
          totalReadContent += bytesRead;
          if (m_currentSegmentOffset == m_currentSegmentLength)
          {
            m_currentRegion = StructuredMessageCurrentRegion::SegmentFooter;
          }
          if (bytesRead != bytesToRead)
          {
            return totalReadContent;
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
            // in current version, segment footer contains crc64 hash of the segment content.
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

          m_currentSegmentOffset = 0;
          m_currentRegion = m_currentSegmentNumber == m_segmentCount
              ? StructuredMessageCurrentRegion::StreamFooter
              : StructuredMessageCurrentRegion::SegmentHeader;
          break;
        }
        case StructuredMessageCurrentRegion::StreamFooter: {
          if (m_flags == StructuredMessageFlags::Crc64)
          {
            std::vector<uint8_t> streamFooterBuffer(m_streamFooterLength);
            auto bytesRead
                = m_inner->ReadToCount(streamFooterBuffer.data(), m_streamFooterLength, context);
            if (bytesRead != m_streamFooterLength)
            {
              throw StorageException(
                  "Unexpected end of stream while reading structured message stream footer.");
            }
            // in current version, stream footer contains crc64 hash of all segment content.
            auto calculatedCrc64 = m_streamCrc64Hash->Final();
            auto reportedCrc64 = StructuredMessageHelper::ReadCrc64(
                streamFooterBuffer.data(), m_streamFooterLength);
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
          m_currentRegion = StructuredMessageCurrentRegion::Completed;
          break;
        }
        case StructuredMessageCurrentRegion::Completed: {
          break;
        }
      }
    }
    return totalReadContent;
  }
}}} // namespace Azure::Storage::_internal
