// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/test/playback_test_proxy_policy.hpp"
#include "azure/core/test/network_models.hpp"
#include "azure/core/test/test_context_manager.hpp"

#include <azure/core/internal/strings.hpp>

#include <string>
#include <vector>

using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http;
using namespace Azure::Core::Test;
using namespace Azure::Core::_internal;

/**
 * @brief Records network request and response into RecordedData.
 *
 * @param ctx The context for canceling the request.
 * @param request The HTTP request that is sent.
 * @param nextHttpPolicy The next policy in the pipeline.
 * @return The HTTP raw response.
 */
std::unique_ptr<RawResponse> PlaybackTestProxyPolicy::Send(
    Request& request,
    NextHttpPolicy nextHttpPolicy,
    Context const& ctx) const
{
  std::string const recordId(m_testProxy->GetRecordingId());
  if (recordId.empty())
  {
    return nextHttpPolicy.Send(request, ctx);
  }
 
  // Use a new request to redirect
  auto redirectRequest = Azure::Core::Http::Request(
      request.GetMethod(), Azure::Core::Url(m_testProxy->GetTestProxy()), request.GetBodyStream());
  if (!request.ShouldBufferResponse())
  {
    // This is a download with keep connection open. Let's switch the request
    redirectRequest = Azure::Core::Http::Request(
        request.GetMethod(), Azure::Core::Url(m_testProxy->GetTestProxy()), false);
  }

  redirectRequest.GetUrl().SetPath(request.GetUrl().GetPath());

  // Copy all headers
  for (auto& header : request.GetHeaders())
  {
    redirectRequest.SetHeader(header.first, header.second);
  }
  // QP
  for (auto const& qp : request.GetUrl().GetQueryParameters())
  {
    redirectRequest.GetUrl().AppendQueryParameter(qp.first, qp.second);
  }
  // Set x-recording-upstream-base-uri
  {
    auto const& url = request.GetUrl();
    auto const port = url.GetPort();
    auto const host
        = url.GetScheme() + "://" + url.GetHost() + (port != 0 ? ":" + std::to_string(port) : "");
    redirectRequest.SetHeader("x-recording-upstream-base-uri", host);
  }
  // Set recording-id
  redirectRequest.SetHeader("x-recording-id", recordId);

  // RECORDING mode
  redirectRequest.SetHeader("x-recording-mode", "playback");
  return nextHttpPolicy.Send(redirectRequest, ctx);
}
