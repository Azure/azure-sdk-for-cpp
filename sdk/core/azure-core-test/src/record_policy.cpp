// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/test/record_network_call_policy.hpp"
#include "private/environment.hpp"

using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http;

/**
 * @brief Records network request and response into RecordedData.
 *
 * @param ctx The context for canceling the request.
 * @param request The HTTP request that is sent.
 * @param nextHttpPolicy The next policy in the pipeline.
 * @return The HTTP raw response.
 */
std::unique_ptr<RawResponse> Azure::Core::Test::RecordNetworkCallPolicy::Send(
    Request& request,
    NextHttpPolicy nextHttpPolicy,
    Context const& ctx) const
{
  return nextHttpPolicy.Send(request, ctx);
}

Azure::Core::Test::RecordNetworkCallPolicy::RecordNetworkCallPolicy(
    Azure::Core::Test::RecordedData& recordedData)
    : m_recordedData(recordedData)
{
  m_testMode = Azure::Core::Test::_detail::Environment::GetTestMode();
}
