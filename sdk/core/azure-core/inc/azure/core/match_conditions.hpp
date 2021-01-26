// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define MatchConditions
 */

#pragma once

#include "azure/core/etag.hpp"
#include "azure/core/nullable.hpp"

#include <string>

namespace Azure { namespace Core {

  /**
   * @brief Specifies HTTP options for conditional requests.
   */
  class MatchConditions {
  public:
    /**
     * @brief Optionally limit requests to resources that have a matching ETag.
     */
    Nullable<ETag> IfMatch;

    /**
     * @brief Optionally limit requests that do not match the ETag.
     */
    Nullable<ETag> IfNoneMatch;
  };
}} // namespace Azure::Core
