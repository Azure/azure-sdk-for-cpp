// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/queues.hpp>

#include "test/ut/test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class QueueClientTest : public Azure::Storage::Test::StorageTest {
  protected:
    void SetUp();
    void TearDown();

    std::shared_ptr<Queues::QueueServiceClient> m_queueServiceClient;
    std::shared_ptr<Queues::QueueClient> m_queueClient;
    std::string m_queueName;
    Queues::QueueClientOptions m_options;
    std::string m_testName;
    std::string m_testNameLowercase;
  };

}}} // namespace Azure::Storage::Test