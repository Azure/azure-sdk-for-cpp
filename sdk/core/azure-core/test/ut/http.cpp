// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include "http.hpp"
#include <azure/core/http/http.hpp>

#include <string>
#include <vector>

using namespace Azure::Core;

namespace Azure { namespace Core { namespace Test {

  TEST(TestHttp, getters)
  {
    Http::HttpMethod httpMethod = Http::HttpMethod::Get;
    Http::Url url("http://test.url.com");
    Http::Request req(httpMethod, url);

    // EXPECT_PRED works better than just EQ because it will print values in log
    EXPECT_PRED2(
        [](Http::HttpMethod a, Http::HttpMethod b) { return a == b; }, req.GetMethod(), httpMethod);
    EXPECT_PRED2(
        [](std::string a, std::string b) { return a == b; },
        req.GetUrl().GetAbsoluteUrl(),
        url.GetAbsoluteUrl());

    EXPECT_NO_THROW(req.AddHeader("Name", "value"));
    EXPECT_NO_THROW(req.AddHeader("naME2", "value2"));

    auto headers = req.GetHeaders();

    // Headers should be lower-cased
    EXPECT_TRUE(headers.count("name"));
    EXPECT_TRUE(headers.count("name2"));
    EXPECT_FALSE(headers.count("newHeader"));

    auto value = headers.find("name");
    EXPECT_PRED2([](std::string a, std::string b) { return a == b; }, value->second, "value");
    auto value2 = headers.find("name2");
    EXPECT_PRED2([](std::string a, std::string b) { return a == b; }, value2->second, "value2");

    // now add to retry headers
    req.StartTry();

    // same headers first, then one new
    EXPECT_NO_THROW(req.AddHeader("namE", "retryValue"));
    EXPECT_NO_THROW(req.AddHeader("HEADER-to-Lower-123", "retryValue2"));
    EXPECT_NO_THROW(req.AddHeader("newHeader", "new"));

    headers = req.GetHeaders();

    EXPECT_TRUE(headers.count("name"));
    EXPECT_TRUE(headers.count("header-to-lower-123"));
    EXPECT_TRUE(headers.count("newheader"));

    value = headers.find("name");
    EXPECT_PRED2([](std::string a, std::string b) { return a == b; }, value->second, "retryValue");
    value2 = headers.find("header-to-lower-123");
    EXPECT_PRED2(
        [](std::string a, std::string b) { return a == b; }, value2->second, "retryValue2");
    auto value3 = headers.find("newheader");
    EXPECT_PRED2([](std::string a, std::string b) { return a == b; }, value3->second, "new");

    req.RemoveHeader("name");
    headers = req.GetHeaders();
    EXPECT_FALSE(headers.count("name"));
    EXPECT_TRUE(headers.count("header-to-lower-123"));
    EXPECT_TRUE(headers.count("newheader"));

    req.RemoveHeader("header-to-lower-123");
    headers = req.GetHeaders();
    EXPECT_FALSE(headers.count("name"));
    EXPECT_FALSE(headers.count("header-to-lower-123"));
    EXPECT_TRUE(headers.count("newheader"));

    req.RemoveHeader("newheader");
    headers = req.GetHeaders();
    EXPECT_FALSE(headers.count("name"));
    EXPECT_FALSE(headers.count("header-to-lower-123"));
    EXPECT_FALSE(headers.count("newheader"));
  }

  TEST(TestHttp, query_parameter)
  {
    Http::HttpMethod httpMethod = Http::HttpMethod::Put;
    Http::Url url("http://test.com");
    EXPECT_NO_THROW(url.AppendQueryParameter("query", "value"));

    Http::Request req(httpMethod, url);

    EXPECT_PRED2(
        [](std::string a, std::string b) { return a == b; },
        req.GetUrl().GetAbsoluteUrl(),
        url.GetAbsoluteUrl());

    Http::Url url_with_query("http://test.com?query=1");
    Http::Request req_with_query(httpMethod, url_with_query);

    // override if adding same query parameter key that is already in url
    EXPECT_NO_THROW(req_with_query.GetUrl().AppendQueryParameter("query", "value"));
    EXPECT_PRED2(
        [](std::string a, std::string b) { return a == b; },
        req_with_query.GetUrl().GetAbsoluteUrl(),
        "http://test.com?query=value");

    // retry query params testing
    req_with_query.StartTry();
    // same query parameter should override previous
    EXPECT_NO_THROW(req_with_query.GetUrl().AppendQueryParameter("query", "retryValue"));

    EXPECT_TRUE(req_with_query.m_retryModeEnabled);

    EXPECT_PRED2(
        [](std::string a, std::string b) { return a == b; },
        req_with_query.GetUrl().GetAbsoluteUrl(),
        "http://test.com?query=retryValue");
  }

  TEST(TestHttp, query_parameter_encode_decode)
  {
    Http::HttpMethod httpMethod = Http::HttpMethod::Put;
    Http::Url url("http://test.com");
    EXPECT_NO_THROW(url.AppendQueryParameter("query", Http::Url::Encode("va=lue")));

    // Default encoder from URL won't encode an equal symbol
    EXPECT_PRED2(
        [](std::string a, std::string b) { return a == b; },
        url.GetAbsoluteUrl(),
        "http://test.com?query=va%3Dlue");

    // Provide symbol to do not encode
    EXPECT_NO_THROW(url.AppendQueryParameter("query", Http::Url::Encode("va=lue", "=")));
    EXPECT_PRED2(
        [](std::string a, std::string b) { return a == b; },
        url.GetAbsoluteUrl(),
        "http://test.com?query=va=lue");

    // Provide more than one extra symbol to be encoded
    EXPECT_NO_THROW(url.AppendQueryParameter("query", Http::Url::Encode("va=l u?e", " ?")));
    EXPECT_PRED2(
        [](std::string a, std::string b) { return a == b; },
        url.GetAbsoluteUrl(),
        "http://test.com?query=va%3Dl u?e");

    // default, encode it all
    EXPECT_NO_THROW(url.AppendQueryParameter("query", Http::Url::Encode("va=l u?e")));
    EXPECT_PRED2(
        [](std::string a, std::string b) { return a == b; },
        url.GetAbsoluteUrl(),
        "http://test.com?query=va%3Dl%20u%3Fe");
  }

  TEST(TestHttp, add_path)
  {
    Http::HttpMethod httpMethod = Http::HttpMethod::Post;
    Http::Url url("http://test.com");
    Http::Request req(httpMethod, url);

    EXPECT_NO_THROW(req.GetUrl().AppendPath("path"));
    EXPECT_PRED2(
        [](std::string a, std::string b) { return a == b; },
        req.GetUrl().GetAbsoluteUrl(),
        "http://test.com/path");

    EXPECT_NO_THROW(req.GetUrl().AppendQueryParameter("query", "value"));
    EXPECT_PRED2(
        [](std::string a, std::string b) { return a == b; },
        req.GetUrl().GetAbsoluteUrl(),
        "http://test.com/path?query=value");

    EXPECT_NO_THROW(req.GetUrl().AppendPath("path2"));
    EXPECT_PRED2(
        [](std::string a, std::string b) { return a == b; },
        req.GetUrl().GetAbsoluteUrl(),
        "http://test.com/path/path2?query=value");

    EXPECT_NO_THROW(req.GetUrl().AppendPath("path3"));
    EXPECT_PRED2(
        [](std::string a, std::string b) { return a == b; },
        req.GetUrl().GetAbsoluteUrl(),
        "http://test.com/path/path2/path3?query=value");
  }

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
