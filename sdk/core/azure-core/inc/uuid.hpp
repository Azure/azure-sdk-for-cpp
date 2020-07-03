// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <new> // for placement new
#include <string>
#include <utility> // for swap and move
#include <random>

namespace Azure { namespace Core {

  class UUID {

  private:
    static const size_t UUIDSize = 16;

    uint8_t m_uuid[UUIDSize];
    // The UUID reserved variants.
    static constexpr uint8_t ReservedNCS = 0x80;
    static constexpr uint8_t ReservedRFC4122 = 0x40;
    static constexpr uint8_t ReservedMicrosoft = 0x20;
    static constexpr uint8_t ReservedFuture = 0x00;

  public:
    UUID()
    {
      std::random_device rd;
      std::mt19937 gen(rd());
      for (int i = 0; i < UUIDSize; i+=4)
        *reinterpret_cast<uint32_t*>(m_uuid + i) = gen();

      //SetVariant to ReservedRFC4122
      m_uuid[8] = (m_uuid[8] | ReservedRFC4122) & 0x7F;

      constexpr uint8_t version = 4;

      m_uuid[6] = (m_uuid[6] & 0xF) | (version << 4);
    }

    std::string GetUUIDString() {
      //Guid is 36 characters
      //  Add one byte for the \0
      char s[37];

      std::snprintf(&s[0], sizeof(s), "%2.2x%2.2x%2.2x%2.2x-%2.2x%2.2x-%2.2x%2.2x-%2.2x%2.2x-%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x",
          m_uuid[0], m_uuid[1], m_uuid[2], m_uuid[3], 
          m_uuid[4], m_uuid[5], 
          m_uuid[6], m_uuid[7], 
          m_uuid[8], m_uuid[9],
          m_uuid[10], m_uuid[11], m_uuid[12], m_uuid[13], m_uuid[14], m_uuid[15]);

      return std::string(s);
    }
  };
}} // namespace Azure::Core
