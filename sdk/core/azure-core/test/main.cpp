// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <http/http.hpp>

#include "gtest/gtest.h"

#include <string>
#include <vector>

using namespace azure::core;

TEST(Http_Request, getters)
{
  std::string http_method = "GET";
  std::string url = "http://test.url.com";
  http::Request req(http::http_method::GET, url);

  // EXPECT_PRED works better than just EQ because it will print values in log
  EXPECT_PRED2(
      [](http::http_method::HttpMethod a, http::http_method::HttpMethod b) { return a == b; },
      req.getMethod(),
      http::http_method::GET);
  EXPECT_PRED2([](std::string a, std::string b) { return a == b; }, req.getUrl(), url);
  EXPECT_PRED2([](std::string a, std::string b) { return a == b; }, req.getBody(), "");

  std::string body = "a body";
  http::Request req_with_body(http::http_method::GET, url, body);

  EXPECT_PRED2(
      [](http::http_method::HttpMethod a, http::http_method::HttpMethod b) { return a == b; },
      req_with_body.getMethod(),
      http::http_method::GET);
  EXPECT_PRED2([](std::string a, std::string b) { return a == b; }, req_with_body.getUrl(), url);
  EXPECT_PRED2([](std::string a, std::string b) { return a == b; }, req_with_body.getBody(), body);

  EXPECT_NO_THROW(req.addHeader("name", "value"));
  EXPECT_NO_THROW(req.addHeader("name2", "value2"));

  auto headers = req.getHeaders();

  EXPECT_TRUE(headers.count("name"));
  EXPECT_TRUE(headers.count("name2"));
  EXPECT_FALSE(headers.count("value"));

  auto value = headers.find("name");
  EXPECT_PRED2([](std::string a, std::string b) { return a == b; }, value->second, "value");
  auto value2 = headers.find("name2");
  EXPECT_PRED2([](std::string a, std::string b) { return a == b; }, value2->second, "value2");
}

TEST(Http_Request, query_parameter)
{
  std::string http_method = "GET";
  std::string url = "http://test.com";
  http::Request req(http::http_method::GET, url);

  EXPECT_NO_THROW(req.addQueryParameter("query", "value"));
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; }, req.getUrl(), url + "?query=value");

  EXPECT_NO_THROW(req.addQueryParameter("query2", "value2"));
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      req.getUrl(),
      url + "?query=value&query2=value2");

  std::string url_with_query = "http://test.com?query";
  http::Request req_with_query(http::http_method::GET, url_with_query);

  EXPECT_NO_THROW(req_with_query.addQueryParameter("query", "value"));
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      req_with_query.getUrl(),
      url_with_query + "&query=value");
}

TEST(Http_Request, add_path)
{
  std::string http_method = "GET";
  std::string url = "http://test.com";
  http::Request req(http::http_method::GET, url);

  EXPECT_NO_THROW(req.addPath("path"));
  EXPECT_PRED2([](std::string a, std::string b) { return a == b; }, req.getUrl(), url + "/path");

  EXPECT_NO_THROW(req.addQueryParameter("query", "value"));
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; }, req.getUrl(), url + "/path?query=value");

  EXPECT_NO_THROW(req.addPath("path2"));
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      req.getUrl(),
      url + "/path/path2?query=value");

  EXPECT_NO_THROW(req.addPath("path3"));
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      req.getUrl(),
      url + "/path/path2/path3?query=value");
}