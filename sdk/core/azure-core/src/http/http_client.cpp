// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "http/http.hpp"
#include "http/http_client.hpp"
#include "http/pipeline.hpp"
#include "http/policy.hpp"

#include "azure.hpp"
#include "http/curl/curl.hpp"

#include <type_traits>
#include <memory>
#include <vector>

using namespace Azure::Core::Http;

HttpClient::HttpClient(HttpClientOptions& options) : m_transportKind(options.Transport)
{
  m_pHttpPipeline = new HttpPipeline(std::make_unique<CurlTransport>());
}

std::unique_ptr<Response> HttpClient::Send(Context& context, Request& request)
{
  if (m_pHttpPipeline == nullptr)
    throw;

  return m_pHttpPipeline->Process(context, request);
}
