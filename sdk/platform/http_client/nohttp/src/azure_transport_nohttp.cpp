// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <http/http.hpp>

using namespace azure::core::http;

// implement send method
std::shared_ptr<Response> Client::Send(Request& request)
{
  (void)request;
  throw;
}
