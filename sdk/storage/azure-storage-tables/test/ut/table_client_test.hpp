// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "test/ut/test_base.hpp"

#include <azure/storage/tables/rest_client.hpp>

namespace Azure { namespace Storage { namespace Test {

  class TablesClientTest : public Azure::Storage::Test::StorageTest {
  protected:
    void SetUp() override;

   // Azure::Storage::Tables::TableClient GetTableClientForTest(
   //     const std::string& queueName,
   //     Table::QueueClientOptions clientOptions = Queues::QueueClientOptions());

  protected:
   // std::shared_ptr<Queues::QueueServiceClient> m_queueServiceClient;
   // std::shared_ptr<Queues::QueueClient> m_queueClient;
   // std::string m_queueName;
      std::shared_ptr<Tables::TableServicesClient> m_tableServiceClient;
  };

}}} // namespace Azure::Storage::Test
