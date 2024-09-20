// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../../src/impl/uamqp/amqp/private/connection_impl.hpp"

#include <azure/core/amqp/internal/connection.hpp>

#include <azure_uamqp_c/amqp_definitions_fields.h>

#include <azure_uamqp_c/amqp_definitions_error.h>

#include <azure_uamqp_c/connection.h>

#include <gtest/gtest.h>

namespace Azure { namespace Core { namespace Amqp { namespace _detail { namespace Tests {

  class TestUAMQPInsertions : public testing::Test {
  protected:
    void SetUp() override {}
    void TearDown() override {}
  };

#define TEST_OSTREAM_INSERTER(ENUMERATION, ENUMERATOR) \
  { \
    std::stringstream ss; \
    ss << ENUMERATION::ENUMERATOR; \
    EXPECT_EQ(#ENUMERATOR, ss.str()); \
  }
  TEST_F(TestUAMQPInsertions, TestInsertions)
  {
    {
      TEST_OSTREAM_INSERTER(Azure::Core::Amqp::_internal::ConnectionState, Start);
      TEST_OSTREAM_INSERTER(Azure::Core::Amqp::_internal::ConnectionState, HeaderReceived);
      TEST_OSTREAM_INSERTER(Azure::Core::Amqp::_internal::ConnectionState, HeaderSent);
      TEST_OSTREAM_INSERTER(Azure::Core::Amqp::_internal::ConnectionState, HeaderExchanged);
      TEST_OSTREAM_INSERTER(Azure::Core::Amqp::_internal::ConnectionState, OpenPipe);
      TEST_OSTREAM_INSERTER(Azure::Core::Amqp::_internal::ConnectionState, OcPipe);
      TEST_OSTREAM_INSERTER(Azure::Core::Amqp::_internal::ConnectionState, OpenReceived);
      TEST_OSTREAM_INSERTER(Azure::Core::Amqp::_internal::ConnectionState, OpenSent);
      TEST_OSTREAM_INSERTER(Azure::Core::Amqp::_internal::ConnectionState, ClosePipe);
      TEST_OSTREAM_INSERTER(Azure::Core::Amqp::_internal::ConnectionState, CloseReceived);
      TEST_OSTREAM_INSERTER(Azure::Core::Amqp::_internal::ConnectionState, CloseSent);
      TEST_OSTREAM_INSERTER(Azure::Core::Amqp::_internal::ConnectionState, Discarding);
      TEST_OSTREAM_INSERTER(Azure::Core::Amqp::_internal::ConnectionState, End);
      TEST_OSTREAM_INSERTER(Azure::Core::Amqp::_internal::ConnectionState, Error);
    }

#define TEST_C_INSERTER(ENUMERATOR) \
  { \
    std::stringstream ss; \
    ss << ENUMERATOR; \
    EXPECT_EQ(#ENUMERATOR, ss.str()); \
  }
    {
      TEST_C_INSERTER(CONNECTION_STATE_START);
      TEST_C_INSERTER(CONNECTION_STATE_HDR_RCVD);
      TEST_C_INSERTER(CONNECTION_STATE_HDR_SENT);
      TEST_C_INSERTER(CONNECTION_STATE_HDR_EXCH);
      TEST_C_INSERTER(CONNECTION_STATE_OPEN_PIPE);
      TEST_C_INSERTER(CONNECTION_STATE_OC_PIPE);
      TEST_C_INSERTER(CONNECTION_STATE_OPEN_RCVD);
      TEST_C_INSERTER(CONNECTION_STATE_OPEN_SENT);
      TEST_C_INSERTER(CONNECTION_STATE_CLOSE_PIPE);
      TEST_C_INSERTER(CONNECTION_STATE_OPENED);
      TEST_C_INSERTER(CONNECTION_STATE_CLOSE_RCVD);
      TEST_C_INSERTER(CONNECTION_STATE_CLOSE_SENT);
      TEST_C_INSERTER(CONNECTION_STATE_DISCARDING);
      TEST_C_INSERTER(CONNECTION_STATE_END);
      TEST_C_INSERTER(CONNECTION_STATE_ERROR);
    }

    CONNECTION_STATE state;
    GTEST_LOG_(INFO) << CONNECTION_STATE_ERROR;
    state = static_cast<CONNECTION_STATE>(3257);
    GTEST_LOG_(INFO) << state;
  }
}}}}} // namespace Azure::Core::Amqp::_detail::Tests
