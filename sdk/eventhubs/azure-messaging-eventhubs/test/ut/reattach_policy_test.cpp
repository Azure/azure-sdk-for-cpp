// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Tests for the decisions that drive link reattach. ShouldRebuildReceiver decides
// whether a receive fault permits a new attach. TranslateAuthenticationFailure turns an
// authentication failure into the exception that feeds that decision.
// ShouldInvalidateSender decides whether a send fault makes the cached sender unusable.
// ResumeStartPosition decides where a new receiver starts.

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

  // Compare two start positions. StartPosition has no equality operator, so this helper
  // compares each anchor and the Inclusive flag.
  static void ExpectSamePosition(
      Models::StartPosition const& expected,
      Models::StartPosition const& actual)
  {
    ASSERT_EQ(expected.Offset.HasValue(), actual.Offset.HasValue());
    if (expected.Offset.HasValue())
    {
      EXPECT_EQ(expected.Offset.Value(), actual.Offset.Value());
    }
    ASSERT_EQ(expected.SequenceNumber.HasValue(), actual.SequenceNumber.HasValue());
    if (expected.SequenceNumber.HasValue())
    {
      EXPECT_EQ(expected.SequenceNumber.Value(), actual.SequenceNumber.Value());
    }
    ASSERT_EQ(expected.EnqueuedTime.HasValue(), actual.EnqueuedTime.HasValue());
    if (expected.EnqueuedTime.HasValue())
    {
      EXPECT_EQ(expected.EnqueuedTime.Value(), actual.EnqueuedTime.Value());
    }
    ASSERT_EQ(expected.Earliest.HasValue(), actual.Earliest.HasValue());
    if (expected.Earliest.HasValue())
    {
      EXPECT_EQ(expected.Earliest.Value(), actual.Earliest.Value());
    }
    ASSERT_EQ(expected.Latest.HasValue(), actual.Latest.HasValue());
    if (expected.Latest.HasValue())
    {
      EXPECT_EQ(expected.Latest.Value(), actual.Latest.Value());
    }
    EXPECT_EQ(expected.Inclusive, actual.Inclusive);
  }

  // Make sure that a resumed position holds the given offset and no other anchor.
  static void ExpectAnchoredOnOffset(
      Models::StartPosition const& position,
      std::string const& offset)
  {
    ASSERT_TRUE(position.Offset.HasValue());
    EXPECT_EQ(offset, position.Offset.Value());
    EXPECT_FALSE(position.Inclusive);
    EXPECT_FALSE(position.SequenceNumber.HasValue());
    EXPECT_FALSE(position.EnqueuedTime.HasValue());
    EXPECT_FALSE(position.Earliest.HasValue());
    EXPECT_FALSE(position.Latest.HasValue());
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

// On uAMQP, PutTokenForAudience raises AuthenticationException for every CBS result that
// is not Ok (issue #7330). So the type covers a refused claim and a dead `$cbs` link, and
// the client cannot tell the two apart. The translation must permit the next attach
// attempt, and it must not claim that the failure is transient, because nothing
// established that.
TEST_F(ReattachPolicyTest, TranslateAnAuthenticationFailureIntoAnotherAttempt)
{
  Azure::Core::Credentials::AuthenticationException failure{"put-token failed"};

  EventHubsException translated{EventHubsDetail::TranslateAuthenticationFailure(failure)};

  EXPECT_TRUE(EventHubsDetail::ShouldRebuildReceiver(translated));
  EXPECT_FALSE(translated.IsTransient);
  EXPECT_TRUE(translated.ErrorCondition.empty());
  EXPECT_EQ("put-token failed", translated.ErrorDescription);
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

// The client gave the caller no event yet, so it has no offset to start after. A rebuild
// must then use the position that the caller asked for.
TEST_F(ReattachPolicyTest, KeepTheOriginalPositionBeforeTheFirstEvent)
{
  Models::StartPosition earliest;
  earliest.Earliest = true;
  ExpectSamePosition(earliest, EventHubsDetail::ResumeStartPosition(earliest, {}));

  Models::StartPosition latest;
  latest.Latest = true;
  ExpectSamePosition(latest, EventHubsDetail::ResumeStartPosition(latest, {}));

  Models::StartPosition defaultPosition;
  ExpectSamePosition(defaultPosition, EventHubsDetail::ResumeStartPosition(defaultPosition, {}));
}

// An explicit anchor must survive a rebuild that happens before the first event. The
// Inclusive flag belongs to that anchor, so it survives too.
TEST_F(ReattachPolicyTest, KeepAnExplicitOriginalAnchorBeforeTheFirstEvent)
{
  Models::StartPosition offsetPosition;
  offsetPosition.Offset = "1234";
  offsetPosition.Inclusive = true;
  ExpectSamePosition(offsetPosition, EventHubsDetail::ResumeStartPosition(offsetPosition, {}));

  Models::StartPosition timePosition;
  timePosition.EnqueuedTime = Azure::DateTime(2026, 8, 14, 1, 2, 3);
  ExpectSamePosition(timePosition, EventHubsDetail::ResumeStartPosition(timePosition, {}));

  Models::StartPosition sequencePosition;
  sequencePosition.SequenceNumber = 42;
  ExpectSamePosition(sequencePosition, EventHubsDetail::ResumeStartPosition(sequencePosition, {}));
}

// The caller already has the event at this offset. The new receiver must start after it,
// so the position holds the offset and Inclusive is false.
TEST_F(ReattachPolicyTest, StartAfterTheOffsetOfTheLastEvent)
{
  Models::StartPosition earliest;
  earliest.Earliest = true;

  ExpectAnchoredOnOffset(
      EventHubsDetail::ResumeStartPosition(earliest, Azure::Nullable<std::string>{"1234"}), "1234");
}

// The original position points at a place that the client already passed. The offset of the
// last event replaces it, and it also replaces an Inclusive flag that the caller set.
TEST_F(ReattachPolicyTest, TheOffsetOfTheLastEventReplacesTheOriginalPosition)
{
  Azure::Nullable<std::string> const lastOffset{"5678"};

  Models::StartPosition latest;
  latest.Latest = true;
  ExpectAnchoredOnOffset(EventHubsDetail::ResumeStartPosition(latest, lastOffset), "5678");

  Models::StartPosition timePosition;
  timePosition.EnqueuedTime = Azure::DateTime(2026, 8, 14, 1, 2, 3);
  ExpectAnchoredOnOffset(EventHubsDetail::ResumeStartPosition(timePosition, lastOffset), "5678");

  Models::StartPosition sequencePosition;
  sequencePosition.SequenceNumber = 42;
  ExpectAnchoredOnOffset(
      EventHubsDetail::ResumeStartPosition(sequencePosition, lastOffset), "5678");

  Models::StartPosition inclusiveOffset;
  inclusiveOffset.Offset = "1234";
  inclusiveOffset.Inclusive = true;
  ExpectAnchoredOnOffset(EventHubsDetail::ResumeStartPosition(inclusiveOffset, lastOffset), "5678");
}

// The service sends the offset as a string, so an empty string is possible. The client
// received an event, and it must not go back to the original position. So the empty offset
// stays, and the filter that comes from it is the report of that fault.
TEST_F(ReattachPolicyTest, AnEmptyOffsetOfTheLastEventStillReplacesTheOriginalPosition)
{
  Models::StartPosition latest;
  latest.Latest = true;

  ExpectAnchoredOnOffset(
      EventHubsDetail::ResumeStartPosition(latest, Azure::Nullable<std::string>{""}), "");
}
