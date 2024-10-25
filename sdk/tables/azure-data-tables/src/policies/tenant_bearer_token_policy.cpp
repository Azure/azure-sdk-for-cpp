// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "../private/policies/tenant_bearer_token_policy.hpp"

#include <azure/core/internal/credentials/authorization_challenge_parser.hpp>

namespace Azure { namespace Data { namespace Tables { namespace _detail { namespace Policies {

  std::unique_ptr<Azure::Core::Http::RawResponse>
  TenantBearerTokenAuthenticationPolicy::AuthorizeAndSendRequest(
      Azure::Core::Http::Request& request,
      Azure::Core::Http::Policies::NextHttpPolicy& nextPolicy,
      Azure::Core::Context const& context) const
  {
    std::string tenantId = m_safeTenantId.Get();
    if (!tenantId.empty() || !m_enableTenantDiscovery)
    {
      Azure::Core::Credentials::TokenRequestContext tokenRequestContext;
      tokenRequestContext.Scopes = m_scopes;
      tokenRequestContext.TenantId = tenantId;
      AuthenticateAndAuthorizeRequest(request, tokenRequestContext, context);
    }
    return nextPolicy.Send(request, context);
  }

  bool TenantBearerTokenAuthenticationPolicy::AuthorizeRequestOnChallenge(
      std::string const& challenge,
      Azure::Core::Http ::Request& request,
      Azure::Core::Context const& context) const
  {
    if (!m_enableTenantDiscovery)
    {
      return false;
    }
    std::string authorizationUri
        = Azure::Core::Credentials::_internal::AuthorizationChallengeParser::GetChallengeParameter(
            challenge, "Bearer", "authorization_uri");

    // tenantId should be the guid as seen in this example:
    // https://login.microsoftonline.com/72f988bf-86f1-41af-91ab-2d7cd011db47/oauth2/authorize
    std::string path = Azure::Core::Url(authorizationUri).GetPath();
    std::string tenantId = path.substr(0, path.find('/'));
    m_safeTenantId.Set(tenantId);

    Azure::Core::Credentials::TokenRequestContext tokenRequestContext;
    tokenRequestContext.Scopes = m_scopes;
    tokenRequestContext.TenantId = tenantId;
    AuthenticateAndAuthorizeRequest(request, tokenRequestContext, context);
    return true;
  }

}}}}} // namespace Azure::Data::Tables::_detail::Policies
