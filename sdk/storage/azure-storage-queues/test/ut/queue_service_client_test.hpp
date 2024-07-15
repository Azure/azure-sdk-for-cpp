// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "test/ut/test_base.hpp"

#include <azure/storage/queues.hpp>

namespace Azure { namespace Storage { namespace Test {

  class QueueServiceClientTest : public Azure::Storage::Test::StorageTest {
  protected:
    void SetUp();

    void TearDown() { StorageTest::TearDown(); }

    std::string GetQueueServiceUrl()
    {
      return "https://" + StandardStorageAccountName() + ".queue.core.windows.net";
    }

    std::shared_ptr<Queues::QueueServiceClient> m_queueServiceClient;
    Queues::QueueClientOptions m_options;
  };

}}} // namespace Azure::Storage::Test
