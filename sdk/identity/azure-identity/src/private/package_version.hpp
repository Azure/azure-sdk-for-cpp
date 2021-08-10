// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Provides version information.
 */

#pragma once

#include <cstdint>

#define AZURE_IDENTITY_VERSION_MAJOR 1
#define AZURE_IDENTITY_VERSION_MINOR 2
#define AZURE_IDENTITY_VERSION_PATCH 0
#define AZURE_IDENTITY_VERSION_PRERELEASE "beta.1"

#define AZURE_IDENTITY_VERSION_ITOA_HELPER(i) #i
#define AZURE_IDENTITY_VERSION_ITOA(i) AZURE_IDENTITY_VERSION_ITOA_HELPER(i)

namespace Azure { namespace Identity { namespace _detail {
  /**
   * @brief Provides version information.
   *
   */
  class PackageVersion final {
  public:
    /// Major numeric identifier.
    static constexpr int32_t Major = AZURE_IDENTITY_VERSION_MAJOR;

    /// Minor numeric identifier.
    static constexpr int32_t Minor = AZURE_IDENTITY_VERSION_MINOR;

    /// Patch numeric identifier.
    static constexpr int32_t Patch = AZURE_IDENTITY_VERSION_PATCH;

    /// Indicates whether the SDK is in a pre-release state.
    static constexpr bool IsPreRelease = sizeof(AZURE_IDENTITY_VERSION_PRERELEASE) != sizeof("");

    /**
     * @brief The version in string format used for telemetry following the `semver.org` standard
     * (https://semver.org).
     */
    static constexpr const char* ToString()
    {
      return IsPreRelease
          ? AZURE_IDENTITY_VERSION_ITOA(AZURE_IDENTITY_VERSION_MAJOR) "." AZURE_IDENTITY_VERSION_ITOA(
              AZURE_IDENTITY_VERSION_MINOR) "." AZURE_IDENTITY_VERSION_ITOA(AZURE_IDENTITY_VERSION_PATCH) "-" AZURE_IDENTITY_VERSION_PRERELEASE
          : AZURE_IDENTITY_VERSION_ITOA(AZURE_IDENTITY_VERSION_MAJOR) "." AZURE_IDENTITY_VERSION_ITOA(
              AZURE_IDENTITY_VERSION_MINOR) "." AZURE_IDENTITY_VERSION_ITOA(AZURE_IDENTITY_VERSION_PATCH);
    }
  };
}}} // namespace Azure::Identity::_detail

#undef AZURE_IDENTITY_VERSION_ITOA_HELPER
#undef AZURE_IDENTITY_VERSION_ITOA

#undef AZURE_IDENTITY_VERSION_MAJOR
#undef AZURE_IDENTITY_VERSION_MINOR
#undef AZURE_IDENTITY_VERSION_PATCH
#undef AZURE_IDENTITY_VERSION_PRERELEASE
