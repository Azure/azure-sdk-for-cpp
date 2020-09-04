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
  struct StroageRetryOptions : public Azure::Core::Http::RetryOptions
  {
    /**
     * RetryReadsFromSecondaryHost specifies whether the retry policy should retry a read
     * operation against another host. If RetryReadsFromSecondaryHost is "" (the default) then
     * operations are not retried against another host. NOTE: Before setting this field, make sure
     * you understand the issues around reading stale & potentially-inconsistent data at this
     * webpage: https://docs.microsoft.com/en-us/azure/storage/common/geo-redundant-design.
     */
    std::string RetryReadsFromSecondaryHost;
  };

  class StorageRetryPolicy : public Azure::Core::Http::RetryPolicy {
  public:
    explicit StorageRetryPolicy(const StroageRetryOptions& options)
        : RetryPolicy(options), m_secondaryHost(options.RetryReadsFromSecondaryHost)
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
    std::string m_secondaryHost;
  };

}} // namespace Azure::Storage
