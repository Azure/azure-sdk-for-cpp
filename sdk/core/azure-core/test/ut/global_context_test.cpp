// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief This test is expected to be in one binary alone because it checks the cancellation of the
 * global application context. Any other test using the global context after this test would fail.
 *
 * Do not add more tests to this file unless the tests will not use the global context.
 *
 */

#include <azure/core/context.hpp>

#include <chrono>
#include <thread>

#include <gtest/gtest.h>

using namespace Azure::Core;

// Disable deprecation warning
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

TEST(Context, ApplicationContext)
{
  Context appContext = Context::ApplicationContext;

  int value = 42;
  EXPECT_FALSE(appContext.TryGetValue(Context::Key(), value));
  EXPECT_EQ(value, 42);

  auto duration = std::chrono::milliseconds(250);
  EXPECT_FALSE(appContext.IsCancelled());
  std::this_thread::sleep_for(duration);
  EXPECT_FALSE(appContext.IsCancelled());

  appContext.Cancel();
  EXPECT_TRUE(appContext.IsCancelled());

  // AppContext2 is the same context as AppContext
  //  The context should be cancelled
  Context appContext2 = Context::ApplicationContext;
  EXPECT_TRUE(appContext2.IsCancelled());
}
#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif // _MSC_VER
