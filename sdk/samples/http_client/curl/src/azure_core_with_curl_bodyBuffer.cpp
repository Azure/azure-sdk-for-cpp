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
    doGetRequest(context, httpPipeline);
    doPutRequest(context, httpPipeline);
    doHeadRequest(context, httpPipeline);
    doDeleteRequest(context, httpPipeline);
    doPatchRequest(context, httpPipeline);
    doFileRequest(context, httpPipeline);
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

#ifdef POSIX
void doFileRequest(Context context, HttpPipeline& pipeline)
{
  (void)pipeline;
  string host("https://httpbin.org/put");
  cout << "Creating a File request to" << endl << "Host: " << host << endl;

  int fd = open("/home/vagrant/workspace/a", O_RDONLY);
  auto requestBodyStream = std::make_unique<FileBodyStream>(fd, 0, 10);

  auto body = Http::BodyStream::ReadToEnd(context, *requestBodyStream);
  cout << body->data() << endl << body->size() << endl;

  close(fd);
}
#endif

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
  cout << body->data() << endl << body->size() << endl;

  CloseHandle(hFile);
}
#endif // Windows

void doGetRequest(Context context, HttpPipeline& pipeline)
{
  string host("https://httpbin.org/get");
  cout << "Creating a GET request to" << endl << "Host: " << host << endl;

  auto requestBodyStream = std::make_unique<MemoryBodyStream>(buffer.data(), buffer.size());
  auto request = Http::Request(Http::HttpMethod::Get, host, *requestBodyStream);
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
  auto request = Http::Request(Http::HttpMethod::Put, host, *requestBodyStream);
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
  if (responseBodyVector != nullptr)
  {
    // print body only if response has a body. Head Response won't have body
    auto bodyVector = *responseBodyVector.get();
    cout << std::string(bodyVector.begin(), bodyVector.end()) << endl;
  }

  // std::cin.ignore();
  return;
}

void doPatchRequest(Context context, HttpPipeline& pipeline)
{
  string host("https://httpbin.org/patch");
  cout << "Creating an PATCH request to" << endl << "Host: " << host << endl;

  auto body = std::make_unique<NullBodyStream>();
  auto request = Http::Request(Http::HttpMethod::Patch, host, *body);
  request.AddHeader("Host", "httpbin.org");

  cout << endl << "PATCH:";
  printRespose(std::move(pipeline.Send(context, request)));
}

void doDeleteRequest(Context context, HttpPipeline& pipeline)
{
  string host("https://httpbin.org/delete");
  cout << "Creating an DELETE request to" << endl << "Host: " << host << endl;

  auto body = std::make_unique<NullBodyStream>();
  auto request = Http::Request(Http::HttpMethod::Delete, host, *body);
  request.AddHeader("Host", "httpbin.org");

  cout << endl << "DELETE:";
  printRespose(std::move(pipeline.Send(context, request)));
}

void doHeadRequest(Context context, HttpPipeline& pipeline)
{
  string host("https://httpbin.org/get");
  cout << "Creating an HEAD request to" << endl << "Host: " << host << endl;

  auto body = std::make_unique<NullBodyStream>();
  auto request = Http::Request(Http::HttpMethod::Head, host, *body);
  request.AddHeader("Host", "httpbin.org");

  cout << endl << "HEAD:";
  printRespose(std::move(pipeline.Send(context, request)));
}
