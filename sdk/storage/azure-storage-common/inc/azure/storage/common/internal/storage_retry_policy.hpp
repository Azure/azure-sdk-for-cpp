// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/http/policies/policy.hpp>

namespace Azure { namespace Storage { namespace _internal {

  class StorageRetryPolicy final : public Core::Http::Policies::_internal::RetryPolicy {
  public:
    explicit StorageRetryPolicy(Core::Http::Policies::RetryOptions options)
        : RetryPolicy(std::move(options))
    {
    }
    ~StorageRetryPolicy() override {}

  protected:
    bool ShouldRetryOnResponse(
        const Core::Http::RawResponse& response,
        const Core::Http::Policies::RetryOptions& retryOptions,
        int32_t attempt,
        std::chrono::milliseconds& retryAfter,
        double jitterFactor = -1) const override;
  };

}}} // namespace Azure::Storage::_internal
