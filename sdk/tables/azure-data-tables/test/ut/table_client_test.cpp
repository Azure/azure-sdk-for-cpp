// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "table_client_test.hpp"

#include "azure/data/tables/account_sas_builder.hpp"
#include "azure/data/tables/tables_sas_builder.hpp"

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
        case AuthType::ConnectionString:
          m_tableServiceClient = std::make_shared<Tables::TableServiceClient>(
              Tables::TableServiceClient::CreateFromConnectionString(
                  GetConnectionString(), clientOptions));
          m_tableClient = std::make_shared<Tables::TableClient>(
              Tables::TableClient::CreateFromConnectionString(
                  GetConnectionString(), m_tableName, tableClientOptions));
          break;
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
        case AuthType::SAS:
          auto creds = std::make_shared<Azure::Data::Tables::Credentials::NamedKeyCredential>(
              GetAccountName(), GetAccountKey());
          Azure::Data::Tables::Sas::AccountSasBuilder sasBuilder;
          sasBuilder.ExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);
          sasBuilder.ResourceTypes = Azure::Data::Tables::Sas::AccountSasResourceType::All;
          sasBuilder.Services = Azure::Data::Tables::Sas::AccountSasServices::All;
          sasBuilder.Protocol = Azure::Data::Tables::Sas::SasProtocol::HttpsOnly;
          sasBuilder.SetPermissions(Azure::Data::Tables::Sas::AccountSasPermissions::All);
          std::string serviceUrl = "https://" + GetAccountName() + ".table.core.windows.net/";
          auto sasCreds = std::make_shared<Azure::Data::Tables::Credentials::AzureSasCredential>(
              sasBuilder.GenerateSasToken(*creds));
          m_tableServiceClient = std::make_shared<Tables::TableServiceClient>(
              Tables::TableServiceClient(serviceUrl, sasCreds, clientOptions));

          Azure::Data::Tables::Sas::TablesSasBuilder tableSasBuilder;
          tableSasBuilder.Protocol = Azure::Data::Tables::Sas::SasProtocol::HttpsOnly;
          tableSasBuilder.StartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
          tableSasBuilder.ExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);
          tableSasBuilder.SetPermissions(Azure::Data::Tables::Sas::TablesSasPermissions::All);
          tableSasBuilder.TableName = m_tableName;
          auto tableSasCreds
              = std::make_shared<Azure::Data::Tables::Credentials::AzureSasCredential>(
                  tableSasBuilder.GenerateSasToken(*creds));
          m_tableClient = std::make_shared<Tables::TableClient>(
              Tables::TableClient(serviceUrl, tableSasCreds, m_tableName, tableClientOptions));
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

  Azure::Data::Tables::TableClient TablesClientTest::CreateKeyTableClientForTest(
      Tables::TableClientOptions& clientOptions)
  {
    m_tableName = GetTestNameLowerCase() + LowercaseRandomString(10);
    auto tableClient
        = Tables::TableClient(GetEnv("DATA_TABLES_URL"), m_tableName, m_credential, clientOptions);
    return tableClient;
  }

  Azure::Data::Tables::TableClient TablesClientTest::CreateTableClientForTest(
      Tables::TableClientOptions& clientOptions)
  {
    m_tableName = GetTestNameLowerCase() + LowercaseRandomString(10);
    auto tableClient = Tables::TableClient::CreateFromConnectionString(
        GetEnv("STANDARD_STORAGE_CONNECTION_STRING"), m_tableName, clientOptions);
    return tableClient;
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

  TEST_P(TablesClientTest, GetAccessPolicy_LIVEONLY_)
  {
    if (GetParam() != AuthType::ConnectionString)
    {
      SkipTest();
      return;
    }
    auto createResponse = m_tableServiceClient->CreateTable(m_tableName);

    auto getResponse = m_tableClient->GetAccessPolicy();
    EXPECT_EQ(getResponse.Value.SignedIdentifiers.size(), 0);
  }

  TEST_P(TablesClientTest, SetAccessPolicy_LIVEONLY_)
  {
    if (GetParam() != AuthType::ConnectionString)
    {
      SkipTest();
      return;
    }
    auto createResponse = m_tableServiceClient->CreateTable(m_tableName);
    Azure::Data::Tables::Models::TableAccessPolicy newPolicy{};
    Azure::Data::Tables::Models::SignedIdentifier newIdentifier{};
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

  TEST_P(TablesClientTest, ListTables)
  {
    if (GetParam() == AuthType::ConnectionString)
    {
      EXPECT_TRUE(true);
    }
    else
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
  }

  TEST_P(TablesClientTest, DeleteTable)
  {
    auto createResponse = m_tableServiceClient->CreateTable(m_tableName);

    auto response = m_tableServiceClient->DeleteTable(m_tableName);
    EXPECT_EQ(response.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
  }

  TEST_P(TablesClientTest, ServiceClientConstructors)
  {
    EXPECT_FALSE(m_tableServiceClient == nullptr);
  }

  TEST_P(TablesClientTest, ServiceClientGetProperties)
  {
    if (GetParam() == AuthType::ConnectionString)
    {
      EXPECT_TRUE(true);
    }
    else
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
  }

  TEST_P(TablesClientTest, ServiceClientSet_LIVEONLY_)
  {
    auto response = m_tableServiceClient->GetServiceProperties();

    Azure::Data::Tables::Models::SetServicePropertiesOptions setOptions;
    setOptions.ServiceProperties = std::move(response.Value);
    auto response2 = m_tableServiceClient->SetServiceProperties(setOptions);
    EXPECT_EQ(response2.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::Accepted);
  }

  TEST_P(TablesClientTest, ServiceClientStatistics_LIVEONLY_)
  {
    auto response = m_tableServiceClient->GetStatistics();

    EXPECT_EQ(response.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::Ok);
    EXPECT_EQ(response.Value.GeoReplication.Status.ToString(), "live");
  }

  TEST_P(TablesClientTest, EntityCreate)
  {
    if (GetParam() == AuthType::Key)
    {
      EXPECT_TRUE(true);
      return;
    }
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

  TEST_P(TablesClientTest, EntityUpdate)
  {
    if (GetParam() == AuthType::Key)
    {
      EXPECT_TRUE(true);
      return;
    }
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
    if (GetParam() == AuthType::Key)
    {
      EXPECT_TRUE(true);
      return;
    }
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
    if (GetParam() == AuthType::Key)
    {
      EXPECT_TRUE(true);
      return;
    }
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

  TEST_P(TablesClientTest, EntityUpsert)
  {
    if (GetParam() == AuthType::Key)
    {
      EXPECT_TRUE(true);
      return;
    }
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
    auto updateResponse = m_tableClient->MergeEntity(entity, options);

    EXPECT_EQ(
        updateResponse.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(updateResponse.Value.ETag.empty());

    Azure::Data::Tables::Models::UpsertEntityOptions options2;
    options2.UpsertType = Azure::Data::Tables::Models::UpsertKind::Merge;
    entity.Properties["Product3"] = TableEntityProperty("Tables3");
    entity.SetETag(updateResponse.Value.ETag);
    auto updateResponse2 = m_tableClient->MergeEntity(entity, options2);

    EXPECT_EQ(
        updateResponse2.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(updateResponse2.Value.ETag.empty());
  }

  TEST_P(TablesClientTest, EntityQuery)
  {
    if (GetParam() == AuthType::Key)
    {
      EXPECT_TRUE(true);
      return;
    }
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

  TEST_P(TablesClientTest, EntityGet)
  {
    if (GetParam() == AuthType::Key)
    {
      EXPECT_TRUE(true);
      return;
    }
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

  TEST_P(TablesClientTest, TransactionCreateFail_LIVEONLY_)
  {
    if (GetParam() == AuthType::SAS)
    {
      SkipTest();
      return;
    }
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
    if (GetParam() != AuthType::ConnectionString)
    {
      SkipTest();
      return;
    }
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
    if (GetParam() != AuthType::ConnectionString)
    {
      SkipTest();
      return;
    }
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
    if (GetParam() != AuthType::ConnectionString)
    {
      SkipTest();
      return;
    }
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
    if (GetParam() != AuthType::ConnectionString)
    {
      SkipTest();
      return;
    }
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

  namespace {
    static std::string GetSuffix(const testing::TestParamInfo<AuthType>& info)
    {
      std::string stringValue = "";
      switch (info.param)
      {
        case AuthType::ConnectionString:
          stringValue = "connectionstring";
          break;
        case AuthType::Key:
          stringValue = "key";
          break;
        case AuthType::SAS:
          stringValue = "sas";
          break;
        default:
          stringValue = "key";
          break;
      }
      return stringValue;
    }
  } // namespace
  INSTANTIATE_TEST_SUITE_P(
      Tables,
      TablesClientTest,
      ::testing::Values(AuthType::Key, AuthType::ConnectionString, AuthType::SAS),
      GetSuffix);
}}} // namespace Azure::Data::Test
