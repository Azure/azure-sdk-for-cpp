// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "queue_service_client_test.hpp"

namespace Azure { namespace Storage { namespace Test {

  class QueueClientTest : public QueueServiceClientTest {
  protected:
    void SetUp() override;

    std::string GetQueueUrl(const std::string& queueName)
    {
      return GetQueueServiceUrl() + "/" + queueName;
    }

    Queues::QueueClient GetQueueClientForTest(
        const std::string& queueName,
        Queues::QueueClientOptions clientOptions = Queues::QueueClientOptions());

  protected:
    std::shared_ptr<Queues::QueueClient> m_queueClient;
    std::string m_queueName;
  };

}}} // namespace Azure::Storage::Test
