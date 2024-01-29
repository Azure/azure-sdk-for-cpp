// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../../src/models/private/performatives/detach_impl.hpp"
#include "../../src/models/private/performatives/transfer_impl.hpp"
#include "azure/core/amqp/internal/models/performatives/amqp_detach.hpp"
#include "azure/core/amqp/internal/models/performatives/amqp_transfer.hpp"

#include <gtest/gtest.h>

using namespace Azure::Core::Amqp::Models::_internal;

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

    transfer.HandleValue = 17;
    transfer.DeliveryId = 92;
    transfer.Aborted = true;

    auto transferHandle{Azure::Core::Amqp::Models::_detail::AmqpTransferFactory::ToAmqp(transfer)};
  }
}
