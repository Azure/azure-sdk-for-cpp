// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#define AZURE_TEMPLATE_VERSION_MAJOR 1
#define AZURE_TEMPLATE_VERSION_MINOR 0
#define AZURE_TEMPLATE_VERSION_PATCH 0
#define AZURE_TEMPLATE_VERSION_PRERELEASE "beta.16"

namespace Azure { namespace Template { namespace Details {

  /**
   * @brief Provides version information.
   */
  class Version {
  public:
    /// Major numeric identifier.
    static constexpr int Major = AZURE_TEMPLATE_VERSION_MAJOR;

    /// Minor numeric identifier.
    static constexpr int Minor = AZURE_TEMPLATE_VERSION_MINOR;

    /// Patch numeric identifier.
    static constexpr int Patch = AZURE_TEMPLATE_VERSION_PATCH;

    /// Optional pre-release identifier. SDK is in a pre-release state when not empty.
    static std::string const PreRelease;

    /**
     * @brief The version in string format used for telemetry following the `semver.org` standard
     * (https://semver.org).
     */
    static std::string VersionString();

  private:
    // To avoid leaking out the #define values we smuggle out the value
    // which will later be used to initialize the PreRelease std::string
    static constexpr const char* secret = AZURE_TEMPLATE_VERSION_PRERELEASE;
  };

}}} // namespace Azure::Template::Details

#undef AZURE_TEMPLATE_VERSION_MAJOR
#undef AZURE_TEMPLATE_VERSION_MINOR
#undef AZURE_TEMPLATE_VERSION_PATCH
#undef AZURE_TEMPLATE_VERSION_PRERELEASE
