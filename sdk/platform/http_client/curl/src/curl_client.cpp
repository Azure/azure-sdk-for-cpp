// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <curl_client.hpp>
#include <http/http.hpp>

#include <iostream>

using namespace Azure::Core::Http;
using namespace std;

Response CurlClient::send()
{
  auto performing = perform();

  if (performing != CURLE_OK)
  {
    switch (performing)
    {
      case CURLE_COULDNT_RESOLVE_HOST:
      {
        throw Azure::Core::Http::CouldNotResolveHostException();
      }
      case CURLE_WRITE_ERROR:
      {
        throw Azure::Core::Http::ErrorWhileWrittingResponse();
      }
      default:
      {
        throw Azure::Core::Http::TransportException();
      }
    }
  }

  return Response(200, "OK\n");
}
