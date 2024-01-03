// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Provides version information.
 */

#pragma once

#define AZURE_CORE_XML_VERSION_MAJOR 1
#define AZURE_CORE_XML_VERSION_MINOR 0
#define AZURE_CORE_XML_VERSION_PATCH 0
#define AZURE_CORE_XML_VERSION_PRERELEASE ""

#define AZURE_CORE_XML_VERSION_ITOA_HELPER(i) #i
#define AZURE_CORE_XML_VERSION_ITOA(i) AZURE_CORE_XML_VERSION_ITOA_HELPER(i)

namespace Azure { namespace Core { namespace Xml { namespace _detail {
  /**
   * @brief Provides version information.
   */
  class PackageVersion final {
  public:
    /**
     * @brief Major numeric identifier.
     */
    static constexpr int Major = AZURE_CORE_XML_VERSION_MAJOR;

    /**
     * @brief Minor numeric identifier.
     */
    static constexpr int Minor = AZURE_CORE_XML_VERSION_MINOR;

    /**
     * @brief Patch numeric identifier.
     */
    static constexpr int Patch = AZURE_CORE_XML_VERSION_PATCH;

    /**
     * @brief Indicates whether the SDK is in a pre-release state.
     */
    static constexpr bool IsPreRelease = sizeof(AZURE_CORE_XML_VERSION_PRERELEASE) != sizeof("");

    /**
     * @brief The version in string format used for telemetry following the `semver.org`
     * standard (https://semver.org).
     */
    static constexpr const char* ToString()
    {
      return IsPreRelease
          ? AZURE_CORE_XML_VERSION_ITOA(AZURE_CORE_XML_VERSION_MAJOR) "." AZURE_CORE_XML_VERSION_ITOA(
              AZURE_CORE_XML_VERSION_MINOR) "." AZURE_CORE_XML_VERSION_ITOA(AZURE_CORE_XML_VERSION_PATCH) "-" AZURE_CORE_XML_VERSION_PRERELEASE
          : AZURE_CORE_XML_VERSION_ITOA(AZURE_CORE_XML_VERSION_MAJOR) "." AZURE_CORE_XML_VERSION_ITOA(
              AZURE_CORE_XML_VERSION_MINOR) "." AZURE_CORE_XML_VERSION_ITOA(AZURE_CORE_XML_VERSION_PATCH);
    }
  };
}}}} // namespace Azure::Core::Xml::_detail

#undef AZURE_CORE_XML_VERSION_ITOA_HELPER
#undef AZURE_CORE_XML_VERSION_ITOA

#undef AZURE_CORE_XML_VERSION_MAJOR
#undef AZURE_CORE_XML_VERSION_MINOR
#undef AZURE_CORE_XML_VERSION_PATCH
#undef AZURE_CORE_XML_VERSION_PRERELEASE
