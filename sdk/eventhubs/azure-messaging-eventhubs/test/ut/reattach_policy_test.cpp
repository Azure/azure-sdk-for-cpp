// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Tests for the two decisions that drive link reattach. ShouldRebuildReceiver decides
// whether a receive fault permits a new attach. ShouldInvalidateSender decides whether a
// send fault makes the cached sender unusable.

#include "../src/private/eventhubs_utilities.hpp"
#include "azure/messaging/eventhubs.hpp"
#include "eventhubs_test_base.hpp"

#include <gtest/gtest.h>

using namespace Azure::Messaging::EventHubs;
namespace EventHubsDetail = Azure::Messaging::EventHubs::_detail;

class ReattachPolicyTest : public EventHubsTestBase {
protected:
  // Build the exception that the client gets from an AMQP error.
  static EventHubsException ExceptionFor(std::string const& condition, bool isTransient)
  {
    EventHubsException exception{"test"};
    exception.ErrorCondition = condition;
    exception.IsTransient = isTransient;
    return exception;
  }
};

// A transient condition means that the service or the network dropped the link, so a new
// attach can succeed.
TEST_F(ReattachPolicyTest, RebuildTheReceiverOnATransientCondition)
{
  EXPECT_TRUE(
      EventHubsDetail::ShouldRebuildReceiver(ExceptionFor("amqp:link:detach-forced", true)));
  EXPECT_TRUE(EventHubsDetail::ShouldRebuildReceiver(ExceptionFor("amqp:connection:forced", true)));
}

// The Rust transport reports a receive fault with no condition, and some uAMQP paths do the
// same. An attach attempt is then the only test that the client has.
TEST_F(ReattachPolicyTest, RebuildTheReceiverOnAnEmptyCondition)
{
  EXPECT_TRUE(EventHubsDetail::ShouldRebuildReceiver(ExceptionFor("", false)));
}

// A condition that a new attach cannot correct must reach the caller. A client that keeps
// attaching against a stolen link fights the new owner, and one that keeps attaching without
// permission spends its whole budget on a failure that stays.
TEST_F(ReattachPolicyTest, DoNotRebuildTheReceiverOnAPermanentCondition)
{
  EXPECT_FALSE(
      EventHubsDetail::ShouldRebuildReceiver(ExceptionFor("amqp:unauthorized-access", false)));
  EXPECT_FALSE(EventHubsDetail::ShouldRebuildReceiver(ExceptionFor("amqp:link:stolen", false)));
  EXPECT_FALSE(EventHubsDetail::ShouldRebuildReceiver(ExceptionFor("amqp:not-allowed", false)));
}

// The service rejects a transfer that is too large, and it keeps the link attached. So the
// cached sender stays valid, and the client must not tear its stack down.
TEST_F(ReattachPolicyTest, KeepTheSenderOnAMessageSizeRejection)
{
  EXPECT_FALSE(EventHubsDetail::ShouldInvalidateSender(
      ExceptionFor("amqp:link:message-size-exceeded", false)));
}

// Every other send failure can mean a link, a session, or a connection that is gone. This
// layer cannot tell which one it is, so it discards all three and builds new ones.
TEST_F(ReattachPolicyTest, DiscardTheSenderOnEveryOtherFailure)
{
  EXPECT_TRUE(
      EventHubsDetail::ShouldInvalidateSender(ExceptionFor("amqp:link:detach-forced", true)));
  EXPECT_TRUE(
      EventHubsDetail::ShouldInvalidateSender(ExceptionFor("amqp:unauthorized-access", false)));
  EXPECT_TRUE(EventHubsDetail::ShouldInvalidateSender(ExceptionFor("", false)));
}
