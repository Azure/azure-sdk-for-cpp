// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Simulates customer application that is linked with azure-core and azure-transport-curl
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

#define BUFFER_SIZE 50

std::vector<uint8_t> buffer(BUFFER_SIZE);
Http::Request createGetRequest();
Http::Request createPutRequest();
void printRespose(std::unique_ptr<Http::Response> response);

int main()
{
  try
  {
    // Both requests uses a body buffer to be uploaded that would produce responses with bodyBuffer
    auto getRequest = createGetRequest();
    auto putRequest = createPutRequest();

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
    std::unique_ptr<Http::Response> getResponse = httpPipeline.Send(context, getRequest);
    std::unique_ptr<Http::Response> putResponse = httpPipeline.Send(context, putRequest);

    printRespose(std::move(getResponse));
    printRespose(std::move(putResponse));
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

Http::Request createGetRequest()
{
  string host("https://httpbin.org/get");
  cout << "Creating a GET request to" << endl << "Host: " << host << endl;

  auto request = Http::Request(Http::HttpMethod::Get, host, new Http::MemoryBodyStream(buffer));
  request.AddHeader("one", "header");
  request.AddHeader("other", "header2");
  request.AddHeader("header", "value");

  request.AddHeader("Host", "httpbin.org");

  return request;
}

Http::Request createPutRequest()
{
  string host("https://httpbin.org/put");
  cout << "Creating a PUT request to" << endl << "Host: " << host << endl;

  std::fill(buffer.begin(), buffer.end(), 'x');
  buffer[0] = '{';
  buffer[1] = '\"';
  buffer[3] = '\"';
  buffer[4] = ':';
  buffer[5] = '\"';
  buffer[BUFFER_SIZE - 2] = '\"';
  buffer[BUFFER_SIZE - 1] = '}'; // set buffer to look like a Json `{"x":"xxx...xxx"}`

  auto request
      = Http::Request(Http::HttpMethod::Put, host, new Http::MemoryBodyStream(std::move(buffer)));
  request.AddHeader("one", "header");
  request.AddHeader("other", "header2");
  request.AddHeader("header", "value");

  request.AddHeader("Host", "httpbin.org");
  request.AddHeader("Content-Length", std::to_string(BUFFER_SIZE));

  return request;
}

void printRespose(std::unique_ptr<Http::Response> response)
{
  if (response == nullptr)
  {
    cout << "Error. Response returned as null";
    throw;
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
  cout << "Body (buffer):" << endl;
  auto bodyVector = *Http::Response::GetBodyBufferFromStream(response->GetBodyStream()).get();
  cout << std::string(bodyVector.begin(), bodyVector.end());

  return;
}
