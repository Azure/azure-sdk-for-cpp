// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "transport_adapter.hpp"
#include <context.hpp>
#include <response.hpp>
#include <string>

namespace Azure { namespace Core { namespace Test {

  static std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> CreatePolicies()
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> p;
    std::shared_ptr<Azure::Core::Http::HttpTransport> transport
        = std::make_shared<Azure::Core::Http::CurlTransport>();

    p.push_back(std::make_unique<Azure::Core::Http::TransportPolicy>(std::move(transport)));
    return p;
  }

  std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> TransportAdapter::policies
      = CreatePolicies();

  Azure::Core::Http::HttpPipeline TransportAdapter::pipeline(policies);
  Azure::Core::Context TransportAdapter::context = Azure::Core::GetApplicationContext();

  TEST_F(TransportAdapter, get)
  {
    std::string host("http://httpbin.org/get");

    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host);
    auto response = pipeline.Send(context, request);
    checkResponseCode(response->GetStatusCode());
    auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
    CheckBodyFromBuffer(*response, expectedResponseBodySize);

    // Add a header and send again. RawResponse should return that header in the body
    request.AddHeader("123", "456");
    response = pipeline.Send(context, request);
    checkResponseCode(response->GetStatusCode());
    // header length is 6 (data) + 13 (formating) -> `    "123": "456"\r\n,`
    CheckBodyFromBuffer(*response, expectedResponseBodySize + 6 + 13);
  }

  TEST_F(TransportAdapter, getLoop)
  {
    std::string host("http://httpbin.org/get");

    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host);

    // loop sending request
    for (auto i = 0; i < 20; i++)
    {
      auto response = pipeline.Send(context, request);
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
      checkResponseCode(response->GetStatusCode());
      CheckBodyFromBuffer(*response, expectedResponseBodySize);
    }
  }

  TEST_F(TransportAdapter, head)
  {
    std::string host("http://httpbin.org/get");
    auto expectedResponseBodySize = 0;

    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Head, host);
    auto response = pipeline.Send(context, request);
    checkResponseCode(response->GetStatusCode());
    CheckBodyFromBuffer(*response, expectedResponseBodySize);

    // Check content-length header to be greater than 0
    int64_t contentLengthHeader = std::stoull(response->GetHeaders().at("content-length"));
    EXPECT_TRUE(contentLengthHeader > 0);
  }

  TEST_F(TransportAdapter, put)
  {
    std::string host("http://httpbin.org/put");

    // PUT 1K
    auto requestBodyVector = std::vector<uint8_t>(1024, 'x');
    auto bodyRequest = Azure::Core::Http::MemoryBodyStream(requestBodyVector);
    auto request
        = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, host, &bodyRequest);
    auto response = pipeline.Send(context, request);
    checkResponseCode(response->GetStatusCode());
    auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));

    CheckBodyFromBuffer(*response, expectedResponseBodySize);
  }

  TEST_F(TransportAdapter, deleteRequest)
  {
    std::string host("http://httpbin.org/delete");

    // Delete with 1k payload
    auto requestBodyVector = std::vector<uint8_t>(1024, 'x');
    auto bodyRequest = Azure::Core::Http::MemoryBodyStream(requestBodyVector);
    auto request
        = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Delete, host, &bodyRequest);
    auto response = pipeline.Send(context, request);
    checkResponseCode(response->GetStatusCode());

    auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
    CheckBodyFromBuffer(*response, expectedResponseBodySize);
  }

  TEST_F(TransportAdapter, patch)
  {
    std::string host("http://httpbin.org/patch");

    // Patch with 1kb payload
    auto requestBodyVector = std::vector<uint8_t>(1024, 'x');
    auto bodyRequest = Azure::Core::Http::MemoryBodyStream(requestBodyVector);
    auto request
        = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Patch, host, &bodyRequest);
    auto response = pipeline.Send(context, request);
    checkResponseCode(response->GetStatusCode());

    auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
    CheckBodyFromBuffer(*response, expectedResponseBodySize);
  }

  TEST_F(TransportAdapter, getChunk)
  {
    std::string host("http://anglesharp.azurewebsites.net/Chunked");
    auto expectedResponseBodySize = -1; // chunked will return unknown body length
    auto expectedChunkResponse = std::string(
        "<!DOCTYPE html>\r\n<html lang=en>\r\n<head>\r\n<meta charset='utf-8'>\r\n<title>Chunked "
        "transfer encoding test</title>\r\n</head>\r\n<body><h1>Chunked transfer encoding "
        "test</h1><h5>This is a chunked response after 100 ms.</h5><h5>This is a chunked "
        "response after 1 second. The server should not close the stream before all chunks are "
        "sent to a client.</h5></body></html>");

    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host);
    auto response = pipeline.Send(context, request);

    checkResponseCode(response->GetStatusCode());
    CheckBodyFromBuffer(*response, expectedResponseBodySize, expectedChunkResponse);
  }

  // **********************
  // ***Same tests but getting stream to pull from socket, simulating the Download Op
  // **********************

  TEST_F(TransportAdapter, getWithStream)
  {
    std::string host("http://httpbin.org/get");

    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host, true);
    auto response = pipeline.Send(context, request);
    checkResponseCode(response->GetStatusCode());
    auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
    CheckBodyFromStream(*response, expectedResponseBodySize);

    // Add a header and send again. Response should return that header in the body
    request.AddHeader("123", "456");
    response = pipeline.Send(context, request);
    checkResponseCode(response->GetStatusCode());
    // header length is 6 (data) + 13 (formating) -> `    "123": "456"\r\n,`
    CheckBodyFromStream(*response, expectedResponseBodySize + 6 + 13);
  }

  TEST_F(TransportAdapter, getLoopWithStream)
  {
    std::string host("http://httpbin.org/get");

    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host, true);

    // loop sending request
    for (auto i = 0; i < 20; i++)
    {
      auto response = pipeline.Send(context, request);
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
      checkResponseCode(response->GetStatusCode());
      CheckBodyFromStream(*response, expectedResponseBodySize);
    }
  }

  TEST_F(TransportAdapter, headWithStream)
  {
    std::string host("http://httpbin.org/get");
    auto expectedResponseBodySize = 0;

    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Head, host, true);
    auto response = pipeline.Send(context, request);
    checkResponseCode(response->GetStatusCode());
    CheckBodyFromStream(*response, expectedResponseBodySize);

    // Check content-length header to be greater than 0
    int64_t contentLengthHeader = std::stoull(response->GetHeaders().at("content-length"));
    EXPECT_TRUE(contentLengthHeader > 0);
  }

  TEST_F(TransportAdapter, putWithStream)
  {
    std::string host("http://httpbin.org/put");

    // PUT 1k
    auto requestBodyVector = std::vector<uint8_t>(1024, 'x');
    auto bodyRequest = Azure::Core::Http::MemoryBodyStream(requestBodyVector);
    auto request
        = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, host, &bodyRequest, true);
    auto response = pipeline.Send(context, request);
    checkResponseCode(response->GetStatusCode());
    auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));

    CheckBodyFromStream(*response, expectedResponseBodySize);
  }

  TEST_F(TransportAdapter, deleteRequestWithStream)
  {
    std::string host("http://httpbin.org/delete");

    // Delete with 1k payload
    auto requestBodyVector = std::vector<uint8_t>(1024, 'x');
    auto bodyRequest = Azure::Core::Http::MemoryBodyStream(requestBodyVector);
    auto request = Azure::Core::Http::Request(
        Azure::Core::Http::HttpMethod::Delete, host, &bodyRequest, true);
    auto response = pipeline.Send(context, request);
    checkResponseCode(response->GetStatusCode());

    auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
    CheckBodyFromStream(*response, expectedResponseBodySize);
  }

  TEST_F(TransportAdapter, patchWithStream)
  {
    std::string host("http://httpbin.org/patch");

    // Patch with 1kb payload
    auto requestBodyVector = std::vector<uint8_t>(1024, 'x');
    auto bodyRequest = Azure::Core::Http::MemoryBodyStream(requestBodyVector);
    auto request = Azure::Core::Http::Request(
        Azure::Core::Http::HttpMethod::Patch, host, &bodyRequest, true);
    auto response = pipeline.Send(context, request);
    checkResponseCode(response->GetStatusCode());

    auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
    CheckBodyFromStream(*response, expectedResponseBodySize);
  }

  TEST_F(TransportAdapter, getChunkWithStream)
  {
    std::string host("http://anglesharp.azurewebsites.net/Chunked");
    auto expectedResponseBodySize = -1; // chunked will return unknown body length
    auto expectedChunkResponse = std::string(
        "<!DOCTYPE html>\r\n<html lang=en>\r\n<head>\r\n<meta charset='utf-8'>\r\n<title>Chunked "
        "transfer encoding test</title>\r\n</head>\r\n<body><h1>Chunked transfer encoding "
        "test</h1><h5>This is a chunked response after 100 ms.</h5><h5>This is a chunked "
        "response after 1 second. The server should not close the stream before all chunks are "
        "sent to a client.</h5></body></html>");

    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host, true);
    auto response = pipeline.Send(context, request);

    checkResponseCode(response->GetStatusCode());
    CheckBodyFromStream(*response, expectedResponseBodySize, expectedChunkResponse);
  }

  TEST_F(TransportAdapter, createResponseT)
  {
    std::string host("http://httpbin.org/get");
    std::string expectedType("This is the Response Type");

    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host, false);
    auto response = pipeline.Send(context, request);

    Azure::Core::Response<std::string> responseT(expectedType, std::move(response));
    auto& r = responseT.GetRawResponse();

    EXPECT_TRUE(r.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok);
    auto expectedResponseBodySize = std::stoull(r.GetHeaders().at("content-length"));
    CheckBodyFromBuffer(r, expectedResponseBodySize);

    // Direct access
    EXPECT_STREQ((*responseT).data(), expectedType.data());
    EXPECT_STREQ(responseT->data(), expectedType.data());
    // extracting T out of response
    auto result = responseT.ExtractValue();
    EXPECT_STREQ(result.data(), expectedType.data());
    // Test that calling getValue again will return empty
    result = responseT.ExtractValue();
    EXPECT_STREQ(result.data(), std::string("").data());
  }

  TEST_F(TransportAdapter, customSizePut)
  {
    std::string host("http://httpbin.org/put");

    // PUT 1MB
    auto requestBodyVector = std::vector<uint8_t>(1024 * 1024, 'x');
    auto bodyRequest = Azure::Core::Http::MemoryBodyStream(requestBodyVector);
    auto request
        = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, host, &bodyRequest);
    // Make transport adapter to read all stream content for uploading instead of chunks
    request.SetUploadChunkSize(1024 * 1024);
    {
      auto response = pipeline.Send(context, request);
      checkResponseCode(response->GetStatusCode());
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
      CheckBodyFromBuffer(*response, expectedResponseBodySize);
    }
  }

}}} // namespace Azure::Core::Test
