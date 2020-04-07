// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <http/http.hpp>

using namespace Azure::Core::Http;

// implement Send method
Response Client::Send(Request& request)
{
  (void)request;
  throw;
}
