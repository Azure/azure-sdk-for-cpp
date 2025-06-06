// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/http/policies/policy.hpp>

#include <memory>

namespace Azure { namespace Storage { namespace _internal {

  class StorageRetryPolicy final : public Core::Http::Policies::_internal::RetryPolicy {
  public:
    ~StorageRetryPolicy() override {}

    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<StorageRetryPolicy>(*this);
    }

    bool ShouldRetryOnResponse(
        Core::Http::RawResponse const& response,
        Core::Http::Policies::RetryOptions const& retryOptions,
        int32_t attempt,
        std::chrono::milliseconds& retryAfter,
        double jitterFactor = -1) const override;
  };

}}} // namespace Azure::Storage::_internal
