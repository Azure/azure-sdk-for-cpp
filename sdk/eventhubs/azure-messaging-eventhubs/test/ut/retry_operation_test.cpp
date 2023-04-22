// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <azure/core/context.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/environment.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>
#include <azure/messaging/eventhubs/retry_operation.hpp>
#include <functional>

namespace LocalTest {
bool testFunc() { return true; };
bool testNegative() { return false; };
Azure::Core::Http::Policies::RetryOptions retryOptions;
} // namespace LocalTest

TEST(RetryOperationTest, ExecuteTrue)
{
  Azure::Messaging::EventHubs::_internal::RetryOperation retryOp(LocalTest::retryOptions);
  EXPECT_TRUE(retryOp.Execute(LocalTest::testFunc));
}

TEST(RetryOperationTest, ExecuteFalse)
{
  Azure::Messaging::EventHubs::_internal::RetryOperation retryOp(LocalTest::retryOptions);
  EXPECT_FALSE(retryOp.Execute(LocalTest::testNegative));
}