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
      auto tableClientOptions = InitStorageClientOptions<Tables::TableClientOptions>();
      m_tableClient
          = std::make_shared<Tables::TableClient>(CreateTableClientForTest(tableClientOptions));
    }
  }

  Azure::Storage::Tables::TableClient TablesClientTest::CreateTableClientForTest(
      Tables::TableClientOptions clientOptions)
  {

    Tables::TableClientOptions tableOptions;
    m_tableName = GetTestNameLowerCase() + LowercaseRandomString(10);
    tableOptions.EnableTenantDiscovery = true;

    auto tableClient
        = Tables::TableClient(GetEnv("STORAGE_SUBSCRIPTION_ID"), m_credential, clientOptions);
    return tableClient;
  }

  TEST_F(TablesClientTest, ClientConstructor) { EXPECT_FALSE(m_tableClient == nullptr); }

  TEST_F(TablesClientTest, CreateTable)
  {
    Tables::CreateOptions createOptions;
    createOptions.ResourceGroupName = GetEnv("STORAGE_RESOURCE_GROUP");
    createOptions.AccountName = GetEnv("TABLES_STORAGE_ACCOUNT_NAME");
    createOptions.TableName = m_tableName;

    auto createResponse = m_tableClient->Create(createOptions);
    EXPECT_EQ(createResponse.Value.Properties.TableName, m_tableName);
  }

  TEST_F(TablesClientTest, GetTable)
  {
    Tables::CreateOptions createOptions;
    createOptions.ResourceGroupName = GetEnv("STORAGE_RESOURCE_GROUP");
    createOptions.AccountName = GetEnv("TABLES_STORAGE_ACCOUNT_NAME");
    createOptions.TableName = m_tableName;

    auto createResponse = m_tableClient->Create(createOptions);
    EXPECT_EQ(createResponse.Value.Properties.TableName, m_tableName);

    Tables::GetOptions getOptions;
    getOptions.ResourceGroupName = GetEnv("STORAGE_RESOURCE_GROUP");
    getOptions.AccountName = GetEnv("TABLES_STORAGE_ACCOUNT_NAME");
    getOptions.TableName = m_tableName;

    auto getResponse = m_tableClient->Get(getOptions);
    EXPECT_EQ(getResponse.Value.Properties.TableName, m_tableName);
  }

  TEST_F(TablesClientTest, UpdateTable)
  {
    Tables::CreateOptions createOptions;
    createOptions.ResourceGroupName = GetEnv("STORAGE_RESOURCE_GROUP");
    createOptions.AccountName = GetEnv("TABLES_STORAGE_ACCOUNT_NAME");
    createOptions.TableName = m_tableName;

    auto createResponse = m_tableClient->Create(createOptions);
    EXPECT_EQ(createResponse.Value.Properties.TableName, m_tableName);

    Tables::GetOptions getOptions;
    getOptions.ResourceGroupName = GetEnv("STORAGE_RESOURCE_GROUP");
    getOptions.AccountName = GetEnv("TABLES_STORAGE_ACCOUNT_NAME");
    getOptions.TableName = m_tableName;

    auto getResponse = m_tableClient->Get(getOptions);
    EXPECT_EQ(getResponse.Value.Properties.TableName, m_tableName);

    Tables::UpdateOptions updateOptions;
	updateOptions.ResourceGroupName = GetEnv("STORAGE_RESOURCE_GROUP");
	updateOptions.AccountName = GetEnv("TABLES_STORAGE_ACCOUNT_NAME");
	updateOptions.TableName = m_tableName;

	auto updateResponse = m_tableClient->Update(updateOptions);
	EXPECT_EQ(updateResponse.Value.Properties.TableName, m_tableName);
  }

  TEST_F(TablesClientTest, ListTables)
  {
    Tables::CreateOptions createOptions;
    createOptions.ResourceGroupName = GetEnv("STORAGE_RESOURCE_GROUP");
    createOptions.AccountName = GetEnv("TABLES_STORAGE_ACCOUNT_NAME");
    createOptions.TableName = m_tableName;

    auto createResponse = m_tableClient->Create(createOptions);
    EXPECT_EQ(createResponse.Value.Properties.TableName, m_tableName);

    Tables::ListOptions listOptions;
    listOptions.ResourceGroupName = GetEnv("STORAGE_RESOURCE_GROUP");
    listOptions.AccountName = GetEnv("TABLES_STORAGE_ACCOUNT_NAME");

    auto listResponse = m_tableClient->List(listOptions);
    bool found = false;
    for (auto table : listResponse.Value.Value)
    {
      if (table.Properties.TableName == m_tableName)
      {
        found = true;
        break;
      }
    }

    EXPECT_TRUE(found);
  }

  TEST_F(TablesClientTest, DeleteTable)
  {
    Tables::CreateOptions createOptions;
    createOptions.ResourceGroupName = GetEnv("STORAGE_RESOURCE_GROUP");
    createOptions.AccountName = GetEnv("TABLES_STORAGE_ACCOUNT_NAME");
    createOptions.TableName = m_tableName;

    auto createResponse = m_tableClient->Create(createOptions);
    EXPECT_EQ(createResponse.Value.Properties.TableName, m_tableName);

    Tables::ListOptions listOptions;
    listOptions.ResourceGroupName = GetEnv("STORAGE_RESOURCE_GROUP");
    listOptions.AccountName = GetEnv("TABLES_STORAGE_ACCOUNT_NAME");

    auto listResponse = m_tableClient->List(listOptions);
    bool found = false;
    for (auto table : listResponse.Value.Value)
    {
      if (table.Properties.TableName == m_tableName)
      {
        found = true;
        break;
      }
    }

    Tables::DeleteOptions deleteOptions;
    deleteOptions.ResourceGroupName = GetEnv("STORAGE_RESOURCE_GROUP");
    deleteOptions.AccountName = GetEnv("TABLES_STORAGE_ACCOUNT_NAME");
    deleteOptions.TableName = m_tableName;

    m_tableClient->Delete(deleteOptions);
    EXPECT_TRUE(found);

    listResponse = m_tableClient->List(listOptions);
    found = false;
    for (auto table : listResponse.Value.Value)
    {
      if (table.Properties.TableName == m_tableName)
      {
        found = true;
        break;
      }
    }
    EXPECT_FALSE(found);
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
