// ************ CUSTOMER CODE **********************

#include <chrono>
using namespace std::literals;

#include "gtest/gtest.h"

#include "operation_test.hpp"

#include <azure/core/operation.hpp>

using namespace Azure::Core::Test;

TEST(Operation, Basic)
{
  Context context;
  StringClient client;
  auto operation = client.StartStringUpdate();
  EXPECT_FALSE(operation.Done());
  while(!operation.Done())
  {
    auto response = operation.Poll(context);
  }
  EXPECT_TRUE(operation.Done());
}
