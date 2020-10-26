// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Provides version information.
 */

#pragma once

#include <cstring>
#include <string>

#define AZURE_STORAGE_FILES_DATALAKE_VERSION_MAJOR 12
#define AZURE_STORAGE_FILES_DATALAKE_VERSION_MINOR 0
#define AZURE_STORAGE_FILES_DATALAKE_VERSION_PATCH 0
#define AZURE_STORAGE_FILES_DATALAKE_VERSION_PRERELEASE "beta.4"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  /**
   * @brief Provides version information.
   */
  struct Version
  {
  public:
    /// Major numeric identifier.
    constexpr static int Major = AZURE_STORAGE_FILES_DATALAKE_VERSION_MAJOR;

    /// Minor numeric identifier.
    constexpr static int Minor = AZURE_STORAGE_FILES_DATALAKE_VERSION_MINOR;

    /// Patch numeric identifier.
    constexpr static int Patch = AZURE_STORAGE_FILES_DATALAKE_VERSION_PATCH;

    /// Optional pre-release identifier. SDK is in a pre-release state when not empty.
    constexpr static const char* PreRelease = AZURE_STORAGE_FILES_DATALAKE_VERSION_PRERELEASE;

    /**
     * @brief The version in string format used for telemetry following the `semver.org` standard
     * (https://semver.org).
     */
    static std::string VersionString()
    {
      std::string versionString
          = std::to_string(Major) + "." + std::to_string(Minor) + "." + std::to_string(Patch);
      if (std::strlen(PreRelease) != 0)
      {
        versionString += "-";
        versionString += PreRelease;
      }
      return versionString;
    }
  };

}}}} // namespace Azure::Storage::Files::DataLake

#undef AZURE_STORAGE_FILES_DATALAKE_VERSION_MAJOR
#undef AZURE_STORAGE_FILES_DATALAKE_VERSION_MINOR
#undef AZURE_STORAGE_FILES_DATALAKE_VERSION_PATCH
#undef AZURE_STORAGE_FILES_DATALAKE_VERSION_PRERELEASE
