// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/azure_assert.hpp>

#include <random>
#include <string>
#include <vector>

#include <gtest/gtest.h>

TEST(TestAssert, AssertTest) { EXPECT_DEATH(AZURE_ASSERT(false), ".*"); }

TEST(TestAssert, NoReturnPathTest)
{
  EXPECT_DEATH(Azure::Core::_internal::AzureNoReturnPath("Test"), ".*");
}