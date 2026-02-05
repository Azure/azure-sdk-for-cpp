// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/common/internal/structured_message_helper.hpp"

namespace Azure { namespace Storage { namespace _internal {
  const size_t StructuredMessageHelper::Crc64Length = 8;
  const uint8_t StructuredMessageHelper::StructuredMessageVersion = 1;
  const size_t StructuredMessageHelper::StreamHeaderLength = 13;
  const int64_t StructuredMessageHelper::StreamHeaderVersionOffset = 0;
  const int64_t StructuredMessageHelper::StreamHeaderMessageLengthOffset = 1;
  const int64_t StructuredMessageHelper::StreamHeaderFlagsOffset = 9;
  const int64_t StructuredMessageHelper::StreamHeaderSegmentCountOffset = 11;
  const size_t StructuredMessageHelper::SegmentHeaderLength = 10;
  const int64_t StructuredMessageHelper::SegmentHeaderNumOffset = 0;
  const int64_t StructuredMessageHelper::SegmentHeaderContentLengthOffset = 2;

  void StructuredMessageHelper::WriteStreamHeader(
      uint8_t* buffer,
      uint64_t messageLength,
      uint16_t flags,
      uint16_t segmentCount)
  {
    buffer[StructuredMessageHelper::StreamHeaderVersionOffset]
        = StructuredMessageHelper::StructuredMessageVersion;
    std::copy(
        reinterpret_cast<const uint8_t*>(&messageLength),
        reinterpret_cast<const uint8_t*>(&messageLength) + sizeof(uint64_t),
        buffer + StructuredMessageHelper::StreamHeaderMessageLengthOffset);
    std::copy(
        reinterpret_cast<const uint8_t*>(&flags),
        reinterpret_cast<const uint8_t*>(&flags) + sizeof(uint16_t),
        buffer + StructuredMessageHelper::StreamHeaderFlagsOffset);
    std::copy(
        reinterpret_cast<const uint8_t*>(&segmentCount),
        reinterpret_cast<const uint8_t*>(&segmentCount) + sizeof(uint16_t),
        buffer + StructuredMessageHelper::StreamHeaderSegmentCountOffset);
  }

  void StructuredMessageHelper::WriteSegmentHeader(
      uint8_t* buffer,
      uint16_t segmentNum,
      uint64_t segmentLength)
  {
    std::copy(
        reinterpret_cast<const uint8_t*>(&segmentNum),
        reinterpret_cast<const uint8_t*>(&segmentNum) + sizeof(uint16_t),
        buffer + StructuredMessageHelper::SegmentHeaderNumOffset);
    std::copy(
        reinterpret_cast<const uint8_t*>(&segmentLength),
        reinterpret_cast<const uint8_t*>(&segmentLength) + sizeof(uint64_t),
        buffer + StructuredMessageHelper::SegmentHeaderContentLengthOffset);
  }

  void StructuredMessageHelper::WriteCrc64(uint8_t* buffer, uint8_t const* crc64)
  {
    if (crc64 == nullptr)
    {
      return;
    }
    std::copy(crc64, crc64 + Crc64Length, buffer);
  }

  void StructuredMessageHelper::ReadStreamHeader(
      uint8_t const* buffer,
      uint64_t& messageLength,
      StructuredMessageFlags& flags,
      uint16_t& segmentCount)
  {
    messageLength = *reinterpret_cast<const uint64_t*>(
        buffer + StructuredMessageHelper::StreamHeaderMessageLengthOffset);
    flags = static_cast<StructuredMessageFlags>(*reinterpret_cast<const uint16_t*>(
        buffer + StructuredMessageHelper::StreamHeaderFlagsOffset));
    segmentCount = *reinterpret_cast<const uint16_t*>(
        buffer + StructuredMessageHelper::StreamHeaderSegmentCountOffset);
  }

  void StructuredMessageHelper::ReadSegmentHeader(
      uint8_t const* buffer,
      uint16_t& segmentNumber,
      uint64_t& segmentLength)
  {
    segmentNumber = *reinterpret_cast<const uint16_t*>(
        buffer + StructuredMessageHelper::SegmentHeaderNumOffset);
    segmentLength = *reinterpret_cast<const uint64_t*>(
        buffer + StructuredMessageHelper::SegmentHeaderContentLengthOffset);
  }

}}} // namespace Azure::Storage::_internal