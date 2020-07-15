// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "transport_adapter.hpp"
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
  Azure::Core::Context TransportAdapter::context = Azure::Core::Context();

  void TransportAdapter::CheckBodyStreamLength(
      Azure::Core::Http::BodyStream& body,
      int64_t size,
      std::string expectedBody)
  {
    EXPECT_EQ(body.Length(), size);
    auto bodyVector = Azure::Core::Http::BodyStream::ReadToEnd(context, body);
    if (size > 0)
    { // only for known body size
      EXPECT_EQ(bodyVector.size(), size);
    }

    if (expectedBody.size() > 0)
    {
      auto bodyString = std::string(bodyVector.begin(), bodyVector.end());
      EXPECT_STREQ(expectedBody.data(), bodyString.data());
    }
  }

  TEST_F(TransportAdapter, get)
  {
    std::string host("http://httpbin.org/get");

    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host);
    auto response = pipeline.Send(context, request);
    EXPECT_TRUE(response->GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok);
    auto body = response->GetBodyStream();
    auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
    CheckBodyStreamLength(*body, expectedResponseBodySize);

    // Add a header and send again. RawResponse should return that header in the body
    request.AddHeader("123", "456");
    response = pipeline.Send(context, request);
    EXPECT_TRUE(response->GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok);
    body = response->GetBodyStream();
    // header length is 6 (data) + 13 (formating) -> `    "123": "456"\r\n,`
    CheckBodyStreamLength(*body, expectedResponseBodySize + 6 + 13);
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
      EXPECT_TRUE(response->GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok);
      auto body = response->GetBodyStream();
      CheckBodyStreamLength(*body, expectedResponseBodySize);
    }
  }

  TEST_F(TransportAdapter, head)
  {
    std::string host("http://httpbin.org/get");
    auto expectedResponseBodySize = 0;

    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Head, host);
    auto response = pipeline.Send(context, request);
    EXPECT_TRUE(response->GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok);
    auto body = response->GetBodyStream();
    CheckBodyStreamLength(*body, expectedResponseBodySize);

    // Check content-length header to be greater than 0
    int64_t contentLengthHeader = std::stoull(response->GetHeaders().at("content-length"));
    EXPECT_TRUE(contentLengthHeader > 0);
  }

  TEST_F(TransportAdapter, put)
  {
    std::string host("http://httpbin.org/put");

    // PUT 1MB
    auto requestBodyVector = std::vector<uint8_t>(1024 * 1024, 'x');
    auto bodyRequest = Azure::Core::Http::MemoryBodyStream(requestBodyVector);
    auto request
        = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, host, &bodyRequest);
    auto response = pipeline.Send(context, request);
    EXPECT_TRUE(response->GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok);
    auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));

    auto body = response->GetBodyStream();
    CheckBodyStreamLength(*body, expectedResponseBodySize);
  }

  TEST_F(TransportAdapter, deleteRequest)
  {
    std::string host("http://httpbin.org/delete");

    // Delete with 1MB payload
    auto requestBodyVector = std::vector<uint8_t>(1024 * 1024, 'x');
    auto bodyRequest = Azure::Core::Http::MemoryBodyStream(requestBodyVector);
    auto request
        = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Delete, host, &bodyRequest);
    auto response = pipeline.Send(context, request);
    EXPECT_TRUE(response->GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok);

    auto body = response->GetBodyStream();
    auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
    CheckBodyStreamLength(*body, expectedResponseBodySize);
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
    EXPECT_TRUE(response->GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok);

    auto body = response->GetBodyStream();
    auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
    CheckBodyStreamLength(*body, expectedResponseBodySize);
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
    EXPECT_TRUE(response->GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok);
    auto body = response->GetBodyStream();
    CheckBodyStreamLength(*body, expectedResponseBodySize, expectedChunkResponse);
  }

}}} // namespace Azure::Core::Test
