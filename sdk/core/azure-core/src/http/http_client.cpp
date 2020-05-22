// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "http/http_client.hpp"

#include "azure.hpp"

#include <type_traits>

using namespace Azure::Core::Http;

HttpClient::HttpClient(HttpClientOptions& options)
{
  if (options.Transport)
  {
    m_transport = std::move(options.Transport);
  }
}

Response HttpClient::Send(Context& context, Request& request)
{
  AZURE_UNREFERENCED_PARAMETER(context);
  AZURE_UNREFERENCED_PARAMETER(request);

  return m_transport->Send(context, request);
}
