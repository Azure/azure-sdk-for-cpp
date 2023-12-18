// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "table_client_test.hpp"

#include <azure/core/internal/strings.hpp>

#include <chrono>
#include <string>
#include <thread>
using namespace Azure::Data;
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

      std::replace(m_tableName.begin(), m_tableName.end(), '-', '0');
      switch (param)
      {
        case AuthType::ConnectionString:
          m_tableServiceClient = std::make_shared<Tables::TableServicesClient>(
              Tables::TableServicesClient::CreateFromConnectionString(
                  GetEnv("STANDARD_STORAGE_CONNECTION_STRING"), clientOptions));
          m_tableClient = std::make_shared<Tables::TableClient>(
              Tables::TableClient::CreateFromConnectionString(
                  GetEnv("STANDARD_STORAGE_CONNECTION_STRING"), m_tableName, tableClientOptions));
          break;
        case AuthType::Key:
          m_credential = CreateClientSecretCredential(
              GetEnv("STORAGE_TENANT_ID"),
              GetEnv("STORAGE_CLIENT_ID"),
              GetEnv("STORAGE_CLIENT_SECRET"));
          m_tableServiceClient = std::make_shared<Tables::TableServicesClient>(
              Azure::Data::Tables::TableServicesClient(
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
          auto creds = std::make_shared<Azure::Storage::StorageSharedKeyCredential>(
              GetAccountName(), GetAccountKey());
          Azure::Storage::Sas::AccountSasBuilder sasBuilder;
          sasBuilder.ExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);
          sasBuilder.ResourceTypes = Azure::Storage::Sas::AccountSasResource::All;
          sasBuilder.Services = Azure::Storage::Sas::AccountSasServices::All;
          sasBuilder.Protocol = Azure::Storage::Sas::SasProtocol::HttpsOnly;
          sasBuilder.SetPermissions(Azure::Storage::Sas::AccountSasPermissions::All);
          auto sasToken = sasBuilder.GenerateSasToken(*creds);
          m_tableServiceClient
              = std::make_shared<Tables::TableServicesClient>(Tables::TableServicesClient(
                  "https://" + GetAccountName() + ".table.core.windows.net/" + sasToken,
                  clientOptions));
          m_tableClient = std::make_shared<Tables::TableClient>(Tables::TableClient(
              "https://" + GetAccountName() + ".table.core.windows.net/" + sasToken,
              m_tableName,
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

        auto deleteResponse = m_tableClient->Delete();
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
    auto tableClient = Tables::TableClient(
        GetEnv("DATA_TABLES_URL"), m_tableName, m_credential, clientOptions);
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
    auto createResponse = m_tableClient->Create();
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
    auto createResponse = m_tableClient->Create();

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
    auto createResponse = m_tableClient->Create();
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

      auto createResponse = m_tableClient->Create();

      Azure::Data::Tables::Models::ListTablesOptions listOptions;

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
  }

  TEST_P(TablesClientTest, DeleteTable)
  {
    auto createResponse = m_tableClient->Create();

    auto response = m_tableClient->Delete();
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
      Azure::Data::Tables::Models::GetServicePropertiesOptions getOptions;

      auto response = m_tableServiceClient->GetServiceProperties(getOptions);
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

    Azure::Data::Tables::Models::GetServicePropertiesOptions getOptions;

    auto response = m_tableServiceClient->GetServiceProperties(getOptions);

    Azure::Data::Tables::Models::SetServicePropertiesOptions setOptions;
    setOptions.ServiceProperties = std::move(response.Value);
    auto response2 = m_tableServiceClient->SetServiceProperties(setOptions);
    EXPECT_EQ(response2.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::Accepted);
  }

  TEST_P(TablesClientTest, ServiceClientStatistics_LIVEONLY_)
  {
    Azure::Data::Tables::Models::GetServiceStatisticsOptions statsOptions;

    auto response = m_tableServiceClient->GetStatistics(statsOptions);

    EXPECT_EQ(response.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::Ok);
    EXPECT_EQ(response.Value.GeoReplication.Status.ToString(), "live");
  }

  TEST_P(TablesClientTest, EntityCreate)
  {
    if (GetParam() == AuthType::Key
        && Azure::Core::_internal::StringExtensions::ToLower(GetEnv("AZURE_TEST_MODE")) == "live")
    {
      EXPECT_TRUE(true);
      return;
    }
    Azure::Data::Tables::Models::TableEntity entity;

    entity.PartitionKey = "P1";
    entity.RowKey = "R1";
    entity.Properties["Name"] = "Azure";
    entity.Properties["Product"] = "Tables";
    auto createResponse = m_tableClient->Create();
    auto response = m_tableClient->CreateEntity(entity);

    EXPECT_EQ(response.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(response.Value.ETag.empty());
  }

  TEST_P(TablesClientTest, EntityUpdate)
  {
    if (GetParam() == AuthType::Key
        && Azure::Core::_internal::StringExtensions::ToLower(GetEnv("AZURE_TEST_MODE")) == "live")
    {
      EXPECT_TRUE(true);
      return;
    }
    Azure::Data::Tables::Models::TableEntity entity;

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

  TEST_P(TablesClientTest, EntityMerge)
  {
    if (GetParam() == AuthType::Key
        && Azure::Core::_internal::StringExtensions::ToLower(GetEnv("AZURE_TEST_MODE")) == "live")
    {
      EXPECT_TRUE(true);
      return;
    }
    Azure::Data::Tables::Models::TableEntity entity;

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

  TEST_P(TablesClientTest, EntityDelete)
  {
    if (GetParam() == AuthType::Key
        && Azure::Core::_internal::StringExtensions::ToLower(GetEnv("AZURE_TEST_MODE")) == "live")
    {
      EXPECT_TRUE(true);
      return;
    }
    Azure::Data::Tables::Models::TableEntity entity;

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

  TEST_P(TablesClientTest, EntityUpsert)
  {
    if (GetParam() == AuthType::Key
        && Azure::Core::_internal::StringExtensions::ToLower(GetEnv("AZURE_TEST_MODE")) == "live")
    {
      EXPECT_TRUE(true);
      return;
    }
    Azure::Data::Tables::Models::TableEntity entity;

    entity.PartitionKey = "P1";
    entity.RowKey = "R1";
    entity.Properties["Name"] = "Azure";
    entity.Properties["Product"] = "Tables";
    auto createResponse = m_tableClient->Create();
    auto response = m_tableClient->UpsertEntity(entity);
    EXPECT_EQ(response.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(response.Value.ETag.empty());

    Azure::Data::Tables::Models::UpsertEntityOptions options;
    options.UpsertType = Azure::Data::Tables::Models::UpsertKind::Update;

    entity.Properties["Product"] = "Tables2";
    auto updateResponse = m_tableClient->MergeEntity(entity, options);

    EXPECT_EQ(
        updateResponse.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(updateResponse.Value.ETag.empty());

    Azure::Data::Tables::Models::UpsertEntityOptions options2;
    options2.UpsertType = Azure::Data::Tables::Models::UpsertKind::Merge;
    entity.Properties["Product3"] = "Tables3";
    entity.ETag = updateResponse.Value.ETag;
    auto updateResponse2 = m_tableClient->MergeEntity(entity, options2);

    EXPECT_EQ(
        updateResponse2.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    EXPECT_FALSE(updateResponse2.Value.ETag.empty());
  }

  TEST_P(TablesClientTest, EntityQuery)
  {
    if (GetParam() == AuthType::Key
        && Azure::Core::_internal::StringExtensions::ToLower(GetEnv("AZURE_TEST_MODE")) == "live")
    {
      EXPECT_TRUE(true);
      return;
    }
    Azure::Data::Tables::Models::TableEntity entity;

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

  TEST_P(TablesClientTest, TransactionCreateFail_LIVEONLY_)
  {
    Azure::Data::Tables::Models::TableEntity entity;
    Azure::Data::Tables::Models::TableEntity entity2;
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

  TEST_P(TablesClientTest, TransactionCreateOK_LIVEONLY_)
  {
    if (GetParam() == AuthType::Key
        && Azure::Core::_internal::StringExtensions::ToLower(GetEnv("AZURE_TEST_MODE")) == "live")
    {
      EXPECT_TRUE(true);
      return;
    }
    Azure::Data::Tables::Models::TableEntity entity;
    Azure::Data::Tables::Models::TableEntity entity2;
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

  TEST_P(TablesClientTest, TransactionDelete_LIVEONLY_)
  {
    if (GetParam() == AuthType::Key
        && Azure::Core::_internal::StringExtensions::ToLower(GetEnv("AZURE_TEST_MODE")) == "live")
    {
      EXPECT_TRUE(true);
      return;
    }
    Azure::Data::Tables::Models::TableEntity entity;
    Azure::Data::Tables::Models::TableEntity entity2;
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
    EXPECT_FALSE(response.Value.Error.HasValue());
  }

  TEST_P(TablesClientTest, TransactionMerge_LIVEONLY_)
  {
    if (GetParam() == AuthType::Key
        && Azure::Core::_internal::StringExtensions::ToLower(GetEnv("AZURE_TEST_MODE")) == "live")
    {
      EXPECT_TRUE(true);
      return;
    }
    Azure::Data::Tables::Models::TableEntity entity;
    Azure::Data::Tables::Models::TableEntity entity2;
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
    EXPECT_FALSE(response.Value.Error.HasValue());
  }

  TEST_P(TablesClientTest, TransactionUpdate_LIVEONLY_)
  {
    if (GetParam() == AuthType::Key
        && Azure::Core::_internal::StringExtensions::ToLower(GetEnv("AZURE_TEST_MODE")) == "live")
    {
      EXPECT_TRUE(true);
      return;
    }
    Azure::Data::Tables::Models::TableEntity entity;
    Azure::Data::Tables::Models::TableEntity entity2;
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
