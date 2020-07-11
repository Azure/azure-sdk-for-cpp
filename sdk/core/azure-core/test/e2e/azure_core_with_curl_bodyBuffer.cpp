// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Simulates customer application that is linked with azure-core and azure-transport-curl
 *
 */

#include "http/pipeline.hpp"

#ifdef POSIX
#include <fcntl.h>
#endif // Posix

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif // Windows

#include <http/curl/curl.hpp>
#include <http/http.hpp>
#include <iostream>
#include <memory>

using namespace Azure::Core;
using namespace Azure::Core::Http;
using namespace std;

constexpr auto BufferSize = 50;

std::vector<uint8_t> buffer(BufferSize);
void doGetRequest(Context context, HttpPipeline& pipeline);
void doPutRequest(Context context, HttpPipeline& pipeline);
void doHeadRequest(Context context, HttpPipeline& pipeline);
void doDeleteRequest(Context context, HttpPipeline& pipeline);
void doPatchRequest(Context context, HttpPipeline& pipeline);
void printRespose(std::unique_ptr<Http::Response> response);
void doFileRequest(Context context, HttpPipeline& pipeline);

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
    auto context = Context();

    // Both requests uses a body buffer to be uploaded that would produce responses with bodyBuffer
    doHeadRequest(context, httpPipeline);
    doFileRequest(context, httpPipeline);
    doGetRequest(context, httpPipeline);
    doPutRequest(context, httpPipeline);
    doDeleteRequest(context, httpPipeline);
    doPatchRequest(context, httpPipeline);
  }
  catch (Http::CouldNotResolveHostException const& e)
  {
    cout << e.what() << endl;
  }
  catch (Http::TransportException const& e)
  {
    cout << e.what() << endl;
  }

  return 0;
}

#ifdef POSIX
void doFileRequest(Context context, HttpPipeline& pipeline)
{

  string host("https://httpbin.org/put");
  cout << "Creating a Put From File request to" << endl << "Host: " << host << endl;

  // Open a file that contains: {{"key":"value"}, {"key2":"value2"}, {"key3":"value3"}}
  int fd = open("/home/vagrant/workspace/a", O_RDONLY);
  // Create Stream from file starting with offset 18 to 100
  auto requestBodyStream = FileBodyStream(fd, 18, 100);
  // Limit stream to read up to 17 postions ( {"key2","value2"} )
  auto limitedStream = LimitBodyStream(&requestBodyStream, 17);

  // Send request
  auto request = Http::Request(Http::HttpMethod::Put, host, &limitedStream);
  request.AddHeader("Content-Length", std::to_string(limitedStream.Length()));

  auto response = pipeline.Send(context, request);
  // File can be closed at this point
  close(fd);

  // Response Stream
  auto bodyStreamResponse = response->GetBodyStream();
  // limit to read response
  auto limitedResponse = LimitBodyStream(bodyStreamResponse.get(), 300);

  auto body = Http::BodyStream::ReadToEnd(context, limitedResponse);
  cout << body.data() << endl << body.size() << endl;
}
#endif // Posix

#ifdef WINDOWS
void doFileRequest(Context context, HttpPipeline& pipeline)
{
  (void)pipeline;
  string host("https://httpbin.org/put");
  cout << "Creating a File request to" << endl << "Host: " << host << endl;

  // NOTE: To run the sample: Create folder 'home' on main hard drive (like C:/) and then add a file
  // `a` in there
  //
  HANDLE hFile = CreateFile(
      "/home/a",
      GENERIC_READ,
      FILE_SHARE_READ,
      NULL,
      OPEN_EXISTING,
      FILE_FLAG_SEQUENTIAL_SCAN,
      NULL);
  auto requestBodyStream = std::make_unique<FileBodyStream>(hFile, 20, 200);

  auto body = Http::BodyStream::ReadToEnd(context, *requestBodyStream);
  cout << body.data() << endl << body.size() << endl;

  CloseHandle(hFile);
}
#endif // Windows

void doGetRequest(Context context, HttpPipeline& pipeline)
{
  string host("https://httpbin.org/get");
  cout << "Creating a GET request to" << endl << "Host: " << host << endl;

  auto requestBodyStream = std::make_unique<MemoryBodyStream>(buffer.data(), buffer.size());
  auto request = Http::Request(Http::HttpMethod::Get, host, requestBodyStream.get());
  request.AddHeader("one", "header");
  request.AddHeader("other", "header2");
  request.AddHeader("header", "value");
  request.AddHeader("Host", "httpbin.org");

  cout << endl << "GET:";
  printRespose(std::move(pipeline.Send(context, request)));
}

void doPutRequest(Context context, HttpPipeline& pipeline)
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

  auto requestBodyStream = std::make_unique<MemoryBodyStream>(buffer.data(), buffer.size());
  auto request = Http::Request(Http::HttpMethod::Put, host, requestBodyStream.get());
  request.AddHeader("one", "header");
  request.AddHeader("other", "header2");
  request.AddHeader("header", "value");

  request.AddHeader("Host", "httpbin.org");
  request.AddHeader("Content-Length", std::to_string(BufferSize));

  printRespose(std::move(pipeline.Send(context, request)));
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
  auto bodyStream = response->GetBodyStream();
  if (bodyStream == nullptr)
  {
    // No body in response
    return;
  }
  Context context;
  auto responseBodyVector = Http::BodyStream::ReadToEnd(context, *bodyStream);

  // print body only if response has a body. Head Response won't have body
  cout << std::string(responseBodyVector.begin(), responseBodyVector.end()) << endl;

  // std::cin.ignore();
  return;
}

void doPatchRequest(Context context, HttpPipeline& pipeline)
{
  string host("https://httpbin.org/patch");
  cout << "Creating an PATCH request to" << endl << "Host: " << host << endl;

  auto request = Http::Request(Http::HttpMethod::Patch, host);
  request.AddHeader("Host", "httpbin.org");

  cout << endl << "PATCH:";
  printRespose(std::move(pipeline.Send(context, request)));
}

void doDeleteRequest(Context context, HttpPipeline& pipeline)
{
  string host("https://httpbin.org/delete");
  cout << "Creating an DELETE request to" << endl << "Host: " << host << endl;

  auto request = Http::Request(Http::HttpMethod::Delete, host);
  request.AddHeader("Host", "httpbin.org");

  cout << endl << "DELETE:";
  printRespose(std::move(pipeline.Send(context, request)));
}

void doHeadRequest(Context context, HttpPipeline& pipeline)
{
  string host("https://httpbin.org/get");
  cout << "Creating an HEAD request to" << endl << "Host: " << host << endl;

  auto request = Http::Request(Http::HttpMethod::Head, host);
  request.AddHeader("Host", "httpbin.org");

  cout << endl << "HEAD:";
  printRespose(std::move(pipeline.Send(context, request)));
}
