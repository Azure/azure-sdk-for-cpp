// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Base type for all client option types, exposes various common client options like Retry
 * and Transport.
 */

#pragma once

#include "azure/core/http/http.hpp"
#include "azure/core/http/policy.hpp"

#include <memory>
#include <vector>

namespace Azure { namespace Core { namespace Internal {

  /**
   * @brief  Base type for all client option types, exposes various common client options like Retry
   * and Transport.
   *
   */
  struct ClientOptions
  {

    /**
     * @brief Define policies to be called one time for every Http request from an sdk client.
     *
     */
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> PerOperationPolicies;

    /**
     * @brief Define policies to be called each time and sdk client tries to send the Http request.
     *
     */
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> PerRetryPolicies;

    /**
     * @brief Move each policy from \p options into the new instance.
     *
     */
    ClientOptions(ClientOptions&& options) = default;

    /**
     * @brief Copy each policy to the new instance.
     *
     */
    ClientOptions(ClientOptions const& options)
        : Retry(options.Retry), Transport(options.Transport), Telemetry(options.Telemetry)
    {
      PerOperationPolicies.reserve(options.PerOperationPolicies.size());
      for (auto& policy : options.PerOperationPolicies)
      {
        PerOperationPolicies.emplace_back(policy->Clone());
      }
      PerRetryPolicies.reserve(options.PerRetryPolicies.size());
      for (auto& policy : options.PerRetryPolicies)
      {
        PerRetryPolicies.emplace_back(policy->Clone());
      }
    }

    ClientOptions() = default;

    /**
     * @brief Specify the number of retries and other retry-related options.
     */
    Azure::Core::Http::RetryOptions Retry;

    /**
     * @brief Customized HTTP client. We're going to use the default one if this is empty.
     */
    Azure::Core::Http::TransportOptions Transport;

    /**
     * @brief Telemetry options.
     */
    Azure::Core::Http::TelemetryOptions Telemetry;
  };

}}} // namespace Azure::Core::Internal
