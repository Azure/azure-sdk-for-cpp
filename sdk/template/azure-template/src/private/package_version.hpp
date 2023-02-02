// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Provides version information.
 */

#pragma once

#define AZURE_TEMPLATE_VERSION_MAJOR 1
#define AZURE_TEMPLATE_VERSION_MINOR 0
#define AZURE_TEMPLATE_VERSION_PATCH 0
#define AZURE_TEMPLATE_VERSION_PRERELEASE ""

#define AZURE_TEMPLATE_VERSION_ITOA_HELPER(i) #i
#define AZURE_TEMPLATE_VERSION_ITOA(i) AZURE_TEMPLATE_VERSION_ITOA_HELPER(i)

namespace Azure { namespace Template { namespace _detail {
  /**
   * @brief Provides version information.
   */
  class PackageVersion final {
  public:
    /**
     * @brief Major numeric identifier.
     */
    static constexpr int Major = AZURE_TEMPLATE_VERSION_MAJOR;

    /**
     * @brief Minor numeric identifier.
     */
    static constexpr int Minor = AZURE_TEMPLATE_VERSION_MINOR;

    /**
     * @brief Patch numeric identifier.
     */
    static constexpr int Patch = AZURE_TEMPLATE_VERSION_PATCH;

    /**
     * @brief Indicates whether the SDK is in a pre-release state.
     */
    static constexpr bool IsPreRelease = sizeof(AZURE_TEMPLATE_VERSION_PRERELEASE) != sizeof("");

    /**
     * @brief The version in string format used for telemetry following the `semver.org` standard
     * (https://semver.org).
     */
    static constexpr const char* ToString()
    {
      return IsPreRelease
          ? AZURE_TEMPLATE_VERSION_ITOA(AZURE_TEMPLATE_VERSION_MAJOR) "." AZURE_TEMPLATE_VERSION_ITOA(
              AZURE_TEMPLATE_VERSION_MINOR) "." AZURE_TEMPLATE_VERSION_ITOA(AZURE_TEMPLATE_VERSION_PATCH) "-" AZURE_TEMPLATE_VERSION_PRERELEASE
          : AZURE_TEMPLATE_VERSION_ITOA(AZURE_TEMPLATE_VERSION_MAJOR) "." AZURE_TEMPLATE_VERSION_ITOA(
              AZURE_TEMPLATE_VERSION_MINOR) "." AZURE_TEMPLATE_VERSION_ITOA(AZURE_TEMPLATE_VERSION_PATCH);
    }
  };
}}} // namespace Azure::Template::_detail

#undef AZURE_TEMPLATE_VERSION_ITOA_HELPER
#undef AZURE_TEMPLATE_VERSION_ITOA

#undef AZURE_TEMPLATE_VERSION_MAJOR
#undef AZURE_TEMPLATE_VERSION_MINOR
#undef AZURE_TEMPLATE_VERSION_PATCH
#undef AZURE_TEMPLATE_VERSION_PRERELEASE
