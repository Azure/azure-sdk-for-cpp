// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>

#include <azure/core/http/policies/policy.hpp>

namespace Azure { namespace Storage { namespace _internal {

  class StoragePerRetryPolicy : public Core::Http::Policies::HttpPolicy {
  public:
    ~StoragePerRetryPolicy() override {}

    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<StoragePerRetryPolicy>(*this);
    }

    std::unique_ptr<Core::Http::RawResponse> Send(
        Core::Http::Request& request,
        Core::Http::Policies::NextHttpPolicy nextHttpPolicy,
        Core::Context const& ctx) const override;
  };

}}} // namespace Azure::Storage::_internal
