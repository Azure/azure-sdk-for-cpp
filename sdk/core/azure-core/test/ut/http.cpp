// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include "http.hpp"
#include <azure/core/http/http.hpp>

#include <string>
#include <vector>

using namespace Azure::Core;

namespace Azure { namespace Core { namespace Test {

  // Request - Add header
  TEST(TestHttp, add_headers)
  {
    Http::HttpMethod httpMethod = Http::HttpMethod::Post;
    Http::Url url("http://test.com");
    Http::Request req(httpMethod, url);
    std::pair<std::string, std::string> expected("valid", "header");

    EXPECT_NO_THROW(req.AddHeader(expected.first, expected.second));
    EXPECT_PRED2(
        [](std::map<std::string, std::string> headers,
           std::pair<std::string, std::string> expected) {
          auto firstHeader = headers.begin();
          return firstHeader->first == expected.first && firstHeader->second == expected.second
              && headers.size() == 1;
        },
        req.GetHeaders(),
        expected);

    EXPECT_THROW(req.AddHeader("invalid()", "header"), std::runtime_error);

    // same header will just override
    std::pair<std::string, std::string> expectedOverride("valid", "override");
    EXPECT_NO_THROW(req.AddHeader(expectedOverride.first, expectedOverride.second));
    EXPECT_PRED2(
        [](std::map<std::string, std::string> headers,
           std::pair<std::string, std::string> expected) {
          auto firstHeader = headers.begin();
          return firstHeader->first == expected.first && firstHeader->second == expected.second
              && headers.size() == 1;
        },
        req.GetHeaders(),
        expectedOverride);

    // adding header after one error happened before
    std::pair<std::string, std::string> expected2("valid2", "header2");
    EXPECT_NO_THROW(req.AddHeader(expected2.first, expected2.second));
    EXPECT_PRED2(
        [](std::map<std::string, std::string> headers,
           std::pair<std::string, std::string> expected) {
          auto secondHeader = headers.begin();
          secondHeader++;
          return secondHeader->first == expected.first && secondHeader->second == expected.second
              && headers.size() == 2;
        },
        req.GetHeaders(),
        expected2);
  }

  // Response - Add header
  TEST(TestHttp, response_add_headers)
  {

    Http::RawResponse response(1, 1, Http::HttpStatusCode::Accepted, "Test");
    std::pair<std::string, std::string> expected("valid", "header");

    EXPECT_NO_THROW(response.AddHeader(expected.first, expected.second));
    EXPECT_PRED2(
        [](std::map<std::string, std::string> headers,
           std::pair<std::string, std::string> expected) {
          auto firstHeader = headers.begin();
          return firstHeader->first == expected.first && firstHeader->second == expected.second
              && headers.size() == 1;
        },
        response.GetHeaders(),
        expected);

    EXPECT_THROW(
        response.AddHeader("invalid()", "header"), Azure::Core::Http::InvalidHeaderException);

    // same header will just override
    std::pair<std::string, std::string> expectedOverride("valid", "override");
    EXPECT_NO_THROW(response.AddHeader(expectedOverride.first, expectedOverride.second));
    EXPECT_PRED2(
        [](std::map<std::string, std::string> headers,
           std::pair<std::string, std::string> expected) {
          auto firstHeader = headers.begin();
          return firstHeader->first == expected.first && firstHeader->second == expected.second
              && headers.size() == 1;
        },
        response.GetHeaders(),
        expectedOverride);

    // adding header after on error happened
    std::pair<std::string, std::string> expected2("valid2", "header2");
    EXPECT_NO_THROW(response.AddHeader(expected2.first, expected2.second));
    EXPECT_PRED2(
        [](std::map<std::string, std::string> headers,
           std::pair<std::string, std::string> expected) {
          auto secondtHeader = headers.begin();
          secondtHeader++;
          return secondtHeader->first == expected.first && secondtHeader->second == expected.second
              && headers.size() == 2;
        },
        response.GetHeaders(),
        expected2);

    // Response addHeader overload method to add from string
    EXPECT_THROW(response.AddHeader("inv(): header"), Azure::Core::Http::InvalidHeaderException);
    EXPECT_THROW(
        response.AddHeader("no delimiter header"), Azure::Core::Http::InvalidHeaderException);

    // adding header after previous error just happened on add from string
    EXPECT_NO_THROW(response.AddHeader("valid3: header3"));
    EXPECT_PRED2(
        [](std::map<std::string, std::string> headers,
           std::pair<std::string, std::string> expected) {
          auto secondtHeader = headers.begin();
          secondtHeader++;
          secondtHeader++;
          return secondtHeader->first == expected.first && secondtHeader->second == expected.second
              && headers.size() == 3;
        },
        response.GetHeaders(),
        (std::pair<std::string, std::string>("valid3", "header3")));
  }
}}} // namespace Azure::Core::Test
