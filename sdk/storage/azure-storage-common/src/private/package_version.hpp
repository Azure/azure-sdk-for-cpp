// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Provides version information.
 */

#pragma once

#include <array>

#define AZURE_STORAGE_COMMON_VERSION_MAJOR 12
#define AZURE_STORAGE_COMMON_VERSION_MINOR 3
#define AZURE_STORAGE_COMMON_VERSION_PATCH 0
#define AZURE_STORAGE_COMMON_VERSION_PRERELEASE "beta.1"

#define AZURE_STORAGE_COMMON_VERSION_ITOA_HELPER(i) #i
#define AZURE_STORAGE_COMMON_VERSION_ITOA(i) AZURE_STORAGE_COMMON_VERSION_ITOA_HELPER(i)

namespace Azure { namespace Storage { namespace Common { namespace _detail {
  /**
   * @brief Provides version information.
   */
  class PackageVersion final {
  public:
    /// Major numeric identifier.
    static constexpr int Major = AZURE_STORAGE_COMMON_VERSION_MAJOR;

    /// Minor numeric identifier.
    static constexpr int Minor = AZURE_STORAGE_COMMON_VERSION_MINOR;

    /// Patch numeric identifier.
    static constexpr int Patch = AZURE_STORAGE_COMMON_VERSION_PATCH;

    /// Indicates whether the SDK is in a pre-release state.
    static constexpr bool IsPreRelease
        = sizeof(AZURE_STORAGE_COMMON_VERSION_PRERELEASE) != sizeof("");

    /**
     * @brief The version in string format used for telemetry following the `semver.org` standard
     * (https://semver.org).
     */
    static constexpr const char* ToString()
    {
      return IsPreRelease
          ? AZURE_STORAGE_COMMON_VERSION_ITOA(AZURE_STORAGE_COMMON_VERSION_MAJOR) "." AZURE_STORAGE_COMMON_VERSION_ITOA(
              AZURE_STORAGE_COMMON_VERSION_MINOR) "." AZURE_STORAGE_COMMON_VERSION_ITOA(AZURE_STORAGE_COMMON_VERSION_PATCH) "-" AZURE_STORAGE_COMMON_VERSION_PRERELEASE
          : AZURE_STORAGE_COMMON_VERSION_ITOA(AZURE_STORAGE_COMMON_VERSION_MAJOR) "." AZURE_STORAGE_COMMON_VERSION_ITOA(
              AZURE_STORAGE_COMMON_VERSION_MINOR) "." AZURE_STORAGE_COMMON_VERSION_ITOA(AZURE_STORAGE_COMMON_VERSION_PATCH);
    }
  };
}}}} // namespace Azure::Storage::Common::_detail

#undef AZURE_STORAGE_COMMON_VERSION_ITOA_HELPER
#undef AZURE_STORAGE_COMMON_VERSION_ITOA

#undef AZURE_STORAGE_COMMON_VERSION_MAJOR
#undef AZURE_STORAGE_COMMON_VERSION_MINOR
#undef AZURE_STORAGE_COMMON_VERSION_PATCH
#undef AZURE_STORAGE_COMMON_VERSION_PRERELEASE
