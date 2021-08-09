// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/policies/policy.hpp"

#include <chrono>

using Azure::Core::Context;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;

std::unique_ptr<RawResponse> BearerTokenAuthenticationPolicy::Send(
    Request& request,
    NextHttpPolicy nextPolicy,
    Context const& context) const
{
  {
    std::lock_guard<std::mutex> lock(m_accessTokenMutex);

    // Refresh the token in 2 or less minutes before the actual expiration.
    if (std::chrono::system_clock::now() > (m_accessToken.ExpiresOn - std::chrono::minutes(2)))
    {
      m_accessToken = m_credential->GetToken(m_tokenRequestContext, context);
    }

    request.SetHeader("authorization", "Bearer " + m_accessToken.Token);
  }

  return nextPolicy.Send(request, context);
}
