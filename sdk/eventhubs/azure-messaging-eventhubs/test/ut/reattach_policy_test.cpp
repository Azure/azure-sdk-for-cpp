// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../src/private/eventhubs_utilities.hpp"
#include "azure/messaging/eventhubs.hpp"
#include "eventhubs_test_base.hpp"

#include <gtest/gtest.h>

using namespace Azure::Messaging::EventHubs;
namespace EventHubsDetail = Azure::Messaging::EventHubs::_detail;

class ReattachPolicyTest : public EventHubsTestBase {
protected:
  static EventHubsException ExceptionFor(std::string const& condition, bool isTransient)
  {
    EventHubsException exception{"test"};
    exception.ErrorCondition = condition;
    exception.IsTransient = isTransient;
    return exception;
  }

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

TEST_F(ReattachPolicyTest, RebuildTheReceiverOnATransientCondition)
{
  EXPECT_TRUE(
      EventHubsDetail::ShouldRebuildReceiver(ExceptionFor("amqp:link:detach-forced", true)));
  EXPECT_TRUE(EventHubsDetail::ShouldRebuildReceiver(ExceptionFor("amqp:connection:forced", true)));
}

TEST_F(ReattachPolicyTest, RebuildTheReceiverOnAnEmptyCondition)
{
  EXPECT_TRUE(EventHubsDetail::ShouldRebuildReceiver(ExceptionFor("", false)));
}

TEST_F(ReattachPolicyTest, DoNotRebuildTheReceiverOnAPermanentCondition)
{
  EXPECT_FALSE(
      EventHubsDetail::ShouldRebuildReceiver(ExceptionFor("amqp:unauthorized-access", false)));
  EXPECT_FALSE(EventHubsDetail::ShouldRebuildReceiver(ExceptionFor("amqp:link:stolen", false)));
  EXPECT_FALSE(EventHubsDetail::ShouldRebuildReceiver(ExceptionFor("amqp:not-allowed", false)));
}

// On uAMQP this also covers a dead `$cbs` link (#7330); retry is allowed without a transient claim.
TEST_F(ReattachPolicyTest, TranslateAnAuthenticationFailureIntoAnotherAttempt)
{
  Azure::Core::Credentials::AuthenticationException failure{"put-token failed"};

  EventHubsException translated{EventHubsDetail::TranslateAuthenticationFailure(failure)};

  EXPECT_TRUE(EventHubsDetail::ShouldRebuildReceiver(translated));
  EXPECT_FALSE(translated.IsTransient);
  EXPECT_TRUE(translated.ErrorCondition.empty());
  EXPECT_EQ("put-token failed", translated.ErrorDescription);
}

TEST_F(ReattachPolicyTest, KeepTheSenderOnAMessageSizeRejection)
{
  EXPECT_FALSE(EventHubsDetail::ShouldInvalidateSender(
      ExceptionFor("amqp:link:message-size-exceeded", false)));
}

TEST_F(ReattachPolicyTest, DiscardTheSenderOnEveryOtherFailure)
{
  EXPECT_TRUE(
      EventHubsDetail::ShouldInvalidateSender(ExceptionFor("amqp:link:detach-forced", true)));
  EXPECT_TRUE(
      EventHubsDetail::ShouldInvalidateSender(ExceptionFor("amqp:unauthorized-access", false)));
  EXPECT_TRUE(EventHubsDetail::ShouldInvalidateSender(ExceptionFor("", false)));
}

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

TEST_F(ReattachPolicyTest, StartAfterTheOffsetOfTheLastEvent)
{
  Models::StartPosition earliest;
  earliest.Earliest = true;

  ExpectAnchoredOnOffset(
      EventHubsDetail::ResumeStartPosition(earliest, Azure::Nullable<std::string>{"1234"}), "1234");
}

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

TEST_F(ReattachPolicyTest, AnEmptyOffsetOfTheLastEventStillReplacesTheOriginalPosition)
{
  Models::StartPosition latest;
  latest.Latest = true;

  ExpectAnchoredOnOffset(
      EventHubsDetail::ResumeStartPosition(latest, Azure::Nullable<std::string>{""}), "");
}
