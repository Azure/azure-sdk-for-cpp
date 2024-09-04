// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../src/models/private/error_impl.hpp"
#include "../src/models/private/value_impl.hpp"
#include "azure/core/amqp/internal/models/amqp_error.hpp"

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
    AmqpError error{AmqpErrorCondition::DecodeError, "test", {{"test", "test"}, {23, 299}}};
    AmqpValue value{_detail::AmqpErrorFactory::ToAmqp(error)};
    GTEST_LOG_(INFO) << value;

    AMQP_VALUE amqpValue = _detail::AmqpValueFactory::ToImplementation(value);
    ERROR_HANDLE errorHandle{};
    ASSERT_EQ(0, amqpvalue_get_error(amqpValue, &errorHandle));
    AmqpError error2 = _detail::AmqpErrorFactory::FromImplementation(errorHandle);
    const char* conditionValue;
    error_get_condition(errorHandle, &conditionValue);
    EXPECT_EQ(std::string(conditionValue), AmqpErrorCondition::DecodeError.ToString());
    error_destroy(errorHandle);
  }
}

TEST_F(TestError, ErrorInAmqpList)
{

  AmqpList test;
  _internal::AmqpError error;
  error.Condition = _internal::AmqpErrorCondition("test:error");
  error.Description = "test description";
  test.push_back(_detail::AmqpErrorFactory::ToAmqp(error));
  EXPECT_EQ(1, test.size());
  EXPECT_EQ(AmqpValueType::Composite, test[0].GetType());
  AmqpComposite testAsComposite{test[0].AsComposite()};
  EXPECT_EQ(testAsComposite.GetDescriptor(), AmqpValue{static_cast<uint64_t>(29ll)});
  {
    AmqpValue testAsValue{test.AsAmqpValue()};
    EXPECT_EQ(AmqpValueType::List, testAsValue.GetType());

    auto testAsList{testAsValue.AsList()};
    EXPECT_EQ(AmqpValueType::Composite, testAsList[0].GetType());
    EXPECT_EQ(test[0], testAsList[0]);
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
