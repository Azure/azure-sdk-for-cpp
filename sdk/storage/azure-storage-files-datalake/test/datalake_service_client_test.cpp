// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake_service_client_test.hpp"

#include <algorithm>
#include <chrono>

namespace Azure { namespace Storage { namespace Test {

  const size_t FileSystemTestSize = 5;

  std::shared_ptr<Files::DataLake::DataLakeServiceClient>
      DataLakeServiceClientTest::m_dataLakeServiceClient;
  std::vector<std::string> DataLakeServiceClientTest::m_fileSystemNameSetA;
  std::vector<std::string> DataLakeServiceClientTest::m_fileSystemNameSetB;
  std::string DataLakeServiceClientTest::m_fileSystemPrefixA;
  std::string DataLakeServiceClientTest::m_fileSystemPrefixB;

  void DataLakeServiceClientTest::SetUpTestSuite()
  {
    m_dataLakeServiceClient = std::make_shared<Files::DataLake::DataLakeServiceClient>(
        Files::DataLake::DataLakeServiceClient::CreateFromConnectionString(
            AdlsGen2ConnectionString()));
    m_fileSystemPrefixA = LowercaseRandomString();
    m_fileSystemPrefixB = LowercaseRandomString();
    m_fileSystemNameSetA.clear();
    m_fileSystemNameSetB.clear();
    for (size_t i = 0; i < FileSystemTestSize; ++i)
    {
      {
        auto name = m_fileSystemPrefixA + LowercaseRandomString();
        m_dataLakeServiceClient->GetFileSystemClient(name).Create();
        m_fileSystemNameSetA.emplace_back(std::move(name));
      }
      {
        auto name = m_fileSystemPrefixB + LowercaseRandomString();
        m_dataLakeServiceClient->GetFileSystemClient(name).Create();
        m_fileSystemNameSetB.emplace_back(std::move(name));
      }
    }
  }

  void DataLakeServiceClientTest::TearDownTestSuite()
  {
    for (const auto& name : m_fileSystemNameSetA)
    {
      m_dataLakeServiceClient->GetFileSystemClient(name).Delete();
    }
    for (const auto& name : m_fileSystemNameSetB)
    {
      m_dataLakeServiceClient->GetFileSystemClient(name).Delete();
    }
  }

  std::vector<Files::DataLake::Models::FileSystemItem>
  DataLakeServiceClientTest::ListAllFileSystems(const std::string& prefix)
  {
    std::vector<Files::DataLake::Models::FileSystemItem> result;
    std::string continuation;
    Files::DataLake::ListFileSystemsOptions options;
    if (!prefix.empty())
    {
      options.Prefix = prefix;
    }
    for (auto pageResult = m_dataLakeServiceClient->ListFileSystems(options); pageResult.HasPage();
         pageResult.MoveToNextPage())
    {
      result.insert(result.end(), pageResult.FileSystems.begin(), pageResult.FileSystems.end());
    }
    return result;
  }

  TEST_F(DataLakeServiceClientTest, ListFileSystemsSegment)
  {
    {
      // Normal list without prefix.
      auto result = ListAllFileSystems();
      for (const auto& name : m_fileSystemNameSetA)
      {
        auto iter = std::find_if(
            result.begin(),
            result.end(),
            [&name](const Files::DataLake::Models::FileSystemItem& fileSystem) {
              return fileSystem.Name == name;
            });
        EXPECT_EQ(iter->Name.substr(0U, m_fileSystemPrefixA.size()), m_fileSystemPrefixA);
        EXPECT_NE(result.end(), iter);
      }
      for (const auto& name : m_fileSystemNameSetB)
      {
        auto iter = std::find_if(
            result.begin(),
            result.end(),
            [&name](const Files::DataLake::Models::FileSystemItem& fileSystem) {
              return fileSystem.Name == name;
            });
        EXPECT_EQ(iter->Name.substr(0U, m_fileSystemPrefixB.size()), m_fileSystemPrefixB);
        EXPECT_NE(result.end(), iter);
      }
    }
    {
      // List prefix.
      auto result = ListAllFileSystems(m_fileSystemPrefixA);
      for (const auto& name : m_fileSystemNameSetA)
      {
        auto iter = std::find_if(
            result.begin(),
            result.end(),
            [&name](const Files::DataLake::Models::FileSystemItem& fileSystem) {
              return fileSystem.Name == name;
            });
        EXPECT_EQ(iter->Name.substr(0U, m_fileSystemPrefixA.size()), m_fileSystemPrefixA);
        EXPECT_NE(result.end(), iter);
      }
      for (const auto& name : m_fileSystemNameSetB)
      {
        auto iter = std::find_if(
            result.begin(),
            result.end(),
            [&name](const Files::DataLake::Models::FileSystemItem& fileSystem) {
              return fileSystem.Name == name;
            });
        EXPECT_EQ(result.end(), iter);
      }
    }
    {
      // List max result
      Files::DataLake::ListFileSystemsOptions options;
      options.PageSizeHint = 2;
      auto response = m_dataLakeServiceClient->ListFileSystems(options);
      EXPECT_LE(2U, response.FileSystems.size());
    }
  }

  TEST_F(DataLakeServiceClientTest, AnonymousConstructorsWorks)
  {
    auto keyCredential
        = Azure::Storage::_internal::ParseConnectionString(AdlsGen2ConnectionString())
              .KeyCredential;
    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.StartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    accountSasBuilder.ExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);
    accountSasBuilder.Services = Sas::AccountSasServices::Blobs;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;
    accountSasBuilder.SetPermissions(Sas::AccountSasPermissions::All);
    auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);

    // Create from Anonymous credential.
    auto datalakeServiceUrl
        = Azure::Storage::Files::DataLake::DataLakeServiceClient::CreateFromConnectionString(
              AdlsGen2ConnectionString())
              .GetUrl();
    auto datalakeServiceClient
        = Azure::Storage::Files::DataLake::DataLakeServiceClient(datalakeServiceUrl + sasToken);
    EXPECT_NO_THROW(datalakeServiceClient.ListFileSystems());
  }

}}} // namespace Azure::Storage::Test
