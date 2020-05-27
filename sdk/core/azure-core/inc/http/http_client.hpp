// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "http.hpp"
#include "policy.hpp"
#include "pipeline.hpp"
#include "transport.hpp"

#include <type_traits>
#include <vector>

namespace Azure { namespace Core { namespace Http {

  class HttpClientOptions {
  public:
    std::vector<HttpPolicy> PerRequestPolicies;
    std::vector<HttpPolicy> PerRetryPolicies;
    TransportKind Transport;
  };

  class HttpClient {
  private:
    HttpPipeline* m_pHttpPipeline;
    TransportKind m_transportKind;

  public:
    HttpClient(HttpClientOptions& options);

    std::unique_ptr<Response> Send(Context& context, Request& request);
  };

}}} // namespace Azure::Core::Http
