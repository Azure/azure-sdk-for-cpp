// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Universally unique identifier.
 */

#pragma once

#include <cstring>
#include <new> // for placement new
#include <random>
#include <string>
#include <utility> // for swap and move

namespace Azure { namespace Core {
  /**
   * @brief Universally unique identifier.
   */
  class Uuid {

  private:
    static constexpr int UuidSize = 16;

    uint8_t m_uuid[UuidSize];
    // The UUID reserved variants.
    static constexpr uint8_t ReservedNCS = 0x80;
    static constexpr uint8_t ReservedRFC4122 = 0x40;
    static constexpr uint8_t ReservedMicrosoft = 0x20;
    static constexpr uint8_t ReservedFuture = 0x00;

  private:
    Uuid(uint8_t const uuid[UuidSize]) { std::memcpy(m_uuid, uuid, UuidSize); }

  public:
    /**
     * Gets UUID as a string.
     * @details A string is in canonical format (4-2-2-2-6 lowercase hex and dashes only)
     */
    std::string ToString()
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

    /**
     * @brief Create a new random UUID.
     */
    static Uuid CreateUuid()
    {
      std::random_device rd;

      uint8_t uuid[UuidSize] = {};

      for (int i = 0; i < UuidSize; i += 4)
      {
        const uint32_t x = rd();
        std::memcpy(uuid + i, &x, 4);
      }

      // SetVariant to ReservedRFC4122
      uuid[8] = (uuid[8] | ReservedRFC4122) & 0x7F;

      constexpr uint8_t version = 4;

      uuid[6] = (uuid[6] & 0xF) | (version << 4);

      return Uuid(uuid);
    }
  };
}} // namespace Azure::Core
