// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "http/policy.hpp"

namespace Azure { namespace Storage {

  class CommonHeadersRequestPolicy : public Core::Http::HttpPolicy {
  public:
    explicit CommonHeadersRequestPolicy() {}
    ~CommonHeadersRequestPolicy() override {}

    HttpPolicy* Clone() const override { return new CommonHeadersRequestPolicy(*this); }

    std::unique_ptr<Core::Http::Response> Send(
        Core::Context& ctx,
        Core::Http::Request& request,
        Core::Http::NextHttpPolicy nextHttpPolicy) const override;
  };

}} // namespace Azure::Storage
