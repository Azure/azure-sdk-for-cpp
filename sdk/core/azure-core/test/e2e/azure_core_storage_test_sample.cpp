// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "azure/core/http/curl/curl.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/http/pipeline.hpp"

#include <array>
#include <iostream>
#include <memory>
#include <vector>

using namespace Azure::Core;
using namespace Azure::Core::Http;
using namespace std;

int main()
{

  // Create the Transport
  std::shared_ptr<HttpTransport> transport = std::make_unique<CurlTransport>();

  std::vector<std::unique_ptr<HttpPolicy>> policies;

  // Add the transport policy
  policies.push_back(std::make_unique<TransportPolicy>(std::move(transport)));

  auto httpPipeline = Http::HttpPipeline(policies);

  auto context = Azure::Core::GetApplicationContext();

  // STORAGE_BLOB_WITH_SAS = like
  // "https://account.windows.net/azure/container/blob?sv=...&ss=...&..."
  Azure::Core::Http::Url host(std::getenv("STORAGE_BLOB_WITH_SAS"));

  std::vector<uint8_t> request_bodydata(500 * 1024 * 1024, '1');
  cout << request_bodydata.size() << endl;

  MemoryBodyStream requestBodyStream(request_bodydata.data(), request_bodydata.size());
  auto request = Http::Request(Http::HttpMethod::Put, host, &requestBodyStream);
  request.AddHeader("Content-Length", std::to_string(request_bodydata.size()));
  request.AddHeader("x-ms-version", "2019-07-07");
  request.AddHeader("x-ms-blob-type", "BlockBlob");

  auto response = httpPipeline.Send(context, request);

  auto bodyS = response->GetBodyStream();
  auto body = BodyStream::ReadToEnd(context, *bodyS);
  cout << body.data() << endl;

  return 0;
}
