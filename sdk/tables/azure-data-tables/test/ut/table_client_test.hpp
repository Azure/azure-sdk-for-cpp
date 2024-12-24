#pragma once
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/data/tables/models.hpp"
#include "azure/data/tables/table_client.hpp"
#include "azure/data/tables/table_service_client.hpp"
#include "test/ut/test_base.hpp"

namespace Azure { namespace Data { namespace Test {
  enum class AuthType
  {
    Key = 0x0
  };

  class TablesClientTest : public Azure::Storage::Test::StorageTest,
                           public ::testing::WithParamInterface<AuthType> {
  protected:
    void SetUp() override;
    void TearDown() override;

    std::string GetAccountName()
    {
      static std::string accountName = "";

      if (!accountName.empty())
      {
        return accountName;
      }
      const static std::string envAccountName = GetEnv("ACCOUNT_NAME");
      if (!envAccountName.empty())
      {
        accountName = envAccountName;
        return envAccountName;
      }
      throw std::runtime_error("Cannot find Account Name.");
    }

    std::string GetAccountKey()
    {
      static std::string accountKey = "";

      if (!accountKey.empty())
      {
        return accountKey;
      }
      const static std::string envAccountKey = GetEnv("ACCOUNT_KEY");
      if (!envAccountKey.empty())
      {
        accountKey = envAccountKey;
        return envAccountKey;
      }
      throw std::runtime_error("Cannot find Account Key.");
    }

  protected:
    std::string m_tableName;
    std::shared_ptr<Tables::TableServiceClient> m_tableServiceClient;
    std::shared_ptr<Tables::TableClient> m_tableClient;
  };
}}} // namespace Azure::Data::Test
