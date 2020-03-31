// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <http/http.hpp>

#include <string>

using namespace Azure::Core::Http;

void customer()
{
  auto retry = std::make_unique<RetryPolicy>();
  using pipeline_type = RetryPolicy<RequestIdPolicy>;
  pipeline_type pipeline;

  std::string url;

  Request req = Request(HttpMethod::PUT, url);

  Azure::Core::Context ctx;

  

  pipeline_type::per_request_state state; // per request allocation
  auto response = pipeline.process(req, state);
}