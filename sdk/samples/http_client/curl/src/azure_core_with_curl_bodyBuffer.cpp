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

constexpr auto BufferSize = 50;

std::vector<uint8_t> buffer(BufferSize);
Http::Request createGetRequest();
Http::Request createPutRequest();
Http::Request createHeadRequest();
Http::Request createDeleteRequest();
Http::Request createPatchRequest();
void printRespose(std::unique_ptr<Http::Response> response);

int main()
{
  try
  {
    // Both requests uses a body buffer to be uploaded that would produce responses with bodyBuffer
    auto getRequest = createGetRequest();
    auto putRequest = createPutRequest();
    auto headRequest = createHeadRequest();
    auto deleteRequest = createDeleteRequest();
    auto patchRequest = createPatchRequest();

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

    cout << endl << "GET:";
    std::unique_ptr<Http::Response> getResponse = httpPipeline.Send(context, getRequest);
    printRespose(std::move(getResponse));

    cout << endl << "PUT:";
    std::unique_ptr<Http::Response> putResponse = httpPipeline.Send(context, putRequest);
    printRespose(std::move(putResponse));

    cout << endl << "HEAD:";
    std::unique_ptr<Http::Response> headResponse = httpPipeline.Send(context, headRequest);
    printRespose(std::move(headResponse));

    cout << endl << "DELETE:";
    std::unique_ptr<Http::Response> deleteResponse = httpPipeline.Send(context, deleteRequest);
    printRespose(std::move(deleteResponse));

    cout << endl << "PATCH:";
    std::unique_ptr<Http::Response> patchResponse = httpPipeline.Send(context, patchRequest);
    printRespose(std::move(patchResponse));
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
  buffer[BufferSize - 2] = '\"';
  buffer[BufferSize - 1] = '}'; // set buffer to look like a Json `{"x":"xxx...xxx"}`

  auto request
      = Http::Request(Http::HttpMethod::Put, host, new Http::MemoryBodyStream(std::move(buffer)));
  request.AddHeader("one", "header");
  request.AddHeader("other", "header2");
  request.AddHeader("header", "value");

  request.AddHeader("Host", "httpbin.org");
  request.AddHeader("Content-Length", std::to_string(BufferSize));

  return request;
}

void printRespose(std::unique_ptr<Http::Response> response)
{
  if (response == nullptr)
  {
    cout << "Error. Response returned as null";
    throw;
  }

  cout << endl
       << static_cast<typename std::underlying_type<Http::HttpStatusCode>::type>(
              response->GetStatusCode())
       << endl;
  cout << response->GetReasonPhrase() << endl;
  cout << "headers:" << endl;
  for (auto header : response->GetHeaders())
  {
    cout << header.first << " : " << header.second << endl;
  }
  cout << "Body (buffer):" << endl;
  auto responseBodyVector
      = Http::Response::ConstructBodyBufferFromStream(response->GetBodyStream());
  if (responseBodyVector != nullptr)
  {
    // print body only if response has a body. Head Response won't have body
    auto bodyVector = *responseBodyVector.get();
    cout << std::string(bodyVector.begin(), bodyVector.end()) << endl;
  }

  std::cin.ignore();
  return;
}

Http::Request createPatchRequest()
{
  string host("https://httpbin.org/patch");
  cout << "Creating an PATCH request to" << endl << "Host: " << host << endl;

  auto request = Http::Request(Http::HttpMethod::Patch, host);
  request.AddHeader("Host", "httpbin.org");

  return request;
}

Http::Request createDeleteRequest()
{
  string host("https://httpbin.org/delete");
  cout << "Creating an DELETE request to" << endl << "Host: " << host << endl;

  auto request = Http::Request(Http::HttpMethod::Delete, host);
  request.AddHeader("Host", "httpbin.org");

  return request;
}

Http::Request createHeadRequest()
{
  string host("https://httpbin.org/get");
  cout << "Creating an HEAD request to" << endl << "Host: " << host << endl;

  auto request = Http::Request(Http::HttpMethod::Head, host);
  request.AddHeader("Host", "httpbin.org");

  return request;
}
