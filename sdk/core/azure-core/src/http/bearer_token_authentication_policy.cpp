// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/policy.hpp"

#include <chrono>

using Azure::Core::Context;
using namespace Azure::Core::Http;

std::unique_ptr<RawResponse> BearerTokenAuthenticationPolicy::Send(
    Context const& context,
    Request& request,
    NextHttpPolicy policy) const
{
  {
    std::lock_guard<std::mutex> lock(m_accessTokenMutex);

    // Refresh the token in 2 or less minutes before the actual expiration.
    if ((std::chrono::system_clock::now() - std::chrono::minutes(2)) > m_accessToken.ExpiresOn)
    {
      m_accessToken = m_credential->GetToken(context, m_tokenRequestOptions);
    }

    request.AddHeader("authorization", "Bearer " + m_accessToken.Token);
  }

  return policy.Send(context, request);
}
