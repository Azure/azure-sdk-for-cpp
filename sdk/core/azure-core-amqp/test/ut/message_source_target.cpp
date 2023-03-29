// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include <gtest/gtest.h>

#include <azure/core/amqp/models/message_source.hpp>
#include <azure/core/amqp/models/message_target.hpp>

class TestSourceTarget : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

using namespace Azure::Core::Amqp::Models;
using namespace Azure::Core::Amqp::Models::_internal;

TEST_F(TestSourceTarget, SimpleSourceTarget)
{
  {
    MessageSource source;
    MessageTarget target;
  }
  {
    MessageSource source(std::string("test"));
    MessageTarget target(std::string("test"));
  }

  {
    EXPECT_ANY_THROW(MessageSource source(Value{}));
    EXPECT_ANY_THROW(MessageSource source(Value::CreateArray()));
  }
  {
    EXPECT_ANY_THROW(MessageTarget target(Value{}));
    EXPECT_ANY_THROW(MessageTarget target(Value::CreateArray()));
  }
}

TEST_F(TestSourceTarget, TargetProperties)
{
  {
    MessageTarget target;
    EXPECT_ANY_THROW(target.GetAddress());
    EXPECT_EQ(TerminusDurability::None, target.GetTerminusDurability());
    EXPECT_ANY_THROW(target.GetCapabilities());
    EXPECT_EQ(TerminusExpiryPolicy::SessionEnd, target.GetExpiryPolicy());
    EXPECT_EQ(false, target.GetDynamic());
  }

  {
    MessageTarget target(std::string("test"));
    EXPECT_EQ(Value{"test"}, target.GetAddress());
  }
}