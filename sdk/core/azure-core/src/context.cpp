// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <context.hpp>

using namespace Azure::Core;
using time_point = std::chrono::system_clock::time_point;

namespace {
static Context g_applicationContext;
}

Context& Azure::Core::GetApplicationContext() { return g_applicationContext; }

Context const& Azure::Core::GetApplicationContext() const { return g_applicationContext; }

time_point Context::CancelWhen() const
{
  auto result = time_point::max();
  for (auto ptr = m_contextSharedState; ptr; ptr = ptr->Parent)
  {
    auto cancelAt = ContextSharedState::FromMsecSinceEpoch(ptr->CancelAtMillisecondsSinceEpoch);
    if (result > cancelAt)
    {
      result = cancelAt;
    }
  }

  return result;
}
