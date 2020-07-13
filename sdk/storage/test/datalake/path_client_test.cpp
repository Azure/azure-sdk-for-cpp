// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "path_client_test.hpp"

#include <algorithm>

namespace Azure { namespace Storage { namespace Test {

  std::shared_ptr<DataLake::PathClient> PathClientTest::m_pathClient;
  std::string PathClientTest::m_pathName;

  void PathClientTest::SetUpTestSuite()
  {
    FileSystemClientTest::SetUpTestSuite();
    m_pathName = LowercaseRandomString(10);
    m_pathClient
        = std::make_shared<DataLake::PathClient>(m_fileSystemClient->GetPathClient(m_pathName));
    m_pathClient->CreateFile();
  }

  void PathClientTest::TearDownTestSuite()
  {
    m_pathClient->Delete();
    FileSystemClientTest::TearDownTestSuite();
  }

  TEST_F(PathClientTest, CreateDeletePaths)
  {
    {
      // Normal create/delete.
      std::vector<DataLake::PathClient> pathClient;
      for (int32_t i = 0; i < 5; ++i)
      {
        auto client = m_fileSystemClient->GetPathClient(LowercaseRandomString());
        EXPECT_NO_THROW(client.CreateFile());
        pathClient.emplace_back(std::move(client));
      }
      for (const auto& client : pathClient)
      {
        EXPECT_NO_THROW(client.Delete());
      }
    }
    {
      // Normal delete with last modified access condition.
      std::vector<DataLake::PathClient> pathClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetPathClient(LowercaseRandomString());
        EXPECT_NO_THROW(client.CreateFile());
        pathClient.emplace_back(std::move(client));
      }
      for (const auto& client : pathClient)
      {
        auto response = client.GetProperties();
        DataLake::PathDeleteOptions options1;
        options1.AccessConditions.IfModifiedSince = response.LastModified;
        EXPECT_THROW(client.Delete(options1), StorageError);
        DataLake::PathDeleteOptions options2;
        options2.AccessConditions.IfUnmodifiedSince = response.LastModified;
        EXPECT_NO_THROW(client.Delete(options2));
      }
    }
    {
      // Normal delete with if match access condition.
      std::vector<DataLake::PathClient> pathClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetPathClient(LowercaseRandomString());
        EXPECT_NO_THROW(client.CreateFile());
        pathClient.emplace_back(std::move(client));
      }
      for (const auto& client : pathClient)
      {
        auto response = client.GetProperties();
        DataLake::PathDeleteOptions options1;
        options1.AccessConditions.IfNoneMatch = response.ETag;
        EXPECT_THROW(client.Delete(options1), StorageError);
        DataLake::PathDeleteOptions options2;
        options2.AccessConditions.IfMatch = response.ETag;
        EXPECT_NO_THROW(client.Delete(options2));
      }
    }
  }

  TEST_F(PathClientTest, RenamePaths)
  {
    {
      // Normal create/rename/delete.
      std::vector<DataLake::PathClient> pathClient;
      for (int32_t i = 0; i < 5; ++i)
      {
        auto client = m_fileSystemClient->GetPathClient(LowercaseRandomString());
        EXPECT_NO_THROW(client.CreateFile());
        pathClient.emplace_back(std::move(client));
      }
      auto pathClientClone = pathClient;
      for (auto& client : pathClient)
      {
        EXPECT_NO_THROW(client.Rename(LowercaseRandomString()));
      }
      for (const auto& client : pathClientClone)
      {
        EXPECT_THROW(client.Delete(), StorageError);
      }
      for (const auto& client : pathClient)
      {
        EXPECT_NO_THROW(client.Delete());
      }
    }
    {
      // Normal rename with last modified access condition.
      std::vector<DataLake::PathClient> pathClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetPathClient(LowercaseRandomString());
        EXPECT_NO_THROW(client.CreateFile());
        pathClient.emplace_back(std::move(client));
      }
      for (auto& client : pathClient)
      {
        auto response = client.GetProperties();
        DataLake::PathRenameOptions options1;
        options1.SourceAccessConditions.IfModifiedSince = response.LastModified;
        EXPECT_THROW(client.Rename(LowercaseRandomString(), options1), StorageError);
        DataLake::PathRenameOptions options2;
        options2.SourceAccessConditions.IfUnmodifiedSince = response.LastModified;
        EXPECT_NO_THROW(client.Rename(LowercaseRandomString(), options2));
        EXPECT_NO_THROW(client.Delete());
      }
    }
    {
      // Normal rename with if match access condition.
      std::vector<DataLake::PathClient> pathClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetPathClient(LowercaseRandomString());
        EXPECT_NO_THROW(client.CreateFile());
        pathClient.emplace_back(std::move(client));
      }
      for (auto& client : pathClient)
      {
        auto response = client.GetProperties();
        DataLake::PathRenameOptions options1;
        options1.SourceAccessConditions.IfNoneMatch = response.ETag;
        EXPECT_THROW(client.Rename(LowercaseRandomString(), options1), StorageError);
        DataLake::PathRenameOptions options2;
        options2.SourceAccessConditions.IfMatch = response.ETag;
        EXPECT_NO_THROW(client.Rename(LowercaseRandomString(), options2));
        EXPECT_NO_THROW(client.Delete());
      }
    }
    {
      // Rename to a destination file system.
      std::vector<DataLake::PathClient> pathClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetPathClient(LowercaseRandomString());
        EXPECT_NO_THROW(client.CreateFile());
        pathClient.emplace_back(std::move(client));
      }
      {
        // Rename to a non-existing file system will fail but will not change URI.
        DataLake::PathRenameOptions options;
        options.DestinationFileSystem = LowercaseRandomString();
        for (auto& client : pathClient)
        {
          EXPECT_THROW(client.Rename(LowercaseRandomString(), options), StorageError);
          EXPECT_NO_THROW(client.GetProperties());
        }
      }
      {
        // Rename to a non-existing file system will succeed and changes URI.
        auto newfileSystemName = LowercaseRandomString(10);
        auto newfileSystemClient = std::make_shared<DataLake::FileSystemClient>(
            DataLake::FileSystemClient::CreateFromConnectionString(
                ADLSGen2ConnectionString(), newfileSystemName));
        newfileSystemClient->Create();
        DataLake::PathRenameOptions options;
        options.DestinationFileSystem = newfileSystemName;
        for (auto& client : pathClient)
        {
          EXPECT_NO_THROW(client.Rename(LowercaseRandomString(), options));
          EXPECT_NO_THROW(client.GetProperties());
          EXPECT_NO_THROW(client.Delete());
        }
      }
    }
  }

  TEST_F(PathClientTest, PathMetadata)
  {
    auto metadata1 = RandomMetadata();
    auto metadata2 = RandomMetadata();
    {
      // Set/Get Metadata works
      EXPECT_NO_THROW(m_pathClient->SetMetadata(metadata1));
      auto result = m_pathClient->GetProperties().Metadata;
      EXPECT_EQ(metadata1, result);
      EXPECT_NO_THROW(m_pathClient->SetMetadata(metadata2));
      result = m_pathClient->GetProperties().Metadata;
      EXPECT_EQ(metadata2, result);
    }

    {
      // Create path with metadata works
      auto client1 = m_fileSystemClient->GetPathClient(LowercaseRandomString());
      auto client2 = m_fileSystemClient->GetPathClient(LowercaseRandomString());
      DataLake::PathCreateOptions options1;
      DataLake::PathCreateOptions options2;
      options1.Metadata = metadata1;
      options2.Metadata = metadata2;

      EXPECT_NO_THROW(client1.CreateFile(options1));
      EXPECT_NO_THROW(client2.CreateFile(options2));
      auto result = client1.GetProperties().Metadata;
      EXPECT_EQ(metadata1, result);
      result = client2.GetProperties().Metadata;
      EXPECT_EQ(metadata2, result);
    }
  }

  TEST_F(PathClientTest, PathProperties)
  {
    auto metadata1 = RandomMetadata();
    auto metadata2 = RandomMetadata();
    {
      // Get Metadata via properties works
      EXPECT_NO_THROW(m_pathClient->SetMetadata(metadata1));
      auto result = m_pathClient->GetProperties();
      EXPECT_EQ(metadata1, result.Metadata);
      EXPECT_NO_THROW(m_pathClient->SetMetadata(metadata2));
      result = m_pathClient->GetProperties();
      EXPECT_EQ(metadata2, result.Metadata);
    }

    {
      // Last modified Etag works.
      auto properties1 = m_pathClient->GetProperties();
      auto properties2 = m_pathClient->GetProperties();
      EXPECT_EQ(properties1.ETag, properties2.ETag);
      EXPECT_EQ(properties1.LastModified, properties2.LastModified);

      // This operation changes ETag/LastModified.
      EXPECT_NO_THROW(m_pathClient->SetMetadata(metadata1));

      auto properties3 = m_pathClient->GetProperties();
      EXPECT_NE(properties1.ETag, properties3.ETag);
      EXPECT_NE(properties1.LastModified, properties3.LastModified);
    }

    {
      // Http headers works.
      auto httpHeader = GetInterestingHttpHeaders();
      std::vector<DataLake::PathClient> pathClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetPathClient(LowercaseRandomString());
        DataLake::PathCreateOptions options;
        options.HttpHeaders = httpHeader;
        EXPECT_NO_THROW(client.CreateFile(options));
        pathClient.emplace_back(std::move(client));
      }
      for (const auto& client : pathClient)
      {
        auto result = client.GetProperties();
        EXPECT_EQ(httpHeader.CacheControl, result.HttpHeaders.CacheControl);
        EXPECT_EQ(httpHeader.ContentDisposition, result.HttpHeaders.ContentDisposition);
        EXPECT_EQ(httpHeader.ContentLanguage, result.HttpHeaders.ContentLanguage);
        EXPECT_EQ(httpHeader.ContentType, result.HttpHeaders.ContentType);
        EXPECT_NO_THROW(client.Delete());
      }
    }
  }

  TEST_F(PathClientTest, PathDataActions)
  {
    const int32_t bufferSize = 4 * 1024; // 4KB data size
    auto buffer = RandomBuffer(bufferSize);
    auto bufferStream = std::make_unique<Azure::Core::Http::MemoryBodyStream>(
        Azure::Core::Http::MemoryBodyStream(buffer));
    auto properties1 = m_pathClient->GetProperties();

    // Append
    m_pathClient->AppendData(std::move(bufferStream), 0);
    auto properties2 = m_pathClient->GetProperties();
    // Append does not change etag because not committed yet.
    EXPECT_EQ(properties1.ETag, properties2.ETag);
    EXPECT_EQ(properties1.LastModified, properties2.LastModified);

    // Flush
    m_pathClient->FlushData(bufferSize);
    auto properties3 = m_pathClient->GetProperties();
    EXPECT_NE(properties2.ETag, properties3.ETag);
    EXPECT_NE(properties2.LastModified, properties3.LastModified);

    // Read
    auto result = m_pathClient->Read();
    auto downloaded = ReadBodyStream(result.Body);
    EXPECT_EQ(buffer, downloaded);
  }

  TEST_F(PathClientTest, PathReadReturns)
  {
    const int32_t bufferSize = 4 * 1024; // 4KB data size
    auto buffer = RandomBuffer(bufferSize);
    auto bufferStream = std::make_unique<Azure::Core::Http::MemoryBodyStream>(
        Azure::Core::Http::MemoryBodyStream(buffer));
    auto properties1 = m_pathClient->GetProperties();

    // Append
    m_pathClient->AppendData(std::move(bufferStream), 0);
    auto properties2 = m_pathClient->GetProperties();
    // Append does not change etag because not committed yet.
    EXPECT_EQ(properties1.ETag, properties2.ETag);
    EXPECT_EQ(properties1.LastModified, properties2.LastModified);

    // Flush
    m_pathClient->FlushData(bufferSize);
    auto properties3 = m_pathClient->GetProperties();
    EXPECT_NE(properties2.ETag, properties3.ETag);
    EXPECT_NE(properties2.LastModified, properties3.LastModified);

    // Read
    auto result = m_pathClient->Read();
    auto downloaded = ReadBodyStream(result.Body);
    EXPECT_EQ(buffer, downloaded);

    // Read Range
    {
      auto firstHalf = std::vector<uint8_t>(buffer.begin(), buffer.begin() + (bufferSize / 2) - 1);
      DataLake::PathReadOptions options;
      options.Offset = 0;
      options.Length = bufferSize / 2;
      result = m_pathClient->Read(options);
      downloaded = ReadBodyStream(result.Body);
      EXPECT_EQ(firstHalf, downloaded);
    }
    {
      auto secondHalf = std::vector<uint8_t>(buffer.begin() + bufferSize / 2, buffer.end());
      DataLake::PathReadOptions options;
      options.Offset = bufferSize / 2;
      options.Length = bufferSize / 2;
      result = m_pathClient->Read(options);
      downloaded = ReadBodyStream(result.Body);
      EXPECT_EQ(secondHalf, downloaded);
    }
    {
      // Read with last modified access condition.
      auto response = m_pathClient->GetProperties();
      DataLake::PathReadOptions options1;
      options1.AccessConditions.IfModifiedSince = response.LastModified;
      EXPECT_THROW(m_pathClient->Read(options1), StorageError);
      DataLake::PathReadOptions options2;
      options2.AccessConditions.IfUnmodifiedSince = response.LastModified;
      EXPECT_NO_THROW(result = m_pathClient->Read(options2));
      downloaded = ReadBodyStream(result.Body);
      EXPECT_EQ(buffer, downloaded);
    }
    {
      // Read with if match access condition.
      auto response = m_pathClient->GetProperties();
      DataLake::PathReadOptions options1;
      options1.AccessConditions.IfNoneMatch = response.ETag;
      m_pathClient->Read(options1);
      EXPECT_THROW(m_pathClient->Read(options1), StorageError);
      DataLake::PathReadOptions options2;
      options2.AccessConditions.IfMatch = response.ETag;
      EXPECT_NO_THROW(result = m_pathClient->Read(options2));
      downloaded = ReadBodyStream(result.Body);
      EXPECT_EQ(buffer, downloaded);
    }
  }

}}} // namespace Azure::Storage::Test
