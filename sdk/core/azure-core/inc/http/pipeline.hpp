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
    std::shared_ptr<Transport> m_transport;

  public:
    HttpPipeline(std::unique_ptr<Transport> transport)
    {
      m_policies.push_back(std::make_unique<RequestIdPolicy>());
      RetryOptions retryOptions;
      m_policies.push_back(std::make_unique<RetryPolicy>(retryOptions));
      //Add the transport policy
      m_policies.push_back(std::make_unique<TransportPolicy>(std::move(transport)));
    }

    HttpPipeline(const HttpPipeline& other)
    {
      m_policies.reserve(other.m_policies.size());
      //for (auto&& policy : m_policies)
      //{
        //m_policies.emplace_back(policy->Clone());
      //}
    }

    /**
     * @brief Starts the pipeline
     * @param ctx A cancellation token.  Can also be used to provide overrides to individual policies
     * @param request The request to be processed
     * @return unique_ptr<Response>
    */
    std::unique_ptr<Response> Process(Context& ctx, Request& request) const
    {
      return m_policies[0]->Process(ctx, request, NextHttpPolicy(&m_policies));
    }

    void insert_after(std::size_t idx, std::unique_ptr<HttpPolicy>&& next)
    {
      m_policies.insert(m_policies.begin() + idx, std::move(next));
    }
    void push_back(std::unique_ptr<HttpPolicy>&& next) { m_policies.push_back(std::move(next)); }
  };

}}} // namespace Azure::Core::Http
