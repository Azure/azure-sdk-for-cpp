// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "table_client_test.hpp"

#include <azure/core/internal/strings.hpp>

#include <chrono>
#include <cstdlib>
#include <ctime>
// useful for debugging to avoid table conflicts when creating tables
// as it takes a while from then a table is deleted to when it can be recreated
// #define RANDOM_TABLE_NAME

#ifdef RANDOM_TABLE_NAME
#include <iostream>
#include <string>
#include <thread>
#endif

using namespace Azure::Data;
using namespace Azure::Data::Tables::Models;

namespace Azure { namespace Data { namespace Test {
  std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_credential;
  void TablesClientTest::SetUp()
  {
    auto param = GetParam();

    Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);
    StorageTest::SetUp();
    if (shouldSkipTest())
    {
      return;
    }
    if (m_tableServiceClient.get() == nullptr)
    {
      auto clientOptions = InitStorageClientOptions<Tables::TableClientOptions>();
      auto tableClientOptions = InitStorageClientOptions<Tables::TableClientOptions>();
      m_tableName = GetTestNameLowerCase();

#ifdef RANDOM_TABLE_NAME
      srand(static_cast<unsigned>(time(nullptr)));
      int random_number = rand() % 1000 + 1;
      m_tableName = m_tableName + std::to_string(random_number);
#endif

      std::replace(m_tableName.begin(), m_tableName.end(), '-', '0');
      switch (param)
      {
        case AuthType::Key:
          m_credential = GetTestCredential();
          m_tableServiceClient = std::make_shared<Tables::TableServiceClient>(
              Azure::Data::Tables::TableServiceClient(
                  "https://" + GetAccountName() + ".table.core.windows.net/",
                  m_credential,
                  clientOptions));
          m_tableClient = std::make_shared<Tables::TableClient>(Tables::TableClient(
              "https://" + GetAccountName() + ".table.core.windows.net/",
              m_tableName,
              m_credential,
              tableClientOptions));
          break;
      }
    }
  }

  void TablesClientTest::TearDown()
  {
    if (!m_tableName.empty())
    {
      try
      {
        auto deleteResponse = m_tableServiceClient->DeleteTable(m_tableName);
      }
      catch (...)
      {
      }
    }
    StorageTest::TearDown();
  }

  TEST_P(TablesClientTest, ClientConstructor) { EXPECT_FALSE(m_tableClient == nullptr); }

  TEST_P(TablesClientTest, CreateTable)
  {
    auto createResponse = m_tableServiceClient->CreateTable(m_tableName);
    EXPECT_EQ(createResponse.Value.TableName, m_tableName);
    EXPECT_EQ(createResponse.Value.EditLink, "Tables('" + m_tableName + "')");
    EXPECT_TRUE(createResponse.Value.Type.find(".Tables") != std::string::npos);
    EXPECT_TRUE(createResponse.Value.Id.find(m_tableName) != std::string::npos);
  }

  TEST_P(TablesClientTest, CreateTableFail)
  {
    try
    {
      auto createResponse = m_tableServiceClient->CreateTable("+++");
    }
    catch (Azure::Core::RequestFailedException& e)
    {
      EXPECT_EQ(e.StatusCode, Azure::Core::Http::HttpStatusCode::BadRequest);
    }
  }

  TEST_P(TablesClientTest, ListTables)
  {
    auto createResponse = m_tableServiceClient->CreateTable(m_tableName);

    Azure::Data::Tables::Models::QueryTablesOptions listOptions;

    auto listResponse = m_tableServiceClient->QueryTables(listOptions);

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

  TEST_P(TablesClientTest, DeleteTable)
  {
    auto createResponse = m_tableServiceClient->CreateTable(m_tableName);

    auto response = m_tableServiceClient->DeleteTable(m_tableName);
    EXPECT_EQ(response.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
  }

  TEST_P(TablesClientTest, DeleteTableFail)
  {
    try
    {
      auto response = m_tableServiceClient->DeleteTable(m_tableName);
    }
    catch (Azure::Core::RequestFailedException& e)
    {
      EXPECT_EQ(e.StatusCode, Azure::Core::Http::HttpStatusCode::NotFound);
    }
  }

  TEST_P(TablesClientTest, ServiceClientConstructors)
  {
    EXPECT_FALSE(m_tableServiceClient == nullptr);
  }

  TEST_P(TablesClientTest, ServiceClientGetProperties)
  {
    auto response = m_tableServiceClient->GetServiceProperties();
    EXPECT_EQ(response.Value.Logging.RetentionPolicyDefinition.IsEnabled, false);
    EXPECT_EQ(response.Value.Logging.Version, "1.0");
    EXPECT_EQ(response.Value.Logging.Delete, false);
    EXPECT_EQ(response.Value.HourMetrics.RetentionPolicyDefinition.IsEnabled, true);
    EXPECT_EQ(response.Value.HourMetrics.Version, "1.0");
    EXPECT_EQ(response.Value.HourMetrics.IsEnabled, true);
    EXPECT_EQ(response.Value.HourMetrics.IncludeApis.Value(), true);
    EXPECT_EQ(response.Value.MinuteMetrics.RetentionPolicyDefinition.IsEnabled, false);
    EXPECT_EQ(response.Value.MinuteMetrics.Version, "1.0");
    EXPECT_EQ(response.Value.MinuteMetrics.IsEnabled, false);
  }

  TEST_P(TablesClientTest, ServiceClientSet)
  {
    auto response = m_tableServiceClient->GetServiceProperties();

    Azure::Data::Tables::Models::SetServicePropertiesOptions setOptions;
    setOptions.ServiceProperties = std::move(response.Value);
    auto response2 = m_tableServiceClient->SetServiceProperties(setOptions);
    EXPECT_EQ(response2.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::Accepted);
  }

  TEST_P(TablesClientTest, ServiceClientStatistics)
  {
    auto response = m_tableServiceClient->GetStatistics();

    EXPECT_EQ(response.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::Ok);
    EXPECT_EQ(response.Value.GeoReplication.Status.ToString(), "live");
  }

  TEST_P(TablesClientTest, EntityCreate)
  {
    Azure::Data::Tables::Models::TableEntity entity;

    entity.SetPartitionKey("P1");
    entity.SetRowKey("R1");
    entity.Properties["Name"] = TableEntityProperty("Azure");
    entity.Properties["Product"] = TableEntityProperty("Tables");
    auto createResponse = m_tableServiceClient->CreateTable(m_tableName);
    auto response = m_tableClient->AddEntity(entity);

    EXPECT_EQ(response.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(response.Value.ETag.empty());
  }

  TEST_P(TablesClientTest, EntityCreateFail)
  {
    Azure::Data::Tables::Models::TableEntity entity;

    entity.SetPartitionKey("P1");
    entity.SetRowKey("R1");
    entity.Properties["Name"] = TableEntityProperty("Azure");
    entity.Properties["Product"] = TableEntityProperty("Tables");
    auto createResponse = m_tableServiceClient->CreateTable(m_tableName);
    {
      auto response = m_tableClient->AddEntity(entity);

      EXPECT_EQ(
          response.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
      EXPECT_FALSE(response.Value.ETag.empty());
    }
    {
      try
      {
        auto response = m_tableClient->AddEntity(entity);
      }
      catch (Azure::Core::RequestFailedException& e)
      {
        EXPECT_EQ(e.StatusCode, Azure::Core::Http::HttpStatusCode::Conflict);
      }
    }
  }

  TEST_P(TablesClientTest, EntityUpdate)
  {
    Azure::Data::Tables::Models::TableEntity entity;

    entity.SetPartitionKey("P1");
    entity.SetRowKey("R1");
    entity.Properties["Name"] = TableEntityProperty("Azure");
    entity.Properties["Product"] = TableEntityProperty("Tables");
    auto createResponse = m_tableServiceClient->CreateTable(m_tableName);
    auto response = m_tableClient->AddEntity(entity);
    EXPECT_EQ(response.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(response.Value.ETag.empty());

    entity.Properties["Product"] = TableEntityProperty("Tables2");
    auto updateResponse = m_tableClient->UpdateEntity(entity);

    EXPECT_EQ(
        updateResponse.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(updateResponse.Value.ETag.empty());

    entity.Properties["Product"] = TableEntityProperty("Tables3");
    entity.SetETag(updateResponse.Value.ETag);
    auto updateResponse2 = m_tableClient->UpdateEntity(entity);

    EXPECT_EQ(
        updateResponse2.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(updateResponse2.Value.ETag.empty());
  }

  TEST_P(TablesClientTest, EntityMerge)
  {
    Azure::Data::Tables::Models::TableEntity entity;

    entity.SetPartitionKey("P1");
    entity.SetRowKey("R1");
    entity.Properties["Name"] = TableEntityProperty("Azure");
    entity.Properties["Product"] = TableEntityProperty("Tables");
    auto createResponse = m_tableServiceClient->CreateTable(m_tableName);
    auto response = m_tableClient->AddEntity(entity);
    EXPECT_EQ(response.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(response.Value.ETag.empty());

    entity.Properties["Product2"] = TableEntityProperty("Tables2");
    auto updateResponse = m_tableClient->MergeEntity(entity);

    EXPECT_EQ(
        updateResponse.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(updateResponse.Value.ETag.empty());

    entity.Properties["Product3"] = TableEntityProperty("Tables3");
    entity.SetETag(updateResponse.Value.ETag);
    auto updateResponse2 = m_tableClient->MergeEntity(entity);

    EXPECT_EQ(
        updateResponse2.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(updateResponse2.Value.ETag.empty());
  }

  TEST_P(TablesClientTest, EntityDelete)
  {
    Azure::Data::Tables::Models::TableEntity entity;

    entity.SetPartitionKey("P1");
    entity.SetRowKey("R1");
    entity.Properties["Name"] = TableEntityProperty("Azure");
    entity.Properties["Product"] = TableEntityProperty("Tables");
    auto createResponse = m_tableServiceClient->CreateTable(m_tableName);
    auto response = m_tableClient->AddEntity(entity);
    EXPECT_EQ(response.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(response.Value.ETag.empty());

    entity.Properties["Product2"] = TableEntityProperty("Tables2");
    auto updateResponse = m_tableClient->DeleteEntity(entity);

    EXPECT_EQ(
        updateResponse.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);

    response = m_tableClient->AddEntity(entity);
    EXPECT_EQ(response.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(response.Value.ETag.empty());
    entity.Properties["Product3"] = TableEntityProperty("Tables3");
    entity.SetETag(response.Value.ETag);
    auto updateResponse2 = m_tableClient->DeleteEntity(entity);

    EXPECT_EQ(
        updateResponse2.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
  }

  TEST_P(TablesClientTest, EntityDeleteFail)
  {
    Azure::Data::Tables::Models::TableEntity entity;

    entity.SetPartitionKey("P1");
    entity.SetRowKey("R1");
    entity.Properties["Name"] = TableEntityProperty("Azure");
    entity.Properties["Product"] = TableEntityProperty("Tables");
    auto createResponse = m_tableServiceClient->CreateTable(m_tableName);
    try
    {
      auto updateResponse2 = m_tableClient->DeleteEntity(entity);
    }
    catch (Azure::Core::RequestFailedException& e)
    {
      EXPECT_EQ(e.StatusCode, Azure::Core::Http::HttpStatusCode::NotFound);
    }
  }

  TEST_P(TablesClientTest, EntityUpsert)
  {
    Azure::Data::Tables::Models::TableEntity entity;

    entity.SetPartitionKey("P1");
    entity.SetRowKey("R1");
    entity.Properties["Name"] = TableEntityProperty("Azure");
    entity.Properties["Product"] = TableEntityProperty("Tables");
    auto createResponse = m_tableServiceClient->CreateTable(m_tableName);
    auto response = m_tableClient->UpsertEntity(entity);
    EXPECT_EQ(response.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(response.Value.ETag.empty());

    Azure::Data::Tables::Models::UpsertEntityOptions options;
    options.UpsertType = Azure::Data::Tables::Models::UpsertKind::Update;

    entity.Properties["Product"] = TableEntityProperty("Tables2");
    auto updateResponse = m_tableClient->MergeEntity(entity);

    EXPECT_EQ(
        updateResponse.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(updateResponse.Value.ETag.empty());

    Azure::Data::Tables::Models::UpsertEntityOptions options2;
    options2.UpsertType = Azure::Data::Tables::Models::UpsertKind::Merge;
    entity.Properties["Product3"] = TableEntityProperty("Tables3");
    entity.SetETag(updateResponse.Value.ETag);
    auto updateResponse2 = m_tableClient->MergeEntity(entity);

    EXPECT_EQ(
        updateResponse2.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(updateResponse2.Value.ETag.empty());
  }

  TEST_P(TablesClientTest, EntityQuery)
  {
    Azure::Data::Tables::Models::TableEntity entity;

    entity.SetPartitionKey("P1");
    entity.SetRowKey("R1");
    entity.Properties["Name"] = TableEntityProperty("Azure");
    entity.Properties["Product"] = TableEntityProperty("Tables");
    auto createResponse = m_tableServiceClient->CreateTable(m_tableName);
    auto response = m_tableClient->AddEntity(entity);
    EXPECT_EQ(response.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(response.Value.ETag.empty());

    entity.Properties["Product"] = TableEntityProperty("Tables2");
    entity.SetRowKey("R2");
    m_tableClient->AddEntity(entity);

    entity.Properties["Product"] = TableEntityProperty("Tables3");
    entity.SetRowKey("R3");
    m_tableClient->AddEntity(entity);

    Azure::Data::Tables::Models::QueryEntitiesOptions options;

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

  TEST_P(TablesClientTest, QueryEntityPagedResponse_LIVEONLY_)
  {
    auto createResponse = m_tableServiceClient->CreateTable(m_tableName);
    for (int i = 0; i < 1010; i++)
    {
      auto entity = Azure::Data::Tables::Models::TableEntity();
      entity.SetPartitionKey("partition");
      entity.SetRowKey("rowKey" + std::to_string(i));
      m_tableClient->AddEntity(entity);
    }

    Azure::Data::Tables::Models::QueryEntitiesOptions options;
    auto response = m_tableClient->QueryEntities(options);
    EXPECT_EQ(response.TableEntities.size(), 1000);
    EXPECT_EQ(response.TableEntities[0].GetRowKey().Value, "rowKey0");

    response.MoveToNextPage();
    EXPECT_EQ(response.TableEntities.size(), 10);
  }

  TEST_P(TablesClientTest, EntityGet)
  {
    Azure::Data::Tables::Models::TableEntity entity;

    entity.SetPartitionKey("P1");
    entity.SetRowKey("R1");
    entity.Properties["Name"] = TableEntityProperty("Azure");
    entity.Properties["Product"] = TableEntityProperty("Tables");
    auto createResponse = m_tableServiceClient->CreateTable(m_tableName);
    auto response = m_tableClient->AddEntity(entity);
    EXPECT_EQ(response.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(response.Value.ETag.empty());

    entity.Properties["Product"] = TableEntityProperty("Tables2");
    entity.SetRowKey("R2");
    m_tableClient->AddEntity(entity);

    entity.Properties["Product"] = TableEntityProperty("Tables3");
    entity.SetRowKey("R3");
    m_tableClient->AddEntity(entity);

    std::string partitionKey = "P1";
    std::string rowKey = "R1";
    auto responseQuery = m_tableClient->GetEntity(partitionKey, rowKey);
    EXPECT_EQ(responseQuery.Value.GetPartitionKey().Value, "P1");
    EXPECT_EQ(responseQuery.Value.GetRowKey().Value, "R1");
    EXPECT_EQ(responseQuery.Value.Properties["Name"].Value, "Azure");
    EXPECT_EQ(responseQuery.Value.Properties["Product"].Value, "Tables");
    EXPECT_EQ(
        responseQuery.Value.Properties["Timestamp"].Type.Value(), TableEntityDataType::EdmDateTime);
  }

  TEST_P(TablesClientTest, EntityGetFail)
  {
    Azure::Data::Tables::Models::TableEntity entity;

    std::string partitionKey = "P1";
    std::string rowKey = "R1";
    try
    {
      auto responseQuery = m_tableClient->GetEntity(partitionKey, rowKey);
    }
    catch (Azure::Core::RequestFailedException& e)
    {
      EXPECT_EQ(e.StatusCode, Azure::Core::Http::HttpStatusCode::NotFound);
    }
  }

  TEST_P(TablesClientTest, TransactionCreateFail_LIVEONLY_)
  {
    Azure::Data::Tables::Models::TableEntity entity;
    Azure::Data::Tables::Models::TableEntity entity2;
    entity.SetPartitionKey("P1");
    entity.SetRowKey("R1");
    entity.Properties["Name"] = TableEntityProperty("Azure");
    entity.Properties["Product"] = TableEntityProperty("Tables");
    entity2.SetPartitionKey("P1");
    entity2.SetRowKey("R1");
    entity2.Properties["Name"] = TableEntityProperty("Azure");
    entity2.Properties["Product"] = TableEntityProperty("Tables");
    auto createResponse = m_tableServiceClient->CreateTable(m_tableName);

    std::vector<Azure::Data::Tables::Models::TransactionStep> steps;
    // conflicting entities in the same transaction
    steps.emplace_back(Azure::Data::Tables::Models::TransactionStep{
        Azure::Data::Tables::Models::TransactionActionType::Add, entity});
    steps.emplace_back(Azure::Data::Tables::Models::TransactionStep{
        Azure::Data::Tables::Models::TransactionActionType::Add, entity2});

    auto response = m_tableClient->SubmitTransaction(steps);
    EXPECT_TRUE(response.Value.Error.HasValue());
  }

  TEST_P(TablesClientTest, TransactionCreateOK_LIVEONLY_)
  {
    Azure::Data::Tables::Models::TableEntity entity;
    Azure::Data::Tables::Models::TableEntity entity2;
    entity.SetPartitionKey("P1");
    entity.SetRowKey("R1");
    entity.Properties["Name"] = TableEntityProperty("Azure");
    entity.Properties["Product"] = TableEntityProperty("Tables");
    entity2.SetPartitionKey("P1");
    entity2.SetRowKey("R2");
    entity2.Properties["Name"] = TableEntityProperty("Azure");
    entity2.Properties["Product"] = TableEntityProperty("Tables");
    auto createResponse = m_tableServiceClient->CreateTable(m_tableName);

    std::vector<Azure::Data::Tables::Models::TransactionStep> steps;
    // create two entities in the same transaction
    steps.emplace_back(Azure::Data::Tables::Models::TransactionStep{
        Azure::Data::Tables::Models::TransactionActionType::Add, entity});
    steps.emplace_back(Azure::Data::Tables::Models::TransactionStep{
        Azure::Data::Tables::Models::TransactionActionType::Add, entity2});

    auto response = m_tableClient->SubmitTransaction(steps);
    EXPECT_FALSE(response.Value.Error.HasValue());
  }

  TEST_P(TablesClientTest, TransactionDelete_LIVEONLY_)
  {
    Azure::Data::Tables::Models::TableEntity entity;
    Azure::Data::Tables::Models::TableEntity entity2;
    entity.SetPartitionKey("P1");
    entity.SetRowKey("R1");
    entity.Properties["Name"] = TableEntityProperty("Azure");
    entity.Properties["Product"] = TableEntityProperty("Tables");
    entity2.SetPartitionKey("P1");
    entity2.SetRowKey("R2");
    entity2.Properties["Name"] = TableEntityProperty("Azure");
    entity2.Properties["Product"] = TableEntityProperty("Tables");
    auto createResponse = m_tableServiceClient->CreateTable(m_tableName);

    std::vector<Azure::Data::Tables::Models::TransactionStep> steps;

    steps.emplace_back(Azure::Data::Tables::Models::TransactionStep{
        Azure::Data::Tables::Models::TransactionActionType::Add, entity});
    steps.emplace_back(Azure::Data::Tables::Models::TransactionStep{
        Azure::Data::Tables::Models::TransactionActionType::Add, entity2});
    auto response = m_tableClient->SubmitTransaction(steps);

    steps.clear();
    // delete entity
    steps.emplace_back(Azure::Data::Tables::Models::TransactionStep{
        Azure::Data::Tables::Models::TransactionActionType::Delete, entity2});
    response = m_tableClient->SubmitTransaction(steps);
    EXPECT_FALSE(response.Value.Error.HasValue());
  }

  TEST_P(TablesClientTest, TransactionMerge_LIVEONLY_)
  {
    Azure::Data::Tables::Models::TableEntity entity;
    Azure::Data::Tables::Models::TableEntity entity2;
    entity.SetPartitionKey("P1");
    entity.SetRowKey("R1");
    entity.Properties["Name"] = TableEntityProperty("Azure");
    entity.Properties["Product"] = TableEntityProperty("Tables");
    entity2.SetPartitionKey("P1");
    entity2.SetRowKey("R1");
    entity2.Properties["Name"] = TableEntityProperty("Azure2");
    entity2.Properties["Product"] = TableEntityProperty("Tables3");
    auto createResponse = m_tableServiceClient->CreateTable(m_tableName);

    std::vector<Azure::Data::Tables::Models::TransactionStep> steps;

    steps.emplace_back(Azure::Data::Tables::Models::TransactionStep{
        Azure::Data::Tables::Models::TransactionActionType::Add, entity});
    auto response = m_tableClient->SubmitTransaction(steps);

    steps.clear();
    // merge entity
    steps.emplace_back(Azure::Data::Tables::Models::TransactionStep{
        Azure::Data::Tables::Models::TransactionActionType::UpdateMerge, entity2});

    response = m_tableClient->SubmitTransaction(steps);

    EXPECT_FALSE(response.Value.Error.HasValue());
  }

  TEST_P(TablesClientTest, TransactionUpdate_LIVEONLY_)
  {
    Azure::Data::Tables::Models::TableEntity entity;
    Azure::Data::Tables::Models::TableEntity entity2;
    entity.SetPartitionKey("P1");
    entity.SetRowKey("R1");
    entity.Properties["Name"] = TableEntityProperty("Azure");
    entity.Properties["Product"] = TableEntityProperty("Tables");
    entity2.SetPartitionKey("P1");
    entity2.SetRowKey("R1");
    entity2.Properties["Name"] = TableEntityProperty("Azure2");
    entity2.Properties["Product"] = TableEntityProperty("Tables3");
    auto createResponse = m_tableServiceClient->CreateTable(m_tableName);
    std::vector<Azure::Data::Tables::Models::TransactionStep> steps;

    steps.emplace_back(Azure::Data::Tables::Models::TransactionStep{
        Azure::Data::Tables::Models::TransactionActionType::Add, entity});
    auto response = m_tableClient->SubmitTransaction(steps);

    steps.clear();
    // replace entity
    steps.emplace_back(Azure::Data::Tables::Models::TransactionStep{
        Azure::Data::Tables::Models::TransactionActionType::UpdateReplace, entity2});
    response = m_tableClient->SubmitTransaction(steps);

    EXPECT_FALSE(response.Value.Error.HasValue());
  }

  TEST_P(TablesClientTest, TransactionInsertReplace_LIVEONLY_)
  {
    Azure::Data::Tables::Models::TableEntity entity;
    Azure::Data::Tables::Models::TableEntity entity2;
    entity.SetPartitionKey("P1");
    entity.SetRowKey("R1");
    entity.Properties["Name"] = TableEntityProperty("Azure");
    entity.Properties["Product"] = TableEntityProperty("Tables");
    entity2.SetPartitionKey("P1");
    entity2.SetRowKey("R2");
    entity2.Properties["Name"] = TableEntityProperty("Azure2");
    entity2.Properties["Product"] = TableEntityProperty("Tables3");
    auto createResponse = m_tableServiceClient->CreateTable(m_tableName);
    std::vector<Azure::Data::Tables::Models::TransactionStep> steps;

    steps.emplace_back(Azure::Data::Tables::Models::TransactionStep{
        Azure::Data::Tables::Models::TransactionActionType::InsertReplace, entity});
    auto response = m_tableClient->SubmitTransaction(steps);

    steps.clear();
    // replace entity
    steps.emplace_back(Azure::Data::Tables::Models::TransactionStep{
        Azure::Data::Tables::Models::TransactionActionType::InsertReplace, entity2});
    response = m_tableClient->SubmitTransaction(steps);

    EXPECT_FALSE(response.Value.Error.HasValue());
  }

  namespace {
    static std::string GetSuffix(const testing::TestParamInfo<AuthType>& info)
    {
      std::string stringValue = "";
      switch (info.param)
      {
        case AuthType::Key:
          stringValue = "key";
          break;
        default:
          stringValue = "key";
          break;
      }
      return stringValue;
    }
  } // namespace
  INSTANTIATE_TEST_SUITE_P(Tables, TablesClientTest, ::testing::Values(AuthType::Key), GetSuffix);
}}} // namespace Azure::Data::Test
