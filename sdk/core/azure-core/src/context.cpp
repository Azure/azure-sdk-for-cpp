// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/context.hpp"

using namespace Azure::Core;
using time_point = std::chrono::system_clock::time_point;

Context& Azure::Core::Context::GetApplicationContext()
{
  static Context context;
  return context;
}

time_point Azure::Core::Context::CancelWhen() const
{
  auto result = time_point::max();
  for (auto ptr = m_contextSharedState; ptr; ptr = ptr->Parent)
  {
    auto cancelAt = ContextSharedState::FromMsecSinceEpoch(ptr->CancelAtMsecSinceEpoch);
    if (result > cancelAt)
    {
      result = cancelAt;
    }
  }

  return result;
}
