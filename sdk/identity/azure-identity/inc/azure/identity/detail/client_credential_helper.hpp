// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/identity/dll_import_export.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/url.hpp>

#include <string>

namespace Azure { namespace Identity { namespace _detail {
  AZ_IDENTITY_DLLEXPORT extern std::string const g_aadGlobalAuthority;

  class ClientCredentialHelper final {
    Core::Url const m_authorityHost;
    std::string const m_tenantId;
    bool m_disableTenantDiscovery;

  public:
    bool IsAdfs;

    ClientCredentialHelper(
        std::string tenantId,
        std::string const& authorityHost,
        bool disableTenantDiscovery);

    Core::Url GetRequestUrl(
        Core::Credentials::TokenRequestContext const& tokenRequestContext) const;

    static bool IsTenantDiscoveryDisabledByDefault();
  };
}}} // namespace Azure::Identity::_detail
