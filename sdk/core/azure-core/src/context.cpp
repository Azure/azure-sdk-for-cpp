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

inline time_point Context::ContextSharedState::GetCancelAt() const
{
  std::lock_guard<std::mutex> guard{Mutex};
  return CancelAt;
}

time_point Context::CancelWhen() const
{
  auto result = time_point::max();
  for (auto ptr = m_contextSharedState.get(); ptr; ptr = ptr->Parent.get())
  {
    const auto tmp = ptr->GetCancelAt();
    if (result > tmp)
    {
      result = tmp;
    }
  }

  return result;
}
