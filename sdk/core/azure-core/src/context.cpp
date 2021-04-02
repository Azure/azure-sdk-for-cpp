// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/context.hpp"

using namespace Azure::Core;

Context& Azure::Core::Context::GetApplicationContext()
{
  static Context context;
  return context;
}

Azure::DateTime Azure::Core::Context::GetExpiration() const
{
  auto result = DateTime::max();
  for (auto ptr = m_contextSharedState; ptr; ptr = ptr->Parent)
  {
    auto cancelAt = ContextSharedState::FromDateTimeRepresentation(ptr->Expiration);
    if (result > cancelAt)
    {
      result = cancelAt;
    }
  }

  return result;
}
