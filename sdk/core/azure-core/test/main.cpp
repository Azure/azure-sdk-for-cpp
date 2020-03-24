// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <http/http.hpp>

#include "gtest/gtest.h"

#include <stdlib.h>

using namespace azure::core;

TEST(Http_Request, getters)
{
  std::string http_method = "GET";
  std::string url = "http://test.url.com";
  http::Request req(http_method, url);

  // EXPECT_PRED works better than just EQ because it will print values in log
  EXPECT_PRED2([](string a, string b) { return a == b; }, req.getMethod(), http_method);
  EXPECT_PRED2([](string a, string b) { return a == b; }, req.getUrl(), url);
  EXPECT_PRED2([](string a, string b) { return a == b; }, req.getBody(), "");

  std::string body = "a body";
  http::Request req_with_body(http_method, url, body);

  EXPECT_PRED2([](string a, string b) { return a == b; }, req_with_body.getMethod(), http_method);
  EXPECT_PRED2([](string a, string b) { return a == b; }, req_with_body.getUrl(), url);
  EXPECT_PRED2([](string a, string b) { return a == b; }, req_with_body.getBody(), body);

  EXPECT_NO_THROW(req.addHeader("name", "value"));
  EXPECT_NO_THROW(req.addHeader("name2", "value2"));

  std::vector<http::Header> headers = req.getHeaders();

  EXPECT_PRED2([](string a, string b) { return a == b; }, headers[0].getName(), "name");
  EXPECT_PRED2([](string a, string b) { return a == b; }, headers[0].getValue(), "value");
  EXPECT_PRED2([](string a, string b) { return a == b; }, headers[1].getName(), "name2");
  EXPECT_PRED2([](string a, string b) { return a == b; }, headers[1].getValue(), "value2");
}

TEST(Http_Request, query_parameter)
{
  std::string http_method = "GET";
  std::string url = "http://test.com";
  http::Request req(http_method, url);

  EXPECT_NO_THROW(req.addQueryParameter("query", "value"));
  EXPECT_PRED2([](string a, string b) { return a == b; }, req.getUrl(), url + "?query=value");

  EXPECT_NO_THROW(req.addQueryParameter("query2", "value2"));
  EXPECT_PRED2(
      [](string a, string b) { return a == b; }, req.getUrl(), url + "?query=value&query2=value2");

  std::string url_with_query = "http://test.com?query";
  http::Request req_with_query(http_method, url_with_query);

  EXPECT_NO_THROW(req_with_query.addQueryParameter("query", "value"));
  EXPECT_PRED2(
      [](string a, string b) { return a == b; },
      req_with_query.getUrl(),
      url_with_query + "&query=value");
}