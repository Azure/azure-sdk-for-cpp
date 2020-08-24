// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/http/policy.hpp"

namespace Azure { namespace Storage {

  class CommonHeadersRequestPolicy : public Core::Http::HttpPolicy {
  public:
    explicit CommonHeadersRequestPolicy() {}
    ~CommonHeadersRequestPolicy() override {}

    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<CommonHeadersRequestPolicy>(*this);
    }

    std::unique_ptr<Core::Http::RawResponse> Send(
        Core::Context const& ctx,
        Core::Http::Request& request,
        Core::Http::NextHttpPolicy nextHttpPolicy) const override;
  };

}} // namespace Azure::Storage
