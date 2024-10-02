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

class ValidValues : public ::testing::TestWithParam<std::string> {
public:
  std::string GetValidResourceId() { return GetParam(); }
};

TEST_P(ValidValues, )
{
  ResourceIdentifier resourceIdentifier(GetValidResourceId());
  EXPECT_EQ(resourceIdentifier.ToString(), GetValidResourceId());
}

INSTANTIATE_TEST_SUITE_P(
    ResourceIdentifier,
    ValidValues,
    testing::Values(
        "/subscriptions/0c2f6471-1bf0-4dda-aec3-cb9272f09575/resourceGroups/myRg/providers/"
        "Microsoft.Compute/virtualMachines/myVm",
        "/subscriptions/0c2f6471-1bf0-4dda-aec3-cb9272f09575/resourceGroups/myRg/providers/"
        "Microsoft.Network/virtualNetworks/myNet/subnets/mySubnet",
        "/subscriptions/0c2f6471-1bf0-4dda-aec3-cb9272f09575/resourceGroups/myRg",
        "/subscriptions/0c2f6471-1bf0-4dda-aec3-cb9272f09575/locations/MyLocation",
        "/subscriptions/0c2f6471-1bf0-4dda-aec3-cb9272f09575",
        "/providers/Microsoft.Billing/billingAccounts/"
        "3984c6f4-2d2a-4b04-93ce-43cf4824b698%3Ae2f1492a-a492-468d-909f-bf7fe6662c01_2019-05-31",
        "/subscriptions/17fecd63-33d8-4e43-ac6f-0aafa111b38d/locations/westus2",
        "/subscriptions/db1ab6f0-4769-4b27-930e-01e2ef9c123c/"
        "providers/Microsoft.Compute/locations/westus2"));

TEST(ResourceIdentifier, Invalid)
{
  // empty
  EXPECT_THROW(ResourceIdentifier(""), std::invalid_argument);

  // invalid tenant
  EXPECT_THROW(ResourceIdentifier("/MicrosoftSomething/billingAccounts/"), std::invalid_argument);
  EXPECT_THROW(
      ResourceIdentifier("providers/subscription/MicrosoftSomething/billingAccounts/"),
      std::invalid_argument);
  EXPECT_THROW(ResourceIdentifier("/providers"), std::invalid_argument);

  // invalid input
  EXPECT_THROW(ResourceIdentifier(" "), std::invalid_argument);
  EXPECT_THROW(ResourceIdentifier("asdfghj"), std::invalid_argument); // cspell:disable-line
  EXPECT_THROW(ResourceIdentifier("123456"), std::invalid_argument);
  EXPECT_THROW(ResourceIdentifier("!@#$%^&*/"), std::invalid_argument);
  EXPECT_THROW(
      ResourceIdentifier("/0c2f6471-1bf0-4dda-aec3-cb9272f09575/myRg/"), std::invalid_argument);

  // too few elements
  EXPECT_THROW(ResourceIdentifier("UnformattedString"), std::invalid_argument);

  // no known parts
  EXPECT_THROW(ResourceIdentifier("/subs/sub1/rgs/rg1/"), std::invalid_argument);
}
