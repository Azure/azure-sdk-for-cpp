// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/common/internal/structured_message_helper.hpp"

#include <azure/core/azure_assert.hpp>

namespace Azure { namespace Storage { namespace _internal {

  namespace {
    // Helper functions for little-endian encoding/decoding
    inline void WriteUInt16LE(uint8_t* buffer, uint16_t value)
    {
      buffer[0] = static_cast<uint8_t>(value & 0xFF);
      buffer[1] = static_cast<uint8_t>((value >> 8) & 0xFF);
    }

    inline void WriteUInt64LE(uint8_t* buffer, uint64_t value)
    {
      buffer[0] = static_cast<uint8_t>(value & 0xFF);
      buffer[1] = static_cast<uint8_t>((value >> 8) & 0xFF);
      buffer[2] = static_cast<uint8_t>((value >> 16) & 0xFF);
      buffer[3] = static_cast<uint8_t>((value >> 24) & 0xFF);
      buffer[4] = static_cast<uint8_t>((value >> 32) & 0xFF);
      buffer[5] = static_cast<uint8_t>((value >> 40) & 0xFF);
      buffer[6] = static_cast<uint8_t>((value >> 48) & 0xFF);
      buffer[7] = static_cast<uint8_t>((value >> 56) & 0xFF);
    }

    inline uint16_t ReadUInt16LE(uint8_t const* buffer)
    {
      return static_cast<uint16_t>(buffer[0]) | (static_cast<uint16_t>(buffer[1]) << 8);
    }

    inline uint64_t ReadUInt64LE(uint8_t const* buffer)
    {
      return static_cast<uint64_t>(buffer[0]) | (static_cast<uint64_t>(buffer[1]) << 8)
          | (static_cast<uint64_t>(buffer[2]) << 16) | (static_cast<uint64_t>(buffer[3]) << 24)
          | (static_cast<uint64_t>(buffer[4]) << 32) | (static_cast<uint64_t>(buffer[5]) << 40)
          | (static_cast<uint64_t>(buffer[6]) << 48) | (static_cast<uint64_t>(buffer[7]) << 56);
    }
  } // namespace

  void StructuredMessageHelper::WriteStreamHeader(
      uint8_t* buffer,
      size_t bufferSize,
      uint64_t messageLength,
      StructuredMessageFlags flags,
      uint16_t segmentCount)
  {
    AZURE_ASSERT(bufferSize >= StreamHeaderLength);
    buffer[StreamHeaderVersionOffset] = StructuredMessageVersion;
    WriteUInt64LE(buffer + StreamHeaderMessageLengthOffset, messageLength);
    WriteUInt16LE(buffer + StreamHeaderFlagsOffset, static_cast<uint16_t>(flags));
    WriteUInt16LE(buffer + StreamHeaderSegmentCountOffset, segmentCount);
  }

  void StructuredMessageHelper::WriteSegmentHeader(
      uint8_t* buffer,
      size_t bufferSize,
      uint16_t segmentNum,
      uint64_t segmentLength)
  {
    AZURE_ASSERT(bufferSize >= SegmentHeaderLength);
    WriteUInt16LE(buffer + SegmentHeaderNumOffset, segmentNum);
    WriteUInt64LE(buffer + SegmentHeaderContentLengthOffset, segmentLength);
  }

  void StructuredMessageHelper::WriteCrc64(
      uint8_t* buffer,
      size_t bufferSize,
      std::vector<uint8_t> const& crc64)
  {
    AZURE_ASSERT(bufferSize >= Crc64Length);
    AZURE_ASSERT(crc64.size() == Crc64Length);
    std::copy(crc64.begin(), crc64.end(), buffer);
  }

  std::vector<uint8_t> StructuredMessageHelper::ReadCrc64(uint8_t const* buffer, size_t bufferSize)
  {
    AZURE_ASSERT(bufferSize >= Crc64Length);
    return std::vector<uint8_t>(buffer, buffer + Crc64Length);
  }

  void StructuredMessageHelper::ReadStreamHeader(
      uint8_t const* buffer,
      size_t bufferSize,
      uint8_t& version,
      uint64_t& messageLength,
      StructuredMessageFlags& flags,
      uint16_t& segmentCount)
  {
    AZURE_ASSERT(bufferSize >= StreamHeaderLength);
    version = buffer[StreamHeaderVersionOffset];
    messageLength = ReadUInt64LE(buffer + StreamHeaderMessageLengthOffset);
    flags = static_cast<StructuredMessageFlags>(ReadUInt16LE(buffer + StreamHeaderFlagsOffset));
    segmentCount = ReadUInt16LE(buffer + StreamHeaderSegmentCountOffset);
  }

  void StructuredMessageHelper::ReadSegmentHeader(
      uint8_t const* buffer,
      size_t bufferSize,
      uint16_t& segmentNumber,
      uint64_t& segmentLength)
  {
    AZURE_ASSERT(bufferSize >= SegmentHeaderLength);
    segmentNumber = ReadUInt16LE(buffer + SegmentHeaderNumOffset);
    segmentLength = ReadUInt64LE(buffer + SegmentHeaderContentLengthOffset);
  }

}}} // namespace Azure::Storage::_internal
