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
    // Are we out of retry attempts?
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
