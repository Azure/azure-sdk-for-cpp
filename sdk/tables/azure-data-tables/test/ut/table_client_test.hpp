#pragma once
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/data/tables/models.hpp"
#include "azure/data/tables/tables_clients.hpp"
#include "test/ut/test_base.hpp"

namespace Azure { namespace Data { namespace Test {
  enum class AuthType
  {
    Key = 0x0,
    SAS = 0x1,
    ConnectionString = 0x2,
  };

  class TablesClientTest : public Azure::Storage::Test::StorageTest,
                           public ::testing::WithParamInterface<AuthType> {
  protected:
    void SetUp() override;
    void TearDown() override;
    Azure::Data::Tables::TableClient CreateTableClientForTest(
        Tables::TableClientOptions& clientOptions);

    Azure::Data::Tables::TableClient CreateKeyTableClientForTest(
        Tables::TableClientOptions& clientOptions);
    std::string GetConnectionString()
    {
      const static std::string ConnectionString = "";

      if (!ConnectionString.empty())
      {
        return ConnectionString;
      }
      const static std::string envConnectionString = GetEnv("STANDARD_STORAGE_CONNECTION_STRING");
      if (!envConnectionString.empty())
      {
        return envConnectionString;
      }
      throw std::runtime_error("Cannot find connection string.");
    }

    std::string GetAccountName()
    {
      return Azure::Data::Tables::Credentials::_detail::ParseConnectionString(GetConnectionString())
          .AccountName;
    }

    std::string GetAccountKey()
    {
      return Azure::Data::Tables::Credentials::_detail::ParseConnectionString(GetConnectionString())
          .AccountKey;
    }

  protected:
    std::string m_tableName;
    std::shared_ptr<Tables::TableServiceClient> m_tableServiceClient;
    std::shared_ptr<Tables::TableClient> m_tableClient;
  };
}}} // namespace Azure::Data::Test
