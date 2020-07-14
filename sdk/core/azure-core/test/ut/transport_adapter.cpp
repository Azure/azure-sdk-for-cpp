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

  TEST_F(TransportAdapter, get)
  {
    std::string host("http://httpbin.org/get");
    auto expectedResponseBodySize
        = 199; // May fail in the future if Server change the response size.

    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host);
    auto response = pipeline.Send(context, request);
    EXPECT_TRUE(response->GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok);
    auto body = response->GetBodyStream();
    EXPECT_EQ(body->Length(), expectedResponseBodySize);

    // Add a header and send again. Response should return that header in the body
    request.AddHeader("123", "456");
    response = pipeline.Send(context, request);
    EXPECT_TRUE(response->GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok);
    body = response->GetBodyStream();
    EXPECT_EQ(
        body->Length(),
        expectedResponseBodySize + 6
            + 13); // header length is 6 (data) + 13 (formating) -> `    "123": "456"\r\n,`
  }

  TEST_F(TransportAdapter, getLoop)
  {
    std::string host("http://httpbin.org/get");
    auto expectedResponseBodySize
        = 199; // May fail in the future if Server change the response size.

    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host);

    // loop sending request
    for (auto i = 0; i < 20; i++)
    {
      auto response = pipeline.Send(context, request);
      EXPECT_TRUE(response->GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok);
      auto body = response->GetBodyStream();
      EXPECT_EQ(body->Length(), expectedResponseBodySize);
      auto bodyVector = Azure::Core::Http::BodyStream::ReadToEnd(context, *body);
      EXPECT_EQ(expectedResponseBodySize, bodyVector.size());
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
    EXPECT_EQ(body->Length(), expectedResponseBodySize);
    auto bodyVector = Azure::Core::Http::BodyStream::ReadToEnd(context, *body);
    // Head response should be 0
    EXPECT_EQ(0, bodyVector.size());
    // Check content-length header to be greater than 0
    auto headers = response->GetHeaders();
    int64_t contentLengthHeader = std::stoull(headers["content-length"]);
    EXPECT_TRUE(contentLengthHeader > 0);
  }

  TEST_F(TransportAdapter, put)
  {
    std::string host("http://httpbin.org/put");
    auto expectedResponseBodySize = 262;

    // PUT 1MB
    auto requestBodyVector = std::vector<uint8_t>(1024 * 1024, 'x');
    auto bodyRequest = Azure::Core::Http::MemoryBodyStream(requestBodyVector);
    auto request
        = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, host, &bodyRequest);
    auto response = pipeline.Send(context, request);
    EXPECT_TRUE(response->GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok);

    auto body = response->GetBodyStream();
    EXPECT_EQ(body->Length(), expectedResponseBodySize);
    auto bodyVector = Azure::Core::Http::BodyStream::ReadToEnd(context, *body);
    EXPECT_EQ(expectedResponseBodySize, bodyVector.size());
  }

  TEST_F(TransportAdapter, deleteRequest)
  {
    std::string host("http://httpbin.org/delete");
    auto expectedResponseBodySize = 265;

    // Delete with 1MB payload
    auto requestBodyVector = std::vector<uint8_t>(1024 * 1024, 'x');
    auto bodyRequest = Azure::Core::Http::MemoryBodyStream(requestBodyVector);
    auto request
        = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Delete, host, &bodyRequest);
    auto response = pipeline.Send(context, request);
    EXPECT_TRUE(response->GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok);

    auto body = response->GetBodyStream();
    EXPECT_EQ(body->Length(), expectedResponseBodySize);
    auto bodyVector = Azure::Core::Http::BodyStream::ReadToEnd(context, *body);
    EXPECT_EQ(expectedResponseBodySize, bodyVector.size());
  }

  TEST_F(TransportAdapter, patch)
  {
    std::string host("http://httpbin.org/patch");
    auto expectedResponseBodySize = 264;

    // Patch with 1kb payload
    auto requestBodyVector = std::vector<uint8_t>(1024, 'x');
    auto bodyRequest = Azure::Core::Http::MemoryBodyStream(requestBodyVector);
    auto request
        = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Patch, host, &bodyRequest);
    auto response = pipeline.Send(context, request);
    EXPECT_TRUE(response->GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok);

    auto body = response->GetBodyStream();
    EXPECT_EQ(body->Length(), expectedResponseBodySize);
    auto bodyVector = Azure::Core::Http::BodyStream::ReadToEnd(context, *body);
    EXPECT_EQ(expectedResponseBodySize, bodyVector.size());
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
    EXPECT_EQ(body->Length(), expectedResponseBodySize);

    // Read body
    auto bodyVector = Azure::Core::Http::BodyStream::ReadToEnd(context, *body);
    EXPECT_EQ(expectedChunkResponse.size(), bodyVector.size());
    auto bodyString = std::string(bodyVector.begin(), bodyVector.end());
    EXPECT_STREQ(expectedChunkResponse.data(), bodyString.data());
  }

}}} // namespace Azure::Core::Test
