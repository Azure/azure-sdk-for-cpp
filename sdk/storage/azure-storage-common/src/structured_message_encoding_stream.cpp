// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/common/internal/structured_message_encoding_stream.hpp"

#include "azure/storage/common/crypt.hpp"

#include <azure/core/http/http.hpp>

using Azure::Core::Context;
using Azure::Core::IO::BodyStream;

namespace Azure { namespace Storage { namespace _internal {

  size_t StructuredMessageEncodingStream::OnRead(
      uint8_t* buffer,
      size_t count,
      Context const& context)
  {
    size_t totalBytesRead = 0;
    while (totalBytesRead < count && m_currentRegion != StructuredMessageCurrentRegion::StreamEnd)
    {
      switch (m_currentRegion)
      {
        case StructuredMessageCurrentRegion::StreamHeader: {
          if (m_streamHeaderBuffer.empty())
          {
            m_streamHeaderBuffer.resize(m_streamHeaderLength);
            StructuredMessageHelper::WriteStreamHeader(
                m_streamHeaderBuffer.data(),
                m_streamHeaderLength,
                this->Length(),
                m_options.Flags,
                m_segmentCount);
          }
          size_t bytesToRead = std::min<size_t>(
              count - totalBytesRead,
              static_cast<size_t>(m_streamHeaderLength - m_currentRegionOffset));
          std::memcpy(
              buffer + totalBytesRead,
              m_streamHeaderBuffer.data() + m_currentRegionOffset,
              bytesToRead);
          m_offset += bytesToRead;
          m_currentRegionOffset += bytesToRead;
          totalBytesRead += bytesToRead;
          if (m_currentRegionOffset == m_streamHeaderLength)
          {
            m_currentRegion = m_segmentCount == 0 ? StructuredMessageCurrentRegion::StreamFooter
                                                  : StructuredMessageCurrentRegion::SegmentHeader;
            m_currentRegionOffset = 0;
          }
          break;
        }
        case StructuredMessageCurrentRegion::SegmentHeader: {
          if (m_segmentHeaderBuffer.empty())
          {
            m_segmentHeaderBuffer.resize(m_segmentHeaderLength);
            m_segmentNumber += 1;
            StructuredMessageHelper::WriteSegmentHeader(
                m_segmentHeaderBuffer.data(),
                m_segmentHeaderLength,
                m_segmentNumber,
                std::min<uint64_t>(m_options.MaxSegmentLength, m_inner->Length() - m_innerOffset));
          }
          size_t bytesToRead = std::min<size_t>(
              count - totalBytesRead,
              static_cast<size_t>(m_segmentHeaderLength - m_currentRegionOffset));
          std::memcpy(
              buffer + totalBytesRead,
              m_segmentHeaderBuffer.data() + m_currentRegionOffset,
              bytesToRead);
          m_offset += bytesToRead;
          m_currentRegionOffset += bytesToRead;
          totalBytesRead += bytesToRead;
          if (m_currentRegionOffset == m_segmentHeaderLength)
          {
            m_currentRegion = StructuredMessageCurrentRegion::SegmentContent;
            m_currentRegionOffset = 0;
          }
          break;
        }
        case StructuredMessageCurrentRegion::SegmentContent: {
          size_t bytesToRead = std::min<size_t>(
              count - totalBytesRead,
              static_cast<size_t>(m_options.MaxSegmentLength - m_currentRegionOffset));
          auto bytesRead = m_inner->ReadToCount(buffer + totalBytesRead, bytesToRead, context);
          if (m_options.Flags == StructuredMessageFlags::Crc64)
          {
            m_segmentCrc64Hash->Append(buffer + totalBytesRead, bytesRead);
          }
          m_offset += bytesRead;
          m_innerOffset += bytesRead;
          m_currentRegionOffset += bytesRead;
          totalBytesRead += bytesRead;
          if (m_currentRegionOffset == static_cast<uint64_t>(m_options.MaxSegmentLength)
              || m_innerOffset >= m_inner->Length())
          {
            m_currentRegion = StructuredMessageCurrentRegion::SegmentFooter;
            m_currentRegionOffset = 0;
          }
          break;
        }
        case StructuredMessageCurrentRegion::SegmentFooter: {
          if (m_options.Flags == StructuredMessageFlags::Crc64)
          {
            if (m_segmentFooterBuffer.empty())
            {
              m_segmentFooterBuffer.resize(m_segmentFooterLength);
              StructuredMessageHelper::WriteCrc64(
                  m_segmentFooterBuffer.data(), m_segmentFooterLength, m_segmentCrc64Hash->Final());
              // Accumulate segment hash into stream hash once, when finalized.
              m_streamCrc64Hash->Concatenate(*m_segmentCrc64Hash);
              m_segmentCrc64Hash = std::make_unique<Crc64Hash>();
            }
            size_t bytesToRead = std::min<size_t>(
                count - totalBytesRead,
                static_cast<size_t>(m_segmentFooterLength - m_currentRegionOffset));
            std::memcpy(
                buffer + totalBytesRead,
                m_segmentFooterBuffer.data() + m_currentRegionOffset,
                bytesToRead);
            m_offset += bytesToRead;
            m_currentRegionOffset += bytesToRead;
            totalBytesRead += bytesToRead;
          }
          if (m_currentRegionOffset == m_segmentFooterLength)
          {
            m_currentRegion = m_innerOffset == m_inner->Length()
                ? StructuredMessageCurrentRegion::StreamFooter
                : StructuredMessageCurrentRegion::SegmentHeader;
            m_currentRegionOffset = 0;
            m_segmentHeaderBuffer.clear();
            m_segmentFooterBuffer.clear();
          }
          break;
        }
        case StructuredMessageCurrentRegion::StreamFooter: {
          if (m_options.Flags == StructuredMessageFlags::Crc64)
          {
            if (m_streamFooterBuffer.empty())
            {
              m_streamFooterBuffer.resize(m_streamFooterLength);
              StructuredMessageHelper::WriteCrc64(
                  m_streamFooterBuffer.data(), m_streamFooterLength, m_streamCrc64Hash->Final());
            }
            size_t bytesToRead = std::min<size_t>(
                count - totalBytesRead,
                static_cast<size_t>(m_streamFooterLength - m_currentRegionOffset));
            std::memcpy(
                buffer + totalBytesRead,
                m_streamFooterBuffer.data() + m_currentRegionOffset,
                bytesToRead);
            m_offset += bytesToRead;
            m_currentRegionOffset += bytesToRead;
            totalBytesRead += bytesToRead;
          }
          if (m_currentRegionOffset == m_streamFooterLength)
          {
            m_currentRegion = StructuredMessageCurrentRegion::StreamEnd;
          }
          break;
        }
        case StructuredMessageCurrentRegion::StreamEnd: {
          break;
        }
      }
    }
    return totalBytesRead;
  }
}}} // namespace Azure::Storage::_internal
