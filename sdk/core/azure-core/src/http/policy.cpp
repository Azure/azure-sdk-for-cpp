// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/policy.hpp"
#include "azure/core/http/http.hpp"

using namespace Azure::Core::Http;

#ifndef _MSC_VER
// Non-MSVC compilers do require allocation of statics, even if they are const constexpr.
// MSVC, on the other hand, has problem if you "redefine" static constexprs.
Azure::Core::Logging::LogClassification const Azure::Core::Http::LogClassification::Request;
Azure::Core::Logging::LogClassification const Azure::Core::Http::LogClassification::Response;
Azure::Core::Logging::LogClassification const Azure::Core::Http::LogClassification::Retry;
Azure::Core::Logging::LogClassification const
    Azure::Core::Http::LogClassification::HttpTransportAdapter;
#endif

// The NextHttpPolicy can't be created from a nullptr because it is a reference. So we don't need to
// check if m_policies is nullptr.
std::unique_ptr<RawResponse> NextHttpPolicy::Send(Context const& ctx, Request& req)
{
  if (m_index == m_policies.size() - 1)
  {
    // All the policies have run without running a transport policy
    throw std::invalid_argument("Invalid pipeline. No transport policy found. Endless policy.");
  }

  return m_policies[m_index + 1]->Send(ctx, req, NextHttpPolicy{m_index + 1, m_policies});
}
