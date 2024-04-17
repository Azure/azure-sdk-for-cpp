// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/common/internal/storage_retry_policy.hpp"

namespace Azure { namespace Storage { namespace _internal {

  bool StorageRetryPolicy::ShouldRetryOnResponse(
      const Core::Http::RawResponse& response,
      const Core::Http::Policies::RetryOptions& retryOptions,
      int32_t attempt,
      std::chrono::milliseconds& retryAfter,
      double jitterFactor) const
  {
    bool ret = RetryPolicy::ShouldRetryOnResponse(
        response, retryOptions, attempt, retryAfter, jitterFactor);
    if (ret)
    {
      return true;
    }

    if (attempt > retryOptions.MaxRetries)
    {
      return false;
    }

    if (static_cast<std::underlying_type_t<Core::Http::HttpStatusCode>>(response.GetStatusCode())
        >= 400)
    {
      const auto& headers = response.GetHeaders();
      auto ite = headers.find("x-ms-copy-source-status-code");
      if (ite != headers.end())
      {
        auto innerStatusCode = static_cast<Core::Http::HttpStatusCode>(std::stoi(ite->second));
        if (retryOptions.StatusCodes.find(innerStatusCode) != retryOptions.StatusCodes.end())
        {
          return true;
        }
      }
    }
    return false;
  }

}}} // namespace Azure::Storage::_internal
