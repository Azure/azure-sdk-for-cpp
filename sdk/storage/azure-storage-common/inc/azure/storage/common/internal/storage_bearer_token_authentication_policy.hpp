// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/http/policies/policy.hpp>

namespace Azure { namespace Storage { namespace _internal {

  class StorageBearerTokenAuthenticationPolicy final
      : public Core::Http::Policies::_internal::BearerTokenAuthenticationPolicy {
  public:
    /**
     * @brief Construct a Storage Bearer Token challenge authentication policy.
     *
     * @param credential An #Azure::Core::TokenCredential to use with this policy.
     * @param tokenRequestContext A context to get the token in.
     * @param enableTenantDiscovery Enables tenant discovery through the authorization challenge.
     */
    explicit StorageBearerTokenAuthenticationPolicy(
        std::shared_ptr<Azure::Core::Credentials::TokenCredential const> credential,
        Azure::Core::Credentials::TokenRequestContext tokenRequestContext,
        bool enableTenantDiscovery = false)
        : BearerTokenAuthenticationPolicy(std::move(credential), tokenRequestContext),
          m_Scopes(tokenRequestContext.Scopes), m_TenantId(tokenRequestContext.TenantId),
          m_EnableTenantDiscovery(enableTenantDiscovery)
    {
    }

    ~StorageBearerTokenAuthenticationPolicy() override {}

    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<StorageBearerTokenAuthenticationPolicy>(*this);
    }

  private:
    std::vector<std::string> m_Scopes;
    mutable std::string m_TenantId;
    bool m_EnableTenantDiscovery;

    std::unique_ptr<Azure::Core::Http::RawResponse> AuthorizeAndSendRequest(
        Azure::Core::Http::Request& request,
        Azure::Core::Http::Policies::NextHttpPolicy& nextPolicy,
        Azure::Core::Context const& context) const override;

    bool AuthorizeRequestOnChallenge(
        std::string const& challenge,
        Azure::Core::Http ::Request& request,
        Azure::Core::Context const& context) const override;
  };

}}} // namespace Azure::Storage::_internal
