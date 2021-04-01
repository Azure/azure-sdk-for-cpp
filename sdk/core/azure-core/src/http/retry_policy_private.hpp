// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/http/http.hpp"
#include "azure/core/http/policies/policy.hpp"

#include <chrono>

namespace Azure { namespace Core { namespace Http { namespace Policies { namespace _detail {

  class RetryLogic {
    RetryLogic() = delete;
    ~RetryLogic() = delete;

  public:
    static bool ShouldRetryOnTransportFailure(
        RetryOptions const& retryOptions,
        int32_t attempt,
        std::chrono::milliseconds& retryAfter,
        double jitterFactor = -1);

    static bool ShouldRetryOnResponse(
        RawResponse const& response,
        RetryOptions const& retryOptions,
        int32_t attempt,
        std::chrono::milliseconds& retryAfter,
        double jitterFactor = -1);
  };

}}}}} // namespace Azure::Core::Http::Policies::_detail
