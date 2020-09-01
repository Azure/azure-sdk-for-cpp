// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <azure/core/http/http.hpp>
#include <string>
#include <vector>

using namespace Azure::Core;

TEST(Http_Request, getters)
{
  Http::HttpMethod httpMethod = Http::HttpMethod::Get;
  Http::URL url("http://test.url.com");
  Http::Request req(httpMethod, url);

  // EXPECT_PRED works better than just EQ because it will print values in log
  EXPECT_PRED2(
      [](Http::HttpMethod a, Http::HttpMethod b) { return a == b; }, req.GetMethod(), httpMethod);
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; }, req.GetURL().ToString(), url.ToString());

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
  req.StartRetry();
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
  EXPECT_PRED2([](std::string a, std::string b) { return a == b; }, value2->second, "retryValue2");
  auto value3 = headers.find("newheader");
  EXPECT_PRED2([](std::string a, std::string b) { return a == b; }, value3->second, "new");
}

TEST(Http_Request, query_parameter)
{
  Http::HttpMethod httpMethod = Http::HttpMethod::Put;
  Http::URL url("http://test.com");
  EXPECT_NO_THROW(url.AddQueryParameter("query", "value"));

  Http::Request req(httpMethod, url);

  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; }, req.GetURL().ToString(), url.ToString());

  Http::URL url_with_query("http://test.com?query=1");
  Http::Request req_with_query(httpMethod, url_with_query);

  // override if adding same query parameter key that is already in url
  EXPECT_NO_THROW(req_with_query.GetURL().AppendQuery("query", "value"));
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      req_with_query.GetURL().ToString(),
      "http://test.com?query=value");

  // retry query params testing
  req.StartRetry();
  // same query parameter should override previous
  EXPECT_NO_THROW(req.AddQueryParameter("query", "retryValue"));

  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; }, req.GetURL().ToString(), url.ToString());
}

TEST(Http_Request, add_path)
{
  Http::HttpMethod httpMethod = Http::HttpMethod::Post;
  Http::URL url("http://test.com");
  Http::Request req(httpMethod, url);

  EXPECT_NO_THROW(req.GetURL().AppendPath("path"));
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      req.GetURL().ToString(),
      "http://test.com/path");

  EXPECT_NO_THROW(req.AddQueryParameter("query", "value"));
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      req.GetURL().ToString(),
      "http://test.com/path?query=value");

  EXPECT_NO_THROW(req.GetURL().AppendPath("path2"));
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      req.GetURL().ToString(),
      "http://test.com/path/path2?query=value");

  EXPECT_NO_THROW(req.GetURL().AppendPath("path3"));
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      req.GetURL().ToString(),
      "http://test.com/path/path2/path3?query=value");
}
