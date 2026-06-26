// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/http/policies/policy.hpp>

#include <chrono>
#include <cstdint>
#include <memory>

namespace Azure { namespace Storage { namespace _internal {

  class StorageRetryPolicy final : public Core::Http::Policies::_internal::RetryPolicyBase {
  public:
    explicit StorageRetryPolicy(Core::Http::Policies::RetryOptions options)
        : Core::Http::Policies::_internal::RetryPolicyBase(std::move(options))
    {
    }

    std::unique_ptr<Core::Http::Policies::HttpPolicy> Clone() const override
    {
      return std::make_unique<StorageRetryPolicy>(*this);
    }

  protected:
    bool ShouldRetryOnResponse(
        Core::Http::RawResponse const& response,
        Core::Http::Policies::RetryOptions const& retryOptions,
        int32_t attempt,
        std::chrono::milliseconds& retryAfter,
        double jitterFactor = -1) const override;
  };

}}} // namespace Azure::Storage::_internal
