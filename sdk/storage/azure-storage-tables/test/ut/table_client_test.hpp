#pragma once
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "test/ut/test_base.hpp"

#include <azure/storage/tables/rest_client.hpp>
#include <azure/storage/tables/transactions.hpp>
#include <azure/storage/tables/models.hpp>

namespace Azure { namespace Storage { namespace Test {

  class TablesClientTest : public Azure::Storage::Test::StorageTest {
  protected:
    void SetUp() override;

    Azure::Storage::Tables::TableClient CreateTableClientForTest(
        Tables::TableClientOptions& clientOptions);

  protected:
    std::string m_tableName;
    std::shared_ptr<Tables::TableServicesClient> m_tableServiceClient;
    std::shared_ptr<Tables::TableClient> m_tableClient;
 
  };
}}} // namespace Azure::Storage::Test
