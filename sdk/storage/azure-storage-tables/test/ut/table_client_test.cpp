// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "table_client_test.hpp"

#include <chrono>
#include <thread>

namespace Azure { namespace Storage { namespace Test {
  std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_credential;
  void TablesClientTest::SetUp()
  {
    Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);
    StorageTest::SetUp();
    if (shouldSkipTest())
    {
      return;
    }
    if (m_tableServiceClient.get() == nullptr)
    {
      auto clientOptions = InitStorageClientOptions<Tables::TableClientOptions>();

      m_credential = CreateClientSecretCredential(
          GetEnv("STORAGE_TENANT_ID"),
          GetEnv("STORAGE_CLIENT_ID"),
          GetEnv("STORAGE_CLIENT_SECRET"));

      m_tableServiceClient = std::make_shared<Tables::TableServicesClient>(
          Tables::TableServicesClient::
              CreateFromConnectionString( //
                                          // Azure::Storage::Tables::TableServicesClient(

                  GetEnv("STANDARD_STORAGE_CONNECTION_STRING"),
                  GetEnv("STORAGE_SUBSCRIPTION_ID"), // m_credential,
                  // Azure::Storage::_internal::TablesManagementPublicEndpoint,
                  clientOptions));
      auto tableClientOptions = InitStorageClientOptions<Tables::TableClientOptions>();
      m_tableClient
          = std::make_shared<Tables::TableClient>(CreateTableClientForTest(clientOptions));
    }
  }

  Azure::Storage::Tables::TableClient TablesClientTest::CreateTableClientForTest(
      Tables::TableClientOptions& clientOptions)
  {

    m_tableName = GetTestNameLowerCase() + LowercaseRandomString(10);
    auto tableClient = Tables::TableClient::CreateFromConnectionString(
        GetEnv("STANDARD_STORAGE_CONNECTION_STRING"), m_tableName, clientOptions);
    return tableClient;
  }

  TEST_F(TablesClientTest, ClientConstructor) { EXPECT_FALSE(m_tableClient == nullptr); }

  TEST_F(TablesClientTest, CreateTable)
  {
    auto createResponse = m_tableClient->Create();
    EXPECT_EQ(createResponse.Value.TableName, m_tableName);
    EXPECT_EQ(createResponse.Value.EditLink, "Tables('" + m_tableName + "')");
    EXPECT_TRUE(createResponse.Value.Type.find(".Tables") != std::string::npos);
    EXPECT_TRUE(createResponse.Value.Id.find(m_tableName) != std::string::npos);
  }

  TEST_F(TablesClientTest, GetAccessPolicy)
  {
    auto createResponse = m_tableClient->Create();

    auto getResponse = m_tableClient->GetAccessPolicy();
    EXPECT_EQ(getResponse.Value.SignedIdentifiers.size(), 0);
  }

  TEST_F(TablesClientTest, SetAccessPolicy)
  {
    auto createResponse = m_tableClient->Create();
    Azure::Storage::Tables::Models::TableAccessPolicy newPolicy{};
    Azure::Storage::Tables::Models::SignedIdentifier newIdentifier{};
    newIdentifier.Id = "testid";
    newIdentifier.Permissions = "r";
    newIdentifier.StartsOn = Azure::DateTime::Parse(
        Azure::DateTime(std::chrono::system_clock::now())
            .ToString(Azure::DateTime::DateFormat::Rfc1123),
        Azure::DateTime::DateFormat::Rfc1123);
    newIdentifier.ExpiresOn = Azure::DateTime::Parse(
        Azure::DateTime(std::chrono::system_clock::now() + std::chrono::seconds(60))
            .ToString(Azure::DateTime::DateFormat::Rfc1123),
        Azure::DateTime::DateFormat::Rfc1123);
    newPolicy.SignedIdentifiers.emplace_back(newIdentifier);

    m_tableClient->SetAccessPolicy(newPolicy);
    if (GetEnv("AZURE_TEST_MODE") != "PLAYBACK")
    {
      // setting policy takes up to 30 seconds to take effect
      std::this_thread::sleep_for(std::chrono::milliseconds(30001));
    }

    auto getResponse = m_tableClient->GetAccessPolicy();

    EXPECT_EQ(getResponse.Value.SignedIdentifiers.size(), 1);
    EXPECT_EQ(getResponse.Value.SignedIdentifiers[0].Id, newIdentifier.Id);
    EXPECT_EQ(getResponse.Value.SignedIdentifiers[0].Permissions, newIdentifier.Permissions);
  }

  TEST_F(TablesClientTest, ListTables)
  {
    auto createResponse = m_tableClient->Create();

    Azure::Storage::Tables::Models::ListTablesOptions listOptions;

    auto listResponse = m_tableServiceClient->ListTables(listOptions);

    for (auto table : listResponse.Tables)
    {
      if (table.TableName == m_tableName)
      {
        EXPECT_EQ(table.TableName, m_tableName);
        EXPECT_EQ(table.EditLink, "Tables('" + m_tableName + "')");
        EXPECT_TRUE(table.Type.find(".Tables") != std::string::npos);
        EXPECT_TRUE(table.Id.find(m_tableName) != std::string::npos);
      }
    }
  }

  TEST_F(TablesClientTest, DeleteTable)
  {
    auto createResponse = m_tableClient->Create();

    auto response = m_tableClient->Delete();
    EXPECT_EQ(response.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
  }

  TEST_F(TablesClientTest, ServiceClientConstructors)
  {
    EXPECT_FALSE(m_tableServiceClient == nullptr);
  }

  TEST_F(TablesClientTest, ServiceClientGetProperties)
  {
    Azure::Storage::Tables::Models::GetServicePropertiesOptions getOptions;

    auto response = m_tableServiceClient->GetServiceProperties(getOptions);
    EXPECT_EQ(response.Value.Logging.RetentionPolicy.IsEnabled, false);
    EXPECT_EQ(response.Value.Logging.Version, "1.0");
    EXPECT_EQ(response.Value.Logging.Delete, false);
    EXPECT_EQ(response.Value.HourMetrics.RetentionPolicy.IsEnabled, true);
    EXPECT_EQ(response.Value.HourMetrics.Version, "1.0");
    EXPECT_EQ(response.Value.HourMetrics.IsEnabled, true);
    EXPECT_EQ(response.Value.HourMetrics.IncludeApis.Value(), true);
    EXPECT_EQ(response.Value.MinuteMetrics.RetentionPolicy.IsEnabled, false);
    EXPECT_EQ(response.Value.MinuteMetrics.Version, "1.0");
    EXPECT_EQ(response.Value.MinuteMetrics.IsEnabled, false);
  }

  TEST_F(TablesClientTest, ServiceClientSet)
  {
    Azure::Storage::Tables::Models::GetServicePropertiesOptions getOptions;

    auto response = m_tableServiceClient->GetServiceProperties(getOptions);

    Azure::Storage::Tables::Models::SetServicePropertiesOptions setOptions;
    setOptions.TableServiceProperties = std::move(response.Value);
    auto response2 = m_tableServiceClient->SetServiceProperties(setOptions);
    EXPECT_EQ(response2.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::Accepted);
  }

  TEST_F(TablesClientTest, ServiceClientStatistics)
  {
    Azure::Storage::Tables::Models::GetServiceStatisticsOptions statsOptions;

    auto response = m_tableServiceClient->GetStatistics(statsOptions);

    EXPECT_EQ(response.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::Ok);
    EXPECT_EQ(response.Value.GeoReplication.Status.ToString(), "live");
  }

  TEST_F(TablesClientTest, EntityCreate)
  {
    Azure::Storage::Tables::Models::TableEntity entity;

    entity.PartitionKey = "P1";
    entity.RowKey = "R1";
    entity.Properties["Name"] = "Azure";
    entity.Properties["Product"] = "Tables";
    auto createResponse = m_tableClient->Create();
    auto response = m_tableClient->CreateEntity(entity);

    EXPECT_EQ(response.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(response.Value.ETag.empty());
  }

  TEST_F(TablesClientTest, EntityUpdate)
  {
    Azure::Storage::Tables::Models::TableEntity entity;

    entity.PartitionKey = "P1";
    entity.RowKey = "R1";
    entity.Properties["Name"] = "Azure";
    entity.Properties["Product"] = "Tables";
    auto createResponse = m_tableClient->Create();
    auto response = m_tableClient->CreateEntity(entity);
    EXPECT_EQ(response.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(response.Value.ETag.empty());

    entity.Properties["Product"] = "Tables2";
    auto updateResponse = m_tableClient->UpdateEntity(entity);

    EXPECT_EQ(
        updateResponse.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(updateResponse.Value.ETag.empty());

    entity.Properties["Product"] = "Tables3";
    entity.ETag = updateResponse.Value.ETag;
    auto updateResponse2 = m_tableClient->UpdateEntity(entity);

    EXPECT_EQ(
        updateResponse2.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(updateResponse2.Value.ETag.empty());
  }

  TEST_F(TablesClientTest, EntityMerge)
  {
    Azure::Storage::Tables::Models::TableEntity entity;

    entity.PartitionKey = "P1";
    entity.RowKey = "R1";
    entity.Properties["Name"] = "Azure";
    entity.Properties["Product"] = "Tables";
    auto createResponse = m_tableClient->Create();
    auto response = m_tableClient->CreateEntity(entity);
    EXPECT_EQ(response.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(response.Value.ETag.empty());

    entity.Properties["Product2"] = "Tables2";
    auto updateResponse = m_tableClient->MergeEntity(entity);

    EXPECT_EQ(
        updateResponse.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(updateResponse.Value.ETag.empty());

    entity.Properties["Product3"] = "Tables3";
    entity.ETag = updateResponse.Value.ETag;
    auto updateResponse2 = m_tableClient->MergeEntity(entity);

    EXPECT_EQ(
        updateResponse2.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(updateResponse2.Value.ETag.empty());
  }

  TEST_F(TablesClientTest, EntityDelete)
  {
    Azure::Storage::Tables::Models::TableEntity entity;

    entity.PartitionKey = "P1";
    entity.RowKey = "R1";
    entity.Properties["Name"] = "Azure";
    entity.Properties["Product"] = "Tables";
    auto createResponse = m_tableClient->Create();
    auto response = m_tableClient->CreateEntity(entity);
    EXPECT_EQ(response.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(response.Value.ETag.empty());

    entity.Properties["Product2"] = "Tables2";
    auto updateResponse = m_tableClient->DeleteEntity(entity);

    EXPECT_EQ(
        updateResponse.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);

    response = m_tableClient->CreateEntity(entity);
    EXPECT_EQ(response.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(response.Value.ETag.empty());
    entity.Properties["Product3"] = "Tables3";
    entity.ETag = response.Value.ETag;
    auto updateResponse2 = m_tableClient->DeleteEntity(entity);

    EXPECT_EQ(
        updateResponse2.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
  }

  TEST_F(TablesClientTest, EntityUpsert)
  {
    Azure::Storage::Tables::Models::TableEntity entity;

    entity.PartitionKey = "P1";
    entity.RowKey = "R1";
    entity.Properties["Name"] = "Azure";
    entity.Properties["Product"] = "Tables";
    auto createResponse = m_tableClient->Create();
    auto response = m_tableClient->UpsertEntity(entity);
    EXPECT_EQ(response.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(response.Value.ETag.empty());

    Azure::Storage::Tables::Models::UpsertEntityOptions options;
    options.UpsertType = Azure::Storage::Tables::Models::UpsertType::Update;

    entity.Properties["Product"] = "Tables2";
    auto updateResponse = m_tableClient->MergeEntity(entity, options);

    EXPECT_EQ(
        updateResponse.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(updateResponse.Value.ETag.empty());

    Azure::Storage::Tables::Models::UpsertEntityOptions options2;
    options2.UpsertType = Azure::Storage::Tables::Models::UpsertType::Merge;
    entity.Properties["Product3"] = "Tables3";
    entity.ETag = updateResponse.Value.ETag;
    auto updateResponse2 = m_tableClient->MergeEntity(entity, options2);

    EXPECT_EQ(
        updateResponse2.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(updateResponse2.Value.ETag.empty());
  }

  TEST_F(TablesClientTest, EntityQuery)
  {
    Azure::Storage::Tables::Models::TableEntity entity;

    entity.PartitionKey = "P1";
    entity.RowKey = "R1";
    entity.Properties["Name"] = "Azure";
    entity.Properties["Product"] = "Tables";
    auto createResponse = m_tableClient->Create();
    auto response = m_tableClient->CreateEntity(entity);
    EXPECT_EQ(response.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(response.Value.ETag.empty());

    entity.Properties["Product"] = "Tables2";
    entity.RowKey = "R2";
    m_tableClient->CreateEntity(entity);

    entity.Properties["Product"] = "Tables3";
    entity.RowKey = "R3";
    m_tableClient->CreateEntity(entity);

    Azure::Storage::Tables::Models::QueryEntitiesOptions options;

    auto responseQuery = m_tableClient->QueryEntities(options);
    EXPECT_EQ(responseQuery.TableEntities.size(), 3);

    options.PartitionKey = "P1";
    options.RowKey = "R1";
    responseQuery = m_tableClient->QueryEntities(options);
    EXPECT_EQ(responseQuery.TableEntities.size(), 1);

    options.SelectColumns = "Name,Product";
    responseQuery = m_tableClient->QueryEntities(options);
    EXPECT_EQ(responseQuery.TableEntities.size(), 1);
  }

  TEST_F(TablesClientTest, TransactionCreateFail)
  {
    Azure::Storage::Tables::Models::TableEntity entity;
    Azure::Storage::Tables::Models::TableEntity entity2;
    entity.PartitionKey = "P1";
    entity.RowKey = "R1";
    entity.Properties["Name"] = "Azure";
    entity.Properties["Product"] = "Tables";
    entity2.PartitionKey = "P1";
    entity2.RowKey = "R1";
    entity2.Properties["Name"] = "Azure";
    entity2.Properties["Product"] = "Tables";
    auto createResponse = m_tableClient->Create();
    auto transaction = m_tableClient->CreateTransaction("P1");

    transaction.CreateEntity(entity);
    transaction.CreateEntity(entity2);

    auto response = m_tableClient->SubmitTransaction(transaction);
    EXPECT_TRUE(response.Value.Error.HasValue());
  }

   TEST_F(TablesClientTest, TransactionCreateOK)
  {
    Azure::Storage::Tables::Models::TableEntity entity;
    Azure::Storage::Tables::Models::TableEntity entity2;
    entity.PartitionKey = "P1";
    entity.RowKey = "R1";
    entity.Properties["Name"] = "Azure";
    entity.Properties["Product"] = "Tables";
    entity2.PartitionKey = "P1";
    entity2.RowKey = "R2";
    entity2.Properties["Name"] = "Azure";
    entity2.Properties["Product"] = "Tables";
    auto createResponse = m_tableClient->Create();
    auto transaction = m_tableClient->CreateTransaction("P1");

    transaction.CreateEntity(entity);
    transaction.CreateEntity(entity2);

    auto response = m_tableClient->SubmitTransaction(transaction);
    EXPECT_FALSE(response.Value.Error.HasValue());
  }

  TEST_F(TablesClientTest, TransactionDelete)
  {
    Azure::Storage::Tables::Models::TableEntity entity;
    Azure::Storage::Tables::Models::TableEntity entity2;
    entity.PartitionKey = "P1";
    entity.RowKey = "R1";
    entity.Properties["Name"] = "Azure";
    entity.Properties["Product"] = "Tables";
    entity2.PartitionKey = "P1";
    entity2.RowKey = "R2";
    entity2.Properties["Name"] = "Azure";
    entity2.Properties["Product"] = "Tables";
    auto createResponse = m_tableClient->Create();
    auto transaction = m_tableClient->CreateTransaction("P1");

    transaction.CreateEntity(entity);
    transaction.CreateEntity(entity2);

    auto response = m_tableClient->SubmitTransaction(transaction);

    auto transaction2 = m_tableClient->CreateTransaction("P1");

    transaction2.DeleteEntity(entity);

    response = m_tableClient->SubmitTransaction(transaction2);
  }

  TEST_F(TablesClientTest, TransactionMerge)
  {
    Azure::Storage::Tables::Models::TableEntity entity;
    Azure::Storage::Tables::Models::TableEntity entity2;
    entity.PartitionKey = "P1";
    entity.RowKey = "R1";
    entity.Properties["Name"] = "Azure";
    entity.Properties["Product"] = "Tables";
    entity2.PartitionKey = "P1";
    entity2.RowKey = "R1";
    entity2.Properties["Name"] = "Azure2";
    entity2.Properties["Product"] = "Tables3";
    auto createResponse = m_tableClient->Create();
    auto transaction = m_tableClient->CreateTransaction("P1");

    transaction.CreateEntity(entity);

    auto response = m_tableClient->SubmitTransaction(transaction);

    auto transaction2 = m_tableClient->CreateTransaction("P1");

    transaction2.MergeEntity(entity2);

    response = m_tableClient->SubmitTransaction(transaction2);
  }

  TEST_F(TablesClientTest, TransactionUpdate)
  {
    Azure::Storage::Tables::Models::TableEntity entity;
    Azure::Storage::Tables::Models::TableEntity entity2;
    entity.PartitionKey = "P1";
    entity.RowKey = "R1";
    entity.Properties["Name"] = "Azure";
    entity.Properties["Product"] = "Tables";
    entity2.PartitionKey = "P1";
    entity2.RowKey = "R1";
    entity2.Properties["Name"] = "Azure2";
    entity2.Properties["Product"] = "Tables3";
    auto createResponse = m_tableClient->Create();
    auto transaction = m_tableClient->CreateTransaction("P1");

    transaction.CreateEntity(entity);

    auto response = m_tableClient->SubmitTransaction(transaction);

    auto transaction2 = m_tableClient->CreateTransaction("P1");

    transaction2.UpdateEntity(entity2);

    response = m_tableClient->SubmitTransaction(transaction2);
  }
}}} // namespace Azure::Storage::Test