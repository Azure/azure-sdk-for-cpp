// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Simulates customer application that is linked with azure-core and azure-transport-curl
 * with Stream Body
 *
 */

#include <azure/core/http/curl/curl.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/pipeline.hpp>

#include <array>
#include <iostream>
#include <memory>
#include <vector>

using namespace Azure::Core;
using namespace Azure::Core::Http;
using namespace std;

// For bodyBuffer
constexpr auto BufferSize = 50;
std::vector<uint8_t> buffer(BufferSize);

// For StreamBody
constexpr auto StreamSize = 1024; // 100 MB
std::array<uint8_t, StreamSize> bufferStream;

void doGetRequest(Context const& context, HttpPipeline& pipeline);
void doNoPathGetRequest(Context const& context, HttpPipeline& pipeline);
void doPutRequest(Context const& context, HttpPipeline& pipeline);
void doPutStreamRequest(Context const& context, HttpPipeline& pipeline);
void printStream(Azure::Core::Context const& context, std::unique_ptr<Http::RawResponse> response);

int main()
{
  try
  {

    // Create the Transport
    std::shared_ptr<HttpTransport> transport = std::make_unique<CurlTransport>();

    std::vector<std::unique_ptr<HttpPolicy>> policies;
    policies.push_back(std::make_unique<RequestIdPolicy>());

    RetryOptions retryOptions;
    policies.push_back(std::make_unique<RetryPolicy>(retryOptions));

    // Add the transport policy
    policies.push_back(std::make_unique<TransportPolicy>(std::move(transport)));

    auto httpPipeline = Http::HttpPipeline(policies);

    std::unique_ptr<Http::RawResponse> response;
    auto context = Azure::Core::GetApplicationContext();

    doGetRequest(context, httpPipeline);
    doPutStreamRequest(context, httpPipeline);
    doNoPathGetRequest(context, httpPipeline);
    doPutRequest(context, httpPipeline);
  }
  catch (Azure::Core::RequestFailedException const& e)
  {
    cout << e.what() << endl;
  }

  return 0;
}

// Request GET with no path
void doNoPathGetRequest(Context const& context, HttpPipeline& pipeline)
{
  Azure::Core::Http::Url host("https://httpbin.org");
  cout << "Creating a GET request to" << endl << "Host: " << host.GetAbsoluteUrl() << endl;

  auto request = Http::Request(Http::HttpMethod::Get, host);
  request.AddHeader("Host", "httpbin.org");

  printStream(context, pipeline.Send(context, request));
}

// Request GET with no body that produces stream response
void doGetRequest(Context const& context, HttpPipeline& pipeline)
{
  Azure::Core::Http::Url host("https://httpbin.org/get//////?arg=1&arg2=2");
  cout << "Creating a GET request to" << endl << "Host: " << host.GetAbsoluteUrl() << endl;

  auto request = Http::Request(Http::HttpMethod::Get, host);
  request.AddHeader("one", "header");
  request.AddHeader("other", "header2");
  request.AddHeader("header", "value");

  request.AddHeader("Host", "httpbin.org");

  request.GetUrl().AppendQueryParameter("dynamicArg", "3");
  request.GetUrl().AppendQueryParameter("dynamicArg2", "4");

  auto response = pipeline.Send(context, request);
  printStream(context, std::move(response));
}

// Put Request with bodyBufferBody that produces stream
void doPutRequest(Context const& context, HttpPipeline& pipeline)
{
  Azure::Core::Http::Url host("https://httpbin.org/put/?a=1");
  cout << "Creating a PUT request to" << endl << "Host: " << host.GetAbsoluteUrl() << endl;

  std::fill(buffer.begin(), buffer.end(), 'x');
  buffer[0] = '{';
  buffer[1] = '\"';
  buffer[3] = '\"';
  buffer[4] = ':';
  buffer[5] = '\"';
  buffer[BufferSize - 2] = '\"';
  buffer[BufferSize - 1] = '}'; // set buffer to look like a Json `{"x":"xxx...xxx"}`

  MemoryBodyStream requestBodyStream(buffer.data(), buffer.size());
  auto request = Http::Request(Http::HttpMethod::Put, host, &requestBodyStream);
  request.AddHeader("one", "header");
  request.AddHeader("other", "header2");
  request.AddHeader("header", "value");

  request.AddHeader("Content-Length", std::to_string(BufferSize));

  printStream(context, pipeline.Send(context, request));
}

// Put Request with stream body that produces stream
void doPutStreamRequest(Context const& context, HttpPipeline& pipeline)
{
  Azure::Core::Http::Url host("https://putsreq.com/SDywlz7z6j90bJFNvyTO");
  cout << "Creating a PUT request to" << endl << "Host: " << host.GetAbsoluteUrl() << endl;

  bufferStream.fill('1');
  bufferStream[0] = '{';
  bufferStream[1] = '\"';
  bufferStream[3] = '\"';
  bufferStream[4] = ':';
  bufferStream[5] = '\"';
  bufferStream[StreamSize - 2] = '\"';
  bufferStream[StreamSize - 1] = '}'; // set buffer to look like a Json `{"1":"111...111"}`

  auto requestBodyStream
      = std::make_unique<MemoryBodyStream>(bufferStream.data(), bufferStream.size());
  auto request = Http::Request(Http::HttpMethod::Put, host, requestBodyStream.get());
  request.AddHeader("one", "header");
  request.AddHeader("other", "header2");
  request.AddHeader("header", "value");

  request.AddHeader("Content-Length", std::to_string(StreamSize));

  request.GetUrl().AppendQueryParameter("dynamicArg", "1");
  request.GetUrl().AppendQueryParameter("dynamicArg2", "1");
  request.GetUrl().AppendQueryParameter("dynamicArg3", "1");

  printStream(context, pipeline.Send(context, request));
}

void printStream(Context const& context, std::unique_ptr<Http::RawResponse> response)
{
  if (response == nullptr)
  {
    cout << "Error. Response returned as null";
    // std::cin.ignore();
    return;
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

  uint8_t b[100];
  auto bodyStream = response->GetBodyStream();
  int64_t readCount;
  do
  {
    readCount = bodyStream->Read(context, b, 10);
    cout << std::string(b, b + readCount);

  } while (readCount > 0);

  cout << endl << "Press any key to continue..." << endl;
  // std::cin.ignore();

  return;
}
