// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/storage/common/crypt.hpp"
#include "azure/storage/common/dll_import_export.hpp"
#include "constants.hpp"
#include "structured_message_helper.hpp"

#include <azure/core/context.hpp>
#include <azure/core/io/body_stream.hpp>

#include <algorithm>
#include <functional>
#include <memory>

namespace Azure { namespace Storage { namespace _internal {

  // Options used by structured message encode stream
  struct StructuredMessageEncodingStreamOptions final
  {
    // configures the maximun segment length
    int64_t MaxSegmentLength = 4 * 1024 * 1024;

    StructuredMessageFlags Flags = StructuredMessageFlags::None;
  };

  /**
   * @brief TODO
   *
   */
  class StructuredMessageEncodingStream final : public Azure::Core::IO::BodyStream {
  private:
    // initial bodyStream.
    Azure::Core::IO::BodyStream* m_inner;
    // Configuration for the encode stream
    StructuredMessageEncodingStreamOptions const m_options;

    size_t m_streamHeaderLength;
    size_t m_segmentHeaderLength;
    size_t m_segmentFooterLength;
    size_t m_streamFooterLength;

    uint16_t m_segmentCount;
    uint16_t m_segmentNumber;

    int64_t m_offset;
    int64_t m_innerOffset;

    StructuredMessageCurrentRegion m_currentRegion;
    uint64_t m_currentRegionOffset;

    std::vector<uint8_t> m_streamHeaderCache;
    std::vector<uint8_t> m_segmentHeaderCache;
    std::vector<uint8_t> m_segmentFooterCache;
    std::vector<uint8_t> m_streamFooterCache;

    std::unique_ptr<Crc64Hash> m_segmentCrc64Hash;
    std::unique_ptr<Crc64Hash> m_streamCrc64Hash;

    size_t OnRead(uint8_t* buffer, size_t count, Azure::Core::Context const& context) override;

  public:
    explicit StructuredMessageEncodingStream(
        Azure::Core::IO::BodyStream* inner,
        StructuredMessageEncodingStreamOptions const options)
        : m_inner(inner), m_options(options),
          m_streamHeaderLength(StructuredMessageHelper::StreamHeaderLength),
          m_segmentHeaderLength(StructuredMessageHelper::SegmentHeaderLength), m_segmentCount(0),
          m_segmentNumber(0), m_offset(0), m_innerOffset(0),
          m_currentRegion(StructuredMessageCurrentRegion::StreamHeader), m_currentRegionOffset(0),
          m_streamHeaderCache(0), m_segmentHeaderCache(0), m_segmentFooterCache(0),
          m_streamFooterCache(0), m_segmentCrc64Hash(std::make_unique<Crc64Hash>()),
          m_streamCrc64Hash(std::make_unique<Crc64Hash>())
    {
      m_segmentFooterLength = m_options.Flags == StructuredMessageFlags::Crc64
          ? StructuredMessageHelper::Crc64Length
          : 0;
      m_streamFooterLength = m_options.Flags == StructuredMessageFlags::Crc64
          ? StructuredMessageHelper::Crc64Length
          : 0;
      m_segmentCount = static_cast<uint16_t>(
          (m_inner->Length() + m_options.MaxSegmentLength - 1) / m_options.MaxSegmentLength);
    }

    int64_t Length() const override
    {
      return m_streamHeaderLength + m_streamFooterLength
          + (m_segmentHeaderLength + m_segmentFooterLength) * m_segmentCount
          + this->m_inner->Length();
    }

    void Rewind() override
    {
      // Rewind directly from a transportAdapter body stream (like libcurl) would throw
      this->m_inner->Rewind();
      this->m_segmentNumber = 0;
      this->m_offset = 0;
      this->m_innerOffset = 0;
      this->m_currentRegion = StructuredMessageCurrentRegion::StreamHeader;
      this->m_currentRegionOffset = 0;
      this->m_streamHeaderCache.clear();
      this->m_segmentHeaderCache.clear();
      this->m_segmentFooterCache.clear();
      this->m_streamFooterCache.clear();
      this->m_segmentCrc64Hash = std::make_unique<Crc64Hash>();
      this->m_streamCrc64Hash = std::make_unique<Crc64Hash>();
    }
  };

}}} // namespace Azure::Storage::_internal
