// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <http/http.hpp>
#include <http/request.hpp>

#include "gtest/gtest.h"

#include <string>
#include <vector>

using namespace azure::core;

TEST(Http_Request, getters)
{
  http::HttpMethod httpMethod = http::HttpMethod::GET;
  std::string url = "http://test.url.com";
  http::Request req(httpMethod, url);

  // EXPECT_PRED works better than just EQ because it will print values in log
  EXPECT_PRED2(
      [](http::HttpMethod a, http::HttpMethod b) { return a == b; }, req.getMethod(), httpMethod);
  EXPECT_PRED2([](std::string a, std::string b) { return a == b; }, req.getEncodedUrl(), url);
  /* EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      req.getBodyStream(),
      http::BodyStream::null);
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      req.getBodyBuffer(),
      http::BodyBuffer::null); */

  uint8_t buffer[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  auto bufferBody = http::BodyBuffer(buffer, sizeof(buffer));
  http::Request requestWithBody(httpMethod, url, bufferBody);

  EXPECT_PRED2(
      [](http::HttpMethod a, http::HttpMethod b) { return a == b; },
      requestWithBody.getMethod(),
      httpMethod);
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; }, requestWithBody.getEncodedUrl(), url);
  /* EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      requestWithBody.getBodyStream(),
      http::BodyStream::null); */

  // body with buffer
  auto body = requestWithBody.getBodyBuffer();
  ASSERT_EQ(body._bodyBufferSize, 10);
  for (auto i = 0; i < 10; i++)
  {
    ASSERT_EQ(body._bodyBuffer[i], i);
  }

  EXPECT_NO_THROW(req.addHeader("name", "value"));
  EXPECT_NO_THROW(req.addHeader("name2", "value2"));

  auto headers = req.getHeaders();

  EXPECT_TRUE(headers.count("name"));
  EXPECT_TRUE(headers.count("name2"));
  EXPECT_FALSE(headers.count("newHeader"));

  auto value = headers.find("name");
  EXPECT_PRED2([](std::string a, std::string b) { return a == b; }, value->second, "value");
  auto value2 = headers.find("name2");
  EXPECT_PRED2([](std::string a, std::string b) { return a == b; }, value2->second, "value2");

  // now add to retry headers
  req.startRetry();
  // same headers first, then one new
  EXPECT_NO_THROW(req.addHeader("name", "retryValue"));
  EXPECT_NO_THROW(req.addHeader("name2", "retryValue2"));
  EXPECT_NO_THROW(req.addHeader("newHeader", "new"));

  headers = req.getHeaders();

  EXPECT_TRUE(headers.count("name"));
  EXPECT_TRUE(headers.count("name2"));
  EXPECT_TRUE(headers.count("newHeader"));

  value = headers.find("name");
  EXPECT_PRED2([](std::string a, std::string b) { return a == b; }, value->second, "retryValue");
  value2 = headers.find("name2");
  EXPECT_PRED2([](std::string a, std::string b) { return a == b; }, value2->second, "retryValue2");
  auto value3 = headers.find("newHeader");
  EXPECT_PRED2([](std::string a, std::string b) { return a == b; }, value3->second, "new");
}

TEST(Http_Request, query_parameter)
{
  http::HttpMethod httpMethod = http::HttpMethod::PUT;
  std::string url = "http://test.com";
  http::Request req(httpMethod, url);

  EXPECT_NO_THROW(req.addQueryParameter("query", "value"));
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      req.getEncodedUrl(),
      url + "?query=value");

  std::string url_with_query = "http://test.com?query=1";
  http::Request req_with_query(httpMethod, url_with_query);

  // ignore if adding same query parameter key that is already in url
  EXPECT_NO_THROW(req_with_query.addQueryParameter("query", "value"));
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      req_with_query.getEncodedUrl(),
      url_with_query);

  // retry query params testing
  req.startRetry();
  // same query parameter should override previous
  EXPECT_NO_THROW(req.addQueryParameter("query", "retryValue"));

  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      req.getEncodedUrl(),
      url + "?query=retryValue");
}

TEST(Http_Request, add_path)
{
  http::HttpMethod httpMethod = http::HttpMethod::POST;
  std::string url = "http://test.com";
  http::Request req(httpMethod, url);

  EXPECT_NO_THROW(req.addPath("path"));
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; }, req.getEncodedUrl(), url + "/path");

  EXPECT_NO_THROW(req.addQueryParameter("query", "value"));
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      req.getEncodedUrl(),
      url + "/path?query=value");

  EXPECT_NO_THROW(req.addPath("path2"));
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      req.getEncodedUrl(),
      url + "/path/path2?query=value");

  EXPECT_NO_THROW(req.addPath("path3"));
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      req.getEncodedUrl(),
      url + "/path/path2/path3?query=value");
}