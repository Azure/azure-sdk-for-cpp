// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "nullable.hpp"

#include <string>

namespace Azure { namespace Storage {

  /**
   * @brief Specifies HTTP options for conditional requests based on modification time value.
   */
  struct ModifiedTimeAccessConditions
  {
    /**
     * @brief Specify this header to perform the operation only if the resource has been
     * modified since the specified time.
     */
    Azure::Core::Nullable<std::string> IfModifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource has not been
     * modified since the specified date/time.
     */
    Azure::Core::Nullable<std::string> IfUnmodifiedSince;
  };

  /**
   * @brief Specifies HTTP options for conditional requests based on and ETag value.
   */
  struct ETagAccessConditions
  {
    /**
     * @brief Specify this header to perform the operation only if the resource's ETag
     * matches the value specified.
     */
    Azure::Core::Nullable<std::string> IfMatch;

    /**
     * @brief Specify this header to perform the operation only if the resource's ETag does
     * not match the value specified. Specify the wildcard character (*) to perform the operation
     * only if the resource does not exist, and fail the operation if it does exist.
     */
    Azure::Core::Nullable<std::string> IfNoneMatch;
  };

  /**
   * @brief Specifies HTTP options for conditional requests based on lease.
   */
  struct LeaseAccessConditions
  {
    /**
     * @brief Specify this header to perform the operation only if the resource has an
     * active lease mathing this id.
     */
    Azure::Core::Nullable<std::string> LeaseId;
  };

}} // namespace Azure::Storage
