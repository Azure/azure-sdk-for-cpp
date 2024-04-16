// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/http/policies/policy.hpp>

#include <memory>

namespace Azure { namespace Storage { namespace _internal {

  class StorageRetryPolicy final : public Core::Http::Policies::_internal::RetryPolicy {
  public:
    explicit StorageRetryPolicy(Core::Http::Policies::RetryOptions options)
        : RetryPolicy(options), m_options(std::move(options))
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

  private:
    Core::Http::Policies::RetryOptions m_options;
  };

}}} // namespace Azure::Storage::_internal
