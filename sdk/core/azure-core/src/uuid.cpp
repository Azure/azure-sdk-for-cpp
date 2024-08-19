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
static char ByteToHexChar(uint8_t byte)
{
  if (byte <= 9)
    return '0' + byte;
  if (byte >= 10 && byte <= 15)
    return 'a' + (byte - 10);
  throw std::out_of_range("Byte value out of range for hexadecimal character.");
}
} // namespace

namespace Azure { namespace Core {
  std::string Uuid::ToString() const
  {
    std::string s(36, '-');

    for (size_t i = 0, j = 0; j < s.size() && i < UuidSize; i++)
    {
      if (i == 4 || i == 6 || i == 8 || i == 10)
      {
        j++; // Add hyphens at the appropriate places
      }

      uint8_t highNibble = (m_uuid[i] >> 4) & 0x0F;
      uint8_t lowNibble = m_uuid[i] & 0x0F;
      s[j++] = ByteToHexChar(highNibble);
      s[j++] = ByteToHexChar(lowNibble);
    }
    return s;
  }

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

}} // namespace Azure::Core
