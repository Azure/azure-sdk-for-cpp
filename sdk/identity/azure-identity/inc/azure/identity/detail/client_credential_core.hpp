//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/identity/dll_import_export.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/url.hpp>

#include <string>

namespace Azure { namespace Identity { namespace _detail {
  class ClientCredentialCore final {
    Core::Url m_authorityHost;
    std::string m_tenantId;
    bool m_isAdfs;

  public:
    AZ_IDENTITY_DLLEXPORT static std::string const AadGlobalAuthority;

    explicit ClientCredentialCore(std::string tenantId, std::string const& authorityHost);

    Core::Url GetRequestUrl() const;

    std::string GetScopesString(decltype(Core::Credentials::TokenRequestContext::Scopes)
                                    const& scopes) const;
  };
}}} // namespace Azure::Identity::_detail
