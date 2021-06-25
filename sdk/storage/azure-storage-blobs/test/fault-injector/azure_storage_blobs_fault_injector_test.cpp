// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Validates the Azure Storage blobs SDK client with fault responses from server.
 *
 * @note This test requires the Http-fault-injector
 * (https://github.com/Azure/azure-sdk-tools/tree/main/tools/http-fault-injector) running. Follow
 * the instructions to install and run the server before running this test.
 *
 */

#if defined(_MSC_VER)
// For using std::getenv()
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <azure/storage/blobs.hpp>

#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
#include <azure/core/http/curl_transport.hpp>
#endif

#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
#include "azure/core/http/win_http_transport.hpp"
#endif

#include <iostream>
#include <memory>
#include <vector>

/**
 * @brief The options to set the #FaultInjectionClient behavior like the injection server and the
 * http client implementation to use.
 *
 */
struct FaultInjectionClientOptions
{
  Azure::Core::Url m_url;
  std::shared_ptr<Azure::Core::Http::HttpTransport> m_transport;
};

/**
 * @brief An special http policy to redirect requests to the Fault injector server.
 *
 */
class FaultInjectionClient : public Azure::Core::Http::HttpTransport {
private:
  FaultInjectionClientOptions m_options;

public:
  FaultInjectionClient(FaultInjectionClientOptions options) : m_options(std::move(options)) {}

  std::unique_ptr<Azure::Core::Http::RawResponse> Send(
      Azure::Core::Http::Request& request,
      Azure::Core::Context const& context) override
  {
    auto redirectRequest = Azure::Core::Http::Request(
        request.GetMethod(), Azure::Core::Url(m_options.m_url.GetAbsoluteUrl()));
    for (auto& header : request.GetHeaders())
    {
      redirectRequest.SetHeader(header.first, header.second);
    }

    {
      auto& url = request.GetUrl();
      auto port = url.GetPort();
      redirectRequest.SetHeader(
          "Host", url.GetHost() + (port != 0 ? ":" + std::to_string(port) : ""));
    }

    return m_options.m_transport->Send(redirectRequest, context);
  }
};

int main()
{
  /* The transport adapter must allow insecure SSL certs.
  If both curl and winHttp are available, curl is preferred for this test.for*/
#if defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
  Azure::Core::Http::CurlTransportOptions curlOptions;
  curlOptions.SslVerifyPeer = false;
  auto implementationClient = std::make_shared<Azure::Core::Http::CurlTransport>(curlOptions);

#elif (BUILD_TRANSPORT_WINHTTP_ADAPTER)
  // TODO: make winHTTP to support insecure SSL certs
  Azure::Core::Http::WinHttpTransportOptions winHttpOptions;
  auto implementationClient = std::make_shared<Azure::Core::Http::WinHttpTransport>(winHttpOptions);
#endif

  std::string connectionString(std::getenv("STORAGE_CONNECTION_STRING"));

  // Set the options for the FaultInjectorClient
  FaultInjectionClientOptions options;
  options.m_url = Azure::Core::Url("https://localhost:7778");
  options.m_transport = implementationClient;

  // Set the FaultInjectorClient as the transport adapter for the blobs client.
  Azure::Storage::Blobs::BlobClientOptions blobClientOptions;
  blobClientOptions.Transport.Transport = std::make_shared<FaultInjectionClient>(options);

  auto blobClient = Azure::Storage::Blobs::BlobClient::CreateFromConnectionString(
      connectionString, "sample", "sample.txt", blobClientOptions);

  std::cout << "Sending request..." << std::endl;

  auto response = blobClient.Download();
  auto content = response.Value.BodyStream->ReadToEnd();

  std::cout << "Content: " << std::string(content.begin(), content.end()) << std::endl;

  return 0;
}
