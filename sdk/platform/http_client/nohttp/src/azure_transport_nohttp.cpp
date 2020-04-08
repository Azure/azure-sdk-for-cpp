// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <http/http.hpp>

using namespace azure::core::http;

// implement send method
Response* Client::Send(Request& request)
{
  (void)request;
  throw;
}
