//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/context.hpp"

using namespace Azure::Core;

Context Context::ApplicationContext;

Azure::DateTime Azure::Core::Context::GetDeadline() const
{
  // Contexts form a tree. Here, we walk from a node all the way back to the root in order to find
  // the earliest deadline value.
  auto result = DateTime::max();
  for (auto ptr = m_contextSharedState; ptr; ptr = ptr->Parent)
  {
    auto deadline = ContextSharedState::FromDateTimeRepresentation(ptr->Deadline);
    if (result > deadline)
    {
      result = deadline;
    }
  }

  return result;
}
