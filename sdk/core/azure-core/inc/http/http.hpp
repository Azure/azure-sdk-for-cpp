// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <internal/contract.hpp>

namespace azure
{
namespace core
{
namespace http
{

class http_request
{

private:
  /* data */
public:
  http_request();
  ~http_request();
};

http_request::http_request() {}
http_request::~http_request() {}

} // namespace http
} // namespace core
} // namespace azure
