// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/common/internal/storage_bearer_token_authentication_policy.hpp"
#include "azure/storage/common/internal/constants.hpp"

#include <azure/core/internal/credentials/authorization_challenge_parser.hpp>

namespace Azure { namespace Storage { namespace _internal {

  std::unique_ptr<Azure::Core::Http::RawResponse>
  StorageBearerTokenAuthenticationPolicy::AuthorizeAndSendRequest(
      Azure::Core::Http::Request& request,
      Azure::Core::Http::Policies::NextHttpPolicy& nextPolicy,
      Azure::Core::Context const& context) const
  {
    if (!m_TenantId.empty() || !m_EnableTenantDiscovery)
    {
      Azure::Core::Credentials::TokenRequestContext tokenRequestContext;
      tokenRequestContext.Scopes = m_Scopes;
      tokenRequestContext.TenantId = m_TenantId;
      AuthenticateAndAuthorizeRequest(request, tokenRequestContext, context);
    }
    return nextPolicy.Send(request, context);
  }

  bool StorageBearerTokenAuthenticationPolicy::AuthorizeRequestOnChallenge(
      std::string const& challenge,
      Azure::Core::Http ::Request& request,
      Azure::Core::Context const& context) const
  {
    std::string authorizationUri
        = Azure::Core::Credentials::_internal::AuthorizationChallengeParser::GetChallengeParameter(
            challenge, "Bearer", "authorization_uri");

    // tenantId should be the guid as seen in this example:
    // https://login.microsoftonline.com/72f988bf-86f1-41af-91ab-2d7cd011db47/oauth2/authorize
    std::string path = Azure::Core::Url(authorizationUri).GetPath();
    m_TenantId = path.substr(0, path.find('/'));

    Azure::Core::Credentials::TokenRequestContext tokenRequestContext;
    tokenRequestContext.Scopes = m_Scopes;
    tokenRequestContext.TenantId = m_TenantId;
    AuthenticateAndAuthorizeRequest(request, tokenRequestContext, context);
    return true;
  }

}}} // namespace Azure::Storage::_internal
