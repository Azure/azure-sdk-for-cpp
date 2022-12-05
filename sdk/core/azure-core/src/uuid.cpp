// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

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

    // SetVariant to ReservedRFC4122
    uuid[8] = (uuid[8] | ReservedRFC4122) & 0x7F;

    constexpr uint8_t version = 4;

    uuid[6] = (uuid[6] & 0xF) | (version << 4);

    return Uuid(uuid);
  }

}} // namespace Azure::Core