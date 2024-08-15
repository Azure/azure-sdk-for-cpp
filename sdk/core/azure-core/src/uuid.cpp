// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/uuid.hpp"

#include <cstdio>
#include <random>

#if defined(AZ_PLATFORM_POSIX)
#include <thread>
namespace {
// 64-bit Mersenne Twister by Matsumoto and Nishimura, 2000
// Used to generate the random numbers for the Uuid.
// The seed is generated with std::random_device.
static thread_local std::mt19937_64 randomGenerator(std::random_device{}());
} // namespace
#endif

namespace {
static uint8_t HexCharToByte(char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;

  throw std::invalid_argument("Invalid hexadecimal character.");
}

static std::string ToStringHelper(std::array<uint8_t, 16> uuid)
{
  // Guid is 36 characters
  //  Add one byte for the \0
  char s[37];

  std::snprintf(
      s,
      sizeof(s),
      "%2.2x%2.2x%2.2x%2.2x-%2.2x%2.2x-%2.2x%2.2x-%2.2x%2.2x-%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x",
      uuid[0],
      uuid[1],
      uuid[2],
      uuid[3],
      uuid[4],
      uuid[5],
      uuid[6],
      uuid[7],
      uuid[8],
      uuid[9],
      uuid[10],
      uuid[11],
      uuid[12],
      uuid[13],
      uuid[14],
      uuid[15]);

  return std::string(s);
}
} // namespace

namespace Azure { namespace Core {
  std::string Uuid::ToString() const { return ToStringHelper(m_uuid); }

  std::string Uuid::ToString() { return ToStringHelper(m_uuid); }

  Uuid Uuid::CreateUuid()
  {
    uint8_t uuid[UuidSize] = {};

#if defined(AZ_PLATFORM_WINDOWS)
    std::random_device rd;
#else
    std::uniform_int_distribution<uint32_t> distribution;
#endif

    for (size_t i = 0; i < UuidSize; i += 4)
    {
#if defined(AZ_PLATFORM_WINDOWS)
      const uint32_t x = rd();
#else
      const uint32_t x = distribution(randomGenerator);
#endif
      std::memcpy(uuid + i, &x, 4);
    }

    // The variant field consists of a variable number of the most significant bits of octet 8 of
    // the UUID.
    // https://www.rfc-editor.org/rfc/rfc4122.html#section-4.1.1
    // For setting the variant to conform to RFC4122, the high bits need to be of the form 10xx,
    // which means the hex value of the first 4 bits can only be either 8, 9, A|a, B|b. The 0-7
    // values are reserved for backward compatibility. The C|c, D|d values are reserved for
    // Microsoft, and the E|e, F|f values are reserved for future use.
    // Therefore, we have to zero out the two high bits, and then set the highest bit to 1.
    uuid[8] = (uuid[8] & 0x3F) | 0x80;

    constexpr uint8_t version = 4; // Version 4: Pseudo-random number

    uuid[6] = (uuid[6] & 0xF) | (version << 4);

    return Uuid(uuid);
  }

  Uuid Uuid::CreateFromArray(std::array<uint8_t, UuidSize> const& uuid)
  {
    Uuid rv{uuid.data()};
    return rv;
  }

  Uuid Uuid::CreateFromString(std::string const& uuid)
  {
    if (uuid.size() != 36)
    {
      throw std::invalid_argument("Uuid string must be exactly 36 characters long.");
    }
    if (uuid[8] != '-' || uuid[13] != '-' || uuid[18] != '-' || uuid[23] != '-')
    {
      throw std::invalid_argument("Only the `8-4-4-4-12` Uuid format is supported.");
    }

    std::array<uint8_t, UuidSize> uuidArray{};

    for (size_t i = 0, j = 0; j < UuidSize && i < uuid.size();)
    {
      if (uuid[i] == '-')
      {
        i++;
        continue; // Skip hyphens
      }

      uuidArray[j] = (HexCharToByte(uuid[i++]) << 4);
      uuidArray[j] |= HexCharToByte(uuid[i++]);
      j++;
    }

    return Uuid::CreateFromArray(uuidArray);
  }

}} // namespace Azure::Core
