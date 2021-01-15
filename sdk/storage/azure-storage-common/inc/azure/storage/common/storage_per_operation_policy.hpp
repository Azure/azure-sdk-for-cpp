// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>

#include <azure/core/http/policy.hpp>

namespace Azure { namespace Storage { namespace Details {

  class StoragePerOperationPolicy : public Core::Http::HttpPolicy {
  public:
    explicit StoragePerOperationPolicy(std::string apiVersion) : m_apiVersion(std::move(apiVersion))
    {
    }
    ~StoragePerOperationPolicy() override {}

    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<StoragePerOperationPolicy>(*this);
    }

    std::unique_ptr<Core::Http::RawResponse> Send(
        Core::Context const& ctx,
        Core::Http::Request& request,
        Core::Http::NextHttpPolicy nextHttpPolicy) const override;

  private:
    std::string m_apiVersion;
  };

}}} // namespace Azure::Storage::Details
