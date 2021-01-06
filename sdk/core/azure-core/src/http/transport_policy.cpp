// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/policy.hpp"

using Azure::Core::Context;
using namespace Azure::Core::Http;

std::shared_ptr<HttpTransport> Azure::Core::Http::Details::GetTransportAdapter()
{
  // The order of these checks is important so that WinHttp is picked over Curl on Windows, when
  // both are defined.
#if defined(BUILD_TRANSPORT_CUSTOM_ADAPTER)
  return ::AzureSdkGetCustomHttpTransport();
#elif defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
  return std::make_shared<Azure::Core::Http::WinHttpTransport>();
#else
  return std::make_shared<Azure::Core::Http::CurlTransport>();
#endif
}

std::unique_ptr<RawResponse> TransportPolicy::Send(
    Context const& ctx,
    Request& request,
    NextHttpPolicy nextHttpPolicy) const
{
  (void)nextHttpPolicy;
  ctx.ThrowIfCancelled();

  /**
   * The transport policy is always the last policy.
   * Call the transport and return
   */
  auto response = m_options.Transport->Send(ctx, request);
  auto statusCode = static_cast<typename std::underlying_type<Http::HttpStatusCode>::type>(
      response->GetStatusCode());

  if (request.IsDownloadViaStream() && statusCode < 300)
  { // special case to return a response with BodyStream to read directly from socket
    // Return only if response is valid (less than 300)
    return response;
  }

  // default behavior for all request is to download body content to Response
  // If ReadToEnd fail, retry policy will eventually call this again
  // Using DownloadViaStream and getting an error code would also get to here to download error from
  // body
  auto bodyStream = response->GetBodyStream();
  response->SetBody(BodyStream::ReadToEnd(ctx, *bodyStream));
  // BodyStream is moved out of response. This makes transport implementation to clean any active
  // session with sockets or internal state.
  return response;
}
