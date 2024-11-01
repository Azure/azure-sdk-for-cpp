// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Declaration of the UserAgentHelper type.
 */

#pragma once

#include <string>

namespace Azure { namespace Core { namespace Http { namespace _internal {
  /**
   * @brief Telemetry User-Agent string helper.
   *
   */
  class UserAgentHelper {
  public:
    /**
     * @brief Builds User-Agent string for telemetry.
     *
     * @param componentName the name of the SDK component.
     * @param componentVersion the version of the SDK component.
     * @param applicationId user application ID
     * @param cplusplusValue value of the `__cplusplus` macro.
     *
     * @return User-Agent string.
     *
     * @see https://azure.github.io/azure-sdk/general_azurecore.html#telemetry-policy
     *
     * @note Values for @a cplusplusValue: `__cplusplus` when value comes from the code being built
     * after the Azure SDK has been built. `0L` when being sent from sample code, `-1L` when being
     * sent from tests code, `-2L` when being sent from the SDK code, and `-3L` when being sent from
     * the SDK code for compatibility reasons.
     *
     */
    static std::string BuildUserAgent(
        std::string const& componentName,
        std::string const& componentVersion,
        std::string const& applicationId,
        long cplusplusValue);
  };
}}}} // namespace Azure::Core::Http::_internal
