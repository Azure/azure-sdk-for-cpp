// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <http/http.hpp>
#include <memory>

using namespace azure::core::http;

// implement send method
std::unique_ptr<Response> Client::Send(Request& request)
{
  (void)request;
  throw;
}
