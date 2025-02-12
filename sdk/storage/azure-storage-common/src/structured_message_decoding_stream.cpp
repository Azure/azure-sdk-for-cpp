// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/common/internal/structured_message_decoding_stream.hpp"

#include "azure/storage/common/crypt.hpp"
#include "azure/storage/common/storage_exception.hpp"

#include <azure/core/http/http.hpp>

using Azure::Core::Context;
using Azure::Core::IO::BodyStream;

namespace Azure { namespace Storage { namespace _internal {

  size_t StructuredMessageDecodingStream::OnRead(
      uint8_t* buffer,
      size_t count,
      Context const& context)
  {
    size_t totalReadContent = 0;
    while (totalReadContent < count && m_offset < m_inner->Length())
    {
      switch (m_currentRegion)
      {
        case StructuredMessageCurrentRegion::StreamHeader: {
          std::vector<uint8_t> streamHeaderBuffer(m_streamHeaderLength);
          auto bytesRead = m_inner->Read(streamHeaderBuffer.data(), m_streamHeaderLength, context);
          StructuredMessageHelper::ReadStreamHeader(
              streamHeaderBuffer.data(), m_length, m_flags, m_segmentCount);
          m_streamFooterLength
              = m_flags == StructuredMessageFlags::Crc64 ? StructuredMessageHelper::Crc64Length : 0;
          m_segmentFooterLength
              = m_flags == StructuredMessageFlags::Crc64 ? StructuredMessageHelper::Crc64Length : 0;
          m_segmentFooterBuffer.resize(m_segmentFooterLength);
          m_offset += bytesRead;
          m_currentRegion = StructuredMessageCurrentRegion::SegmentHeader;
          break;
        }
        case StructuredMessageCurrentRegion::SegmentHeader: {
          auto bytesRead
              = m_inner->Read(m_segmentHeaderBuffer.data(), m_segmentHeaderLength, context);
          StructuredMessageHelper::ReadSegmentHeader(
              m_segmentHeaderBuffer.data(), m_currentSegmentNumber, m_currentSegmentLength);
          m_offset += bytesRead;
          m_currentRegion = StructuredMessageCurrentRegion::SegmentContent;
          break;
        }
        case StructuredMessageCurrentRegion::SegmentContent: {
          size_t readBytes = std::min<size_t>(
              count - totalReadContent,
              static_cast<size_t>(m_currentSegmentLength - m_currentSegmentOffset));
          auto bytesRead
              = m_inner->Read(buffer + totalReadContent, static_cast<size_t>(readBytes), context);
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
          break;
        }
        case StructuredMessageCurrentRegion::SegmentFooter: {
          if (m_flags == StructuredMessageFlags::Crc64)
          {
            auto bytesRead
                = m_inner->Read(m_segmentFooterBuffer.data(), m_segmentFooterLength, context);
            // in current version, segment footer contains crc64 hash of the segment content.
            auto calculatedCrc64 = m_segmentCrc64Hash->Final();
            if (calculatedCrc64 != m_segmentFooterBuffer)
            {
              throw StorageException(
                  "Segment Compared checksums did not match. Invalid data may have been written to the "
                  "destination. calculatedChecksum:"
                  + std::string(calculatedCrc64.begin(), calculatedCrc64.end())
                  + "reportedChecksum: "
                  + std::string(m_segmentFooterBuffer.begin(), m_segmentFooterBuffer.end()));
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
                = m_inner->Read(streamFooterBuffer.data(), m_streamFooterLength, context);
            // in current version, segment footer contains crc64 hash of the segment content.
            auto calculatedCrc64 = m_streamCrc64Hash->Final();
            if (calculatedCrc64 != streamFooterBuffer)
            {
              throw StorageException(
                  "Stream Compared checksums did not match. Invalid data may have been written to the "
                  "destination. calculatedChecksum:"
                  + std::string(calculatedCrc64.begin(), calculatedCrc64.end())
                  + "reportedChecksum: "
                  + std::string(streamFooterBuffer.begin(), streamFooterBuffer.end()));
            }
            m_offset += bytesRead;
            m_streamCrc64Hash->Concatenate(*m_segmentCrc64Hash);
            m_segmentCrc64Hash = std::make_unique<Crc64Hash>();
          }
          break;
        }
      }
    }
    return totalReadContent;
  }
}}} // namespace Azure::Storage::_internal
