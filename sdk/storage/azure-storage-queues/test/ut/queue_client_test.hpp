// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/queues.hpp>

#include "test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class QueueClientTest : public ::testing::Test {
  protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    static std::shared_ptr<Queues::QueueServiceClient> m_queueServiceClient;
    static std::shared_ptr<Queues::QueueClient> m_queueClient;
    static std::string m_queueName;
  };

}}} // namespace Azure::Storage::Test
