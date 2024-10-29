// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Declaration of the UserAgentGenerator type.
 */

#pragma once

#include <string>

namespace Azure { namespace Core { namespace Http { namespace _internal {
  /**
   * @brief Telemetry User-Agent string generator.
   *
   */
  class UserAgentGenerator {
  public:
    /**
     * @brief Generates User-Agent string for telemetry.
     *
     * @param componentName the name of the SDK component.
     * @param componentVersion the version of the SDK component.
     * @param applicationId user application ID
     *
     * @return User-Agent string.
     *
     * @see https://azure.github.io/azure-sdk/general_azurecore.html#telemetry-policy
     *
     */
    static std::string GenerateUserAgent(
        std::string const& componentName,
        std::string const& componentVersion,
        std::string const& applicationId);
  };
}}}} // namespace Azure::Core::Http::_internal
