// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "http/pipeline.hpp"

#include <array>
#include <http/curl/curl.hpp>
#include <http/http.hpp>
#include <iostream>
#include <memory>
#include <vector>

using namespace Azure::Core;
using namespace Azure::Core::Http;
using namespace std;

int main()
{
  try
  {
    // Create the Transport
    std::shared_ptr<HttpTransport> transport = std::make_unique<CurlTransport>();

    std::vector<std::unique_ptr<HttpPolicy>> policies;

    // Add the transport policy
    policies.push_back(std::make_unique<TransportPolicy>(std::move(transport)));

    auto httpPipeline = Http::HttpPipeline(policies);

    auto context = Context();

    string host("https://your_account.blob.core.windows.net/container/"
                "file?sp=rcwd&st=2020-07-08T03:47:32Z&se=2020-08-08T11:47:32Z&spr=https,http&sv="
                "2019-10-10&sr=b&sig=xxxxxxxxxxxxxxx");

    std::vector<uint8_t> request_bodydata(100 * 1024 * 1024, '1');

    MemoryBodyStream requestBodyStream(request_bodydata.data(), request_bodydata.size());
    auto request = Http::Request(Http::HttpMethod::Put, host, &requestBodyStream);
    request.AddHeader("Content-Length", std::to_string(request_bodydata.size()));
    request.AddHeader("x-ms-version", "2019-07-07");
    request.AddHeader("x-ms-blob-type", "BlockBlob");

    auto response = httpPipeline.Send(context, request);
  }
  catch (Http::CouldNotResolveHostException& e)
  {
    cout << e.what() << endl;
  }
  catch (Http::TransportException& e)
  {
    cout << e.what() << endl;
  }

  return 0;
}
