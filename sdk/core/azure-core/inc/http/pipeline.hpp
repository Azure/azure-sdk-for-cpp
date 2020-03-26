// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "policy.hpp"
#include <vector>

namespace azure {
namespace core {
namespace http {
class HttpTransport
{};

class http_pipeline
{
private:
  HttpTransport _transport;
  vector<unique_ptr<HttpPolicy>> _policies;

public:
  http_pipeline(HttpTransport transport, HttpPolicy policies[] = null)
  {
    _transport = transport;
    _policies = policies;
  }

public:
  virtual int process(http_message message, HttpPolicy polices[]) = 0;
};

} // namespace http
} // namespace core
} // namespace azure
