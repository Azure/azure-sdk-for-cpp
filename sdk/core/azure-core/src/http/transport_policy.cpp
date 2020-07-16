// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <http/policy.hpp>

using namespace Azure::Core::Http;

std::unique_ptr<Response> TransportPolicy::Send(
    Context& ctx,
    Request& request,
    NextHttpPolicy nextHttpPolicy) const
{
  AZURE_UNREFERENCED_PARAMETER(nextHttpPolicy);
  /**
   * The transport policy is always the last policy.
   * Call the transport and return
   */
  auto response = m_transport->Send(ctx, request);
  if (ctx.HasKey("no-download"))
  { // special case to return a response with BodyStream to read directly from socket
    return std::move(response);
  }

  // default behavior for all request is to download body content to Response
  // If ReadToEnd fail, retry policy will eventually call this again
  auto bodyStream = response->GetBodyStream();
  response->SetBody(std::move(BodyStream::ReadToEnd(ctx, *bodyStream)));
  // BodyStream is moved out of response. This makes transport implementation to clean any active
  // session with sockets or internal state.
  return response;
}
