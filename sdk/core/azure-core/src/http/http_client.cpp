// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "http/http_client.hpp"

#include "azure.hpp"
#include "http/curl/curl.hpp"

#include <type_traits>

using namespace Azure::Core::Http;

HttpClient::HttpClient(HttpClientOptions& options) : m_transportKind(options.Transport) {}

std::unique_ptr<Response> HttpClient::Send(Context& context, Request& request)
{
  switch (m_transportKind)
  {
    case TransportKind::Curl:
      return std::move(CurlTransport().Send(context, request));

    case TransportKind::WinHttp:
      throw;
  }

  throw;
}
