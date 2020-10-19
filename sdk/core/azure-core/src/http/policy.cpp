// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/policy.hpp"
#include "azure/core/http/http.hpp"

#include <stdexcept>

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

std::unique_ptr<RawResponse> NextHttpPolicy::Send(Context const& ctx, Request& req)
{
  if (m_policies == nullptr)
    throw std::runtime_error("No policies in the pipeline");

  if (LastPolicy())
  {
    // All the policies have run without running a transport policy
    throw std::runtime_error("Missing transport policy in the pipeline");
  }

  return (*m_policies)[m_index + 1]->Send(ctx, req, NextHttpPolicy{m_index + 1, m_policies});
}
