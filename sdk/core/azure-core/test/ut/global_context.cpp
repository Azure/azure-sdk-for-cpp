// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief This test is expected to be in one binary alone because it checks the cancellation of the
 * global application context. Any other test using the global context after this test would fail.
 *
 * Do not add more tests to this file unless the tests will not use the global context.
 *
 */

#include <gtest/gtest.h>

#include <azure/core/context.hpp>

#include <chrono>
#include <string>
#include <thread>

using namespace Azure::Core;

namespace {
extern char const KeyId[] = "key";
Context::Key const Key = _internal::ContextKey::Create<KeyId>();
} // namespace

TEST(Context, ApplicationContext)
{
  Context appContext = Context::GetApplicationContext();

  EXPECT_FALSE(appContext.HasKey(Key));

  auto duration = std::chrono::milliseconds(250);
  EXPECT_FALSE(appContext.IsCancelled());
  std::this_thread::sleep_for(duration);
  EXPECT_FALSE(appContext.IsCancelled());

  appContext.Cancel();
  EXPECT_TRUE(appContext.IsCancelled());

  // AppContext2 is the same context as AppContext
  //  The context should be cancelled
  Context appContext2 = Context::GetApplicationContext();
  EXPECT_TRUE(appContext2.IsCancelled());
}
