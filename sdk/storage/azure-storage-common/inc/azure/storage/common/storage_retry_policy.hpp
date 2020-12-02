// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/http/policy.hpp"

#include <memory>
#include <string>

namespace Azure { namespace Storage {

  /**
   * StorageRetryWithSecondaryOptions configures whether the retry policy should retry a read
   * operation against another host.
   */
  struct StorageRetryWithSecondaryOptions : public Core::Http::RetryOptions
  {
    /**
     * SecondaryHostForRetryReads specifies whether the retry policy should retry a read
     * operation against another host. If SecondaryHostForRetryReads is "" (the default) then
     * operations are not retried against another host. NOTE: Before setting this field, make sure
     * you understand the issues around reading stale & potentially-inconsistent data at this
     * webpage: https://docs.microsoft.com/en-us/azure/storage/common/geo-redundant-design.
     */
    std::string SecondaryHostForRetryReads;
  };

  namespace Details {

    class StorageRetryPolicy : public Azure::Core::Http::HttpPolicy {
    public:
      explicit StorageRetryPolicy(const Core::Http::RetryOptions& options)
      {
        m_options.MaxRetries = options.MaxRetries;
        m_options.RetryDelay = options.RetryDelay;
        m_options.MaxRetryDelay = options.MaxRetryDelay;
        m_options.StatusCodes = options.StatusCodes;
      }

      explicit StorageRetryPolicy(const StorageRetryWithSecondaryOptions& options)
          : m_options(options)
      {
      }

      std::unique_ptr<Azure::Core::Http::HttpPolicy> Clone() const override
      {
        return std::make_unique<StorageRetryPolicy>(*this);
      }

      std::unique_ptr<Azure::Core::Http::RawResponse> Send(
          const Azure::Core::Context& ctx,
          Azure::Core::Http::Request& request,
          Azure::Core::Http::NextHttpPolicy nextHttpPolicy) const override;

    private:
      StorageRetryWithSecondaryOptions m_options;
    };

  } // namespace Details

}} // namespace Azure::Storage
