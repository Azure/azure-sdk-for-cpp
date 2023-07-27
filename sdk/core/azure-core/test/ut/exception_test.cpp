// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "test_traits.hpp"

#include <azure/core/exception.hpp>
#include <azure/core/http/http.hpp>

#include <string>

#include <gtest/gtest.h>

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
  EXPECT_NE(
      std::string(exception.what())
          .find(std::to_string(
              static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  Azure::Core::Http::HttpStatusCode::ServiceUnavailable))),
      std::string::npos);
  EXPECT_NE(std::string(exception.what()).find("retry please :"), std::string::npos);
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
  EXPECT_NE(
      std::string(exception.what())
          .find(std::to_string(
              static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  Azure::Core::Http::HttpStatusCode::ServiceUnavailable))),
      std::string::npos);
  EXPECT_NE(std::string(exception.what()).find("retry please :"), std::string::npos);
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
  EXPECT_NE(
      std::string(exception.what())
          .find(std::to_string(
              static_cast<std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                  Azure::Core::Http::HttpStatusCode::None))),
      std::string::npos);
}

TEST(RequestFailedException, Constructible)
{
  EXPECT_FALSE((ClassTraits<RequestFailedException>::is_constructible()));
  EXPECT_FALSE((ClassTraits<RequestFailedException>::is_trivially_constructible()));
  EXPECT_FALSE((ClassTraits<RequestFailedException>::is_nothrow_constructible()));
  EXPECT_FALSE(ClassTraits<RequestFailedException>::is_default_constructible());
  EXPECT_FALSE(ClassTraits<RequestFailedException>::is_trivially_default_constructible());
  EXPECT_FALSE(ClassTraits<RequestFailedException>::is_nothrow_default_constructible());
}

TEST(RequestFailedException, Destructible)
{
  EXPECT_TRUE(ClassTraits<RequestFailedException>::is_destructible());
  EXPECT_FALSE(ClassTraits<RequestFailedException>::is_trivially_destructible());
  EXPECT_TRUE(ClassTraits<RequestFailedException>::is_nothrow_destructible());
  EXPECT_TRUE(ClassTraits<RequestFailedException>::has_virtual_destructor());
}

TEST(RequestFailedException, CopyAndMoveConstructible)
{
  EXPECT_TRUE(ClassTraits<RequestFailedException>::is_copy_constructible());
  EXPECT_FALSE(ClassTraits<RequestFailedException>::is_trivially_copy_constructible());
  EXPECT_FALSE(ClassTraits<RequestFailedException>::is_nothrow_copy_constructible());
  EXPECT_TRUE(ClassTraits<RequestFailedException>::is_move_constructible());
  EXPECT_FALSE(ClassTraits<RequestFailedException>::is_trivially_move_constructible());
  EXPECT_TRUE(ClassTraits<RequestFailedException>::is_nothrow_move_constructible());
}

TEST(RequestFailedException, Assignable)
{
  EXPECT_FALSE(ClassTraits<RequestFailedException>::is_assignable<RequestFailedException>());
  EXPECT_FALSE(ClassTraits<RequestFailedException>::is_assignable<const RequestFailedException>());
  EXPECT_FALSE(
      ClassTraits<RequestFailedException>::is_trivially_assignable<RequestFailedException>());
  EXPECT_FALSE(
      ClassTraits<RequestFailedException>::is_trivially_assignable<const RequestFailedException>());
  EXPECT_FALSE(
      ClassTraits<RequestFailedException>::is_nothrow_assignable<RequestFailedException>());
  EXPECT_FALSE(
      ClassTraits<RequestFailedException>::is_nothrow_assignable<const RequestFailedException>());
}

TEST(RequestFailedException, CopyAndMoveAssignable)
{
  EXPECT_FALSE(ClassTraits<RequestFailedException>::is_copy_assignable());
  EXPECT_FALSE(ClassTraits<RequestFailedException>::is_trivially_copy_assignable());
  EXPECT_FALSE(ClassTraits<RequestFailedException>::is_nothrow_copy_assignable());
  EXPECT_FALSE(ClassTraits<RequestFailedException>::is_move_assignable());
  EXPECT_FALSE(ClassTraits<RequestFailedException>::is_trivially_move_assignable());
  EXPECT_FALSE(ClassTraits<RequestFailedException>::is_nothrow_move_assignable());
}
