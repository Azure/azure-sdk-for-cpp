// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/common/storage_retry_policy.hpp"
#include "azure/storage/common/storage_common.hpp"

#include <thread>

namespace Azure { namespace Storage {

  std::unique_ptr<Azure::Core::Http::RawResponse> StorageRetryPolicy::Send(
      const Azure::Core::Context& ctx,
      Azure::Core::Http::Request& request,
      Azure::Core::Http::NextHttpPolicy nextHttpPolicy) const
  {
    bool considerSecondary = (request.GetMethod() == Azure::Core::Http::HttpMethod::Get
                              || request.GetMethod() == Azure::Core::Http::HttpMethod::Head)
        && !m_secondaryHost.empty();

    std::string primaryHost = request.GetUrl().GetHost();
    const std::string& secondaryHost = m_secondaryHost;

    bool isUsingSecondary = false;

    auto switchHost
        = [&considerSecondary, &isUsingSecondary, &request, &primaryHost, &secondaryHost]() {
            if (considerSecondary)
            {
              isUsingSecondary = !isUsingSecondary;
            }
            else
            {
              isUsingSecondary = false;
            }
            if (isUsingSecondary)
            {
              request.GetUrl().SetHost(secondaryHost);
            }
            else
            {
              request.GetUrl().SetHost(primaryHost);
            }
          };

    for (int attempt = 1;; ++attempt)
    {
      std::chrono::milliseconds retryAfter{};
      try
      {
        auto response = nextHttpPolicy.Send(ctx, request);

        bool shouldRetry = false;

        if (isUsingSecondary)
        {
          if (response->GetStatusCode() == Azure::Core::Http::HttpStatusCode::NotFound
              || response->GetStatusCode() == Core::Http::HttpStatusCode::PreconditionFailed)
          {
            considerSecondary = false;
            // disgard this response
            shouldRetry = true;
          }
        }

        shouldRetry |= RetryPolicy::ShouldRetryOnResponse(
            *response.get(), m_retryOptions, attempt, retryAfter);

        if (!shouldRetry)
        {
          return response;
        }
      }
      catch (Azure::Core::Http::CouldNotResolveHostException const&)
      {
        if (!ShouldRetryOnTransportFailure(m_retryOptions, attempt, retryAfter))
        {
          throw;
        }
      }
      catch (Azure::Core::Http::TransportException const&)
      {
        if (!ShouldRetryOnTransportFailure(m_retryOptions, attempt, retryAfter))
        {
          throw;
        }
      }

      if (auto bodyStream = request.GetBodyStream())
      {
        bodyStream->Rewind();
      }

      switchHost();

      if (retryAfter.count() > 0)
      {
        std::this_thread::sleep_for(retryAfter);
      }

      ctx.ThrowIfCanceled();
    }
  }

}} // namespace Azure::Storage
