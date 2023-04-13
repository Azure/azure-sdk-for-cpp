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
    MessageHeader header;
  }

  {
    MessageHeader header;
    EXPECT_EQ(0, header.DeliveryCount);
    EXPECT_EQ(4, header.Priority); // Not 100% sure why 4 is the default value, but...
    EXPECT_EQ(false, header.Durable);
    EXPECT_EQ(false, header.IsFirstAcquirer);
    EXPECT_FALSE(header.TimeToLive.HasValue());
  }
}

TEST_F(TestHeaders, TestTtl)
{
  MessageHeader header;
  //  EXPECT_EQ(0, header.GetTimeToLive().count());
  header.TimeToLive = std::chrono::milliseconds(100);

  HEADER_INSTANCE_TAG* handle = header;
  MessageHeader header2(handle);

  EXPECT_EQ(100, header2.TimeToLive.Value().count());

  GTEST_LOG_(INFO) << header;
}

TEST_F(TestHeaders, TestDeliveryCount)
{
  MessageHeader header;
  EXPECT_EQ(0, header.DeliveryCount);
  header.DeliveryCount = 1;

  HEADER_INSTANCE_TAG* handle = header;
  MessageHeader header2(handle);

  EXPECT_EQ(1, header2.DeliveryCount);

  GTEST_LOG_(INFO) << header;
}

TEST_F(TestHeaders, TestPriority)
{
  MessageHeader header;
  header.Priority = 1;

  HEADER_INSTANCE_TAG* handle = header;
  MessageHeader header2(handle);

  EXPECT_EQ(1, header2.Priority);
  GTEST_LOG_(INFO) << header;
}

TEST_F(TestHeaders, TestDurable)
{
  MessageHeader header;
  EXPECT_EQ(false, header.Durable);
  header.Durable = true;

  HEADER_INSTANCE_TAG* handle = header;
  MessageHeader header2(handle);

  EXPECT_EQ(true, header2.Durable);
  GTEST_LOG_(INFO) << header;
}

TEST_F(TestHeaders, TestFirstAcquirer)
{
  MessageHeader header;
  EXPECT_EQ(false, header.IsFirstAcquirer);

  header.IsFirstAcquirer = true;
  HEADER_INSTANCE_TAG* handle = header;
  MessageHeader header2(handle);

  EXPECT_EQ(true, header2.IsFirstAcquirer);
  GTEST_LOG_(INFO) << header;
}
