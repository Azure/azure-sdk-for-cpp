// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <curl_client.hpp>
#include <http/http.hpp>

using namespace Azure::Core::Http;

// implement send method
std::unique_ptr<Response> Client::Send(Request& request)
{
  CurlClient client(request);
  // return request response
  return client.Send();
}
