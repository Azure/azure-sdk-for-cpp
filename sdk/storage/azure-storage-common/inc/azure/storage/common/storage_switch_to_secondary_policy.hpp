// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>

#include <azure/core/http/policy.hpp>

namespace Azure { namespace Storage { namespace Details {

  class StorageSwitchToSecondaryPolicy : public Azure::Core::Http::HttpPolicy {
  public:
    explicit StorageSwitchToSecondaryPolicy(std::string primaryHost, std::string secondaryHost)
        : m_primaryHost(std::move(primaryHost)), m_secondaryHost(std::move(secondaryHost))
    {
    }

    std::unique_ptr<Azure::Core::Http::HttpPolicy> Clone() const override
    {
      return std::make_unique<StorageSwitchToSecondaryPolicy>(*this);
    }

    std::unique_ptr<Azure::Core::Http::RawResponse> Send(
        Azure::Core::Http::Request& request,
        Azure::Core::Http::NextHttpPolicy nextHttpPolicy,
        const Azure::Core::Context& ctx) const override;

  private:
    std::string m_primaryHost;
    std::string m_secondaryHost;
  };

}}} // namespace Azure::Storage::Details
