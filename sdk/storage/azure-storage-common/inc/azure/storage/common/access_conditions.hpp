// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include <azure/core/nullable.hpp>

#include "azure/storage/common/storage_common.hpp"

namespace Azure { namespace Storage {

  /**
   * @brief Specifies HTTP options for conditional requests based on modification time value.
   */
  struct ModifiedTimeConditions
  {
    /**
     * @brief Specify this header to perform the operation only if the resource has been
     * modified since the specified time.
     */
    Azure::Core::Nullable<Azure::Core::DateTime> IfModifiedSince;

    /**
     * @brief Specify this header to perform the operation only if the resource has not been
     * modified since the specified date/time.
     */
    Azure::Core::Nullable<Azure::Core::DateTime> IfUnmodifiedSince;
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

  /**
   * @brief Specifies HTTP options for conditional requests based on ContentHash.
   */
  struct ContentHashAccessConditions
  {
    /**
     * @brief Specify this header to perform the operation only if the resource's ContentHash
     * matches the value specified.
     */
    Azure::Core::Nullable<ContentHash> IfMatchContentHash;

    /**
     * @brief Specify this header to perform the operation only if the resource's ContentHash does
     * not match the value specified.
     */
    Azure::Core::Nullable<ContentHash> IfNoneMatchContentHash;
  };

}} // namespace Azure::Storage
