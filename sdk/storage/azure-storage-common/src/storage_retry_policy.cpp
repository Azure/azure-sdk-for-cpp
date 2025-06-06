// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/common/internal/storage_retry_policy.hpp"

#include "azure/core/internal/diagnostics/log.hpp"

namespace Azure { namespace Storage { namespace _internal {

  bool StorageRetryPolicy::ShouldRetryOnResponse(
      Core::Http::RawResponse const& response,
      Core::Http::Policies::RetryOptions const& retryOptions,
      int32_t attempt,
      std::chrono::milliseconds& retryAfter,
      double jitterFactor = -1) const
  {
    using Azure::Core::Diagnostics::Logger;
    using Azure::Core::Diagnostics::_internal::Log;

    // Are we out of retry attempts?
    if (WasLastAttempt(retryOptions, attempt))
    {
      return false;
    }

    bool shouldRetry = false;
    auto const& statusCodes = retryOptions.StatusCodes;
    auto const sc = response.GetStatusCode();
    // Should we retry on the given response retry code?
    {
      if (statusCodes.find(sc) == statusCodes.end())
      {
        if (Log::ShouldWrite(Logger::Level::Informational))
        {
          Log::Write(
              Logger::Level::Informational,
              std::string("HTTP status code ") + std::to_string(static_cast<int>(sc))
                  + " won't be retried.");
        }
      }
      else if (Log::ShouldWrite(Logger::Level::Informational))
      {
        Log::Write(
            Logger::Level::Informational,
            std::string("HTTP status code ") + std::to_string(static_cast<int>(sc))
                + " will be retried.");
        shouldRetry = true;
      }
    }

    // SHould we retry on copy source error
    if (!shouldRetry)
    {
      static const std::unordered_set<std::string> retriableErrorCodes
          = {"InternalError", "OperationTimedOut", "ServerBusy"};
      auto& headers = response.GetHeaders();
      auto copySourceError = headers.find("x-ms-copy-source-error-code");
      if (copySourceError != headers.end()
          && retriableErrorCodes.count(copySourceError->second) != 0)
      {
        Log::Write(
            Logger::Level::Informational,
            std::string("HTTP status code ") + std::to_string(static_cast<int>(sc))
                + "with CopySourceError" + copySourceError->second + " will be retried.");
        shouldRetry = true;
      }
    }

    if (shouldRetry && !GetResponseHeaderBasedDelay(response, retryAfter))
    {
      retryAfter = CalculateExponentialDelay(retryOptions, attempt, jitterFactor);
    }

    return shouldRetry;
  }

}}} // namespace Azure::Storage::_internal
