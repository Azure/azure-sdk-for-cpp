// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <context.hpp>

using namespace Azure::Core;
using time_point = std::chrono::system_clock::time_point;

Context& Azure::Core::GetApplicationContext()
{
  static Context ctx;
  return ctx;
}

time_point Azure::Core::Context::CancelWhen() const
{
  auto result = time_point::max();
  for (auto ptr = m_contextSharedState; ptr; ptr = ptr->Parent)
  {
    time_point cancelAt = ptr->CancelAt;
    if (result > cancelAt)
    {
      result = cancelAt;
    }
  }

  return result;
}
