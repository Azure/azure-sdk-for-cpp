// Copyright (c) Microsoft Corporation. All rights reserved.
// An SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>

#include <azure/core/http/policies/policy.hpp>

#include "storage_common.hpp"

namespace Azure { namespace Storage { namespace _internal {

  class StorageServiceVersionPolicy : public Azure::Core::Http::Policies::HttpPolicy {
  public:
    explicit StorageServiceVersionPolicy(std::string apiVersion)
        : m_apiVersion(std::move(apiVersion))
    {
    }

    std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy> Clone() const override
    {
      return std::make_unique<StorageServiceVersionPolicy>(*this);
    }

    std::unique_ptr<Azure::Core::Http::RawResponse> Send(
        Azure::Core::Http::Request& request,
        Azure::Core::Http::Policies::NextHttpPolicy nextHttpPolicy,
        const Azure::Core::Context& ctx) const override
    {
      request.SetHeader(HttpHeaderXMsVersion, m_apiVersion);
      return nextHttpPolicy.Send(request, ctx);
    }

  private:
    std::string m_apiVersion;
  };

}}} // namespace Azure::Storage::_internal
