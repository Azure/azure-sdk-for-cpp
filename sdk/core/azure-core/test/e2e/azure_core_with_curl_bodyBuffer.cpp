// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Simulates customer application that is linked with azure-core and azure-transport-curl
 *
 */

#include <azure/core/http/pipeline.hpp>

#ifdef POSIX
#include <fcntl.h>
#endif // Posix

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif // Windows

#include <azure/core/http/curl/curl.hpp>
#include <azure/core/http/http.hpp>

#include <iostream>
#include <memory>

using namespace Azure::Core;
using namespace Azure::Core::Http;
using namespace std;

constexpr auto BufferSize = 50;

std::vector<uint8_t> buffer(BufferSize);
void doGetRequest(Context const& context, HttpPipeline& pipeline);
void doPutRequest(Context const& context, HttpPipeline& pipeline);
void doHeadRequest(Context const& context, HttpPipeline& pipeline);
void doDeleteRequest(Context const& context, HttpPipeline& pipeline);
void doPatchRequest(Context const& context, HttpPipeline& pipeline);
void printRespose(std::unique_ptr<Http::RawResponse> response);
void doFileRequest(Context const& context, HttpPipeline& pipeline);

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
    auto context = Azure::Core::GetApplicationContext();

    // Both requests uses a body buffer to be uploaded that would produce responses with bodyBuffer
    doHeadRequest(context, httpPipeline);
    doFileRequest(context, httpPipeline);
    doGetRequest(context, httpPipeline);
    doPutRequest(context, httpPipeline);
    doDeleteRequest(context, httpPipeline);
    doPatchRequest(context, httpPipeline);
  }
  catch (Azure::Core::RequestFailedException const& e)
  {
    cout << e.what() << endl;
  }

  return 0;
}

#ifdef POSIX
void doFileRequest(Context const& context, HttpPipeline& pipeline)
{

  Azure::Core::Http::Url host("https://httpbin.org/put");
  cout << "Creating a Put From File request to" << endl << "Host: " << host.GetAbsoluteUrl() << endl;

  // Open a file that contains: {{"key":"value"}, {"key2":"value2"}, {"key3":"value3"}}
  int fd = open("/home/vivazqu/workspace/a", O_RDONLY);
  // Create Stream from file starting with offset 18 to 100
  auto requestBodyStream = FileBodyStream(fd, 18, 100);
  // Limit stream to read up to 17 positions ( {"key2","value2"} )
  auto limitedStream = LimitBodyStream(&requestBodyStream, 17);

  // Send request
  auto request = Http::Request(Http::HttpMethod::Put, host, &limitedStream, true);
  request.AddHeader("Content-Length", std::to_string(limitedStream.Length()));
  request.AddHeader("File", "fileeeeeeeeeee");

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
void doFileRequest(Context const& context, HttpPipeline& pipeline)
{
  (void)pipeline;
  Azure::Core::Http::Url host("https://httpbin.org/put");
  cout << "Creating a File request to" << endl << "Host: " << host.GetAbsoluteUrl() << endl;

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

void doGetRequest(Context const& context, HttpPipeline& pipeline)
{
  Azure::Core::Http::Url host("https://httpbin.org/get");
  cout << "Creating a GET request to" << endl << "Host: " << host.GetAbsoluteUrl() << endl;

  auto requestBodyStream = std::make_unique<MemoryBodyStream>(buffer.data(), buffer.size());
  auto request = Http::Request(Http::HttpMethod::Get, host, requestBodyStream.get(), true);
  request.AddHeader("one", "GetHeader");
  request.AddHeader("other", "GetHeader2");
  request.AddHeader("header", "GetValue");
  request.AddHeader("Host", "httpbin.org");

  cout << endl << "GET:";
  printRespose(pipeline.Send(context, request));
}

void doPutRequest(Context const& context, HttpPipeline& pipeline)
{
  Azure::Core::Http::Url host("https://httpbin.org/put");
  cout << "Creating a PUT request to" << endl << "Host: " << host.GetAbsoluteUrl() << endl;

  std::fill(buffer.begin(), buffer.end(), 'x');
  buffer[0] = '{';
  buffer[1] = '\"';
  buffer[3] = '\"';
  buffer[4] = ':';
  buffer[5] = '\"';
  buffer[BufferSize - 2] = '\"';
  buffer[BufferSize - 1] = '}'; // set buffer to look like a Json `{"x":"xxx...xxx"}`

  auto requestBodyStream = std::make_unique<MemoryBodyStream>(buffer.data(), buffer.size());
  auto request = Http::Request(Http::HttpMethod::Put, host, requestBodyStream.get(), true);
  request.AddHeader("PUT", "header");
  request.AddHeader("PUT2", "header2");
  request.AddHeader("PUT3", "value");

  request.AddHeader("Host", "httpbin.org");
  request.AddHeader("Content-Length", std::to_string(BufferSize));

  printRespose(pipeline.Send(context, request));
}

void printRespose(std::unique_ptr<Http::RawResponse> response)
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

void doPatchRequest(Context const& context, HttpPipeline& pipeline)
{
  Azure::Core::Http::Url host("https://httpbin.org/patch");
  cout << "Creating an PATCH request to" << endl << "Host: " << host.GetAbsoluteUrl() << endl;

  auto request = Http::Request(Http::HttpMethod::Patch, host, true);

  cout << endl << "PATCH:";
  printRespose(pipeline.Send(context, request));
}

void doDeleteRequest(Context const& context, HttpPipeline& pipeline)
{
  Azure::Core::Http::Url host("https://httpbin.org/delete");
  cout << "Creating an DELETE request to" << endl << "Host: " << host.GetAbsoluteUrl() << endl;

  auto request = Http::Request(Http::HttpMethod::Delete, host, true);
  // request.AddHeader("deleteeeee", "httpbin.org");

  cout << endl << "DELETE:";
  printRespose(pipeline.Send(context, request));
}

void doHeadRequest(Context const& context, HttpPipeline& pipeline)
{
  Azure::Core::Http::Url host("https://httpbin.org/get");
  cout << "Creating an HEAD request to" << endl << "Host: " << host.GetAbsoluteUrl() << endl;

  auto request = Http::Request(Http::HttpMethod::Head, host, true);
  request.AddHeader("HEAD", "httpbin.org");

  cout << endl << "HEAD:";
  printRespose(pipeline.Send(context, request));
}
