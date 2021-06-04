// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Provides helper method for using POSIX time.
 *
 */

#pragma once

#include <azure/core/datetime.hpp>

#include <chrono>

namespace Azure { namespace Security { namespace KeyVault { namespace _internal {

  /**
   * @brief Provides convertion methods for POSIX time to Azure Core #Azure::Core::DateTime.
   *
   */
  class UnixTimeConverter final {
  public:
    /**
     * @brief Converts POSIX time to a #Azure::Core::Datetime.
     *
     * @param unixTime The number of seconds since 1970.
     * @return Calculated #Azure::Core::DateTime.
     */
    static inline Azure::DateTime UnixTimeToDatetime(int64_t unixTime)
    {
      return Azure::DateTime(1970) + std::chrono::seconds(unixTime);
    }

    /**
     * @brief Converts an #Azure::Core::DateTime to POSIX time.
     *
     * @param dateTime The date time to convert.
     */
    static inline int64_t DatetimeToUnixTime(Azure::DateTime dateTime)
    {
      //  This count starts at the Unix epoch which is January 1st, 1970 UTC.
      auto secondsSince1970
          = std::chrono::duration_cast<std::chrono::seconds>(dateTime - Azure::DateTime(1970));
      return secondsSince1970.count();
    }
  };
}}}} // namespace Azure::Security::KeyVault::_internal
