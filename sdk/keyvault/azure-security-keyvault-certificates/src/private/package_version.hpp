// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Provides version information.
 */

#pragma once

#include <cstdint>

#define AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_MAJOR 4
#define AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_MINOR 2
#define AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_PATCH 0
#define AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_PRERELEASE "beta.2"

#define AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_ITOA_HELPER(i) #i
#define AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_ITOA(i) \
  AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_ITOA_HELPER(i)

namespace Azure { namespace Security { namespace KeyVault { namespace Certificates {
  namespace _detail {
    /**
     * @brief Provides version information.
     *
     */
    class PackageVersion final {
    public:
      /// Major numeric identifier.
      static constexpr int32_t Major = AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_MAJOR;

      /// Minor numeric identifier.
      static constexpr int32_t Minor = AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_MINOR;

      /// Patch numeric identifier.
      static constexpr int32_t Patch = AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_PATCH;

      /// Indicates whether the SDK is in a pre-release state.
      static constexpr bool IsPreRelease
          = sizeof(AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_PRERELEASE) != sizeof("");

      /**
       * @brief The version in string format used for telemetry following the `semver.org` standard
       * (https://semver.org).
       */
      static constexpr const char* ToString()
      {
        return IsPreRelease
            ? AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_ITOA(AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_MAJOR) "." AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_ITOA(
                AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_MINOR) "." AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_ITOA(AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_PATCH) "-" AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_PRERELEASE
            : AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_ITOA(AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_MAJOR) "." AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_ITOA(
                AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_MINOR) "." AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_ITOA(AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_PATCH);
      }
    };
}}}}} // namespace Azure::Security::KeyVault::Certificates::_detail

#undef AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_ITOA_HELPER
#undef AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_ITOA

#undef AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_MAJOR
#undef AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_MINOR
#undef AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_PATCH
#undef AZURE_SECURITY_KEYVAULT_CERTIFICATES_VERSION_PRERELEASE
