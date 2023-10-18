// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/identity/dll_import_export.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/url.hpp>

#include <string>
#include <vector>

namespace Azure { namespace Identity { namespace _detail {
  constexpr auto AzureAuthorityHostEnvVarName = "AZURE_AUTHORITY_HOST";

  class ClientCredentialCore final {
    std::vector<std::string> m_additionallyAllowedTenants;
    Core::Url m_authorityHost;
    std::string m_tenantId;

  public:
    explicit ClientCredentialCore(
        std::string tenantId,
        std::string const& authorityHost,
        std::vector<std::string> additionallyAllowedTenants);

    Core::Url GetRequestUrl(std::string const& tenantId) const;

    std::string GetScopesString(
        std::string const& tenantId,
        decltype(Core::Credentials::TokenRequestContext::Scopes) const& scopes) const;

    std::string const& GetTenantId() const { return m_tenantId; }

    std::vector<std::string> const& GetAdditionallyAllowedTenants() const
    {
      return m_additionallyAllowedTenants;
    }
  };
}}} // namespace Azure::Identity::_detail
