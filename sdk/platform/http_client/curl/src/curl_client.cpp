// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <curl_client.hpp>
#include <http/http.hpp>

#include <iostream>

using namespace azure::core::http;

Response CurlClient::send()
{
  auto performing = perform();

  if (performing != CURLE_OK)
  {
    switch (performing)
    {
      case CURLE_COULDNT_RESOLVE_HOST:
      {
        throw azure::core::http::CouldNotResolveHostException();
      }
      case CURLE_WRITE_ERROR:
      {
        throw azure::core::http::ErrorWhileWrittingResponse();
      }
      default:
      {
        throw azure::core::http::TransportException();
      }
    }
  }

  return Response(200, "OK\n");
}
