// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "table_client_test.hpp"

#include <chrono>
#include <thread>

namespace Azure { namespace Storage { namespace Test {
  std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_credential;
  void TablesClientTest::SetUp()
  {
    StorageTest::SetUp();
    if (m_tableServiceClient.get() == nullptr)
    {
      Tables::TableClientOptions tableOptions;
      tableOptions.EnableTenantDiscovery = true;
      m_credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
          GetEnv("STORAGE_TENANT_ID"),
          GetEnv("STORAGE_CLIENT_ID"),
          GetEnv("STORAGE_CLIENT_SECRET"));
      auto clientOptions = InitStorageClientOptions<Tables::TableClientOptions>();
      m_tableServiceClient = std::make_shared<Tables::TableServicesClient>(
          Azure::Storage::Tables::TableServicesClient::TableServicesClient(
              GetEnv("STORAGE_SUBSCRIPTION_ID"),
              m_credential,
              Azure::Storage::_internal::TablesManagementPublicEndpoint,
              clientOptions));
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
    auto response = m_tableServiceClient->GetServiceProperties(getOptions);
    EXPECT_EQ(response.Value.Properties.Cors.CorsRules.size(), 0);
  }

  TEST_F(TablesClientTest, ServiceClientList)
  {
    Azure::Storage::Tables::ListOptions list;
    list.ResourceGroupName = GetEnv("STORAGE_RESOURCE_GROUP");
    list.AccountName = GetEnv("TABLES_STORAGE_ACCOUNT_NAME");
    auto response = m_tableServiceClient->List(list);
    EXPECT_EQ(response.Value.Value.size(), 1);
  }

  TEST_F(TablesClientTest, ServiceClientSet)
  {
    Azure::Storage::Tables::SetServicePropertiesOptions setOptions;
    setOptions.ResourceGroupName = GetEnv("STORAGE_RESOURCE_GROUP");
    setOptions.AccountName = GetEnv("TABLES_STORAGE_ACCOUNT_NAME");
    auto response = m_tableServiceClient->SetServiceProperties(setOptions);
    EXPECT_EQ(response.Value.Properties.Cors.CorsRules.size(), 0);
  }

  TEST_F(TablesClientTest, ServiceClientSetAndSet)
  {
    Azure::Storage::Tables::SetServicePropertiesOptions setOptions;
    setOptions.ResourceGroupName = GetEnv("STORAGE_RESOURCE_GROUP");
    setOptions.AccountName = GetEnv("TABLES_STORAGE_ACCOUNT_NAME");
    auto setResponse = m_tableServiceClient->SetServiceProperties(setOptions);
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules.size(), 0);

    Tables::CorsRule oneRule;
    oneRule.AllowedHeaders = {"x-ms-meta-data*"};
    oneRule.AllowedMethods = {Tables::AllowedMethods::Get};
    oneRule.AllowedOrigins = {"234"};
    oneRule.ExposedHeaders = {"x-ms-meta-*"};
    oneRule.MaxAgeInSeconds = 100;

    setOptions.Parameters.Properties.Cors.CorsRules.emplace_back(oneRule);

    setResponse = m_tableServiceClient->SetServiceProperties(setOptions);

    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules.size(), 1);
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules[0].AllowedHeaders.size(), 1);
    EXPECT_EQ(
        setResponse.Value.Properties.Cors.CorsRules[0].AllowedHeaders[0],
        "x-ms-meta-data*");
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules[0].AllowedMethods.size(), 1);
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules[0].AllowedMethods[0], Tables::AllowedMethods::Get);
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules[0].AllowedOrigins.size(), 1);
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules[0].AllowedOrigins[0], "234");
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules[0].ExposedHeaders.size(), 1);
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules[0].ExposedHeaders[0], "x-ms-meta-*");
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules[0].MaxAgeInSeconds, 100);
  }

  TEST_F(TablesClientTest, ServiceClientSetAndGet)
  {
    Azure::Storage::Tables::SetServicePropertiesOptions setOptions;
    setOptions.ResourceGroupName = GetEnv("STORAGE_RESOURCE_GROUP");
    setOptions.AccountName = GetEnv("TABLES_STORAGE_ACCOUNT_NAME");
    auto setResponse = m_tableServiceClient->SetServiceProperties(setOptions);
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules.size(), 0);

    Tables::CorsRule oneRule;
    oneRule.AllowedHeaders = {"x-ms-meta-data*"};
    oneRule.AllowedMethods = {Tables::AllowedMethods::Get};
    oneRule.AllowedOrigins = {"234"};
    oneRule.ExposedHeaders = {"x-ms-meta-*"};
    oneRule.MaxAgeInSeconds = 100;

    setOptions.Parameters.Properties.Cors.CorsRules.emplace_back(oneRule);

    setResponse = m_tableServiceClient->SetServiceProperties(setOptions);

    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules.size(), 1);
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules[0].AllowedHeaders.size(), 1);
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules[0].AllowedHeaders[0], "x-ms-meta-data*");
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules[0].AllowedMethods.size(), 1);
    EXPECT_EQ(
        setResponse.Value.Properties.Cors.CorsRules[0].AllowedMethods[0],
        Tables::AllowedMethods::Get);
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules[0].AllowedOrigins.size(), 1);
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules[0].AllowedOrigins[0], "234");
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules[0].ExposedHeaders.size(), 1);
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules[0].ExposedHeaders[0], "x-ms-meta-*");
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules[0].MaxAgeInSeconds, 100);

    Azure::Storage::Tables::GetServicePropertiesOptions getOptions;
    getOptions.ResourceGroupName = GetEnv("STORAGE_RESOURCE_GROUP");
    getOptions.AccountName = GetEnv("TABLES_STORAGE_ACCOUNT_NAME");
    auto getResponse = m_tableServiceClient->GetServiceProperties(getOptions);

    EXPECT_EQ(getResponse.Value.Properties.Cors.CorsRules.size(), 1);
    EXPECT_EQ(getResponse.Value.Properties.Cors.CorsRules[0].AllowedHeaders.size(), 1);
    EXPECT_EQ(getResponse.Value.Properties.Cors.CorsRules[0].AllowedHeaders[0], "x-ms-meta-data*");
    EXPECT_EQ(getResponse.Value.Properties.Cors.CorsRules[0].AllowedMethods.size(), 1);
    EXPECT_EQ(
        getResponse.Value.Properties.Cors.CorsRules[0].AllowedMethods[0],
        Tables::AllowedMethods::Get);
    EXPECT_EQ(getResponse.Value.Properties.Cors.CorsRules[0].AllowedOrigins.size(), 1);
    EXPECT_EQ(getResponse.Value.Properties.Cors.CorsRules[0].AllowedOrigins[0], "234");
    EXPECT_EQ(getResponse.Value.Properties.Cors.CorsRules[0].ExposedHeaders.size(), 1);
    EXPECT_EQ(getResponse.Value.Properties.Cors.CorsRules[0].ExposedHeaders[0], "x-ms-meta-*");
    EXPECT_EQ(getResponse.Value.Properties.Cors.CorsRules[0].MaxAgeInSeconds, 100);
  }

  TEST_F(TablesClientTest, ServiceClientSetAndList)
  {
    Azure::Storage::Tables::SetServicePropertiesOptions setOptions;
    setOptions.ResourceGroupName = GetEnv("STORAGE_RESOURCE_GROUP");
    setOptions.AccountName = GetEnv("TABLES_STORAGE_ACCOUNT_NAME");
    auto setResponse = m_tableServiceClient->SetServiceProperties(setOptions);
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules.size(), 0);

    Tables::CorsRule oneRule;
    oneRule.AllowedHeaders = {"x-ms-meta-data*"};
    oneRule.AllowedMethods = {Tables::AllowedMethods::Get};
    oneRule.AllowedOrigins = {"234"};
    oneRule.ExposedHeaders = {"x-ms-meta-*"};
    oneRule.MaxAgeInSeconds = 100;

    setOptions.Parameters.Properties.Cors.CorsRules.emplace_back(oneRule);

    setResponse = m_tableServiceClient->SetServiceProperties(setOptions);

    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules.size(), 1);
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules[0].AllowedHeaders.size(), 1);
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules[0].AllowedHeaders[0], "x-ms-meta-data*");
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules[0].AllowedMethods.size(), 1);
    EXPECT_EQ(
        setResponse.Value.Properties.Cors.CorsRules[0].AllowedMethods[0],
        Tables::AllowedMethods::Get);
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules[0].AllowedOrigins.size(), 1);
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules[0].AllowedOrigins[0], "234");
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules[0].ExposedHeaders.size(), 1);
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules[0].ExposedHeaders[0], "x-ms-meta-*");
    EXPECT_EQ(setResponse.Value.Properties.Cors.CorsRules[0].MaxAgeInSeconds, 100);

   Azure::Storage::Tables::ListOptions list;
    list.ResourceGroupName = GetEnv("STORAGE_RESOURCE_GROUP");
    list.AccountName = GetEnv("TABLES_STORAGE_ACCOUNT_NAME");
    auto listResponse = m_tableServiceClient->List(list);

    EXPECT_EQ(listResponse.Value.Value.size(), 1);
    EXPECT_EQ(listResponse.Value.Value[0].Properties.Cors.CorsRules.size(), 1);
    EXPECT_EQ(listResponse.Value.Value[0].Properties.Cors.CorsRules[0].AllowedHeaders.size(), 1);
    EXPECT_EQ(
        listResponse.Value.Value[0].Properties.Cors.CorsRules[0].AllowedHeaders[0],
        "x-ms-meta-data*");
    EXPECT_EQ(listResponse.Value.Value[0].Properties.Cors.CorsRules[0].AllowedMethods.size(), 1);
    EXPECT_EQ(
        listResponse.Value.Value[0].Properties.Cors.CorsRules[0].AllowedMethods[0],
        Tables::AllowedMethods::Get);
    EXPECT_EQ(listResponse.Value.Value[0].Properties.Cors.CorsRules[0].AllowedOrigins.size(), 1);
    EXPECT_EQ(listResponse.Value.Value[0].Properties.Cors.CorsRules[0].AllowedOrigins[0], "234");
    EXPECT_EQ(listResponse.Value.Value[0].Properties.Cors.CorsRules[0].ExposedHeaders.size(), 1);
    EXPECT_EQ(
        listResponse.Value.Value[0].Properties.Cors.CorsRules[0].ExposedHeaders[0], "x-ms-meta-*");
    EXPECT_EQ(listResponse.Value.Value[0].Properties.Cors.CorsRules[0].MaxAgeInSeconds, 100);
  }
}}} // namespace Azure::Storage::Test
