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
    ClientOptions(ClientOptions&&) = default;

    /**
     * @brief Copy each policy to the new instance.
     *
     */
    ClientOptions(ClientOptions const& other) { *this = other; }

    ClientOptions() = default;

    /**
     * @brief Move each policy from \p options into the this instance.
     *
     */
    ClientOptions& operator=(ClientOptions&&) = default;

    /**
     * @brief Copy each policy to the this instance.
     *
     */
    ClientOptions& operator=(const ClientOptions& other)
    {
      this->Retry = other.Retry;
      this->Transport = other.Transport;
      this->Telemetry = other.Telemetry;
      this->PerOperationPolicies.reserve(other.PerOperationPolicies.size());
      for (auto& policy : other.PerOperationPolicies)
      {
        this->PerOperationPolicies.emplace_back(policy->Clone());
      }
      this->PerRetryPolicies.reserve(other.PerRetryPolicies.size());
      for (auto& policy : other.PerRetryPolicies)
      {
        this->PerRetryPolicies.emplace_back(policy->Clone());
      }
      return *this;
    }

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
