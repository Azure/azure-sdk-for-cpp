// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/http/policies/policy.hpp>

#include <memory>

namespace Azure { namespace Data { namespace Tables { namespace _detail { namespace Policies {

  class TimeoutPolicy final : public Core::Http::Policies::HttpPolicy {
    constexpr static const char* HttpHeaderDate = "date";
    constexpr static const char* HttpHeaderXMsDate = "x-ms-date";
    constexpr static const char* HttpQueryTimeout = "timeout";

  public:
    ~TimeoutPolicy() override {}

    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<TimeoutPolicy>(*this);
    }

    std::unique_ptr<Core::Http::RawResponse> Send(
        Core::Http::Request& request,
        Core::Http::Policies::NextHttpPolicy nextPolicy,
        Core::Context const& context) const override;
  };

}}}}} // namespace Azure::Data::Tables::_detail::Policies
