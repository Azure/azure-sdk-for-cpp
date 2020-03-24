// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <http/http.hpp>

#include "gtest/gtest.h"

#include <stdlib.h>

using namespace azure::core;

TEST(Http_Request, getters)
{
  std::string http_method = "GET";
  std::string url = "GET";
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
}
