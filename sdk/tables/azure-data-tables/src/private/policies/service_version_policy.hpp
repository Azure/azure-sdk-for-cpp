// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/http/policies/policy.hpp>

#include <memory>
#include <string>

namespace Azure { namespace Data { namespace Tables { namespace _detail { namespace Policies {

  class ServiceVersionPolicy final : public Azure::Core::Http::Policies::HttpPolicy {
    constexpr static const char* HttpHeaderXMsVersion = "x-ms-version";

  public:
    explicit ServiceVersionPolicy(std::string apiVersion) : m_apiVersion{std::move(apiVersion)} {}

    std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy> Clone() const override
    {
      return std::make_unique<ServiceVersionPolicy>(*this);
    }

    std::unique_ptr<Azure::Core::Http::RawResponse> Send(
        Azure::Core::Http::Request& request,
        Azure::Core::Http::Policies::NextHttpPolicy nextPolicy,
        const Azure::Core::Context& context) const override
    {
      if (!m_apiVersion.empty())
      {
        request.SetHeader(HttpHeaderXMsVersion, m_apiVersion);
      }
      return nextPolicy.Send(request, context);
    }

  private:
    std::string m_apiVersion;
  };

}}}}} // namespace Azure::Data::Tables::_detail::Policies
