// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "file_system_client_test.hpp"

#include <algorithm>

namespace Azure { namespace Storage { namespace Test {

  const size_t c_PATH_TEST_SIZE = 5;

  std::shared_ptr<DataLake::FileSystemClient> FileSystemClientTest::m_fileSystemClient;
  std::string FileSystemClientTest::m_fileSystemName;

  std::vector<std::string> FileSystemClientTest::m_PathNameSetA;
  std::vector<std::string> FileSystemClientTest::m_PathNameSetB;
  std::string FileSystemClientTest::m_DirectoryA;
  std::string FileSystemClientTest::m_DirectoryB;

  void FileSystemClientTest::SetUpTestSuite()
  {
    m_fileSystemName = LowercaseRandomString(10);
    m_fileSystemClient = std::make_shared<DataLake::FileSystemClient>(
        DataLake::FileSystemClient::CreateFromConnectionString(
            ADLSGen2ConnectionString(), m_fileSystemName));
    m_fileSystemClient->Create();

    m_DirectoryA = LowercaseRandomString(10);
    m_DirectoryB = LowercaseRandomString(10);
    for (unsigned i = 0; i < c_PATH_TEST_SIZE; ++i)
    {
      {
        auto name = m_DirectoryA + LowercaseRandomString(10);
        m_fileSystemClient->GetPathClient(m_DirectoryA + "/" + name).CreateAsFile();
        m_PathNameSetA.emplace_back(std::move(name));
      }
      {
        auto name = m_DirectoryB + LowercaseRandomString(10);
        m_fileSystemClient->GetPathClient(m_DirectoryB + "/" + name).CreateAsFile();
        m_PathNameSetB.emplace_back(std::move(name));
      }
    }
  }

  void FileSystemClientTest::TearDownTestSuite() { m_fileSystemClient->Delete(); }

  std::vector<DataLake::Path> FileSystemClientTest::ListAllPaths(
      bool recursive,
      const std::string& directory)
  {
    std::vector<DataLake::Path> result;
    std::string continuation;
    DataLake::ListPathsOptions options;
    if (!directory.empty())
    {
      options.Directory = directory;
    }
    do
    {
      auto response = m_fileSystemClient->ListPaths(recursive, options);
      result.insert(result.end(), response.Paths.begin(), response.Paths.end());
      if (response.Continuation.HasValue())
      {
        continuation = response.Continuation.GetValue();
        options.Continuation = continuation;
      }
    } while (!continuation.empty());
    return result;
  }

  DataLake::DataLakeHttpHeaders FileSystemClientTest::GetInterestingHttpHeaders()
  {
    static DataLake::DataLakeHttpHeaders result{
        std::string("no-cache"),
        std::string("attachment"),
        std::string("deflate"),
        std::string("en-US"),
        std::string("text/html; charset=UTF-8")};
    return result;
  }

  TEST_F(FileSystemClientTest, CreateDeleteFileSystems)
  {
    {
      // Normal create/delete.
      std::vector<DataLake::FileSystemClient> fileSystemClient;
      for (int32_t i = 0; i < 5; ++i)
      {
        auto client = DataLake::FileSystemClient::CreateFromConnectionString(
            ADLSGen2ConnectionString(), LowercaseRandomString(10));
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
      std::vector<DataLake::FileSystemClient> fileSystemClient;
      for (int32_t i = 0; i < 5; ++i)
      {
        auto client = DataLake::FileSystemClient::CreateFromConnectionString(
            ADLSGen2ConnectionString(), LowercaseRandomString(10));
        EXPECT_NO_THROW(client.Create());
        fileSystemClient.emplace_back(std::move(client));
      }
      for (const auto& client : fileSystemClient)
      {
        auto response = client.GetProperties();
        DataLake::FileSystemDeleteOptions options1;
        options1.IfModifiedSince = response.LastModified;
        EXPECT_THROW(client.Delete(options1), StorageError);
        DataLake::FileSystemDeleteOptions options2;
        options2.IfUnmodifiedSince = response.LastModified;
        EXPECT_NO_THROW(client.Delete(options2));
      }
    }
  }

  TEST_F(FileSystemClientTest, FileSystemMetadata)
  {
    auto metadata1 = RandomMetadata();
    auto metadata2 = RandomMetadata();
    {
      // Set/Get Metadata works
      EXPECT_NO_THROW(m_fileSystemClient->SetMetadata(metadata1));
      auto result = m_fileSystemClient->GetMetadata();
      EXPECT_EQ(metadata1, result);
      EXPECT_NO_THROW(m_fileSystemClient->SetMetadata(metadata2));
      result = m_fileSystemClient->GetMetadata();
      EXPECT_EQ(metadata2, result);
    }

    {
      // Create file system with metadata works
      auto client1 = DataLake::FileSystemClient::CreateFromConnectionString(
          ADLSGen2ConnectionString(), LowercaseRandomString(10));
      auto client2 = DataLake::FileSystemClient::CreateFromConnectionString(
          ADLSGen2ConnectionString(), LowercaseRandomString(10));
      DataLake::FileSystemCreateOptions options1;
      DataLake::FileSystemCreateOptions options2;
      options1.Metadata = metadata1;
      options2.Metadata = metadata2;

      EXPECT_NO_THROW(client1.Create(options1));
      EXPECT_NO_THROW(client2.Create(options2));
      auto result = client1.GetMetadata();
      EXPECT_EQ(metadata1, result);
      result = client2.GetMetadata();
      EXPECT_EQ(metadata2, result);
    }
  }

  TEST_F(FileSystemClientTest, FileSystemProperties)
  {
    auto metadata1 = RandomMetadata();
    auto metadata2 = RandomMetadata();
    {
      // Get Metadata via properties works
      EXPECT_NO_THROW(m_fileSystemClient->SetMetadata(metadata1));
      auto result = m_fileSystemClient->GetProperties();
      EXPECT_EQ(metadata1, result.Metadata);
      EXPECT_NO_THROW(m_fileSystemClient->SetMetadata(metadata2));
      result = m_fileSystemClient->GetProperties();
      EXPECT_EQ(metadata2, result.Metadata);
    }

    {
      // Last modified Etag works.
      auto properties1 = m_fileSystemClient->GetProperties();
      auto properties2 = m_fileSystemClient->GetProperties();
      EXPECT_EQ(properties1.ETag, properties2.ETag);
      EXPECT_EQ(properties1.LastModified, properties2.LastModified);
      EXPECT_EQ(properties1.NamespaceEnabled, properties2.NamespaceEnabled);

      // This operation changes ETag/LastModified.
      EXPECT_NO_THROW(m_fileSystemClient->SetMetadata(metadata1));

      auto properties3 = m_fileSystemClient->GetProperties();
      EXPECT_NE(properties1.ETag, properties3.ETag);
      EXPECT_NE(properties1.LastModified, properties3.LastModified);
      EXPECT_EQ(properties1.NamespaceEnabled, properties3.NamespaceEnabled);
    }
  }

  TEST_F(FileSystemClientTest, ListPaths)
  {
    {
      // Normal list recursively.
      auto result = ListAllPaths(true);
      for (const auto& name : m_PathNameSetA)
      {
        auto iter = std::find_if(result.begin(), result.end(), [&name](const DataLake::Path& path) {
          return path.Name == name;
        });
        EXPECT_EQ(iter->Name.substr(0U, m_DirectoryA.size()), m_DirectoryA);
        EXPECT_NE(result.end(), iter);
      }
      for (const auto& name : m_PathNameSetB)
      {
        auto iter = std::find_if(result.begin(), result.end(), [&name](const DataLake::Path& path) {
          return path.Name == name;
        });
        EXPECT_EQ(iter->Name.substr(0U, m_DirectoryB.size()), m_DirectoryB);
        EXPECT_NE(result.end(), iter);
      }
    }
    {
      // List with directory.
      auto result = ListAllPaths(true, m_DirectoryA);
      for (const auto& name : m_PathNameSetA)
      {
        auto iter = std::find_if(result.begin(), result.end(), [&name](const DataLake::Path& path) {
          return path.Name == name;
        });
        EXPECT_EQ(iter->Name.substr(0U, m_DirectoryA.size()), m_DirectoryA);
        EXPECT_NE(result.end(), iter);
      }
      for (const auto& name : m_PathNameSetA)
      {
        auto iter = std::find_if(result.begin(), result.end(), [&name](const DataLake::Path& path) {
          return path.Name == name;
        });
        EXPECT_EQ(result.end(), iter);
      }
    }
    {
      // List max result
      DataLake::ListPathsOptions options;
      options.MaxResults = 2;
      auto response = m_fileSystemClient->ListPaths(true, options);
      EXPECT_LE(2U, response.Paths.size());
    }
  }

}}} // namespace Azure::Storage::Test
