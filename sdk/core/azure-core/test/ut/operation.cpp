// ************ CUSTOMER CODE **********************

#include "gtest/gtest.h"

#include "operation_test.hpp"

#include <azure/core/context.hpp>
#include <azure/core/operation.hpp>

#include <chrono>

using namespace Azure::Core::Test;
using namespace std::literals;

TEST(Operation, Poll)
{
  StringClient client;
  auto operation = client.StartStringUpdate();

  EXPECT_FALSE(operation.Done());
  EXPECT_FALSE(operation.HasValue());

  while(!operation.Done())
  {
    EXPECT_FALSE(operation.HasValue());
    auto response = operation.Poll();
  }

  EXPECT_TRUE(operation.Done());
  EXPECT_TRUE(operation.HasValue());

  auto result = operation.Value();
  EXPECT_TRUE(result == "StringOperation-Completed");
}

TEST(Operation, PollUntilDone)
{
  StringClient client;
  auto operation = client.StartStringUpdate();

  EXPECT_FALSE(operation.Done());
  EXPECT_FALSE(operation.HasValue());
  
  auto start = std::chrono::high_resolution_clock::now();
  auto response = operation.PollUntilDone(500ms);
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> elapsed = end - start;
  //StringOperation test code is implemented to poll 2 times
  EXPECT_TRUE(elapsed >= 1s);

  EXPECT_TRUE(operation.Done());
  EXPECT_TRUE(operation.HasValue());

  auto result = operation.Value();
  EXPECT_TRUE(result == "StringOperation-Completed");
}
