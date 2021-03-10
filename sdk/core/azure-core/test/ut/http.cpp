// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include "http.hpp"
#include <azure/core/http/http.hpp>
#include <azure/core/internal/null_body_stream.hpp>

#include <string>
#include <utility>
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

    EXPECT_NO_THROW(req.SetHeader(expected.first, expected.second));
    EXPECT_PRED2(
        [](Azure::Core::CaseInsensitiveMap headers, std::pair<std::string, std::string> expected) {
          auto firstHeader = headers.begin();
          return firstHeader->first == expected.first && firstHeader->second == expected.second
              && headers.size() == 1;
        },
        req.GetHeaders(),
        expected);

    EXPECT_THROW(req.SetHeader("invalid()", "header"), std::runtime_error);

    // same header will just override
    std::pair<std::string, std::string> expectedOverride("valid", "override");
    EXPECT_NO_THROW(req.SetHeader(expectedOverride.first, expectedOverride.second));
    EXPECT_PRED2(
        [](Azure::Core::CaseInsensitiveMap headers, std::pair<std::string, std::string> expected) {
          auto firstHeader = headers.begin();
          return firstHeader->first == expected.first && firstHeader->second == expected.second
              && headers.size() == 1;
        },
        req.GetHeaders(),
        expectedOverride);

    // adding header after one error happened before
    std::pair<std::string, std::string> expected2("valid2", "header2");
    EXPECT_NO_THROW(req.SetHeader(expected2.first, expected2.second));
    EXPECT_PRED2(
        [](Azure::Core::CaseInsensitiveMap headers, std::pair<std::string, std::string> expected) {
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

    EXPECT_NO_THROW(response.SetHeader(expected.first, expected.second));
    EXPECT_PRED2(
        [](Azure::Core::CaseInsensitiveMap headers, std::pair<std::string, std::string> expected) {
          auto firstHeader = headers.begin();
          return firstHeader->first == expected.first && firstHeader->second == expected.second
              && headers.size() == 1;
        },
        response.GetHeaders(),
        expected);

    EXPECT_THROW(
        response.SetHeader("invalid()", "header"), Azure::Core::Http::InvalidHeaderException);

    // same header will just override
    std::pair<std::string, std::string> expectedOverride("valid", "override");
    EXPECT_NO_THROW(response.SetHeader(expectedOverride.first, expectedOverride.second));
    EXPECT_PRED2(
        [](Azure::Core::CaseInsensitiveMap headers, std::pair<std::string, std::string> expected) {
          auto firstHeader = headers.begin();
          return firstHeader->first == expected.first && firstHeader->second == expected.second
              && headers.size() == 1;
        },
        response.GetHeaders(),
        expectedOverride);

    // adding header after on error happened
    std::pair<std::string, std::string> expected2("valid2", "header2");
    EXPECT_NO_THROW(response.SetHeader(expected2.first, expected2.second));
    EXPECT_PRED2(
        [](Azure::Core::CaseInsensitiveMap headers, std::pair<std::string, std::string> expected) {
          auto secondtHeader = headers.begin();
          secondtHeader++;
          return secondtHeader->first == expected.first && secondtHeader->second == expected.second
              && headers.size() == 2;
        },
        response.GetHeaders(),
        expected2);

    // Response SetHeader overload method to add from string
    EXPECT_THROW(response.SetHeader("inv(): header"), Azure::Core::Http::InvalidHeaderException);
    EXPECT_THROW(
        response.SetHeader("no delimiter header"), Azure::Core::Http::InvalidHeaderException);

    // adding header after previous error just happened on add from string
    EXPECT_NO_THROW(response.SetHeader("valid3: header3"));
    EXPECT_PRED2(
        [](Azure::Core::CaseInsensitiveMap headers, std::pair<std::string, std::string> expected) {
          auto secondtHeader = headers.begin();
          secondtHeader++;
          secondtHeader++;
          return secondtHeader->first == expected.first && secondtHeader->second == expected.second
              && headers.size() == 3;
        },
        response.GetHeaders(),
        (std::pair<std::string, std::string>("valid3", "header3")));
  }

  // HTTP Range
  TEST(TestHttp, Range)
  {
    {
      Http::Range r{10, 1};
      EXPECT_EQ(r.Offset, 10);
      EXPECT_TRUE(r.Length.HasValue());
      EXPECT_EQ(r.Length.GetValue(), 1);
    }
    {
      Http::Range r;
      r.Offset = 10;
      EXPECT_EQ(r.Offset, 10);
      EXPECT_FALSE(r.Length.HasValue());
    }
    {
      Http::Range r;
      r.Length = 10;
      EXPECT_EQ(r.Offset, 0);
      EXPECT_TRUE(r.Length.HasValue());
      EXPECT_EQ(r.Length.GetValue(), 10);
    }
    {
      Http::Range r;
      EXPECT_EQ(r.Offset, 0);
      EXPECT_FALSE(r.Length.HasValue());
    }
  }

  TEST(TestHttp, RequestStartTry)
  {
    {
      Http::HttpMethod httpMethod = Http::HttpMethod::Post;
      Http::Url url("http://test.com");
      Http::Request req(httpMethod, url);

      Azure::Core::IO::_internal::NullBodyStream* d
          = dynamic_cast<Azure::Core::IO::_internal::NullBodyStream*>(req.GetBodyStream());
      EXPECT_TRUE(d);

      req.StartTry();

      EXPECT_NO_THROW(req.SetHeader("namE", "retryValue"));

      auto headers = req.GetHeaders();

      EXPECT_TRUE(headers.count("name"));

      req.StartTry();
      headers = req.GetHeaders();

      EXPECT_FALSE(headers.count("name"));

      d = dynamic_cast<Azure::Core::IO::_internal::NullBodyStream*>(req.GetBodyStream());
      EXPECT_TRUE(d);
    }

    {
      Http::HttpMethod httpMethod = Http::HttpMethod::Post;
      Http::Url url("http://test.com");

      std::vector<uint8_t> data = {1, 2, 3, 4};
      Azure::Core::IO::MemoryBodyStream stream(data);

      // Change the offset of the stream to be non-zero by reading a byte.
      std::vector<uint8_t> temp(2);
      EXPECT_EQ(
          Azure::Core::IO::BodyStream::ReadToCount(
              stream, temp.data(), 1, Context::GetApplicationContext()),
          1);

      EXPECT_EQ(temp[0], 1);
      EXPECT_EQ(temp[1], 0);

      Http::Request req(httpMethod, url, &stream);

      Azure::Core::IO::MemoryBodyStream* d
          = dynamic_cast<Azure::Core::IO::MemoryBodyStream*>(req.GetBodyStream());
      EXPECT_TRUE(d);

      req.StartTry();

      d = dynamic_cast<Azure::Core::IO::MemoryBodyStream*>(req.GetBodyStream());
      EXPECT_TRUE(d);

      // Verify that StartTry rewound the stream back.
      auto getStream = req.GetBodyStream();
      EXPECT_EQ(
          Azure::Core::IO::BodyStream::ReadToCount(
              *getStream, temp.data(), 2, Context::GetApplicationContext()),
          2);

      EXPECT_EQ(temp[0], 1);
      EXPECT_EQ(temp[1], 2);
    }
  }

}}} // namespace Azure::Core::Test
