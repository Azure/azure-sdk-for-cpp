// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/uuid.hpp"

#include "azure/core/azure_assert.hpp"
#include "azure/core/internal/strings.hpp"

#include <cstdio>
#include <cstring>
#include <random>
#include <type_traits>

#if defined(AZ_PLATFORM_POSIX)
#include <thread>
namespace {
// 64-bit Mersenne Twister by Matsumoto and Nishimura, 2000
// Used to generate the random numbers for the Uuid.
// The seed is generated with std::random_device.
static thread_local std::mt19937_64 randomGenerator(std::random_device{}());
} // namespace
#endif

using Azure::Core::_internal::StringExtensions;

namespace {
static char ByteToHexChar(uint8_t byte)
{
  if (byte <= 9)
  {
    return '0' + byte;
  }

  AZURE_ASSERT_MSG(
      byte >= 10 && byte <= 15,
      "It is expected, for a valid Uuid, to have byte values, where each of the two nibbles fit "
      "into a hexadecimal character");

  return 'a' + (byte - 10);
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
    Uuid result{};

    using RngResultType = std::uint32_t;
    static_assert(sizeof(RngResultType) == 4, "sizeof(RngResultType) must be 4.");
    constexpr size_t RngResultSize = 4;

#if defined(AZ_PLATFORM_WINDOWS)
    std::random_device rd;

    static_assert(
        std::is_same<RngResultType, decltype(rd())>::value,
        "random_device::result_type must be of RngResultType.");
#else
    std::uniform_int_distribution<RngResultType> distribution;
#endif

    for (size_t i = 0; i < result.m_uuid.size(); i += RngResultSize)
    {
#if defined(AZ_PLATFORM_WINDOWS)
      const RngResultType x = rd();
#else
      const RngResultType x = distribution(randomGenerator);
#endif
      std::memcpy(result.m_uuid.data() + i, &x, RngResultSize);
    }

    // The variant field consists of a variable number of the most significant bits of octet 8 of
    // the UUID.
    // https://www.rfc-editor.org/rfc/rfc4122.html#section-4.1.1
    // For setting the variant to conform to RFC4122, the high bits need to be of the form 10xx,
    // which means the hex value of the first 4 bits can only be either 8, 9, A|a, B|b. The 0-7
    // values are reserved for backward compatibility. The C|c, D|d values are reserved for
    // Microsoft, and the E|e, F|f values are reserved for future use.
    // Therefore, we have to zero out the two high bits, and then set the highest bit to 1.
    result.m_uuid.data()[8] = (result.m_uuid.data()[8] & 0x3F) | 0x80;

    {
      constexpr std::uint8_t Version = 4; // Version 4: Pseudo-random number
      result.m_uuid.data()[6] = (result.m_uuid.data()[6] & 0xF) | (Version << 4);
    }

    return result;
  }

  namespace {
    constexpr size_t UuidStringLength = 36; // 00000000-0000-0000-0000-000000000000
    constexpr bool IsDashIndex(size_t i) { return i == 8 || i == 13 || i == 18 || i == 23; }

    constexpr std::uint8_t HexToNibble(char c) // does not check for errors
    {
      if (c >= 'a')
      {
        return 10 + (c - 'a');
      }

      if (c >= 'A')
      {
        return 10 + (c - 'A'); 
      }

      return c - '0';
    }
  } // namespace

  Uuid Uuid::Parse(std::string const& s)
  {
    bool parseError = false;
    Uuid result;
    if (s.size() != UuidStringLength)
    {
      parseError = true;
    }
    else
    {
      for (size_t i = 0, j = 0; i < UuidStringLength; ++i)
      {
        const auto c = s[i];
        if (IsDashIndex(i))
        {
          if (c != '-')
          {
            parseError = true;
            break;
          }
        }
        else
        {
          const auto c2 = s[i + 1];
          if (!StringExtensions::IsHexDigit(c) || !StringExtensions::IsHexDigit(c2))
          {
            parseError = true;
            break;
          }

          result.m_uuid[j] = (HexToNibble(c) << 4) | HexToNibble(c2);
          ++i;
          ++j;
        }
      }
    }

    return parseError ? throw std::invalid_argument(
               "Error parsing Uuid: '" + s
               + "' is not in the '00112233-4455-6677-8899-aAbBcCdDeEfF' format.")
                      : result;
  }

}} // namespace Azure::Core
