// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Provides helper method for using unix time.
 *
 */

#pragma once

#include <azure/core/datetime.hpp>

#include <chrono>

namespace Azure { namespace Security { namespace KeyVault { namespace Common { namespace Internal {

  /**
   * @brief Provides convertion methods for unix time to Azure Core Datetime.
   */
  class UnixTimeConverter {
  public:
    /**
     * @brief Converts unix time to a #Azure::Core::Datetime.
     *
     * @param unixTime The number of seconds since 1970.
     * @return Calculated Datetime.
     */
    static inline Azure::Core::DateTime UnixTimeToDatetime(uint64_t unixTime)
    {
      return Azure::Core::DateTime(1970) + std::chrono::seconds(unixTime);
    }
  };
}}}}} // namespace Azure::Security::KeyVault::Common::Internal
