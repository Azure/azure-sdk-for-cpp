// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Provides version information.
 */

#pragma once

#define AZURE_CORE_VERSION_MAJOR 1
#define AZURE_CORE_VERSION_MINOR 0
#define AZURE_CORE_VERSION_PATCH 0
#define AZURE_CORE_VERSION_PRERELEASE "beta.9"

#define AZURE_CORE_VERSION_ITOA_HELPER(i) #i
#define AZURE_CORE_VERSION_ITOA(i) AZURE_CORE_VERSION_ITOA_HELPER(i)

namespace Azure { namespace Core { namespace _detail {
  /**
   * @brief Provides version information.
   */
  class PackageVersion {
  public:
    /// Major numeric identifier.
    static constexpr int Major = AZURE_CORE_VERSION_MAJOR;

    /// Minor numeric identifier.
    static constexpr int Minor = AZURE_CORE_VERSION_MINOR;

    /// Patch numeric identifier.
    static constexpr int Patch = AZURE_CORE_VERSION_PATCH;

    /// Optional pre-release identifier. SDK is in a pre-release state when not empty.
    static constexpr const char* PreRelease = AZURE_CORE_VERSION_PRERELEASE;

    /**
     * @brief The version in string format used for telemetry following the `semver.org` standard
     * (https://semver.org).
     */
    static constexpr const char* VersionString = sizeof(AZURE_CORE_VERSION_PRERELEASE) != sizeof("")
        ? AZURE_CORE_VERSION_ITOA(AZURE_CORE_VERSION_MAJOR) "." AZURE_CORE_VERSION_ITOA(
            AZURE_CORE_VERSION_MINOR) "." AZURE_CORE_VERSION_ITOA(AZURE_CORE_VERSION_PATCH) "-" AZURE_CORE_VERSION_PRERELEASE
        : AZURE_CORE_VERSION_ITOA(AZURE_CORE_VERSION_MAJOR) "." AZURE_CORE_VERSION_ITOA(
            AZURE_CORE_VERSION_MINOR) "." AZURE_CORE_VERSION_ITOA(AZURE_CORE_VERSION_PATCH);
  };
}}} // namespace Azure::Core::_detail

constexpr const char* namespace Azure::Core::_detail::PackageVersion::PreRelease;
constexpr const char* namespace Azure::Core::_detail::PackageVersion::VersionString;

#undef AZURE_CORE_VERSION_ITOA_HELPER
#undef AZURE_CORE_VERSION_ITOA

#undef AZURE_CORE_VERSION_MAJOR
#undef AZURE_CORE_VERSION_MINOR
#undef AZURE_CORE_VERSION_PATCH
#undef AZURE_CORE_VERSION_PRERELEASE
