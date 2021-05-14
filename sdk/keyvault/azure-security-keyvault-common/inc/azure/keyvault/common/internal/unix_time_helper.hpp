// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Provides helper method for using unix time.
 *
 */

#pragma once

#include <azure/core/datetime.hpp>

#include <chrono>

namespace Azure { namespace Security { namespace KeyVault { namespace _internal {

  /**
   * @brief Provides convertion methods for unix time to Azure Core Datetime.
   *
   */
  class UnixTimeConverter final {
  public:
    /**
     * @brief Converts unix time to a #Azure::Core::Datetime.
     *
     * @param unixTime The number of seconds since 1970.
     * @return Calculated Datetime.
     */
    static inline Azure::DateTime UnixTimeToDatetime(uint64_t unixTime)
    {
      return Azure::DateTime(1970) + std::chrono::seconds(unixTime);
    }

    /**
     * @brief Converts a #Azure::Core::Datetime to unix time.
     *
     * @param dateTime The date time to convert.
     */
    static inline uint64_t DatetimeToUnixTime(Azure::DateTime dateTime)
    {
      //  This count starts at the Unix Epoch which was January 1st, 1970 at UTC.
      auto secondsSince1970
          = std::chrono::duration_cast<std::chrono::seconds>(dateTime - Azure::DateTime(1970));
      return secondsSince1970.count();
    }
  };
}}}} // namespace Azure::Security::KeyVault::_internal
