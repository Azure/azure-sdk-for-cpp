// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief HTTP pipeline is a stack of HTTP policies.
 * @remark See #policy.hpp
 */

#pragma once

#include "azure/core/context.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/http/policies/policy.hpp"
#include "azure/core/http/transport.hpp"
#include "azure/core/internal/client_options.hpp"

#include <memory>
#include <vector>

namespace Azure { namespace Core { namespace Http { namespace _internal {

  /**
   * @brief HTTP pipeline is a stack of HTTP policies that get applied sequentially.
   *
   * @details Every client is expected to have its own HTTP pipeline, consisting of sequence of
   * individual HTTP policies. Policies shape the behavior of how a HTTP request is being handled,
   * ranging from retrying and logging, up to sending a HTTP request over the wire.
   *
   * @remark See #policy.hpp
   */
  class HttpPipeline final {
  protected:
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> m_policies;

  public:
    /**
     * @brief Construct HTTP pipeline with the sequence of HTTP policies provided.
     *
     * @param policies A sequence of #Azure::Core::Http::Policies::HttpPolicy
     * representing a stack, first element corresponding to the top of the stack.
     *
     * @throw `std::invalid_argument` when policies is empty.
     */
    explicit HttpPipeline(
        const std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>>& policies)
    {
      if (policies.size() == 0)
      {
        throw std::invalid_argument("policies cannot be empty");
      }

      m_policies.reserve(policies.size());
      for (auto& policy : policies)
      {
        m_policies.emplace_back(policy->Clone());
      }
    }

    /**
     * @brief Construct a new HTTP Pipeline object from clientOptions.
     *
     * @remark The client options includes per retry and per call policies which are merged with the
     * service-specific per retry policies.
     *
     * @param clientOptions The SDK client options.
     * @param telemetryServiceName The name of the service for sending telemetry.
     * @param telemetryServiceVersion The version of the service for sending telemetry.
     * @param perRetryPolicies The service-specific per retry policies.
     * @param perCallPolicies The service-specific per call policies.
     */
    explicit HttpPipeline(
        Azure::Core::_internal::ClientOptions const& clientOptions,
        std::string const& telemetryServiceName,
        std::string const& telemetryServiceVersion,
        std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>>&& perRetryPolicies,
        std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>>&& perCallPolicies)
    {
      auto const& perCallClientPolicies = clientOptions.PerOperationPolicies;
      auto const& perRetryClientPolicies = clientOptions.PerRetryPolicies;
      // Adding 5 for:
      // - TelemetryPolicy
      // - RequestIdPolicy
      // - RetryPolicy
      // - LogPolicy
      // - TransportPolicy
      auto pipelineSize = perCallClientPolicies.size() + perRetryClientPolicies.size()
          + perRetryPolicies.size() + perCallPolicies.size() + 5;

      m_policies.reserve(pipelineSize);

      // service-specific per call policies
      for (auto& policy : perCallPolicies)
      {
        m_policies.emplace_back(policy->Clone());
      }

      // Request Id
      m_policies.emplace_back(
          std::make_unique<Azure::Core::Http::Policies::_internal::RequestIdPolicy>());
      // Telemetry
      m_policies.emplace_back(
          std::make_unique<Azure::Core::Http::Policies::_internal::TelemetryPolicy>(
              telemetryServiceName, telemetryServiceVersion, clientOptions.Telemetry));

      // client-options per call policies.
      for (auto& policy : perCallClientPolicies)
      {
        m_policies.emplace_back(policy->Clone());
      }

      // Retry policy
      m_policies.emplace_back(std::make_unique<Azure::Core::Http::Policies::_internal::RetryPolicy>(
          clientOptions.Retry));

      // service-specific per retry policies.
      for (auto& policy : perRetryPolicies)
      {
        m_policies.emplace_back(policy->Clone());
      }
      // client options per retry policies.
      for (auto& policy : perRetryClientPolicies)
      {
        m_policies.emplace_back(policy->Clone());
      }

      // logging - won't update request
      m_policies.emplace_back(
          std::make_unique<Azure::Core::Http::Policies::_internal::LogPolicy>(clientOptions.Log));

      // transport
      m_policies.emplace_back(
          std::make_unique<Azure::Core::Http::Policies::_internal::TransportPolicy>(
              clientOptions.Transport));
    }

    /**
     * @brief Construct HTTP pipeline with the sequence of HTTP policies provided.
     *
     * @param policies A sequence of #Azure::Core::Http::Policies::HttpPolicy
     * representing a stack, first element corresponding to the top of the stack.
     *
     * @throw `std::invalid_argument` when policies is empty.
     */
    explicit HttpPipeline(
        std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>>&& policies)
        : m_policies(std::move(policies))
    {
      if (m_policies.size() == 0)
      {
        throw std::invalid_argument("policies cannot be empty");
      }
    }

    /**
     * @brief Copy constructor.
     *
     * @remark \p other is expected to have at least one policy.
     *
     * @param other Another instance of #Azure::Core::Http::_internal::HttpPipeline to create a copy
     * of.
     */
    HttpPipeline(const HttpPipeline& other)
    {
      m_policies.reserve(other.m_policies.size());
      for (auto& policy : other.m_policies)
      {
        m_policies.emplace_back(policy->Clone());
      }
    }

    /**
     * @brief Start the HTTP pipeline.
     *
     * @param request The HTTP request to be processed.
     * @param context A context to control the request lifetime.
     *
     * @return HTTP response after the request has been processed.
     */
    std::unique_ptr<Azure::Core::Http::RawResponse> Send(
        Azure::Core::Http::Request& request,
        Context const& context) const
    {
      // Accessing position zero is fine because pipeline must be constructed with at least one
      // policy.
      return m_policies[0]->Send(
          request, Azure::Core::Http::Policies::NextHttpPolicy(0, m_policies), context);
    }
  };
}}}} // namespace Azure::Core::Http::_internal
