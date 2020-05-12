// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <http/http.hpp>

using namespace Azure::Core::Http;

// implement send method
Response Client::send(Request& request)
{
  (void)request;
  throw;
}
