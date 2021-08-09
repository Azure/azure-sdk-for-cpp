// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief HTTP Pipeline policy that keeps track of each HTTP request and response that
 * flows through the pipeline.
 */

#pragma once

#include <memory>
#include <string>

#include <azure/core/context.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/response.hpp>

#include "azure/core/test/network_models.hpp"

namespace Azure { namespace Core { namespace Test {

  // Partial class. Required to reference the Interceptor that is defined in the implementation.
  class InterceptorManager;

  /**
   * @brief Creates a policy that records network calls into recordedData.
   *
   */
  class RecordNetworkCallPolicy : public Azure::Core::Http::Policies::HttpPolicy {
  private:
    Azure::Core::Test::InterceptorManager* m_interceptorManager;

  public:
    /**
     * @brief Disable default constructor.
     *
     */
    RecordNetworkCallPolicy() = delete;

    /**
     * @brief Construct the record network policy which will save the HTTP request and response to
     * the \p recordedData.
     *
     * @param interceptorManager A reference to the interceptor manager which holds the recorded
     * data.
     */
    RecordNetworkCallPolicy(Azure::Core::Test::InterceptorManager* interceptorManager)
        : m_interceptorManager(interceptorManager)
    {
    }

    /**
     * @brief Cronstructs a new record network policy with the same recorded data.
     *
     * @return A record network policy with the same recorded data.
     */
    std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy> Clone() const override
    {
      return std::make_unique<RecordNetworkCallPolicy>(m_interceptorManager);
    }

    /**
     * @brief Record HTTP data from request, then call next policy. Record HTTP response before
     * returning.
     *
     * @param ctx The context while sending the request to the network.
     * @param request The HTTP request details.
     * @param nextHttpPolicy The next policy in the pipeline to be called.
     * @return The HTTP raw response from the network after it is recorded.
     */
    std::unique_ptr<Azure::Core::Http::RawResponse> Send(
        Azure::Core::Http::Request& request,
        Azure::Core::Http::Policies::NextHttpPolicy nextHttpPolicy,
        const Azure::Core::Context& ctx) const override;
  };

}}} // namespace Azure::Core::Test
