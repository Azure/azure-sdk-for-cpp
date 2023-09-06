// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/models/amqp_error.hpp"

#include <azure_uamqp_c/amqp_definitions_fields.h>

#include <azure_uamqp_c/amqp_definitions_error.h>

#include <gtest/gtest.h>

using namespace Azure::Core::Amqp::Models::_internal;
using namespace Azure::Core::Amqp::Models;

class TestError : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestError, SimpleCreate)
{
  {
    AmqpError error;
  }

  {
    AmqpError error;
    error.Condition = AmqpErrorCondition::DecodeError;
    error.Description = "test";
    error.Info["test"] = "test";

    GTEST_LOG_(INFO) << error;
  }

  {
    AmqpError error{AmqpErrorCondition::DecodeError, "test", {}};
    AmqpValue value{AmqpErrorFactory::ToAmqp(error)};
    GTEST_LOG_(INFO) << value;

    AMQP_VALUE amqpValue = static_cast<AMQP_VALUE>(value);
    ERROR_HANDLE errorHandle;
    EXPECT_EQ(0, amqpvalue_get_error(amqpValue, &errorHandle));
    AmqpError error2 = AmqpErrorFactory::FromUamqp(errorHandle);
    error_destroy(errorHandle);
    amqpvalue_destroy(amqpValue);
  }
}

TEST_F(TestError, AmqpErrorConditions)
{
  AmqpErrorCondition condition;
  condition = AmqpErrorCondition::FrameSizeTooSmall;
  condition = AmqpErrorCondition::IllegalState;
  condition = AmqpErrorCondition::InternalError;
  condition = AmqpErrorCondition::InvalidField;
  condition = AmqpErrorCondition::NotAllowed;
  condition = AmqpErrorCondition::NotFound;
  condition = AmqpErrorCondition::NotImplemented;
  condition = AmqpErrorCondition::PreconditionFailed;
  condition = AmqpErrorCondition::ResourceDeleted;
  condition = AmqpErrorCondition::ResourceLimitExceeded;
  condition = AmqpErrorCondition::ResourceLocked;
  condition = AmqpErrorCondition::UnauthorizedAccess;
}
