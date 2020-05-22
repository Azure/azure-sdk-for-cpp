// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "http.hpp"
#include "policy.hpp"
#include "transport.hpp"

#include <type_traits>
#include <vector>

namespace Azure { namespace Core { namespace Http {

  class HttpClientOptions {
  public:
    std::vector<HttpPolicy> PerRequestPolicies;
    std::vector<HttpPolicy> PerRetryPolicies;
    std::unique_ptr<Transport> Transport;
  };

  class HttpClient {
  private:
    std::vector<HttpPolicy> m_httpPolicies;
    std::unique_ptr<Transport> m_transport;

  public:
    HttpClient(HttpClientOptions& options);

    Response Send(Context& context, Request& request);
  };

}}} // namespace Azure::Core::Http
