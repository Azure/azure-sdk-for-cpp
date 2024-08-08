// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/identity/managed_identity_credential.hpp"

#include <string>

#include <gtest/gtest.h>

using namespace Azure::Identity;

TEST(ResourceIdentifier, Basic)
{
  std::string resourceId = "/subscriptions/00000000-0000-0000-0000-000000000000/resourceGroups/rg/"
                           "providers/Compute/virtualMachines/vm-name";
  ResourceIdentifier resourceIdentifier(resourceId);
  EXPECT_EQ(resourceIdentifier.ToString(), resourceId);
}
