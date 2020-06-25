// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <credentials/policy/policies.hpp>

using namespace Azure::Core::Credentials::Policy;

std::unique_ptr<Http::Response> BearerTokenAuthenticationPolicy::Send(
    Context& context,
    Http::Request& request,
    Http::NextHttpPolicy policy) const override
{
  if (std::chrono::system_clock::now() > m_accessToken.ExpiresOn)
  {
    m_accessToken = m_credential->GetToken(context, m_scopes);
  }

  request.AddHeader("authorization", "Bearer " + m_accessToken.Token);
}
