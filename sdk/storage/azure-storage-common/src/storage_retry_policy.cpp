// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/common/internal/storage_retry_policy.hpp"

namespace Azure { namespace Storage { namespace _internal {

  bool StorageRetryPolicy::ShouldRetryOnResponse(
      Core::Http::RawResponse const& response,
      Core::Http::Policies::RetryOptions const& retryOptions,
      int32_t attempt,
      std::chrono::milliseconds& retryAfter,
      double jitterFactor) const
  {
    // Are we out of retry attempts? The base ShouldRetryOnResponse below performs the same check
    // today, but we guard explicitly here because a `false` return from the base is ambiguous
    // (retries exhausted vs. non-retryable status code). Once storage-specific retry rules are
    // added after the base call, we must not fall through to them when retries are exhausted.
    if (attempt > retryOptions.MaxRetries)
    {
      return false;
    }

    if (Core::Http::Policies::_internal::RetryPolicyBase::ShouldRetryOnResponse(
            response, retryOptions, attempt, retryAfter, jitterFactor))
    {
      return true;
    }

    // Placeholder for storage-specific retry decisions in the future.

    return false;
  }

}}} // namespace Azure::Storage::_internal
