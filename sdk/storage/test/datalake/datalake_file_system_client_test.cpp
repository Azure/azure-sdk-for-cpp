// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake_file_system_client_test.hpp"

#include <algorithm>

namespace Azure { namespace Storage { namespace Test {

  const size_t c_PATH_TEST_SIZE = 5;

  std::shared_ptr<Files::DataLake::FileSystemClient>
      DataLakeFileSystemClientTest::m_fileSystemClient;
  std::string DataLakeFileSystemClientTest::m_fileSystemName;

  std::vector<std::string> DataLakeFileSystemClientTest::m_pathNameSetA;
  std::vector<std::string> DataLakeFileSystemClientTest::m_pathNameSetB;
  std::string DataLakeFileSystemClientTest::m_directoryA;
  std::string DataLakeFileSystemClientTest::m_directoryB;

  void DataLakeFileSystemClientTest::SetUpTestSuite()
  {
    m_fileSystemName = LowercaseRandomString();
    m_fileSystemClient = std::make_shared<Files::DataLake::FileSystemClient>(
        Files::DataLake::FileSystemClient::CreateFromConnectionString(
            AdlsGen2ConnectionString(), m_fileSystemName));
    m_fileSystemClient->Create();

    m_directoryA = LowercaseRandomString();
    m_directoryB = LowercaseRandomString();
    for (size_t i = 0; i < c_PATH_TEST_SIZE; ++i)
    {
      {
        auto name = m_directoryA + "/" + LowercaseRandomString();
        m_fileSystemClient->GetFileClient(name).Create();
        m_pathNameSetA.emplace_back(std::move(name));
      }
      {
        auto name = m_directoryB + "/" + LowercaseRandomString();
        m_fileSystemClient->GetFileClient(name).Create();
        m_pathNameSetB.emplace_back(std::move(name));
      }
    }
  }

  void DataLakeFileSystemClientTest::TearDownTestSuite() { m_fileSystemClient->Delete(); }

  std::vector<Files::DataLake::Path> DataLakeFileSystemClientTest::ListAllPaths(
      bool recursive,
      const std::string& directory)
  {
    std::vector<Files::DataLake::Path> result;
    std::string continuation;
    Files::DataLake::ListPathsOptions options;
    if (!directory.empty())
    {
      options.Directory = directory;
    }
    do
    {
      auto response = m_fileSystemClient->ListPaths(recursive, options);
      result.insert(result.end(), response->Paths.begin(), response->Paths.end());
      if (response->Continuation.HasValue())
      {
        continuation = response->Continuation.GetValue();
        options.Continuation = continuation;
      }
      else
      {
        continuation.clear();
      }
    } while (!continuation.empty());
    return result;
  }

  Files::DataLake::DataLakeHttpHeaders DataLakeFileSystemClientTest::GetInterestingHttpHeaders()
  {
    static Files::DataLake::DataLakeHttpHeaders result = []() {
      Files::DataLake::DataLakeHttpHeaders ret;
      ret.CacheControl = std::string("no-cache");
      ret.ContentDisposition = std::string("attachment");
      ret.ContentEncoding = std::string("deflate");
      ret.ContentLanguage = std::string("en-US");
      ret.ContentType = std::string("application/octet-stream");
      return ret;
    }();
    return result;
  }

  TEST_F(DataLakeFileSystemClientTest, CreateDeleteFileSystems)
  {
    {
      // Normal create/delete.
      std::vector<Files::DataLake::FileSystemClient> fileSystemClient;
      for (int32_t i = 0; i < 5; ++i)
      {
        auto client = Files::DataLake::FileSystemClient::CreateFromConnectionString(
            AdlsGen2ConnectionString(), LowercaseRandomString());
        EXPECT_NO_THROW(client.Create());
        fileSystemClient.emplace_back(std::move(client));
      }
      for (const auto& client : fileSystemClient)
      {
        EXPECT_NO_THROW(client.Delete());
      }
    }
    {
      // Normal delete with access condition.
      std::vector<Files::DataLake::FileSystemClient> fileSystemClient;
      for (int32_t i = 0; i < 5; ++i)
      {
        auto client = Files::DataLake::FileSystemClient::CreateFromConnectionString(
            AdlsGen2ConnectionString(), LowercaseRandomString());
        EXPECT_NO_THROW(client.Create());
        fileSystemClient.emplace_back(std::move(client));
      }
      for (const auto& client : fileSystemClient)
      {
        auto response = client.GetProperties();
        Files::DataLake::FileSystemDeleteOptions options1;
        options1.AccessConditions.IfModifiedSince = response->LastModified;
        EXPECT_THROW(client.Delete(options1), StorageError);
        Files::DataLake::FileSystemDeleteOptions options2;
        options2.AccessConditions.IfUnmodifiedSince = response->LastModified;
        EXPECT_NO_THROW(client.Delete(options2));
      }
    }
  }

  TEST_F(DataLakeFileSystemClientTest, FileSystemMetadata)
  {
    auto metadata1 = RandomMetadata();
    auto metadata2 = RandomMetadata();
    {
      // Set/Get Metadata works
      EXPECT_NO_THROW(m_fileSystemClient->SetMetadata(metadata1));
      auto result = m_fileSystemClient->GetProperties()->Metadata;
      EXPECT_EQ(metadata1, result);
      EXPECT_NO_THROW(m_fileSystemClient->SetMetadata(metadata2));
      result = m_fileSystemClient->GetProperties()->Metadata;
      EXPECT_EQ(metadata2, result);
    }

    {
      // Create file system with metadata works
      auto client1 = Files::DataLake::FileSystemClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(), LowercaseRandomString());
      auto client2 = Files::DataLake::FileSystemClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(), LowercaseRandomString());
      Files::DataLake::FileSystemCreateOptions options1;
      Files::DataLake::FileSystemCreateOptions options2;
      options1.Metadata = metadata1;
      options2.Metadata = metadata2;

      EXPECT_NO_THROW(client1.Create(options1));
      EXPECT_NO_THROW(client2.Create(options2));
      auto result = client1.GetProperties()->Metadata;
      EXPECT_EQ(metadata1, result);
      result = client2.GetProperties()->Metadata;
      EXPECT_EQ(metadata2, result);
    }
  }

  TEST_F(DataLakeFileSystemClientTest, FileSystemProperties)
  {
    auto metadata1 = RandomMetadata();
    auto metadata2 = RandomMetadata();
    {
      // Get Metadata via properties works
      EXPECT_NO_THROW(m_fileSystemClient->SetMetadata(metadata1));
      auto result = m_fileSystemClient->GetProperties();
      EXPECT_EQ(metadata1, result->Metadata);
      EXPECT_NO_THROW(m_fileSystemClient->SetMetadata(metadata2));
      result = m_fileSystemClient->GetProperties();
      EXPECT_EQ(metadata2, result->Metadata);
    }

    {
      // Last modified Etag works.
      auto properties1 = m_fileSystemClient->GetProperties();
      auto properties2 = m_fileSystemClient->GetProperties();
      EXPECT_EQ(properties1->ETag, properties2->ETag);
      EXPECT_EQ(properties1->LastModified, properties2->LastModified);

      // This operation changes ETag/LastModified.
      EXPECT_NO_THROW(m_fileSystemClient->SetMetadata(metadata1));

      auto properties3 = m_fileSystemClient->GetProperties();
      EXPECT_NE(properties1->ETag, properties3->ETag);
    }
  }

  TEST_F(DataLakeFileSystemClientTest, ListPaths)
  {
    {
      // Normal list recursively.
      auto result = ListAllPaths(true);
      for (const auto& name : m_pathNameSetA)
      {
        auto iter = std::find_if(
            result.begin(), result.end(), [&name](const Files::DataLake::Path& path) {
              return path.Name == name;
            });
        EXPECT_EQ(iter->Name, name);
        EXPECT_EQ(iter->Name.substr(0U, m_directoryA.size()), m_directoryA);
        EXPECT_NE(result.end(), iter);
      }
      for (const auto& name : m_pathNameSetB)
      {
        auto iter = std::find_if(
            result.begin(), result.end(), [&name](const Files::DataLake::Path& path) {
              return path.Name == name;
            });
        EXPECT_EQ(iter->Name, name);
        EXPECT_EQ(iter->Name.substr(0U, m_directoryB.size()), m_directoryB);
        EXPECT_NE(result.end(), iter);
      }
    }
    {
      // List with directory.
      auto result = ListAllPaths(true, m_directoryA);
      for (const auto& name : m_pathNameSetA)
      {
        auto iter = std::find_if(
            result.begin(), result.end(), [&name](const Files::DataLake::Path& path) {
              return path.Name == name;
            });
        EXPECT_EQ(iter->Name, name);
        EXPECT_EQ(iter->Name.substr(0U, m_directoryA.size()), m_directoryA);
        EXPECT_NE(result.end(), iter);
      }
      for (const auto& name : m_pathNameSetB)
      {
        auto iter = std::find_if(
            result.begin(), result.end(), [&name](const Files::DataLake::Path& path) {
              return path.Name == name;
            });
        EXPECT_EQ(result.end(), iter);
      }
    }
    {
      // List max result
      Files::DataLake::ListPathsOptions options;
      options.MaxResults = 2;
      auto response = m_fileSystemClient->ListPaths(true, options);
      EXPECT_LE(2U, response->Paths.size());
    }
  }
}}} // namespace Azure::Storage::Test
