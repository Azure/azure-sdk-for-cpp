// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Simulates customer application that is linked with azure-core and azure-transport-curl
 * with Stream Body
 *
 */

#include "http/pipeline.hpp"

#include <http/curl/curl.hpp>
#include <http/http.hpp>
#include <iostream>
#include <memory>

using namespace Azure::Core;
using namespace Azure::Core::Http;
using namespace std;

#define BUFFER_SIZE 1024

int main()
{
  string host("https://httpbin.org/get");
  cout << "testing curl from transport" << endl << "Host: " << host << endl;

  try
  {

    /*     std::array<uint8_t, BUFFER_SIZE> buffer;
        buffer.fill('1');
             buffer[0] = '{';
            buffer[1] = '\"';
            buffer[3] = '\"';
            buffer[4] = ':';
            buffer[5] = '\"';
            buffer[BUFFER_SIZE - 2] = '\"';
            buffer[BUFFER_SIZE - 1] = '}'; // set buffer to look like a Json `{"x":"xxx...xxx"}`

        // Create Memory Stream on top of memory
        auto bodyReader = MemoryBodyStream(buffer.data(), BUFFER_SIZE);
     */
    auto request = Http::Request(Http::HttpMethod::Get, host, BodyType::Stream);

    request.AddHeader("Host", "httpbin.org"); // TODO: make client to add this header

    request.AddHeader("one", "header2");
    request.AddHeader("other", "header2");
    request.AddHeader("header", "value");

    // Create the Transport
    std::shared_ptr<HttpTransport> transport = std::make_unique<CurlTransport>();

    std::vector<std::unique_ptr<HttpPolicy>> policies;
    policies.push_back(std::make_unique<RequestIdPolicy>());

    RetryOptions retryOptions;
    policies.push_back(std::make_unique<RetryPolicy>(retryOptions));

    // Add the transport policy
    policies.push_back(std::make_unique<TransportPolicy>(std::move(transport)));

    auto httpPipeline = Http::HttpPipeline(policies);

    auto context = Context();
    std::unique_ptr<Http::Response> response = httpPipeline.Send(context, request);

    if (response == nullptr)
    {
      cout << "Error. Response returned as null";
      return 0;
    }

    cout << static_cast<typename std::underlying_type<Http::HttpStatusCode>::type>(
        response->GetStatusCode())
         << endl;
    cout << response->GetReasonPhrase() << endl;
    cout << "headers:" << endl;
    for (auto header : response->GetHeaders())
    {
      cout << header.first << " : " << header.second << endl;
    }
    cout << "Body (stream):" << endl;

    uint8_t b[5500];
    auto bodyStream = response->GetBodyStream();
    uint64_t readCount;
    do
    {
      readCount = bodyStream->Read(b, 1000);
      cout << std::string(b, b + readCount);

    } while (readCount > 0);
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
