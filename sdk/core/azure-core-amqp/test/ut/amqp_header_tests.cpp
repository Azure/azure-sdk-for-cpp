// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include <gtest/gtest.h>

#include "azure/core/amqp/models/amqp_header.hpp"

using namespace Azure::Core::Amqp::Models;

class TestHeaders : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestHeaders, SimpleCreate)
{
  {
    Header header;
  }

  {
    Header header;
    EXPECT_EQ(0, header.GetDeliveryCount());
    EXPECT_EQ(4, header.Priority()); // Not 100% sure why 4 is the default value, but...
    EXPECT_EQ(false, header.IsDurable());
    EXPECT_EQ(false, header.IsFirstAcquirer());
    EXPECT_ANY_THROW(header.GetTimeToLive());
  }
}

TEST_F(TestHeaders, TestTtl)
{
  Header header;
  //  EXPECT_EQ(0, header.GetTimeToLive().count());
  header.SetTimeToLive(std::chrono::milliseconds(100));
  EXPECT_EQ(100, header.GetTimeToLive().count());
}

TEST_F(TestHeaders, TestDeliveryCount)
{
  Header header;
  EXPECT_EQ(0, header.GetDeliveryCount());
  header.SetDeliveryCount(1);
  EXPECT_EQ(1, header.GetDeliveryCount());
}

TEST_F(TestHeaders, TestPriority)
{
  Header header;
  EXPECT_EQ(4, header.Priority());
  header.SetPriority(1);
  EXPECT_EQ(1, header.Priority());
}

TEST_F(TestHeaders, TestDurable)
{
  Header header;
  EXPECT_EQ(false, header.IsDurable());
  header.IsDurable(true);
  EXPECT_EQ(true, header.IsDurable());
}

TEST_F(TestHeaders, TestFirstAcquirer)
{
  Header header;
  EXPECT_EQ(false, header.IsFirstAcquirer());
  header.SetFirstAcquirer(true);
  EXPECT_EQ(true, header.IsFirstAcquirer());
}
