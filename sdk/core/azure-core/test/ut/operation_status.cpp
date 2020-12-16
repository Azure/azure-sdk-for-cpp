// ************ CUSTOMER CODE **********************

#include "gtest/gtest.h"

#include <azure/core/operation_status.hpp>

using namespace Azure::Core;

TEST(OperationStatus, Basic)
{
  OperationStatus status = OperationStatus::Cancelled;
  EXPECT_TRUE(status == OperationStatus::Cancelled);
  EXPECT_TRUE(status.Get() == "Cancelled");
  EXPECT_FALSE(status != OperationStatus::Cancelled);

  status = OperationStatus::Failed;
  EXPECT_TRUE(status == OperationStatus::Failed);
  EXPECT_TRUE(status.Get() == "Failed");
  EXPECT_FALSE(status != OperationStatus::Failed);

  status = OperationStatus::NotStarted;
  EXPECT_TRUE(status == OperationStatus::NotStarted);
  EXPECT_TRUE(status.Get() == "NotStarted");
  EXPECT_FALSE(status != OperationStatus::NotStarted);

  status = OperationStatus::Running;
  EXPECT_TRUE(status == OperationStatus::Running);
  EXPECT_TRUE(status.Get() == "Running");
  EXPECT_FALSE(status != OperationStatus::Running);

  status = OperationStatus::Succeeded;
  EXPECT_TRUE(status == OperationStatus::Succeeded);
  EXPECT_TRUE(status.Get() == "Succeeded");
  EXPECT_FALSE(status != OperationStatus::Succeeded);
}

TEST(OperationStatus, Custom)
{
  OperationStatus status2("CustomValue");
  EXPECT_TRUE(status2.Get() == "CustomValue");
  EXPECT_TRUE(status2 != OperationStatus::NotStarted);

  OperationStatus status3 = OperationStatus("CustomValue");
  EXPECT_TRUE(status3.Get() == "CustomValue");
  EXPECT_TRUE(status3 != OperationStatus::NotStarted);

  std::string custom("CustomValue");
  OperationStatus status4 = OperationStatus(custom);
  EXPECT_TRUE(status4.Get() == custom);
  EXPECT_TRUE(status4 != OperationStatus::NotStarted);

  OperationStatus status5 = OperationStatus(std::string("CustomValue"));
  EXPECT_TRUE(status5.Get() == "CustomValue");
  EXPECT_TRUE(status5 != OperationStatus::NotStarted);
}
