// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "table_client_test.hpp"

#include <chrono>
#include <thread>



namespace Azure { namespace Storage { namespace Test {

  void TablesClientTest::SetUp()
  {
	  if (m_tableServiceClient.get() == nullptr)
	  {
      m_tableServiceClient = std::make_shared<Tables::TableServicesClient>(
          Azure::Storage::Tables::TableServicesClient::TableServicesClient(
              ""));
	}
  }

  TEST_F(TablesClientTest, ServiceClientConstructors)
  {
      EXPECT_FALSE(m_tableServiceClient==nullptr);
  }

  TEST_F(TablesClientTest, ServiceClientGetProperties)
  {
	  std::string tableName = "";
	  Tables::GetServicePropertiesOptions options;
	  options.AccountName = "";
      auto response = m_tableServiceClient->GetServiceProperties();
      EXPECT_EQ(response.Value.Properties.Cors.CorsRules.size(), 0);
  }
}}} // namespace Azure::Storage::Test
