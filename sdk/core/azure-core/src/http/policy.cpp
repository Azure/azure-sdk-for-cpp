// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/policies/policy.hpp"
#include "azure/core/http/http.hpp"

#include <algorithm>

using Azure::Core::Context;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;

// The NextHttpPolicy can't be created from a nullptr because it is a reference. So we don't need to
// check if m_policies is nullptr.
std::unique_ptr<RawResponse> NextHttpPolicy::Send(Request& request, Context const& context)
{
  if (m_index == m_policies.size() - 1)
  {
    // All the policies have run without running a transport policy
    throw std::invalid_argument("Invalid pipeline. No transport policy found. Endless policy.");
  }

  return m_policies[m_index + 1]->Send(request, NextHttpPolicy{m_index + 1, m_policies}, context);
}

std::vector<std::string> Policies::_internal::TokenScopes::GetScopeFromUrl(
    Azure::Core::Url const& url,
    std::string const& defaultScope)
{
  std::vector<std::string> scopes;

  std::string calculatedScope(url.GetScheme() + "://");
  auto const& hostWithAccount = url.GetHost();
  auto hostNoAccountStart = std::find(hostWithAccount.begin(), hostWithAccount.end(), '.');

  // Insert the calculated scope only when then host in the url contains at least a `.`
  // Otherwise, only the default scope will be there.
  // We don't want to throw/validate input but just leave the values go to azure to decide what to
  // do.
  if (hostNoAccountStart != hostWithAccount.end())
  {
    std::string hostNoAccount(hostNoAccountStart + 1, hostWithAccount.end());

    calculatedScope.append(hostNoAccount);
    calculatedScope.append("/.default");

    scopes.emplace_back(calculatedScope);
  }

  if (!defaultScope.empty() && defaultScope != calculatedScope)
  {
    scopes.emplace_back(defaultScope);
  }

  return scopes;
}
