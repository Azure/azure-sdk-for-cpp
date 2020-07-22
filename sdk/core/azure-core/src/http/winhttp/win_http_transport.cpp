// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure.hpp"
#include "http/http.hpp"
#include "http/winhttp/win_http_client.hpp"

#include <string>

using namespace Azure::Core::Http;

WinHttpTansport::WinHttpTansport() : 
    HttpTransport()
{
}

WinHttpTansport::~WinHttpTansport() {}


std::unique_ptr<RawResponse> WinHttpTansport::Send(Context& context, Request& request)
{
  AZURE_UNREFERENCED_PARAMETER(context);
  AZURE_UNREFERENCED_PARAMETER(request);

  throw;
}
