// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../src/models/private/header_impl.hpp"
#include "azure/core/amqp/models/amqp_header.hpp"

#include <gtest/gtest.h>

using namespace Azure::Core::Amqp::Models;

class TestHeadersUamqp : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestHeadersUamqp, SimpleCreate)
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

TEST_F(TestHeadersUamqp, TestTtl)
{
  MessageHeader header;
  header.TimeToLive = std::chrono::milliseconds(100);

  auto handle = _detail::MessageHeaderFactory::ToImplementation(header);
  MessageHeader header2(_detail::MessageHeaderFactory::FromImplementation(handle));

  EXPECT_EQ(100, header2.TimeToLive.Value().count());

  GTEST_LOG_(INFO) << header;
}

TEST_F(TestHeadersUamqp, TestDeliveryCount)
{
  MessageHeader header;
  EXPECT_EQ(0, header.DeliveryCount);
  header.DeliveryCount = 1;

  auto handle = _detail::MessageHeaderFactory::ToImplementation(header);
  MessageHeader header2(_detail::MessageHeaderFactory::FromImplementation(handle));

  EXPECT_EQ(1, header2.DeliveryCount);

  GTEST_LOG_(INFO) << header;
}

TEST_F(TestHeadersUamqp, TestPriority)
{
  MessageHeader header;
  header.Priority = 1;

  auto handle = _detail::MessageHeaderFactory::ToImplementation(header);
  MessageHeader header2(_detail::MessageHeaderFactory::FromImplementation(handle));

  EXPECT_EQ(1, header2.Priority);
  GTEST_LOG_(INFO) << header;
}

TEST_F(TestHeadersUamqp, TestDurable)
{
  MessageHeader header;
  EXPECT_EQ(false, header.Durable);
  header.Durable = true;

  auto handle = _detail::MessageHeaderFactory::ToImplementation(header);
  MessageHeader header2(_detail::MessageHeaderFactory::FromImplementation(handle));

  EXPECT_EQ(true, header2.Durable);
  GTEST_LOG_(INFO) << header;
}

TEST_F(TestHeadersUamqp, TestFirstAcquirer)
{
  MessageHeader header;
  EXPECT_EQ(false, header.IsFirstAcquirer);

  header.IsFirstAcquirer = true;
  auto handle = _detail::MessageHeaderFactory::ToImplementation(header);
  MessageHeader header2(_detail::MessageHeaderFactory::FromImplementation(handle));

  EXPECT_EQ(true, header2.IsFirstAcquirer);
  GTEST_LOG_(INFO) << header;
}
