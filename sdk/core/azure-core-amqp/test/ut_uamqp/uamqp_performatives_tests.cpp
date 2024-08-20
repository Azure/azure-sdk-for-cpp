// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../../src/models/private/error_impl.hpp"
#include "../../src/models/private/performatives/detach_impl.hpp"
#include "../../src/models/private/performatives/transfer_impl.hpp"
#include "../../src/models/private/value_impl.hpp"
#include "azure/core/amqp/internal/models/performatives/amqp_detach.hpp"
#include "azure/core/amqp/internal/models/performatives/amqp_transfer.hpp"

#include <gtest/gtest.h>

using namespace Azure::Core::Amqp::Models::_internal;
using namespace Azure::Core::Amqp::Models;

class TestPerformativesUamqp : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestPerformativesUamqp, SimpleCreate)
{
  {
    Performatives::AmqpDetach detach;
    detach.Closed = true;
    detach.Handle = 23;

    auto detachHandle{Azure::Core::Amqp::Models::_detail::AmqpDetachFactory::ToAmqpDetach(detach)};

    ASSERT_TRUE(detachHandle);
    bool closed;
    ASSERT_EQ(0, detach_get_closed(detachHandle.get(), &closed));
    ASSERT_TRUE(closed);
  }
  {
    Performatives::AmqpTransfer transfer;

    transfer.Handle = 17;
    transfer.DeliveryId = 92;
    transfer.Aborted = true;

    auto transferHandle{Azure::Core::Amqp::Models::_detail::AmqpTransferFactory::ToAmqp(transfer)};
  }
}

TEST_F(TestPerformativesUamqp, AmqpTransferFactory)
{
  Azure::Core::Amqp::Models::_detail::UniqueAmqpTransferHandle amqpTransfer{transfer_create(92)};

  {
    Performatives::AmqpTransfer transfer{
        Azure::Core::Amqp::Models::_detail::AmqpTransferFactory::FromImplementation(amqpTransfer.get())};

    GTEST_LOG_(INFO) << "Transfer: " << transfer;

    ASSERT_EQ(92, transfer.Handle);
    ASSERT_FALSE(transfer.DeliveryId);
    ASSERT_FALSE(transfer.DeliveryTag);
    ASSERT_EQ(Azure::Core::Amqp::Models::AmqpDefaultMessageFormatValue, transfer.MessageFormat);
    ASSERT_FALSE(transfer.Settled);
    ASSERT_EQ(false, transfer.More);
    ASSERT_FALSE(transfer.SettleMode);
    ASSERT_TRUE(transfer.State.IsNull());
    ASSERT_FALSE(transfer.Resume);
    ASSERT_FALSE(transfer.Aborted);
    ASSERT_FALSE(transfer.Batchable);
  }

  {
    ASSERT_EQ(0, transfer_set_delivery_id(amqpTransfer.get(), 17));
    Performatives::AmqpTransfer transfer{
        Azure::Core::Amqp::Models::_detail::AmqpTransferFactory::FromImplementation(amqpTransfer.get())};

    GTEST_LOG_(INFO) << "Transfer: " << transfer;

    ASSERT_EQ(92, transfer.Handle);
    ASSERT_TRUE(transfer.DeliveryId);
    ASSERT_EQ(17, transfer.DeliveryId.Value());
    ASSERT_FALSE(transfer.DeliveryTag);
    ASSERT_EQ(Azure::Core::Amqp::Models::AmqpDefaultMessageFormatValue, transfer.MessageFormat);
    ASSERT_FALSE(transfer.Settled);
    ASSERT_EQ(false, transfer.More);
    ASSERT_FALSE(transfer.SettleMode);
    ASSERT_TRUE(transfer.State.IsNull());
    ASSERT_FALSE(transfer.Resume);
    ASSERT_FALSE(transfer.Aborted);
    ASSERT_FALSE(transfer.Batchable);
  }
  {
    delivery_tag tag;
    uint8_t bytes[] = {1, 2, 3, 4, 5};
    tag.bytes = bytes;
    tag.length = 5;
    ASSERT_EQ(0, transfer_set_delivery_tag(amqpTransfer.get(), tag));

    Performatives::AmqpTransfer transfer{
        Azure::Core::Amqp::Models::_detail::AmqpTransferFactory::FromImplementation(amqpTransfer.get())};

    GTEST_LOG_(INFO) << "Transfer: " << transfer;

    ASSERT_EQ(92, transfer.Handle);
    ASSERT_TRUE(transfer.DeliveryId);
    ASSERT_EQ(17, transfer.DeliveryId.Value());
    ASSERT_TRUE(transfer.DeliveryTag);
    ASSERT_EQ(5, transfer.DeliveryTag.Value().size());
    ASSERT_EQ(Azure::Core::Amqp::Models::AmqpDefaultMessageFormatValue, transfer.MessageFormat);
    ASSERT_FALSE(transfer.Settled);
    ASSERT_EQ(false, transfer.More);
    ASSERT_FALSE(transfer.SettleMode);
    ASSERT_TRUE(transfer.State.IsNull());
    ASSERT_FALSE(transfer.Resume);
    ASSERT_FALSE(transfer.Aborted);
    ASSERT_FALSE(transfer.Batchable);
  }

  // Message Format.
  {
    ASSERT_EQ(0, transfer_set_message_format(amqpTransfer.get(), 95525));
    Performatives::AmqpTransfer transfer{
        Azure::Core::Amqp::Models::_detail::AmqpTransferFactory::FromImplementation(amqpTransfer.get())};

    GTEST_LOG_(INFO) << "Transfer: " << transfer;

    ASSERT_EQ(92, transfer.Handle);
    ASSERT_TRUE(transfer.DeliveryId);
    ASSERT_EQ(17, transfer.DeliveryId.Value());
    ASSERT_TRUE(transfer.DeliveryTag);
    ASSERT_EQ(5, transfer.DeliveryTag.Value().size());
    ASSERT_EQ(95525, transfer.MessageFormat);
    ASSERT_FALSE(transfer.Settled);
    ASSERT_EQ(false, transfer.More);
    ASSERT_FALSE(transfer.SettleMode);
    ASSERT_TRUE(transfer.State.IsNull());
    ASSERT_FALSE(transfer.Resume);
    ASSERT_FALSE(transfer.Aborted);
    ASSERT_FALSE(transfer.Batchable);
  }

  // Settled.
  {
    ASSERT_EQ(0, transfer_set_settled(amqpTransfer.get(), true));
    Performatives::AmqpTransfer transfer{
        Azure::Core::Amqp::Models::_detail::AmqpTransferFactory::FromImplementation(amqpTransfer.get())};

    GTEST_LOG_(INFO) << "Transfer: " << transfer;

    ASSERT_EQ(92, transfer.Handle);
    ASSERT_TRUE(transfer.DeliveryId);
    ASSERT_EQ(17, transfer.DeliveryId.Value());
    ASSERT_TRUE(transfer.DeliveryTag);
    ASSERT_EQ(5, transfer.DeliveryTag.Value().size());
    ASSERT_EQ(95525, transfer.MessageFormat);
    ASSERT_TRUE(transfer.Settled);
    ASSERT_TRUE(transfer.Settled.Value());
    ASSERT_EQ(false, transfer.More);
    ASSERT_FALSE(transfer.SettleMode);
    ASSERT_TRUE(transfer.State.IsNull());
    ASSERT_FALSE(transfer.Resume);
    ASSERT_FALSE(transfer.Aborted);
    ASSERT_FALSE(transfer.Batchable);
  }

  // More.
  {
    ASSERT_EQ(0, transfer_set_more(amqpTransfer.get(), true));
    Performatives::AmqpTransfer transfer{
        Azure::Core::Amqp::Models::_detail::AmqpTransferFactory::FromImplementation(amqpTransfer.get())};

    GTEST_LOG_(INFO) << "Transfer: " << transfer;

    ASSERT_EQ(92, transfer.Handle);
    ASSERT_TRUE(transfer.DeliveryId);
    ASSERT_EQ(17, transfer.DeliveryId.Value());
    ASSERT_TRUE(transfer.DeliveryTag);
    ASSERT_EQ(5, transfer.DeliveryTag.Value().size());
    ASSERT_EQ(95525, transfer.MessageFormat);
    ASSERT_TRUE(transfer.Settled);
    ASSERT_TRUE(transfer.Settled.Value());
    ASSERT_EQ(true, transfer.More);
    ASSERT_FALSE(transfer.SettleMode);
    ASSERT_TRUE(transfer.State.IsNull());
    ASSERT_FALSE(transfer.Resume);
    ASSERT_FALSE(transfer.Aborted);
    ASSERT_FALSE(transfer.Batchable);
  }

  // Receiver Settle Mode first
  {
    ASSERT_EQ(0, transfer_set_rcv_settle_mode(amqpTransfer.get(), receiver_settle_mode_first));
    Performatives::AmqpTransfer transfer{
        Azure::Core::Amqp::Models::_detail::AmqpTransferFactory::FromImplementation(amqpTransfer.get())};

    GTEST_LOG_(INFO) << "Transfer: " << transfer;

    ASSERT_EQ(92, transfer.Handle);
    ASSERT_TRUE(transfer.DeliveryId);
    ASSERT_EQ(17, transfer.DeliveryId.Value());
    ASSERT_TRUE(transfer.DeliveryTag);
    ASSERT_EQ(5, transfer.DeliveryTag.Value().size());
    ASSERT_EQ(95525, transfer.MessageFormat);
    ASSERT_TRUE(transfer.Settled);
    ASSERT_TRUE(transfer.Settled.Value());
    ASSERT_EQ(true, transfer.More);
    ASSERT_TRUE(transfer.SettleMode);
    ASSERT_EQ(Azure::Core::Amqp::_internal::ReceiverSettleMode::First, transfer.SettleMode.Value());
    ASSERT_TRUE(transfer.State.IsNull());
    ASSERT_FALSE(transfer.Resume);
    ASSERT_FALSE(transfer.Aborted);
    ASSERT_FALSE(transfer.Batchable);
  }

  // Receiver Settle Mode second
  {
    ASSERT_EQ(0, transfer_set_rcv_settle_mode(amqpTransfer.get(), receiver_settle_mode_second));
    Performatives::AmqpTransfer transfer{
        Azure::Core::Amqp::Models::_detail::AmqpTransferFactory::FromImplementation(amqpTransfer.get())};

    GTEST_LOG_(INFO) << "Transfer: " << transfer;

    ASSERT_EQ(92, transfer.Handle);
    ASSERT_TRUE(transfer.DeliveryId);
    ASSERT_EQ(17, transfer.DeliveryId.Value());
    ASSERT_TRUE(transfer.DeliveryTag);
    ASSERT_EQ(5, transfer.DeliveryTag.Value().size());
    ASSERT_EQ(95525, transfer.MessageFormat);
    ASSERT_TRUE(transfer.Settled);
    ASSERT_TRUE(transfer.Settled.Value());
    ASSERT_EQ(true, transfer.More);
    ASSERT_TRUE(transfer.SettleMode);
    ASSERT_EQ(
        Azure::Core::Amqp::_internal::ReceiverSettleMode::Second, transfer.SettleMode.Value());
    ASSERT_TRUE(transfer.State.IsNull());
    ASSERT_FALSE(transfer.Resume);
    ASSERT_FALSE(transfer.Aborted);
    ASSERT_FALSE(transfer.Batchable);
  }

  // State
  {
    AmqpValue value{"This is a string value"};

    ASSERT_EQ(0, transfer_set_state(amqpTransfer.get(), _detail::AmqpValueFactory::ToImplementation(value)));
    Performatives::AmqpTransfer transfer{
        Azure::Core::Amqp::Models::_detail::AmqpTransferFactory::FromImplementation(amqpTransfer.get())};

    GTEST_LOG_(INFO) << "Transfer: " << transfer;

    ASSERT_EQ(92, transfer.Handle);
    ASSERT_TRUE(transfer.DeliveryId);
    ASSERT_EQ(17, transfer.DeliveryId.Value());
    ASSERT_TRUE(transfer.DeliveryTag);
    ASSERT_EQ(5, transfer.DeliveryTag.Value().size());
    ASSERT_EQ(95525, transfer.MessageFormat);
    ASSERT_TRUE(transfer.Settled);
    ASSERT_TRUE(transfer.Settled.Value());
    ASSERT_EQ(true, transfer.More);
    ASSERT_TRUE(transfer.SettleMode);
    ASSERT_EQ(
        Azure::Core::Amqp::_internal::ReceiverSettleMode::Second, transfer.SettleMode.Value());
    ASSERT_FALSE(transfer.State.IsNull());
    ASSERT_EQ("This is a string value", static_cast<std::string>(transfer.State));
    ASSERT_FALSE(transfer.Resume);
    ASSERT_FALSE(transfer.Aborted);
    ASSERT_FALSE(transfer.Batchable);
  }

  // Resume
  {
    ASSERT_EQ(0, transfer_set_resume(amqpTransfer.get(), true));
    Performatives::AmqpTransfer transfer{
        Azure::Core::Amqp::Models::_detail::AmqpTransferFactory::FromImplementation(amqpTransfer.get())};

    GTEST_LOG_(INFO) << "Transfer: " << transfer;

    ASSERT_EQ(92, transfer.Handle);
    ASSERT_TRUE(transfer.DeliveryId);
    ASSERT_EQ(17, transfer.DeliveryId.Value());
    ASSERT_TRUE(transfer.DeliveryTag);
    ASSERT_EQ(5, transfer.DeliveryTag.Value().size());
    ASSERT_EQ(95525, transfer.MessageFormat);
    ASSERT_TRUE(transfer.Settled);
    ASSERT_TRUE(transfer.Settled.Value());
    ASSERT_EQ(true, transfer.More);
    ASSERT_TRUE(transfer.SettleMode);
    ASSERT_EQ(
        Azure::Core::Amqp::_internal::ReceiverSettleMode::Second, transfer.SettleMode.Value());
    ASSERT_FALSE(transfer.State.IsNull());
    ASSERT_EQ("This is a string value", static_cast<std::string>(transfer.State));
    ASSERT_TRUE(transfer.Resume);
    ASSERT_FALSE(transfer.Aborted);
    ASSERT_FALSE(transfer.Batchable);
  }

  // Aborted
  {
    transfer_set_aborted(amqpTransfer.get(), true);
    Performatives::AmqpTransfer transfer{
        Azure::Core::Amqp::Models::_detail::AmqpTransferFactory::FromImplementation(amqpTransfer.get())};

    GTEST_LOG_(INFO) << "Transfer: " << transfer;

    ASSERT_EQ(92, transfer.Handle);
    ASSERT_TRUE(transfer.DeliveryId);
    ASSERT_EQ(17, transfer.DeliveryId.Value());
    ASSERT_TRUE(transfer.DeliveryTag);
    ASSERT_EQ(5, transfer.DeliveryTag.Value().size());
    ASSERT_EQ(95525, transfer.MessageFormat);
    ASSERT_TRUE(transfer.Settled);
    ASSERT_TRUE(transfer.Settled.Value());
    ASSERT_EQ(true, transfer.More);
    ASSERT_TRUE(transfer.SettleMode);
    ASSERT_EQ(
        Azure::Core::Amqp::_internal::ReceiverSettleMode::Second, transfer.SettleMode.Value());
    ASSERT_FALSE(transfer.State.IsNull());
    ASSERT_EQ("This is a string value", static_cast<std::string>(transfer.State));
    ASSERT_TRUE(transfer.Resume);
    ASSERT_TRUE(transfer.Aborted);
    ASSERT_FALSE(transfer.Batchable);
  }

  // Batchable
  {
    transfer_set_batchable(amqpTransfer.get(), false);
    Performatives::AmqpTransfer transfer{
        Azure::Core::Amqp::Models::_detail::AmqpTransferFactory::FromImplementation(amqpTransfer.get())};

    GTEST_LOG_(INFO) << "Transfer: " << transfer;

    ASSERT_EQ(92, transfer.Handle);
    ASSERT_TRUE(transfer.DeliveryId);
    ASSERT_EQ(17, transfer.DeliveryId.Value());
    ASSERT_TRUE(transfer.DeliveryTag);
    ASSERT_EQ(5, transfer.DeliveryTag.Value().size());
    ASSERT_EQ(95525, transfer.MessageFormat);
    ASSERT_TRUE(transfer.Settled);
    ASSERT_TRUE(transfer.Settled.Value());
    ASSERT_EQ(true, transfer.More);
    ASSERT_TRUE(transfer.SettleMode);
    ASSERT_EQ(
        Azure::Core::Amqp::_internal::ReceiverSettleMode::Second, transfer.SettleMode.Value());
    ASSERT_FALSE(transfer.State.IsNull());
    ASSERT_EQ("This is a string value", static_cast<std::string>(transfer.State));
    ASSERT_TRUE(transfer.Resume);
    ASSERT_TRUE(transfer.Aborted);
    ASSERT_FALSE(transfer.Batchable);
  }
}

TEST_F(TestPerformativesUamqp, AmqpDetachFactory)
{
  Azure::Core::Amqp::Models::_detail::UniqueAmqpDetachHandle amqpDetach{detach_create(343)};

  {
    Performatives::AmqpDetach detach{
        Azure::Core::Amqp::Models::_detail::AmqpDetachFactory::ToImplementation(amqpDetach.get())};
    GTEST_LOG_(INFO) << "Detach: " << detach;

    ASSERT_EQ(343, detach.Handle);
    ASSERT_FALSE(detach.Closed);
    ASSERT_FALSE(detach.Error);
  }

  {
    ASSERT_EQ(0, detach_set_closed(amqpDetach.get(), true));
    Performatives::AmqpDetach detach{
        Azure::Core::Amqp::Models::_detail::AmqpDetachFactory::ToImplementation(amqpDetach.get())};
    GTEST_LOG_(INFO) << "Detach: " << detach;

    ASSERT_EQ(343, detach.Handle);
    ASSERT_TRUE(detach.Closed);
    ASSERT_FALSE(detach.Error);
  }

  {
    _internal::AmqpError error;

    error.Condition = AmqpErrorCondition::DecodeError;
    error.Description = "A Description of the error";

    ASSERT_EQ(
        0, detach_set_error(amqpDetach.get(), _detail::AmqpErrorFactory::ToAmqpError(error).get()));
    Performatives::AmqpDetach detach{
        Azure::Core::Amqp::Models::_detail::AmqpDetachFactory::ToImplementation(amqpDetach.get())};
    GTEST_LOG_(INFO) << "Detach: " << detach;

    ASSERT_EQ(343, detach.Handle);
    ASSERT_TRUE(detach.Closed);
    ASSERT_TRUE(detach.Error);
    ASSERT_EQ(AmqpErrorCondition::DecodeError, detach.Error.Condition);
  }
}
