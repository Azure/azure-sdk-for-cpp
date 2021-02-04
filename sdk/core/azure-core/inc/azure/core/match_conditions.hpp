// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define MatchConditions
 */

#pragma once

#include "azure/core/etag.hpp"

#include <string>

namespace Azure { namespace Core {

  /**
   * @brief Specifies HTTP options for conditional requests.
   */
  struct MatchConditions {
    /**
     * @brief Optionally limit requests to resources that match the value specified.
     */
    ETag IfMatch;

    /**
     * @brief Optionally limit requests to resources that do not match the value specified. Specify
     * Azure::Core::ETag::Any() to limit requests to resources that do not exist.
     */
    ETag IfNoneMatch;
  };
}} // namespace Azure::Core
