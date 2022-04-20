// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/policies/policy.hpp"

#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
#include "azure/core/http/curl_transport.hpp"
#endif

#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
#include "azure/core/http/win_http_transport.hpp"
#endif

using Azure::Core::Context;
using namespace Azure::Core::IO;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;

std::shared_ptr<HttpTransport> Azure::Core::Http::Policies::_detail::CreateTransportAdapter(
    Azure::Core::Http::_internal::HttpServiceTransportOptions const& options)
{
  (void)options;
  // The order of these checks is important so that WinHTTP is picked over libcurl on Windows, when
  // both are defined.
#if defined(BUILD_TRANSPORT_CUSTOM_ADAPTER)
  // Customer must implement this function and handle setting serviceOptions field
  return ::AzureSdkGetCustomHttpTransport();
#elif defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
  Azure::Core::Http::WinHttpTransportOptions op;
  if (options.IgnoreClientCertificateAuthenticationOnWinHttp)
  {
    op.IgnoreClientCertificate = options.IgnoreClientCertificateAuthenticationOnWinHttp;
  }
  return std::make_shared<Azure::Core::Http::WinHttpTransport>(op);
#elif defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
  return std::make_shared<Azure::Core::Http::CurlTransport>();
#else
  // compilation error to tell customers there's no transport adapter to be used
  return std::shared_ptr<HttpTransport>();
#endif
}

std::unique_ptr<RawResponse> TransportPolicy::Send(
    Request& request,
    NextHttpPolicy nextPolicy,
    Context const& context) const
{
  (void)nextPolicy;
  context.ThrowIfCancelled();

  /*
   * The transport policy is always the last policy.
   *
   * Default behavior for all requests is to download the full response to the RawResponse's
   * buffer.
   *
   ********************************** Notes ************************************************
   *
   * - If ReadToEnd() fails while downloading all the response, the retry policy will make sure to
   * re-send the request to re-start the download.
   *
   * - If the request returns error (statusCode >= 300), even if `request.ShouldBufferResponse()`,
   *the response will be download to the response's buffer.
   *
   ***********************************************************************************
   *
   */
  auto response = m_options.Transport->Send(request, context);
  auto statusCode = static_cast<typename std::underlying_type<Http::HttpStatusCode>::type>(
      response->GetStatusCode());

  // special case to return a response with BodyStream to read directly from socket
  // Return only if response did not fail.
  if (!request.ShouldBufferResponse() && statusCode < 300)
  {
    return response;
  }

  // At this point, either the request is `shouldBufferResponse` or it return with an error code.
  // The entire payload needs must be downloaded to the response's buffer.
  auto bodyStream = response->ExtractBodyStream();
  response->SetBody(bodyStream->ReadToEnd(context));

  // BodyStream is moved out of response. This makes transport implementation to clean any active
  // session with sockets or internal state.
  return response;
}
