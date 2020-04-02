// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <context.hpp>

using namespace Azure::Core;
using time_point = std::chrono::system_clock::time_point;

Context& get_application_context()
{
  static Context ctx;
  return ctx;
}

time_point Context::CancelWhen()
{
  auto result = time_point::max();
  for (auto ptr = impl_; ptr; ptr = ptr->parent)
  {
    if (result > ptr->cancelAt)
    {
      result = ptr->cancelAt;
    }
  }

  return result;
}


