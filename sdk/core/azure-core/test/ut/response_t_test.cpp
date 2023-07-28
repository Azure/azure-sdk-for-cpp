// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "test_traits.hpp"

#include <azure/core/http/http.hpp>
#include <azure/core/response.hpp>

#include <memory>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

using namespace Azure::Core;
using namespace Azure::Core::Http;

TEST(ResponseT, extractAndGet)
{
  // Create Response<T> and extract the rawResponse
  auto rawResponse = std::make_unique<RawResponse>(1, 1, HttpStatusCode::Accepted, "Something");
  std::string fakeT("pretending this is the T");

  Azure::Response<std::string> response(fakeT, std::move(rawResponse));

  // rawResponse moved to Response<T>
  EXPECT_EQ(nullptr, rawResponse);
  // This is fine because the rawResponse is still in the Response<T>
  EXPECT_NO_THROW(response.RawResponse->GetStatusCode());

  rawResponse = std::move(response.RawResponse);
  // rawResponse is now valid again
  EXPECT_NE(nullptr, rawResponse);
  // Can't get rawResponse again since it was moved
  EXPECT_EQ(nullptr, response.RawResponse);

  // This is fine, we can keep extracting the rawResponse, only that the second time the it would be
  // a nullptr
  EXPECT_NO_THROW(rawResponse = std::move(response.RawResponse));
  EXPECT_EQ(nullptr, rawResponse);
}

TEST(ResponseT, value)
{
  // Create Response<T> and test API
  auto rawResponse = std::make_unique<RawResponse>(1, 1, HttpStatusCode::Accepted, "Something");
  std::string fakeT("pretending this is the T");

  Azure::Response<std::string> response(fakeT, std::move(rawResponse));

  EXPECT_EQ(fakeT, response.Value);
  // use the reference to update the T inside the Response.
  EXPECT_NO_THROW(response.Value.clear());
  EXPECT_EQ("", response.Value);

  // const Response
  std::string constFakeT("pretending this is the T");
  Azure::Response<std::string> const constResponse(constFakeT, std::move(response.RawResponse));
  // Fetch Value from const Response
  EXPECT_EQ(constFakeT, constResponse.Value);
}

TEST(ResponseT, Assignable)
{
  EXPECT_FALSE(ClassTraits<RawResponse>::is_assignable<RawResponse>());
  EXPECT_FALSE(ClassTraits<RawResponse>::is_assignable<const RawResponse>());
  EXPECT_FALSE(ClassTraits<RawResponse>::is_trivially_assignable<RawResponse>());
  EXPECT_FALSE(ClassTraits<RawResponse>::is_trivially_assignable<const RawResponse>());
  EXPECT_FALSE(ClassTraits<RawResponse>::is_nothrow_assignable<RawResponse>());
  EXPECT_FALSE(ClassTraits<RawResponse>::is_nothrow_assignable<const RawResponse>());
}

TEST(ResponseT, Constructible)
{
  EXPECT_FALSE((ClassTraits<RawResponse, const std::vector<RawResponse>&>::is_constructible()));
  EXPECT_FALSE(
      (ClassTraits<RawResponse, const std::vector<RawResponse>&>::is_trivially_constructible()));
  EXPECT_FALSE(
      (ClassTraits<RawResponse, const std::vector<RawResponse>&>::is_nothrow_constructible()));
  EXPECT_FALSE(ClassTraits<RawResponse>::is_default_constructible());
  EXPECT_FALSE(ClassTraits<RawResponse>::is_trivially_default_constructible());
  EXPECT_FALSE(ClassTraits<RawResponse>::is_nothrow_default_constructible());
}

TEST(ResponseT, CopyAndMoveConstructible)
{
  EXPECT_TRUE(ClassTraits<RawResponse>::is_copy_constructible());
  EXPECT_FALSE(ClassTraits<RawResponse>::is_trivially_copy_constructible());
  EXPECT_FALSE(ClassTraits<RawResponse>::is_nothrow_copy_constructible());
  EXPECT_TRUE(ClassTraits<RawResponse>::is_move_constructible());
  EXPECT_FALSE(ClassTraits<RawResponse>::is_trivially_move_constructible());
  EXPECT_FALSE(ClassTraits<RawResponse>::is_nothrow_move_constructible());
}

TEST(ResponseT, CopyAndMoveAssignable)
{
  EXPECT_FALSE(ClassTraits<RawResponse>::is_copy_assignable());
  EXPECT_FALSE(ClassTraits<RawResponse>::is_trivially_copy_assignable());
  EXPECT_FALSE(ClassTraits<RawResponse>::is_nothrow_copy_assignable());
  EXPECT_FALSE(ClassTraits<RawResponse>::is_move_assignable());
  EXPECT_FALSE(ClassTraits<RawResponse>::is_trivially_move_assignable());
  EXPECT_FALSE(ClassTraits<RawResponse>::is_nothrow_move_assignable());
}

TEST(ResponseT, Destructible)
{

  EXPECT_TRUE(ClassTraits<RawResponse>::is_destructible());
  EXPECT_FALSE(ClassTraits<RawResponse>::is_trivially_destructible());
  EXPECT_TRUE(ClassTraits<RawResponse>::is_nothrow_destructible());
  EXPECT_FALSE(ClassTraits<RawResponse>::has_virtual_destructor());
}

