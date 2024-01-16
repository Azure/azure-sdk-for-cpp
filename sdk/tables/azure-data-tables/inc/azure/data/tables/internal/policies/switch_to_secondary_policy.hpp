// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/data/tables/dll_import_export.hpp"

#include <azure/core/http/policies/policy.hpp>

#include <memory>
#include <string>

namespace Azure { namespace Data { namespace Tables { namespace _detail { namespace Policies {

  AZ_DATA_TABLES_DLLEXPORT extern const Azure::Core::Context::Key SecondaryHostReplicaStatus;

  inline Azure::Core::Context WithReplicaStatus(const Azure::Core::Context& context)
  {
    return context.WithValue(SecondaryHostReplicaStatus, std::make_shared<bool>(true));
  }

  class SwitchToSecondaryPolicy final : public Azure::Core::Http::Policies::HttpPolicy {
  public:
    explicit SwitchToSecondaryPolicy(std::string primaryHost, std::string secondaryHost)
        : m_primaryHost{std::move(primaryHost)}, m_secondaryHost{std::move(secondaryHost)}
    {
    }

    std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy> Clone() const override
    {
      return std::make_unique<SwitchToSecondaryPolicy>(*this);
    }

    std::unique_ptr<Azure::Core::Http::RawResponse> Send(
        Azure::Core::Http::Request& request,
        Azure::Core::Http::Policies::NextHttpPolicy nextPolicy,
        const Azure::Core::Context& context) const override;

  private:
    std::string m_primaryHost;
    std::string m_secondaryHost;
  };

}}}}} // namespace Azure::Data::Tables::_detail::Policies
