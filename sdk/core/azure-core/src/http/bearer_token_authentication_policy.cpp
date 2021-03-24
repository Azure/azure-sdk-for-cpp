// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/policies/policy.hpp"
#include "azure/core/internal/system_clock.hpp"

#include <chrono>

using Azure::Core::Context;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;

std::unique_ptr<RawResponse> BearerTokenAuthenticationPolicy::Send(
    Request& request,
    NextHttpPolicy policy,
    Context const& context) const
{
  {
    std::lock_guard<std::mutex> lock(m_accessTokenMutex);

    // Refresh the token in 2 or less minutes before the actual expiration.
    if (Azure::Core::_internal::SystemClock::Now()
        > (m_accessToken.ExpiresOn - std::chrono::minutes(2)))
    {
      m_accessToken = m_credential->GetToken(m_tokenRequestContext, context);
    }

    request.SetHeader("authorization", "Bearer " + m_accessToken.Token);
  }

  return policy.Send(request, context);
}
