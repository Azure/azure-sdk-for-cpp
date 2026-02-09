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

  // Options used by structured message decode stream
  struct StructuredMessageDecodingStreamOptions final
  {
    /**
     * Required. The length of the real data in the structured message.
     */
    int64_t ContentLength = 0;
  };

  /**
   * @brief The decoding stream for structured message. For download scenarios.
   */
  class StructuredMessageDecodingStream final : public Azure::Core::IO::BodyStream {
  private:
    // initial bodyStream.
    std::unique_ptr<Azure::Core::IO::BodyStream> m_inner;
    // Configuration for the decode stream
    StructuredMessageDecodingStreamOptions const m_options;

    size_t m_streamHeaderLength;
    size_t m_segmentHeaderLength;
    size_t m_segmentFooterLength;
    size_t m_streamFooterLength;

    // Length of the stream
    uint64_t m_length;
    StructuredMessageFlags m_flags;
    uint16_t m_segmentCount;

    int64_t m_offset;

    StructuredMessageCurrentRegion m_currentRegion;
    uint16_t m_currentSegmentNumber;
    uint64_t m_currentSegmentOffset;
    uint64_t m_currentSegmentLength;

    std::vector<uint8_t> m_segmentHeaderBuffer;
    std::vector<uint8_t> m_segmentFooterBuffer;

    std::unique_ptr<Crc64Hash> m_segmentCrc64Hash;
    std::unique_ptr<Crc64Hash> m_streamCrc64Hash;

    size_t OnRead(uint8_t* buffer, size_t count, Azure::Core::Context const& context) override;

  public:
    explicit StructuredMessageDecodingStream(
        std::unique_ptr<Azure::Core::IO::BodyStream> inner,
        StructuredMessageDecodingStreamOptions const options)
        : m_inner(std::move(inner)), m_options(options),
          m_streamHeaderLength(StructuredMessageHelper::StreamHeaderLength),
          m_segmentHeaderLength(StructuredMessageHelper::SegmentHeaderLength),
          m_segmentFooterLength(0), m_streamFooterLength(0), m_length(0),
          m_flags(StructuredMessageFlags::None), m_segmentCount(0), m_offset(0),
          m_currentRegion(StructuredMessageCurrentRegion::StreamHeader), m_currentSegmentNumber(0),
          m_currentSegmentOffset(0), m_currentSegmentLength(0),
          m_segmentHeaderBuffer(StructuredMessageHelper::SegmentHeaderLength),
          m_segmentFooterBuffer(0), m_segmentCrc64Hash(std::make_unique<Crc64Hash>()),
          m_streamCrc64Hash(std::make_unique<Crc64Hash>())
    {
    }

    int64_t Length() const override { return m_options.ContentLength; }

    void Rewind() override
    {
      // Rewind directly from a transportAdapter body stream (like libcurl) would throw
      this->m_inner->Rewind();
      this->m_segmentFooterLength = 0;
      this->m_streamFooterLength = 0;
      this->m_length = 0;
      this->m_segmentCount = 0;
      this->m_flags = StructuredMessageFlags::None;
      this->m_offset = 0;
      this->m_currentRegion = StructuredMessageCurrentRegion::StreamHeader;
      this->m_currentSegmentNumber = 0;
      this->m_currentSegmentOffset = 0;
      this->m_currentSegmentLength = 0;
      this->m_segmentHeaderBuffer.clear();
      this->m_segmentFooterBuffer.clear();
      this->m_segmentCrc64Hash = std::make_unique<Crc64Hash>();
      this->m_streamCrc64Hash = std::make_unique<Crc64Hash>();
    }
  };

}}} // namespace Azure::Storage::_internal
