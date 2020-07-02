// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdlib> // for abort
#include <new> // for placement new
#include <string>
#include <type_traits>
#include <utility> // for swap and move

namespace Azure { namespace Core {

  class UUID {

  private:
    static const size_t UUIDSize = 16;

    unsigned char m_uuid[UUIDSize];
    // The UUID reserved variants.
    static const unsigned char ReservedNCS = 0x80;
    static const unsigned char ReservedRFC4122 = 0x40;
    static const unsigned char ReservedMicrosoft = 0x20;
    static const unsigned char ReservedFuture = 0x00;

  public:
    UUID()
    {
      std::random_device rd;
      std::mt19937 gen(rd());
      for (int i = 0; i < UUIDSize; i++)
        m_uuid[i] = (unsigned char)gen();

      //SetVariant to ReservedRFC4122
      m_uuid[8] = (m_uuid[8] | ReservedRFC4122) & 0x7F;

      const unsigned char version = 4;

      m_uuid[6] = (m_uuid[6] & 0xF) | (version << 4);
    }

    std::string GetUUIDString() {
      char s[36];

      sprintf(&s[0], "%x%x%x%x-%x%x-%x%x-%x%x-%x%x%x%x%x%x",
          m_uuid[0], m_uuid[1], m_uuid[2], m_uuid[3], 
          m_uuid[4], m_uuid[5], 
          m_uuid[6], m_uuid[7], 
          m_uuid[8], m_uuid[9],
          m_uuid[10], m_uuid[11], m_uuid[12], m_uuid[13], m_uuid[14], m_uuid[15]);

      return std::string(s);
    }
  };
}} // namespace Azure::Core