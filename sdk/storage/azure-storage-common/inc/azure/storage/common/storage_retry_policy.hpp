// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/http/policy.hpp"

#include <memory>
#include <string>

namespace Azure { namespace Storage {

  /**
   * RetryOptions configures the retry policy's behavior.
   */
  struct StroageRetryOptions
  {
    /**
     * @brief Maximum number of attempts to retry.
     */
    int MaxRetries = 3;

    /**
     * @brief Mimimum amount of time between retry attempts.
     */
    std::chrono::milliseconds RetryDelay = std::chrono::seconds(4);

    /**
     * @brief Mimimum amount of time between retry attempts.
     */
    std::chrono::milliseconds MaxRetryDelay = std::chrono::minutes(2);

    /**
     * @brief HTTP status codes to retry on.
     */
    std::vector<Azure::Core::Http::HttpStatusCode> StatusCodes{
        Azure::Core::Http::HttpStatusCode::RequestTimeout,
        Azure::Core::Http::HttpStatusCode::InternalServerError,
        Azure::Core::Http::HttpStatusCode::BadGateway,
        Azure::Core::Http::HttpStatusCode::ServiceUnavailable,
        Azure::Core::Http::HttpStatusCode::GatewayTimeout,
    };

    /**
     * SecondaryHostForRetryReads specifies whether the retry policy should retry a read
     * operation against another host. If SecondaryHostForRetryReads is "" (the default) then
     * operations are not retried against another host. NOTE: Before setting this field, make sure
     * you understand the issues around reading stale & potentially-inconsistent data at this
     * webpage: https://docs.microsoft.com/en-us/azure/storage/common/geo-redundant-design.
     */
    std::string SecondaryHostForRetryReads;
  };

  class StorageRetryPolicy : public Azure::Core::Http::HttpPolicy {
  public:
    explicit StorageRetryPolicy(const StroageRetryOptions& options) : m_options(options) {}

    std::unique_ptr<Azure::Core::Http::HttpPolicy> Clone() const override
    {
      return std::make_unique<StorageRetryPolicy>(*this);
    }

    std::unique_ptr<Azure::Core::Http::RawResponse> Send(
        const Azure::Core::Context& ctx,
        Azure::Core::Http::Request& request,
        Azure::Core::Http::NextHttpPolicy nextHttpPolicy) const override;

  private:
    StroageRetryOptions m_options;
  };

}} // namespace Azure::Storage
