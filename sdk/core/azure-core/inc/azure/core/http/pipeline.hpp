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
#include "azure/core/http/policy.hpp"
#include "azure/core/http/transport.hpp"

#include <vector>

namespace Azure { namespace Core { namespace Http {

  /**
   * @brief HTTP pipeline is a stack of HTTP policies that get applied sequentially.
   *
   * @details Every client is expected to have its own HTTP pipeline, consisting of sequence of
   * individual HTTP policies. Policies shape the behavior of how a HTTP request is being handled,
   * ranging from retrying and logging, up to sending a HTTP request over the wire.
   *
   * @remark See #policy.hpp
   */
  class HttpPipeline {
  protected:
    std::vector<std::unique_ptr<HttpPolicy>> m_policies;

  public:
    /**
     * @brief Construct HTTP pipeline with the sequence of HTTP policies provided.
     *
     * @param policies A sequence of #HttpPolicy representing a stack, first element corresponding
     * to the top of the stack.
     *
     * @throw `std::invalid_argument` when policies is empty.
     */
    explicit HttpPipeline(const std::vector<std::unique_ptr<HttpPolicy>>& policies)
    {
      if (policies.size() == 0)
      {
        throw std::invalid_argument("policies cannot be empty");
      }

      m_policies.reserve(policies.size());
      for (auto&& policy : policies)
      {
        m_policies.emplace_back(policy->Clone());
      }
    }

    /**
     * @brief Construct HTTP pipeline with the sequence of HTTP policies provided.
     *
     * @param policies A sequence of #HttpPolicy representing a stack, first element corresponding
     * to the top of the stack.
     *
     * @throw `std::invalid_argument` when policies is empty.
     */
    explicit HttpPipeline(std::vector<std::unique_ptr<HttpPolicy>>&& policies)
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
     * @param other
     */
    HttpPipeline(const HttpPipeline& other)
    {
      m_policies.reserve(other.m_policies.size());
      for (auto&& policy : m_policies)
      {
        m_policies.emplace_back(policy->Clone());
      }
    }

    /**
     * @brief Start the HTTP pipeline.
     *
     * @param ctx #Context so that operation can be canceled.
     * @param request The HTTP request to be processed.
     *
     * @return HTTP response after the request has been processed.
     */
    std::unique_ptr<RawResponse> Send(Context const& ctx, Request& request) const
    {
      // Accessing position zero is fine because pipeline must be constructed with at least one
      // policy.
      return m_policies[0]->Send(ctx, request, NextHttpPolicy(0, &m_policies));
    }
  };
}}} // namespace Azure::Core::Http
