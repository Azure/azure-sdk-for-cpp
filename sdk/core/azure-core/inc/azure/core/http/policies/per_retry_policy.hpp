// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/http/policies/policy.hpp>

#include <memory>

namespace Azure { namespace Core { namespace Http { namespace Policies { namespace _internal {

  class PerRetryPolicy final : public Core::Http::Policies::HttpPolicy {
    constexpr static const char* HttpHeaderDate = "date";
    constexpr static const char* HttpHeaderXMsDate = "x-ms-date";
    constexpr static const char* HttpQueryTimeout = "timeout";
    constexpr static const char* HttpHeaderClientRequestId = "x-ms-client-request-id";

  public:
    ~PerRetryPolicy() override {}

    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<PerRetryPolicy>(*this);
    }

    std::unique_ptr<Core::Http::RawResponse> Send(
        Core::Http::Request& request,
        Core::Http::Policies::NextHttpPolicy nextPolicy,
        Core::Context const& context) const override;
  };

}}}}} // namespace Azure::Core::Http::Policies::_internal
