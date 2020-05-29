// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "context.hpp"
#include "http.hpp"
#include "policy.hpp"
#include "transport.hpp"

#include <vector>

namespace Azure { namespace Core { namespace Http {

  class HttpPipeline {
  protected:
    std::vector<std::unique_ptr<HttpPolicy>> m_policies;

  public:
    HttpPipeline(std::vector<std::unique_ptr<HttpPolicy>>& policies)
    {
      m_policies.reserve(policies.size());
      for (auto&& policy : policies)
      {
        m_policies.emplace_back(policy->Clone());
      }
    }

    HttpPipeline(const HttpPipeline& other)
    {
      m_policies.reserve(other.m_policies.size());
      for (auto&& policy : m_policies)
      {
        m_policies.emplace_back(policy->Clone());
      }
    }

    /**
     * @brief Starts the pipeline
     * @param ctx A cancellation token.  Can also be used to provide overrides to individual policies
     * @param request The request to be processed
     * @return unique_ptr<Response>
    */
    std::unique_ptr<Response> Send(Context& ctx, Request& request) const
    {
      return m_policies[0]->Send(ctx, request, NextHttpPolicy(0, &m_policies));
    }
  };
}}} // namespace Azure::Core::Http
