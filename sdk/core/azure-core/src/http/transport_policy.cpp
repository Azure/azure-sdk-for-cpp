// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/policies/policy.hpp"
#include "azure/core/platform.hpp"

#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
#include "azure/core/http/curl_transport.hpp"
#endif

#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
#include "azure/core/http/win_http_transport.hpp"
#endif

#include <sstream>
#include <string>

using Azure::Core::Context;
using namespace Azure::Core::IO;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;

namespace Azure { namespace Core { namespace Http { namespace Policies { namespace _detail {
  namespace {
    bool AnyTransportOptionsSpecified(TransportOptions const& transportOptions)
    {
      return (
          transportOptions.HttpProxy.HasValue() || !transportOptions.ProxyPassword.empty()
          || !transportOptions.ProxyUserName.empty()
          || transportOptions.EnableCertificateRevocationListCheck
          || !transportOptions.ExpectedTlsRootCertificate.empty());
    }

    std::string PemEncodeFromBase64(std::string const& base64, std::string const& pemType)
    {
      std::stringstream rv;
      rv << "-----BEGIN " << pemType << "-----" << std::endl;
      std::string encodedValue(base64);

      // Insert crlf characters every 80 characters into the base64 encoded key to make it
      // prettier.
      size_t insertPos = 80;
      while (insertPos < encodedValue.length())
      {
        encodedValue.insert(insertPos, "\r\n");
        insertPos += 82; /* 80 characters plus the \r\n we just inserted */
      }

      rv << encodedValue << std::endl << "-----END " << pemType << "-----" << std::endl;
      return rv.str();
    }
  } // namespace

  std::shared_ptr<HttpTransport> GetTransportAdapter(TransportOptions const& transportOptions)
  {
    // The order of these checks is important so that WinHTTP is picked over libcurl on
    // Windows, when both are defined.
#if defined(BUILD_TRANSPORT_CUSTOM_ADAPTER)
    return ::AzureSdkGetCustomHttpTransport();
#elif defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
    // Since C++11: If multiple threads attempt to initialize the same static local variable
    // concurrently, the initialization occurs exactly once. We depend on this behavior to ensure
    // that the singleton defaultTransport is correctly initialized.
    static std::shared_ptr<HttpTransport> defaultTransport(std::make_shared<WinHttpTransport>());
    if (AnyTransportOptionsSpecified(transportOptions))
    {
      WinHttpTransportOptions httpOptions;
      if (transportOptions.HttpProxy.HasValue())
      {
        // WinHTTP proxy strings are semicolon separated elements, each of which
        // has the following format:
        //  ([<scheme>=][<scheme>"://"]<server>[":"<port>])
        std::string proxyString;
        proxyString = "http=" + transportOptions.HttpProxy.Value();
        proxyString += ";";
        proxyString += "https=" + transportOptions.HttpProxy.Value();
        httpOptions.ProxyInformation = proxyString;
      }
      httpOptions.ProxyUserName = transportOptions.ProxyUserName;
      httpOptions.ProxyPassword = transportOptions.ProxyPassword;
      return std::make_shared<Azure::Core::Http::WinHttpTransport>(httpOptions);
    }
    else
    {
      //      std::call_once(createTransportOnce, []() {
      //      defaultTransport = std::make_shared<Azure::Core::Http::WinHttpTransport>();
      //    });
      return defaultTransport;
    }
#elif defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
    static std::shared_ptr<HttpTransport> defaultTransport(std::make_shared<CurlTransport>());
    if (AnyTransportOptionsSpecified(transportOptions))
    {
      CurlTransportOptions curlOptions;
      curlOptions.EnableCurlTracing = true;
      if (transportOptions.HttpProxy.HasValue())
      {
        curlOptions.Proxy = transportOptions.HttpProxy;
      }
      if (!transportOptions.ProxyUserName.empty())
      {
        curlOptions.ProxyUsername = transportOptions.ProxyUserName;
      }
      if (!transportOptions.ProxyPassword.empty())
      {
        curlOptions.ProxyPassword = transportOptions.ProxyPassword;
      }

      curlOptions.SslOptions.EnableCertificateRevocationListCheck
          = transportOptions.EnableCertificateRevocationListCheck;

      if (!transportOptions.ExpectedTlsRootCertificate.empty())
      {
        curlOptions.SslOptions.PemEncodedExpectedRootCertificates
            = PemEncodeFromBase64(transportOptions.ExpectedTlsRootCertificate, "CERTIFICATE");
      }

      return std::make_shared<Azure::Core::Http::CurlTransport>(curlOptions);
    }
    return defaultTransport;
#else
    return std::shared_ptr<HttpTransport>();
#endif
  }
}}}}} // namespace Azure::Core::Http::Policies::_detail

TransportPolicy::TransportPolicy(TransportOptions const& options) : m_options(options)
{
  // If there's no transport specified, then we need to create one.
  // If there is one specified, it's an error to specify other options.
  if (m_options.Transport)
  {
#if !defined(BUILD_TRANSPORT_CUSTOM_ADAPTER)
    if (_detail::AnyTransportOptionsSpecified(options))
    {
      AZURE_ASSERT_MSG(
          false, "Invalid parameter: Proxies cannot be specified when a transport is specified.");
    }
#endif
  }
  else
  {
    // Configure a transport adapter based on the options and compiler switches.
    m_options.Transport = _detail::GetTransportAdapter(m_options);
  }
}

std::unique_ptr<RawResponse> TransportPolicy::Send(
    Request& request,
    NextHttpPolicy,
    Context const& context) const
{
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
