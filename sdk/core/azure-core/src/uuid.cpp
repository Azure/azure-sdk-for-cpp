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

namespace Azure { namespace Core {
  std::string Uuid::ToString()
  {
    // Guid is 36 characters
    //  Add one byte for the \0
    char s[37];

    std::snprintf(
        s,
        sizeof(s),
        "%2.2x%2.2x%2.2x%2.2x-%2.2x%2.2x-%2.2x%2.2x-%2.2x%2.2x-%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x",
        m_uuid[0],
        m_uuid[1],
        m_uuid[2],
        m_uuid[3],
        m_uuid[4],
        m_uuid[5],
        m_uuid[6],
        m_uuid[7],
        m_uuid[8],
        m_uuid[9],
        m_uuid[10],
        m_uuid[11],
        m_uuid[12],
        m_uuid[13],
        m_uuid[14],
        m_uuid[15]);

    return std::string(s);
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
