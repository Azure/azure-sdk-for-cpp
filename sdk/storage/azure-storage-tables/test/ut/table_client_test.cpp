// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "table_client_test.hpp"

#include <chrono>
#include <thread>

namespace Azure { namespace Storage { namespace Test {
  std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_credential;
  void TablesClientTest::SetUp()
  {
    if (m_tableServiceClient.get() == nullptr)
    {
      Tables::TableClientOptions tableOptions;
      tableOptions.EnableTenantDiscovery = true;
      m_credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
          GetEnv("STORAGE_TENANT_ID"),
          GetEnv("STORAGE_CLIENT_ID"),
          GetEnv("STORAGE_CLIENT_SECRET"));
      m_tableServiceClient = std::make_shared<Tables::TableServicesClient>(
          Azure::Storage::Tables::TableServicesClient::TableServicesClient("", m_credential));
    }
  }

  TEST_F(TablesClientTest, ServiceClientConstructors)
  {
    EXPECT_FALSE(m_tableServiceClient == nullptr);
  }

  TEST_F(TablesClientTest, ServiceClientGetProperties)
  {
    Azure::Storage::Tables::GetServicePropertiesOptions getOptions;
    getOptions.ResourceGroupName = GetEnv("STORAGE_RESOURCE_GROUP");
    getOptions.AccountName = GetEnv("TABLES_STORAGE_ACCOUNT_NAME");
    getOptions.SubscriptionId = GetEnv("STORAGE_SUBSCRIPTION_ID");
    auto response = m_tableServiceClient->GetServiceProperties(getOptions);
    EXPECT_EQ(response.Value.Properties.Cors.CorsRules.size(), 2);
  }

  TEST_F(TablesClientTest, ServiceClientList)
  {
    Azure::Storage::Tables::ListOptions list;
    list.ResourceGroupName = GetEnv("STORAGE_RESOURCE_GROUP");
    list.AccountName = GetEnv("TABLES_STORAGE_ACCOUNT_NAME");
    list.SubcriptionId = GetEnv("STORAGE_SUBSCRIPTION_ID"); 
    auto response = m_tableServiceClient->List(list);
    EXPECT_EQ(response.Value.Value.size(), 1);
  }

  TEST_F(TablesClientTest, ServiceClientSet)
  {
    Azure::Storage::Tables::SetServicePropertiesOptions setOptions;
    setOptions.ResourceGroupName = GetEnv("STORAGE_RESOURCE_GROUP");
    setOptions.AccountName = GetEnv("TABLES_STORAGE_ACCOUNT_NAME");
    setOptions.SubscriptionId= GetEnv("STORAGE_SUBSCRIPTION_ID");
    auto response = m_tableServiceClient->SetServiceProperties(setOptions);
    EXPECT_EQ(response.Value.Properties.Cors.CorsRules.size(), 0);
  }
}}} // namespace Azure::Storage::Test
