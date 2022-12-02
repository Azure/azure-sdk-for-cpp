// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Provides version information.
 */

#pragma once

#define AZURE_ATTESTATION_VERSION_MAJOR 1
#define AZURE_ATTESTATION_VERSION_MINOR 1
#define AZURE_ATTESTATION_VERSION_PATCH 0
#define AZURE_ATTESTATION_VERSION_PRERELEASE "beta.2"

#define AZURE_ATTESTATION_VERSION_ITOA_HELPER(i) #i
#define AZURE_ATTESTATION_VERSION_ITOA(i) AZURE_ATTESTATION_VERSION_ITOA_HELPER(i)

namespace Azure { namespace Security { namespace Attestation { namespace _detail {
  /**
   * @brief Provides version information.
   */
  class PackageVersion final {
  public:
    /**
     * @brief  Major numeric identifier.
     */
    static constexpr int Major = AZURE_ATTESTATION_VERSION_MAJOR;

    /**
     * @brief  Minor numeric identifier.
     */
    static constexpr int Minor = AZURE_ATTESTATION_VERSION_MINOR;

    /**
     * @brief  Patch numeric identifier.
     */
    static constexpr int Patch = AZURE_ATTESTATION_VERSION_PATCH;

    /**
     * @brief Indicates whether the SDK is in a pre-release state.
     */
    static constexpr bool IsPreRelease = sizeof(AZURE_ATTESTATION_VERSION_PRERELEASE) != sizeof("");

    /**
     * @brief The version in string format used for telemetry following the `semver.org` standard
     * (https://semver.org).
     */
    static constexpr const char* ToString()
    {
      return IsPreRelease
          ? AZURE_ATTESTATION_VERSION_ITOA(AZURE_ATTESTATION_VERSION_MAJOR) "." AZURE_ATTESTATION_VERSION_ITOA(
              AZURE_ATTESTATION_VERSION_MINOR) "." AZURE_ATTESTATION_VERSION_ITOA(AZURE_ATTESTATION_VERSION_PATCH) "-" AZURE_ATTESTATION_VERSION_PRERELEASE
          : AZURE_ATTESTATION_VERSION_ITOA(AZURE_ATTESTATION_VERSION_MAJOR) "." AZURE_ATTESTATION_VERSION_ITOA(
              AZURE_ATTESTATION_VERSION_MINOR) "." AZURE_ATTESTATION_VERSION_ITOA(AZURE_ATTESTATION_VERSION_PATCH);
    }
  };
}}}} // namespace Azure::Security::Attestation::_detail

#undef AZURE_ATTESTATION_VERSION_ITOA_HELPER
#undef AZURE_ATTESTATION_VERSION_ITOA

#undef AZURE_ATTESTATION_VERSION_MAJOR
#undef AZURE_ATTESTATION_VERSION_MINOR
#undef AZURE_ATTESTATION_VERSION_PATCH
#undef AZURE_ATTESTATION_VERSION_PRERELEASE