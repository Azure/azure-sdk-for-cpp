// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/http.hpp"
#include "azure/core/http/winhttp/win_http_client.hpp"

#include <string>

using namespace Azure::Core::Http;

WinHttpTansport::WinHttpTansport() : HttpTransport() {}

WinHttpTansport::~WinHttpTansport() {}

std::unique_ptr<RawResponse> WinHttpTansport::Send(Context const& context, Request& request)
{
  void(context);
  void(request);

  throw;
}
