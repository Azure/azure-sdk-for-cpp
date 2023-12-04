// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/amqp/internal/connection.hpp>

#include <azure_uamqp_c/amqp_definitions_fields.h>

#include <azure_uamqp_c/amqp_definitions_error.h>
#include <azure_uamqp_c/connection.h>

#include <gtest/gtest.h>

class TestUAMQPInsertions : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestUAMQPInsertions, TestInsertions)
{
  {
    GTEST_LOG_(INFO) << Azure::Core::Amqp::_internal::ConnectionState::Start;
    GTEST_LOG_(INFO) << Azure::Core::Amqp::_internal::ConnectionState::HeaderReceived;
    GTEST_LOG_(INFO) << Azure::Core::Amqp::_internal::ConnectionState::HeaderSent;
    GTEST_LOG_(INFO) << Azure::Core::Amqp::_internal::ConnectionState::HeaderExchanged;
    GTEST_LOG_(INFO) << Azure::Core::Amqp::_internal::ConnectionState::OpenPipe;
    GTEST_LOG_(INFO) << Azure::Core::Amqp::_internal::ConnectionState::OcPipe;
    GTEST_LOG_(INFO) << Azure::Core::Amqp::_internal::ConnectionState::OpenReceived;
    GTEST_LOG_(INFO) << Azure::Core::Amqp::_internal::ConnectionState::OpenSent;
    GTEST_LOG_(INFO) << Azure::Core::Amqp::_internal::ConnectionState::ClosePipe;
    GTEST_LOG_(INFO) << Azure::Core::Amqp::_internal::ConnectionState::CloseReceived;
    GTEST_LOG_(INFO) << Azure::Core::Amqp::_internal::ConnectionState::CloseSent;
    GTEST_LOG_(INFO) << Azure::Core::Amqp::_internal::ConnectionState::Discarding;
    GTEST_LOG_(INFO) << Azure::Core::Amqp::_internal::ConnectionState::End;
    GTEST_LOG_(INFO) << Azure::Core::Amqp::_internal::ConnectionState::Error;
  }

  {
    CONNECTION_STATE state;
    GTEST_LOG_(INFO) << CONNECTION_STATE_ERROR;
    state = static_cast<CONNECTION_STATE>(3257);
    GTEST_LOG_(INFO) << state;
  }
}
