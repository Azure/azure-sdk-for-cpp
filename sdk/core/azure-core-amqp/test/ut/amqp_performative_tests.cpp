// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/models/performatives/amqp_detach.hpp"
#include "azure/core/amqp/internal/models/performatives/amqp_transfer.hpp"


#include <gtest/gtest.h>

using namespace Azure::Core::Amqp::Models::_internal;

class TestPerformatives: public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestPerformatives, SimpleCreate)
{
  {
    Performatives::AmqpDetach detach;
    Performatives::AmqpTransfer transfer;
  }
}

