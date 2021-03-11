// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define ModifiedConditions
 */

#pragma once

#include "azure/core/datetime.hpp"
#include "azure/core/nullable.hpp"

#include <string>

namespace Azure {

/**
 * @brief Specifies HTTP options for conditional requests based on modification time.
 */
struct ModifiedConditions
{
  /**
   * @brief Optionally limit requests to resources that have only been modified since this point
   * in time.
   */
  Azure::Core::Nullable<Azure::Core::DateTime> IfModifiedSince;

  /**
   * @brief Optionally limit requests to resources that have remained unmodified.
   */
  Azure::Core::Nullable<Azure::Core::DateTime> IfUnmodifiedSince;
};
} // namespace Azure
