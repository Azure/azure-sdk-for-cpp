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

// For bodyBuffer
#define BUFFER_SIZE 50
std::vector<uint8_t> buffer(BUFFER_SIZE);

// For StreamBody
#define STREAM_SIZE 200
std::array<uint8_t, STREAM_SIZE> bufferStream;

Http::Request createGetRequest();
Http::Request createPutRequest();
Http::Request createPutStreamRequest();
void printStream(std::unique_ptr<Http::Response> response);

int main()
{
  try
  {
    // GetRequest. No body, produces stream
    auto getRequest = createGetRequest();
    // PutRequest. buffer body, produces stream
    auto putRequest = createPutRequest();
    // PutRequest. Stream body, produces stream
    auto putStreamRequest = createPutStreamRequest();

    // Create the Transport
    std::shared_ptr<HttpTransport> transport = std::make_unique<CurlTransport>();

    std::vector<std::unique_ptr<HttpPolicy>> policies;
    policies.push_back(std::make_unique<RequestIdPolicy>());

    RetryOptions retryOptions;
    policies.push_back(std::make_unique<RetryPolicy>(retryOptions));

    // Add the transport policy
    policies.push_back(std::make_unique<TransportPolicy>(std::move(transport)));

    auto httpPipeline = Http::HttpPipeline(policies);

    std::unique_ptr<Http::Response> response;
    auto context = Context();

    response = httpPipeline.Send(context, getRequest);
    printStream(std::move(response));

    response = httpPipeline.Send(context, putRequest);
    printStream(std::move(response));

    response = httpPipeline.Send(context, putStreamRequest);
    printStream(std::move(response));
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

// Request GET with no body that produces stream response
Http::Request createGetRequest()
{
  string host("https://httpbin.org/get");
  cout << "Creating a GET request to" << endl << "Host: " << host << endl;

  auto request = Http::Request(Http::HttpMethod::Get, host, BodyType::Stream);
  request.AddHeader("one", "header");
  request.AddHeader("other", "header2");
  request.AddHeader("header", "value");

  request.AddHeader("Host", "httpbin.org");

  return request;
}

// Put Request with bodyBufferBody that produces stream
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

  auto request = Http::Request(Http::HttpMethod::Put, host, buffer, BodyType::Stream);
  request.AddHeader("one", "header");
  request.AddHeader("other", "header2");
  request.AddHeader("header", "value");

  request.AddHeader("Host", "httpbin.org");
  request.AddHeader("Content-Length", std::to_string(BUFFER_SIZE));

  return request;
}

// Put Request with stream body that produces stream
Http::Request createPutStreamRequest()
{
  string host("https://httpbin.org/put");
  cout << "Creating a PUT request to" << endl << "Host: " << host << endl;

  bufferStream.fill('1');
  bufferStream[0] = '{';
  bufferStream[1] = '\"';
  bufferStream[3] = '\"';
  bufferStream[4] = ':';
  bufferStream[5] = '\"';
  bufferStream[STREAM_SIZE - 2] = '\"';
  bufferStream[STREAM_SIZE - 1] = '}'; // set buffer to look like a Json `{"1":"111...111"}`

  auto request = Http::Request(
      Http::HttpMethod::Put, host, new MemoryBodyStream(bufferStream.data(), STREAM_SIZE));
  request.AddHeader("one", "header");
  request.AddHeader("other", "header2");
  request.AddHeader("header", "value");

  request.AddHeader("Host", "httpbin.org");
  request.AddHeader("Content-Length", std::to_string(STREAM_SIZE));

  return request;
}

void printStream(std::unique_ptr<Http::Response> response)
{
  if (response == nullptr)
  {
    cout << "Error. Response returned as null";
    std::cin.ignore();
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
  uint64_t readCount;
  do
  {
    readCount = bodyStream->Read(b, 10);
    cout << std::string(b, b + readCount);

  } while (readCount > 0);

  cout << endl << "Press any key to continue..." << endl;
  std::cin.ignore();

  bodyStream->Close();
  return;
}
