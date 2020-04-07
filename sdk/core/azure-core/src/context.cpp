// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <context.hpp>

using namespace Azure::Core;
using time_point = std::chrono::system_clock::time_point;

Context& GetApplicationContext()
{
  static Context ctx;
  return ctx;
}

time_point Context::CancelWhen()
{
  auto result = time_point::max();
  for (auto ptr = m_contextSharedState; ptr; ptr = ptr->Parent)
  {
    if (result > ptr->CancelAt)
    {
      result = ptr->CancelAt;
    }
  }

  return result;
}


