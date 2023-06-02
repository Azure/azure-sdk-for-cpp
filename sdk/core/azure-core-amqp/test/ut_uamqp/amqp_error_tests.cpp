// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

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
