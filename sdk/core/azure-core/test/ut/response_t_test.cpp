// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include <azure/core/http/http.hpp>
#include <azure/core/response.hpp>

#include <memory>
#include <stdexcept>
#include <string>

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

  // This is fine, we can keep extracting the rawReponse, only that the second time the it would be
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