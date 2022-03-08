// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/exception.hpp>
#include <azure/core/http/http.hpp>
#include <gtest/gtest.h>
#include <string>

using namespace Azure::Core;
using namespace Azure::Core::Http::_internal;

TEST(RequestFailedException, JSONError)
{
  auto response = std::make_unique<Azure::Core::Http::RawResponse>(
      1, 1, Azure::Core::Http::HttpStatusCode::ServiceUnavailable, "retry please :");
  static constexpr uint8_t const responseBody[]
      = "{\"error\":{ \"code\":\"503\",  \"message\":\"JT\"}}";
  static constexpr uint8_t const responseBodyStream[]
      = "{\"error\":{ \"code\":\"503\",  \"message\":\"JT\"}}";

  response->SetHeader(HttpShared::ContentType, "application/json");
  response->SetHeader(HttpShared::MsRequestId, "1");
  response->SetHeader(HttpShared::MsClientRequestId, "2");
  response->SetBody(std::vector<uint8_t>(responseBody, responseBody + sizeof(responseBody)));
  response->SetBodyStream(std::make_unique<Azure::Core::IO::MemoryBodyStream>(
      responseBodyStream, sizeof(responseBodyStream) - 1));

  auto exception = Azure::Core::RequestFailedException(response);

  EXPECT_EQ(exception.StatusCode, Azure::Core::Http::HttpStatusCode::ServiceUnavailable);
  EXPECT_EQ(exception.Message, "JT");
  EXPECT_EQ(exception.ErrorCode, "503");
  EXPECT_EQ(exception.RequestId, "1");
  EXPECT_EQ(exception.ClientRequestId, "2");
  EXPECT_EQ(exception.ReasonPhrase, "retry please :");
  EXPECT_EQ(std::string(exception.what()).find("Received an HTTP unsuccessful status code"), 0);
}

TEST(RequestFailedException, JSONErrorNoError)
{
  auto response = std::make_unique<Azure::Core::Http::RawResponse>(
      1, 1, Azure::Core::Http::HttpStatusCode::ServiceUnavailable, "retry please :");
  static constexpr uint8_t const responseBody[] = "{\"text\" :\"some text\"}";
  static constexpr uint8_t const responseBodyStream[] = "{\"text\" :\"some text\"}";

  response->SetHeader(HttpShared::ContentType, "application/json");
  response->SetHeader(HttpShared::MsRequestId, "1");
  response->SetHeader(HttpShared::MsClientRequestId, "2");
  response->SetBody(std::vector<uint8_t>(responseBody, responseBody + sizeof(responseBody)));
  response->SetBodyStream(std::make_unique<Azure::Core::IO::MemoryBodyStream>(
      responseBodyStream, sizeof(responseBodyStream) - 1));

  auto exception = Azure::Core::RequestFailedException(response);

  EXPECT_EQ(exception.StatusCode, Azure::Core::Http::HttpStatusCode::ServiceUnavailable);
  EXPECT_EQ(exception.Message, "");
  EXPECT_EQ(exception.ErrorCode, "");
  EXPECT_EQ(exception.RequestId, "1");
  EXPECT_EQ(exception.ClientRequestId, "2");
  EXPECT_EQ(exception.ReasonPhrase, "retry please :");
  EXPECT_EQ(std::string(exception.what()).find("Received an HTTP unsuccessful status code"), 0);
}

TEST(RequestFailedException, EmptyValues)
{
  auto response = std::make_unique<Azure::Core::Http::RawResponse>(
      1, 1, Azure::Core::Http::HttpStatusCode::None, std::string());

  auto exception = Azure::Core::RequestFailedException(response);

  EXPECT_EQ(exception.StatusCode, Azure::Core::Http::HttpStatusCode::None);
  EXPECT_EQ(exception.Message, std::string());
  EXPECT_EQ(exception.ErrorCode, std::string());
  EXPECT_EQ(exception.RequestId, std::string());
  EXPECT_EQ(exception.ClientRequestId, std::string());
  EXPECT_EQ(exception.ReasonPhrase, std::string());
  EXPECT_EQ(std::string(exception.what()).find("Received an HTTP unsuccessful status code"), 0);
}
