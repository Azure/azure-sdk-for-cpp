// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake_service_client_test.hpp"

#include <algorithm>

namespace Azure { namespace Storage { namespace Test {

  const size_t c_FILE_SYSTEM_TEST_SIZE = 5;

  std::shared_ptr<Files::DataLake::ServiceClient>
      DataLakeServiceClientTest::m_dataLakeServiceClient;
  std::vector<std::string> DataLakeServiceClientTest::m_fileSystemNameSetA;
  std::vector<std::string> DataLakeServiceClientTest::m_fileSystemNameSetB;
  std::string DataLakeServiceClientTest::m_fileSystemPrefixA;
  std::string DataLakeServiceClientTest::m_fileSystemPrefixB;

  void DataLakeServiceClientTest::SetUpTestSuite()
  {
    m_dataLakeServiceClient = std::make_shared<Files::DataLake::ServiceClient>(
        Files::DataLake::ServiceClient::CreateFromConnectionString(AdlsGen2ConnectionString()));
    m_fileSystemPrefixA = LowercaseRandomString(10);
    m_fileSystemPrefixB = LowercaseRandomString(10);
    m_fileSystemNameSetA.clear();
    m_fileSystemNameSetB.clear();
    for (size_t i = 0; i < c_FILE_SYSTEM_TEST_SIZE; ++i)
    {
      {
        auto name = m_fileSystemPrefixA + LowercaseRandomString(10);
        m_dataLakeServiceClient->GetFileSystemClient(name).Create();
        m_fileSystemNameSetA.emplace_back(std::move(name));
      }
      {
        auto name = m_fileSystemPrefixB + LowercaseRandomString(10);
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

  std::vector<Files::DataLake::FileSystem> DataLakeServiceClientTest::ListAllFileSystems(
      const std::string& prefix)
  {
    std::vector<Files::DataLake::FileSystem> result;
    std::string continuation;
    Files::DataLake::ListFileSystemsSegmentOptions options;
    if (!prefix.empty())
    {
      options.Prefix = prefix;
    }
    do
    {
      auto response = m_dataLakeServiceClient->ListFileSystemsSegement(options);
      result.insert(result.end(), response->Filesystems.begin(), response->Filesystems.end());
      if (response->ContinuationToken.HasValue())
      {
        continuation = response->ContinuationToken.GetValue();
        options.ContinuationToken = continuation;
      }
    } while (!continuation.empty());
    return result;
  }

  TEST_F(DataLakeServiceClientTest, ListFileSystemsSegement)
  {
    {
      // Normal list without prefix.
      auto result = ListAllFileSystems();
      for (const auto& name : m_fileSystemNameSetA)
      {
        auto iter = std::find_if(
            result.begin(), result.end(), [&name](const Files::DataLake::FileSystem& fileSystem) {
              return fileSystem.Name == name;
            });
        EXPECT_EQ(iter->Name.substr(0U, m_fileSystemPrefixA.size()), m_fileSystemPrefixA);
        EXPECT_NE(result.end(), iter);
      }
      for (const auto& name : m_fileSystemNameSetB)
      {
        auto iter = std::find_if(
            result.begin(), result.end(), [&name](const Files::DataLake::FileSystem& fileSystem) {
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
            result.begin(), result.end(), [&name](const Files::DataLake::FileSystem& fileSystem) {
              return fileSystem.Name == name;
            });
        EXPECT_EQ(iter->Name.substr(0U, m_fileSystemPrefixA.size()), m_fileSystemPrefixA);
        EXPECT_NE(result.end(), iter);
      }
      for (const auto& name : m_fileSystemNameSetB)
      {
        auto iter = std::find_if(
            result.begin(), result.end(), [&name](const Files::DataLake::FileSystem& fileSystem) {
              return fileSystem.Name == name;
            });
        EXPECT_EQ(result.end(), iter);
      }
    }
    {
      // List max result
      Files::DataLake::ListFileSystemsSegmentOptions options;
      options.MaxResults = 2;
      auto response = m_dataLakeServiceClient->ListFileSystemsSegement(options);
      EXPECT_LE(2U, response->Filesystems.size());
    }
  }

}}} // namespace Azure::Storage::Test
