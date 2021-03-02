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
  class ClientOptions {
  private:
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> m_perOperationPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> m_perRetryPolicies;

  public:
    /**
     * @brief Move each policy from \p options into the new instance.
     *
     */
    explicit ClientOptions(ClientOptions&& options)
        : m_perOperationPolicies(std::move(options.m_perOperationPolicies)),
          m_perRetryPolicies(std::move(options.m_perRetryPolicies))
    {
    }

    /**
     * @brief Copy each policy to the new instance.
     *
     */
    explicit ClientOptions(ClientOptions const& options)
    {
      m_perOperationPolicies.reserve(options.m_perOperationPolicies.size());
      for (auto& policy : options.m_perOperationPolicies)
      {
        m_perOperationPolicies.emplace_back(policy->Clone());
      }
      m_perRetryPolicies.reserve(options.m_perRetryPolicies.size());
      for (auto& policy : options.m_perRetryPolicies)
      {
        m_perRetryPolicies.emplace_back(policy->Clone());
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

    /**
     * @brief Adds a policy into the client.
     *
     * @remark The order of policy while sending the request is controlled by \p order.
     * If you want the policy to execute once per client request use #HttpPolicyOrder::PerCall,
     * otherwise use #HttpPolicyOrder::PerRetry to run the policy for every retry.
     *
     * @param policy The policy instance to be added to the pipeline.
     * @param order The order to execute the policy.
     */
    void AddPolicy(
        std::unique_ptr<Azure::Core::Http::HttpPolicy> policy,
        Azure::Core::Http::HttpPolicyOrder order)
    {
      switch (order)
      {
        case Azure::Core::Http::HttpPolicyOrder::PerCall:
          m_perOperationPolicies.push_back(std::move(policy));
          break;
        case Azure::Core::Http::HttpPolicyOrder::PerRetry:
          m_perRetryPolicies.push_back(std::move(policy));
          break;

        default:
          throw std::invalid_argument("Invalid order parameter");
      }
    }

    /**
     * @brief Get the Per Call Policies.
     *
     */
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> const& GetPerCallPolicies() const
    {
      return m_perOperationPolicies;
    }

    /**
     * @brief Get the Per Retry Policies.
     *
     */
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> const& GerPerRetryPolicies() const
    {
      return m_perRetryPolicies;
    }
  };

}}} // namespace Azure::Core::Internal
