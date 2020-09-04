// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

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

  Azure::Core::Http::Url host("http://anglesharp.azurewebsites.net/Chunked");

  auto request = Http::Request(Http::HttpMethod::Get, host);

  auto response = httpPipeline.Send(context, request);
  auto response_bodystream = response->GetBodyStream();
  auto response_body = BodyStream::ReadToEnd(context, *response_bodystream);

  cout << response_body.data() << endl;

  return 0;
}
