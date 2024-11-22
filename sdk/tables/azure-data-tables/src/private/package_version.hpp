// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Provides version information.
 */

#pragma once

#define AZURE_DATA_TABLES_VERSION_MAJOR 1
#define AZURE_DATA_TABLES_VERSION_MINOR 0
#define AZURE_DATA_TABLES_VERSION_PATCH 0
#define AZURE_DATA_TABLES_VERSION_PRERELEASE "beta.6"

#define AZURE_DATA_TABLES_VERSION_ITOA_HELPER(i) #i
#define AZURE_DATA_TABLES_VERSION_ITOA(i) AZURE_DATA_TABLES_VERSION_ITOA_HELPER(i)

namespace Azure { namespace Data { namespace Tables { namespace _detail {
  /**
   * @brief Provides version information.
   */
  class PackageVersion final {
  public:
    /**
     * @brief Major numeric identifier.
     */
    static constexpr int32_t Major = AZURE_DATA_TABLES_VERSION_MINOR;

    /**
     * @brief Minor numeric identifier.
     */
    static constexpr int32_t Minor = AZURE_DATA_TABLES_VERSION_MINOR;

    /**
     * @brief Patch numeric identifier.
     */
    static constexpr int32_t Patch = AZURE_DATA_TABLES_VERSION_PATCH;

    /**
     * @brief Indicates whether the SDK is in a pre-release state.
     */
    static constexpr bool IsPreRelease = sizeof(AZURE_DATA_TABLES_VERSION_PRERELEASE) != sizeof("");

    /**
     * @brief The version in string format used for telemetry following the `semver.org` standard
     * (https://semver.org).
     */
    static constexpr const char* ToString()
    {
      return IsPreRelease
          ? AZURE_DATA_TABLES_VERSION_ITOA(AZURE_DATA_TABLES_VERSION_MAJOR) "." AZURE_DATA_TABLES_VERSION_ITOA(
              AZURE_DATA_TABLES_VERSION_MINOR) "." AZURE_DATA_TABLES_VERSION_ITOA(AZURE_DATA_TABLES_VERSION_PATCH) "-" AZURE_DATA_TABLES_VERSION_PRERELEASE
          : AZURE_DATA_TABLES_VERSION_ITOA(AZURE_DATA_TABLES_VERSION_MAJOR) "." AZURE_DATA_TABLES_VERSION_ITOA(
              AZURE_DATA_TABLES_VERSION_MINOR) "." AZURE_DATA_TABLES_VERSION_ITOA(AZURE_DATA_TABLES_VERSION_PATCH);
    }
  };
}}}} // namespace Azure::Data::Tables::_detail

#undef AZURE_DATA_TABLES_VERSION_ITOA_HELPER
#undef AZURE_DATA_TABLES_VERSION_ITOA

#undef AZURE_DATA_TABLES_VERSION_MAJOR
#undef AZURE_DATA_TABLES_VERSION_MINOR
#undef AZURE_DATA_TABLES_VERSION_PATCH
#undef AZURE_DATA_TABLES_VERSION_PRERELEASE
