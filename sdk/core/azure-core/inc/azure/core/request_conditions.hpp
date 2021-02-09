// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define RequestConditions
 */

#pragma once

#include "azure/core/datetime.hpp"
#include "azure/core/etag.hpp"
#include "azure/core/match_conditions.hpp"

#include <string>

namespace Azure { namespace Core {

  /**
   * @brief Specifies HTTP options for conditional requests based on modification time.
   */
  struct RequestConditions : MatchConditions
  {
    /**
     * @brief Optionally limit requests to resources that have only been modified since this point
     * in time.
     */
    Azure::Core::DateTime IfModifiedSince;

    /**
     * @brief Optionally limit requests to resources that have remained unmodified.
     */
    Azure::Core::DateTime IfUnmodifiedSince;
  };
}} // namespace Azure::Core
