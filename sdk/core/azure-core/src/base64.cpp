// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/base64.hpp"
#include "azure/core/platform.hpp"

#include <string>
#include <vector>

namespace {

static char const Base64EncodeArray[65]
    = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static char const EncodingPath = '=';
static int8_t const Base64DecodeArray[256] = {
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    62,
    -1,
    -1,
    -1,
    63, // 62 is placed at index 43 (for +), 63 at index 47 (for /)
    52,
    53,
    54,
    55,
    56,
    57,
    58,
    59,
    60,
    61,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1, // 52-61 are placed at index 48-57 (for 0-9), 64 at index 61 (for =)
    -1,
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,
    15,
    16,
    17,
    18,
    19,
    20,
    21,
    22,
    23,
    24,
    25,
    -1,
    -1,
    -1,
    -1,
    -1, // 0-25 are placed at index 65-90 (for A-Z)
    -1,
    26,
    27,
    28,
    29,
    30,
    31,
    32,
    33,
    34,
    35,
    36,
    37,
    38,
    39,
    40,
    41,
    42,
    43,
    44,
    45,
    46,
    47,
    48,
    49,
    50,
    51,
    -1,
    -1,
    -1,
    -1,
    -1, // 26-51 are placed at index 97-122 (for a-z)
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1, // Bytes over 122 ('z') are invalid and cannot be decoded. Hence, padding the map with 255,
        // which indicates invalid input
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
};

static int32_t base64Encode(const uint8_t* threeBytes)
{
  int32_t i = (*threeBytes << 16) | (*(threeBytes + 1) << 8) | *(threeBytes + 2);

  int32_t i0 = Base64EncodeArray[i >> 18];
  int32_t i1 = Base64EncodeArray[(i >> 12) & 0x3F];
  int32_t i2 = Base64EncodeArray[(i >> 6) & 0x3F];
  int32_t i3 = Base64EncodeArray[i & 0x3F];

  return i0 | (i1 << 8) | (i2 << 16) | (i3 << 24);
}

static int32_t base64EncodeAndPadOne(const uint8_t* twoBytes)
{
  int32_t i = (*twoBytes << 16) | (*(twoBytes + 1) << 8);

  int32_t i0 = Base64EncodeArray[i >> 18];
  int32_t i1 = Base64EncodeArray[(i >> 12) & 0x3F];
  int32_t i2 = Base64EncodeArray[(i >> 6) & 0x3F];

  return i0 | (i1 << 8) | (i2 << 16) | (EncodingPath << 24);
}

static int32_t base64EncodeAndPadTwo(const uint8_t* oneByte)
{
  int32_t i = (*oneByte << 8);

  int32_t i0 = Base64EncodeArray[i >> 10];
  int32_t i1 = Base64EncodeArray[(i >> 4) & 0x3F];

  return i0 | (i1 << 8) | (EncodingPath << 16) | (EncodingPath << 24);
}

static void base64WriteIntAsFourBytes(char* destination, int32_t value)
{
  *(destination + 3) = static_cast<uint8_t>((value >> 24) & 0xFF);
  *(destination + 2) = static_cast<uint8_t>((value >> 16) & 0xFF);
  *(destination + 1) = static_cast<uint8_t>((value >> 8) & 0xFF);
  *(destination + 0) = static_cast<uint8_t>(value & 0xFF);
}

std::string base64Encode(const std::vector<uint8_t>& data)
{
  int64_t sourceIndex = 0;
  auto inputSize = data.size();
  int32_t result = 0;
  auto maxEncodedSize = ((inputSize + 2) / 3) * 4;
  // Use a string with size to the max possible result
  std::string encodedResult(maxEncodedSize, '0');
  // Removing const from the string to update the placeholder string
  auto destination = const_cast<char*>(encodedResult.data());

  while (sourceIndex < static_cast<int64_t>(inputSize - 2))
  {
    result = base64Encode(data.data() + sourceIndex);
    base64WriteIntAsFourBytes(destination, result);
    destination += 4;
    sourceIndex += 3;
  }

  if (sourceIndex == static_cast<int64_t>(inputSize - 1))
  {
    result = base64EncodeAndPadTwo(data.data() + sourceIndex);
    base64WriteIntAsFourBytes(destination, result);
    destination += 4;
    sourceIndex += 1;
  }
  else if (sourceIndex == static_cast<int64_t>(inputSize - 2))
  {
    result = base64EncodeAndPadOne(data.data() + sourceIndex);
    base64WriteIntAsFourBytes(destination, result);
    destination += 4;
    sourceIndex += 2;
  }
  auto destinationLength = static_cast<size_t>(destination - encodedResult.data());
  // If encoding took less than the max-expected
  if (destinationLength < encodedResult.size())
  {
    return encodedResult.substr(0, destinationLength);
  }
  return encodedResult;
}

static int64_t base64Decode(const char* encodedBytes)
{
  int64_t i0 = *encodedBytes;
  int64_t i1 = *(encodedBytes + 1);
  int64_t i2 = *(encodedBytes + 2);
  int64_t i3 = *(encodedBytes + 3);

  i0 = Base64DecodeArray[i0];
  i1 = Base64DecodeArray[i1];
  i2 = Base64DecodeArray[i2];
  i3 = Base64DecodeArray[i3];

  i0 <<= 18;
  i1 <<= 12;
  i2 <<= 6;

  i0 |= i3;
  i1 |= i2;

  i0 |= i1;
  return i0;
}

static void base64WriteThreeLowOrderBytes(std::vector<uint8_t>::iterator destination, int64_t value)
{
  *destination = static_cast<uint8_t>(value >> 16);
  *(destination + 1) = static_cast<uint8_t>(value >> 8);
  *(destination + 2) = static_cast<uint8_t>(value);
}

std::vector<uint8_t> base64Decode(const std::string& text)
{

  if (text.size() < 4)
  {
    return std::vector<uint8_t>(0);
  }

  int64_t sourceIndex = 0;
  int64_t destinationIndex = 0;
  auto inputSize = text.size();
  auto inputPtr = text.data();
  // use the size for the max decoded size
  auto maxDecodedSize = (inputSize / 4) * 3;
  std::vector<uint8_t> destination(maxDecodedSize);
  auto destinationPtr = destination.begin();

  while (sourceIndex < static_cast<int64_t>(inputSize - 4))
  {
    int64_t result = base64Decode(inputPtr + sourceIndex);
    base64WriteThreeLowOrderBytes(destinationPtr, result);
    destinationPtr += 3;
    destinationIndex += 3;
    sourceIndex += 4;
  }

  // We are guaranteed to have an input with at least 4 bytes at this point, with a size that is a
  // multiple of 4.
  int64_t i0 = *(inputPtr + inputSize - 4);
  int64_t i1 = *(inputPtr + inputSize - 3);
  int64_t i2 = *(inputPtr + inputSize - 2);
  int64_t i3 = *(inputPtr + inputSize - 1);

  i0 = Base64DecodeArray[i0];
  i1 = Base64DecodeArray[i1];

  i0 <<= 18;
  i1 <<= 12;

  i0 |= i1;

  if (i3 != EncodingPath)
  {
    i2 = Base64DecodeArray[i2];
    i3 = Base64DecodeArray[i3];

    i2 <<= 6;

    i0 |= i3;
    i0 |= i2;

    base64WriteThreeLowOrderBytes(destinationPtr, i0);
    destinationPtr += 3;
  }
  else if (i2 != EncodingPath)
  {
    i2 = Base64DecodeArray[i2];

    i2 <<= 6;

    i0 |= i2;

    *(destinationPtr + 1) = static_cast<uint8_t>(i0 >> 8);
    *destinationPtr = static_cast<uint8_t>(i0 >> 16);
    destinationPtr += 2;
  }
  else
  {
    *destinationPtr = static_cast<uint8_t>(i0 >> 16);
    destinationPtr += 1;
  }

  auto resultSize = static_cast<size_t>(destinationPtr - destination.begin());
  if (resultSize < destination.size())
  {
    destination.resize(resultSize);
    destination.shrink_to_fit();
  }
  return destination;
}

} // namespace

namespace Azure { namespace Core {

  std::string Convert::Base64Encode(const std::vector<uint8_t>& data) { return base64Encode(data); }

  std::vector<uint8_t> Convert::Base64Decode(const std::string& text) { return base64Decode(text); }

}} // namespace Azure::Core
