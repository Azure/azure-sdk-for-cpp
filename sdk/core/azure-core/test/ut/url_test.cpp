//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include <azure/core/http/http.hpp>

namespace Azure { namespace Core { namespace Test {

  class TestURL : public ::testing::Test {
  };

}}} // namespace Azure::Core::Test

using namespace Azure::Core;

namespace Azure { namespace Core { namespace Test {

  TEST(TestURL, getters)
  {
    Http::HttpMethod httpMethod = Http::HttpMethod::Get;
    Core::Url url("http://test.url.com");
    Http::Request req(httpMethod, url);

    // EXPECT_PRED works better than just EQ because it will print values in log
    EXPECT_PRED2(
        [](Http::HttpMethod a, Http::HttpMethod b) { return a == b; }, req.GetMethod(), httpMethod);
    EXPECT_PRED2(
        [](std::string a, std::string b) { return a == b; },
        req.GetUrl().GetAbsoluteUrl(),
        url.GetAbsoluteUrl());

    EXPECT_NO_THROW(req.SetHeader("Name", "value"));
    EXPECT_NO_THROW(req.SetHeader("naME2", "value2"));

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
    EXPECT_NO_THROW(req.SetHeader("namE", "retryValue"));
    EXPECT_NO_THROW(req.SetHeader("HEADER-to-Lower-123", "retryValue2"));
    EXPECT_NO_THROW(req.SetHeader("newHeader", "new"));

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

  TEST(TestURL, query_parameter)
  {
    Http::HttpMethod httpMethod = Http::HttpMethod::Put;
    Core::Url url("http://test.com");
    EXPECT_NO_THROW(url.AppendQueryParameter("query", "value"));

    Http::Request req(httpMethod, url);

    EXPECT_PRED2(
        [](std::string a, std::string b) { return a == b; },
        req.GetUrl().GetAbsoluteUrl(),
        url.GetAbsoluteUrl());

    Core::Url url_with_query("http://test.com?query=1");
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

    EXPECT_PRED2(
        [](std::string a, std::string b) { return a == b; },
        req_with_query.GetUrl().GetAbsoluteUrl(),
        "http://test.com?query=retryValue");
  }

  TEST(URL, query_parameter_encode_decode)
  {
    Core::Url url("http://test.com");
    EXPECT_NO_THROW(url.AppendQueryParameter("query", Core::Url::Encode("va=lue")));

    // Default encoder from URL won't encode an equal symbol
    EXPECT_PRED2(
        [](std::string a, std::string b) { return a == b; },
        url.GetAbsoluteUrl(),
        "http://test.com?query=va%3Dlue");

    // Provide symbol to do not encode
    EXPECT_NO_THROW(url.AppendQueryParameter("query", Core::Url::Encode("va=lue", "=")));
    EXPECT_PRED2(
        [](std::string a, std::string b) { return a == b; },
        url.GetAbsoluteUrl(),
        "http://test.com?query=va=lue");

    // Provide more than one extra symbol to be encoded
    EXPECT_NO_THROW(url.AppendQueryParameter("query", Core::Url::Encode("va=l u?e", " ?")));
    EXPECT_PRED2(
        [](std::string a, std::string b) { return a == b; },
        url.GetAbsoluteUrl(),
        "http://test.com?query=va%3Dl u?e");

    // default, encode it all
    EXPECT_NO_THROW(url.AppendQueryParameter("query", Core::Url::Encode("va=l u?e")));
    EXPECT_PRED2(
        [](std::string a, std::string b) { return a == b; },
        url.GetAbsoluteUrl(),
        "http://test.com?query=va%3Dl%20u%3Fe");
  }

  TEST(URL, add_path)
  {
    Http::HttpMethod httpMethod = Http::HttpMethod::Post;
    Core::Url url("http://test.com");
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

  TEST(URL, getPort)
  {
    Core::Url url("http://test.com:9090");
    uint16_t expected = 9090;

    EXPECT_PRED2(
        [](int expectedValue, int value) { return expectedValue == value; },
        url.GetPort(),
        expected);
  }

  TEST(URL, getPortConst)
  {
    Core::Url const url("https://test.com:500");
    uint16_t expected = 500;

    EXPECT_PRED2(
        [](uint16_t expectedValue, uint16_t code) { return expectedValue == code; },
        url.GetPort(),
        expected);
  }

  TEST(URL, getScheme)
  {
    Core::Url url("http://test.com:9090");
    std::string expected = "http";

    EXPECT_PRED2(
        [](std::string expectedValue, std::string value) { return expectedValue == value; },
        url.GetScheme(),
        expected);
  }

  TEST(URL, getSchemeConst)
  {
    Core::Url const url("http://test.com:9090");
    std::string expected = "http";

    EXPECT_PRED2(
        [](std::string expectedValue, std::string value) { return expectedValue == value; },
        url.GetScheme(),
        expected);
  }

  TEST(URL, getPortMax) { EXPECT_THROW(Core::Url url("http://test.com:65540"), std::out_of_range); }

  TEST(URL, getPortAfterSet)
  {
    Core::Url url("http://test.com");

    uint16_t expected = 0;

    EXPECT_PRED2(
        [](uint16_t expectedValue, uint16_t code) { return expectedValue == code; },
        url.GetPort(),
        expected);

    url.SetPort(40);
    expected = 40;

    EXPECT_PRED2(
        [](uint16_t expectedValue, uint16_t code) { return expectedValue == code; },
        url.GetPort(),
        expected);

    url.SetPort(90);
    expected = 90;

    EXPECT_PRED2(
        [](uint16_t expectedValue, uint16_t code) { return expectedValue == code; },
        url.GetPort(),
        expected);
  }

  TEST(URL, getPortDefault)
  {
    Core::Url url("http://test.com");
    uint16_t expected = 0;

    EXPECT_PRED2(
        [](uint16_t expectedValue, uint16_t code) { return expectedValue == code; },
        url.GetPort(),
        expected);
  }

  TEST(URL, getPorStartAsNonDigit)
  {
    EXPECT_THROW(Core::Url url("http://test.com:A1"), std::invalid_argument);
  }

  TEST(URL, getPortInvalidInput)
  {
    EXPECT_THROW(Core::Url url("http://test.com:4A"), std::invalid_argument);
  }

  TEST(URL, getPortInvalidArg)
  {
    EXPECT_THROW(Core::Url url("http://test.com:ThisIsNotAPort"), std::invalid_argument);
  }

  TEST(URL, getPortOutfRange)
  {
    EXPECT_THROW(Core::Url url("http://test.com:99999999999999999"), std::out_of_range);
  }

  TEST(URL, empty)
  {
    Core::Url url;
    EXPECT_EQ(url.GetAbsoluteUrl(), std::string());
  }

  TEST(URL, AppendPathSlash)
  {
    Core::Url url1;
    Core::Url url2;

    url1.AppendPath("x");
    EXPECT_EQ(url1.GetPath(), "x");

    url2.AppendPath("x/");
    EXPECT_EQ(url2.GetPath(), "x/");

    url1.AppendPath("y");
    url2.AppendPath("y");

    EXPECT_EQ(url1.GetPath(), "x/y");
    EXPECT_EQ(url2.GetPath(), "x/y");
  }

  TEST(URL, Decode)
  {
    EXPECT_EQ(Core::Url::Decode("+%61b"), " ab");
    EXPECT_THROW(Core::Url::Decode("%"), std::runtime_error);
    EXPECT_THROW(Core::Url::Decode("%GA"), std::runtime_error);
    EXPECT_THROW(Core::Url::Decode("%AG"), std::runtime_error);
  }

  TEST(URL, AppendQueryParameters)
  {
    Core::Url url("http://www.microsoft.com??param=value");
    auto params = url.GetQueryParameters();

    EXPECT_EQ(params.size(), 1);
    EXPECT_NE(params.find("param"), params.end());
    EXPECT_EQ(params["param"], "value");
  }
}}} // namespace Azure::Core::Test
