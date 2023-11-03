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
    // setting policy takes up to 30 seconds to take effect
    std::this_thread::sleep_for(std::chrono::milliseconds(30001));
    auto getResponse = m_tableClient->GetAccessPolicy();

    EXPECT_EQ(getResponse.Value.SignedIdentifiers.size(), 1);
    EXPECT_EQ(getResponse.Value.SignedIdentifiers[0].Id, newIdentifier.Id);
    EXPECT_EQ(getResponse.Value.SignedIdentifiers[0].Permissions, newIdentifier.Permissions);
    EXPECT_EQ(
        getResponse.Value.SignedIdentifiers[0].StartsOn.Value().ToString(
            Azure::DateTime::DateFormat::Rfc1123),
        newIdentifier.StartsOn.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
    EXPECT_EQ(
        getResponse.Value.SignedIdentifiers[0].ExpiresOn.Value().ToString(
            Azure::DateTime::DateFormat::Rfc1123),
        newIdentifier.ExpiresOn.Value().ToString(Azure::DateTime::DateFormat::Rfc1123));
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
}}} // namespace Azure::Storage::Test