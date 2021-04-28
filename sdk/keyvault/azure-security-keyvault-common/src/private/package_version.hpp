// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Provides version information.
 */

#pragma once

#define AZURE_SECURITY_KEYVAULT_COMMON_VERSION_MAJOR 4
#define AZURE_SECURITY_KEYVAULT_COMMON_VERSION_MINOR 0
#define AZURE_SECURITY_KEYVAULT_COMMON_VERSION_PATCH 0
#define AZURE_SECURITY_KEYVAULT_COMMON_VERSION_PRERELEASE "beta.2"

#define AZURE_SECURITY_KEYVAULT_COMMON_VERSION_ITOA_HELPER(i) #i
#define AZURE_SECURITY_KEYVAULT_COMMON_VERSION_ITOA(i) \
  AZURE_SECURITY_KEYVAULT_COMMON_VERSION_ITOA_HELPER(i)

namespace Azure { namespace Security { namespace KeyVault { namespace Common { namespace _detail {
  /**
   * @brief Provides version information.
   */
  class PackageVersion {
  private:
    template <typename = void> struct Strings
    {
      static constexpr const char* PreRelease = AZURE_SECURITY_KEYVAULT_COMMON_VERSION_PRERELEASE;

      static constexpr const char* VersionString
          = sizeof(AZURE_SECURITY_KEYVAULT_COMMON_VERSION_PRERELEASE) != sizeof("")
          ? AZURE_SECURITY_KEYVAULT_COMMON_VERSION_ITOA(AZURE_SECURITY_KEYVAULT_COMMON_VERSION_MAJOR) "." AZURE_SECURITY_KEYVAULT_COMMON_VERSION_ITOA(
              AZURE_SECURITY_KEYVAULT_COMMON_VERSION_MINOR) "." AZURE_SECURITY_KEYVAULT_COMMON_VERSION_ITOA(AZURE_SECURITY_KEYVAULT_COMMON_VERSION_PATCH) "-" AZURE_SECURITY_KEYVAULT_COMMON_VERSION_PRERELEASE
          : AZURE_SECURITY_KEYVAULT_COMMON_VERSION_ITOA(AZURE_SECURITY_KEYVAULT_COMMON_VERSION_MAJOR) "." AZURE_SECURITY_KEYVAULT_COMMON_VERSION_ITOA(
              AZURE_SECURITY_KEYVAULT_COMMON_VERSION_MINOR) "." AZURE_SECURITY_KEYVAULT_COMMON_VERSION_ITOA(AZURE_SECURITY_KEYVAULT_COMMON_VERSION_PATCH);
    };

  public:
    /// Major numeric identifier.
    static constexpr int Major = AZURE_SECURITY_KEYVAULT_COMMON_VERSION_MAJOR;

    /// Minor numeric identifier.
    static constexpr int Minor = AZURE_SECURITY_KEYVAULT_COMMON_VERSION_MINOR;

    /// Patch numeric identifier.
    static constexpr int Patch = AZURE_SECURITY_KEYVAULT_COMMON_VERSION_PATCH;

    /// Optional pre-release identifier. SDK is in a pre-release state when not empty.
    static constexpr const char* PreRelease = Strings<>::PreRelease;

    /**
     * @brief The version in string format used for telemetry following the `semver.org` standard
     * (https://semver.org).
     */
    static constexpr const char* VersionString = Strings<>::VersionString;
  };

}}}}} // namespace Azure::Security::KeyVault::Common::_detail

#undef AZURE_SECURITY_KEYVAULT_COMMON_VERSION_ITOA_HELPER
#undef AZURE_SECURITY_KEYVAULT_COMMON_VERSION_ITOA

#undef AZURE_SECURITY_KEYVAULT_COMMON_VERSION_MAJOR
#undef AZURE_SECURITY_KEYVAULT_COMMON_VERSION_MINOR
#undef AZURE_SECURITY_KEYVAULT_COMMON_VERSION_PATCH
#undef AZURE_SECURITY_KEYVAULT_COMMON_VERSION_PRERELEASE
