// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/resource_identifier.hpp"

#include <string>

#include <gtest/gtest.h>

using namespace Azure::Core;

TEST(ResourceIdentifier, Basic)
{
  std::string resourceId = "/subscriptions/00000000-0000-0000-0000-000000000000/resourceGroups/rg/"
                           "providers/Compute/virtualMachines/vm-name";
  ResourceIdentifier resourceIdentifier(resourceId);
  EXPECT_EQ(resourceIdentifier.ToString(), resourceId);
}
