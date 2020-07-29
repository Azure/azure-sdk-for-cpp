// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <http/http.hpp>
#include <http/policy.hpp>

using namespace Azure::Core::Http;

#ifndef _MSC_VER
Azure::Core::Logging::LogClassification const Azure::Core::Http::LogClassification::Request;
Azure::Core::Logging::LogClassification const Azure::Core::Http::LogClassification::Response;
Azure::Core::Logging::LogClassification const Azure::Core::Http::LogClassification::Retry;
#endif

std::unique_ptr<RawResponse> NextHttpPolicy::Send(Context& ctx, Request& req)
{
  if (m_policies == nullptr)
    throw;

  if (m_index == m_policies->size() - 1)
  {
    // All the policies have run without running a transport policy
    throw;
  }

  return (*m_policies)[m_index + 1]->Send(ctx, req, NextHttpPolicy{m_index + 1, m_policies});
}
