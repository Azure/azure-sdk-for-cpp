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
        && !m_options.SecondaryHostForRetryReads.empty();

    std::string primaryHost = request.GetUrl().GetHost();
    const std::string& secondaryHost = m_options.SecondaryHostForRetryReads;

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

    std::unique_ptr<Azure::Core::Http::RawResponse> pResponse;
    for (int i = 0; i <= m_options.MaxRetries; ++i)
    {
      bool lastAttempt = i == m_options.MaxRetries;
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

        shouldRetry |= response
            && std::find(
                   m_options.StatusCodes.begin(),
                   m_options.StatusCodes.end(),
                   response->GetStatusCode())
                != m_options.StatusCodes.end();

        pResponse = std::move(response);

        if (!shouldRetry)
        {
          break;
        }
      }
      catch (Azure::Core::RequestFailedException const&)
      {
        if (lastAttempt)
        {
          throw;
        }
      }

      if (!lastAttempt)
      {
        if (auto bodyStream = request.GetBodyStream())
        {
          bodyStream->Rewind();
        }

        switchHost();

        ctx.ThrowIfCanceled();

        const int64_t baseRetryDelayMs = m_options.RetryDelay.count();
        const int64_t maxRetryDelayMs = m_options.MaxRetryDelay.count();
        int64_t retryDelayMs = maxRetryDelayMs;
        if (static_cast<std::size_t>(i) < sizeof(int64_t) * 8)
        {
          const int64_t factor = 1LL << i;
          retryDelayMs = baseRetryDelayMs * factor;
          if (baseRetryDelayMs != 0 && retryDelayMs / baseRetryDelayMs != factor)
          {
            retryDelayMs = maxRetryDelayMs;
          }
          else
          {
            static thread_local std::random_device rd;
            std::mt19937_64 gen(rd());
            std::uniform_real_distribution<> dist(0.8, 1.3);

            retryDelayMs = static_cast<decltype(retryDelayMs)>(retryDelayMs * dist(gen));
            if (retryDelayMs < 0 || retryDelayMs > maxRetryDelayMs)
            {
              retryDelayMs = maxRetryDelayMs;
            }
          }
        }

        if (retryDelayMs != 0)
        {
          std::this_thread::sleep_for(std::chrono::milliseconds(retryDelayMs));
        }
      }
    }

    return pResponse;
  }

}} // namespace Azure::Storage
