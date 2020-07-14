// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "file_client_test.hpp"

#include <algorithm>

namespace Azure { namespace Storage { namespace Test {

  std::shared_ptr<Files::DataLake::FileClient> FileClientTest::m_fileClient;
  std::string FileClientTest::m_fileName;

  void FileClientTest::SetUpTestSuite()
  {
    FileSystemClientTest::SetUpTestSuite();
    m_fileName = LowercaseRandomString(10);
    m_fileClient = std::make_shared<Files::DataLake::FileClient>(
        m_fileSystemClient->GetFileClient(m_fileName));
    m_fileSystemClient->GetFileClient(m_fileName).Create();
  }

  void FileClientTest::TearDownTestSuite()
  {
    m_fileSystemClient->GetFileClient(m_fileName).Delete();
    FileSystemClientTest::TearDownTestSuite();
  }

  TEST_F(FileClientTest, CreateDeleteFiles)
  {
    {
      // Normal create/delete.
      std::vector<Files::DataLake::FileClient> fileClient;
      for (int32_t i = 0; i < 5; ++i)
      {
        auto client = m_fileSystemClient->GetFileClient(LowercaseRandomString());
        EXPECT_NO_THROW(client.Create());
        fileClient.emplace_back(std::move(client));
      }
      for (const auto& client : fileClient)
      {
        EXPECT_NO_THROW(client.Delete());
      }
    }
    {
      // Normal delete with last modified access condition.
      std::vector<Files::DataLake::FileClient> fileClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetFileClient(LowercaseRandomString());
        EXPECT_NO_THROW(client.Create());
        fileClient.emplace_back(std::move(client));
      }
      for (const auto& client : fileClient)
      {
        auto response = client.GetProperties();
        Files::DataLake::FileDeleteOptions options1;
        options1.AccessConditions.IfModifiedSince = response.LastModified;
        EXPECT_THROW(client.Delete(options1), StorageError);
        Files::DataLake::FileDeleteOptions options2;
        options2.AccessConditions.IfUnmodifiedSince = response.LastModified;
        EXPECT_NO_THROW(client.Delete(options2));
      }
    }
    {
      // Normal delete with if match access condition.
      std::vector<Files::DataLake::FileClient> fileClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetFileClient(LowercaseRandomString());
        EXPECT_NO_THROW(client.Create());
        fileClient.emplace_back(std::move(client));
      }
      for (const auto& client : fileClient)
      {
        auto response = client.GetProperties();
        Files::DataLake::FileDeleteOptions options1;
        options1.AccessConditions.IfNoneMatch = response.ETag;
        EXPECT_THROW(client.Delete(options1), StorageError);
        Files::DataLake::FileDeleteOptions options2;
        options2.AccessConditions.IfMatch = response.ETag;
        EXPECT_NO_THROW(client.Delete(options2));
      }
    }
  }

  TEST_F(FileClientTest, RenamePaths)
  {
    {
      // Normal create/rename/delete.
      std::vector<Files::DataLake::FileClient> fileClient;
      for (int32_t i = 0; i < 5; ++i)
      {
        auto client = m_fileSystemClient->GetFileClient(LowercaseRandomString());
        EXPECT_NO_THROW(client.Create());
        fileClient.emplace_back(std::move(client));
      }
      auto fileClientClone = fileClient;
      for (auto& client : fileClient)
      {
        EXPECT_NO_THROW(client.Rename(LowercaseRandomString()));
      }
      for (const auto& client : fileClientClone)
      {
        EXPECT_THROW(client.Delete(), StorageError);
      }
      for (const auto& client : fileClient)
      {
        EXPECT_NO_THROW(client.Delete());
      }
    }
    {
      // Normal rename with last modified access condition.
      std::vector<Files::DataLake::FileClient> fileClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetFileClient(LowercaseRandomString());
        EXPECT_NO_THROW(client.Create());
        fileClient.emplace_back(std::move(client));
      }
      for (auto& client : fileClient)
      {
        auto response = client.GetProperties();
        Files::DataLake::FileRenameOptions options1;
        options1.SourceAccessConditions.IfModifiedSince = response.LastModified;
        EXPECT_THROW(client.Rename(LowercaseRandomString(), options1), StorageError);
        Files::DataLake::FileRenameOptions options2;
        options2.SourceAccessConditions.IfUnmodifiedSince = response.LastModified;
        EXPECT_NO_THROW(client.Rename(LowercaseRandomString(), options2));
        EXPECT_NO_THROW(client.Delete());
      }
    }
    {
      // Normal rename with if match access condition.
      std::vector<Files::DataLake::FileClient> fileClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetFileClient(LowercaseRandomString());
        EXPECT_NO_THROW(client.Create());
        fileClient.emplace_back(std::move(client));
      }
      for (auto& client : fileClient)
      {
        auto response = client.GetProperties();
        Files::DataLake::FileRenameOptions options1;
        options1.SourceAccessConditions.IfNoneMatch = response.ETag;
        EXPECT_THROW(client.Rename(LowercaseRandomString(), options1), StorageError);
        Files::DataLake::FileRenameOptions options2;
        options2.SourceAccessConditions.IfMatch = response.ETag;
        EXPECT_NO_THROW(client.Rename(LowercaseRandomString(), options2));
        EXPECT_NO_THROW(client.Delete());
      }
    }
    {
      // Rename to a destination file system.
      std::vector<Files::DataLake::FileClient> fileClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetFileClient(LowercaseRandomString());
        EXPECT_NO_THROW(client.Create());
        fileClient.emplace_back(std::move(client));
      }
      {
        // Rename to a non-existing file system will fail but will not change URI.
        Files::DataLake::FileRenameOptions options;
        options.DestinationFileSystem = LowercaseRandomString();
        for (auto& client : fileClient)
        {
          EXPECT_THROW(client.Rename(LowercaseRandomString(), options), StorageError);
          EXPECT_NO_THROW(client.GetProperties());
        }
      }
      {
        // Rename to a non-existing file system will succeed and changes URI.
        auto newfileSystemName = LowercaseRandomString(10);
        auto newfileSystemClient = std::make_shared<Files::DataLake::FileSystemClient>(
            Files::DataLake::FileSystemClient::CreateFromConnectionString(
                ADLSGen2ConnectionString(), newfileSystemName));
        newfileSystemClient->Create();
        Files::DataLake::FileRenameOptions options;
        options.DestinationFileSystem = newfileSystemName;
        for (auto& client : fileClient)
        {
          EXPECT_NO_THROW(client.Rename(LowercaseRandomString(), options));
          EXPECT_NO_THROW(client.GetProperties());
          EXPECT_NO_THROW(client.Delete());
        }
      }
    }
  }

  TEST_F(FileClientTest, PathMetadata)
  {
    auto metadata1 = RandomMetadata();
    auto metadata2 = RandomMetadata();
    {
      // Set/Get Metadata works
      EXPECT_NO_THROW(m_fileClient->SetMetadata(metadata1));
      auto result = m_fileClient->GetProperties().Metadata;
      EXPECT_EQ(metadata1, result);
      EXPECT_NO_THROW(m_fileClient->SetMetadata(metadata2));
      result = m_fileClient->GetProperties().Metadata;
      EXPECT_EQ(metadata2, result);
    }

    {
      // Create path with metadata works
      auto client1 = m_fileSystemClient->GetFileClient(LowercaseRandomString());
      auto client2 = m_fileSystemClient->GetFileClient(LowercaseRandomString());
      Files::DataLake::FileCreateOptions options1;
      Files::DataLake::FileCreateOptions options2;
      options1.Metadata = metadata1;
      options2.Metadata = metadata2;

      EXPECT_NO_THROW(client1.Create(options1));
      EXPECT_NO_THROW(client2.Create(options2));
      auto result = client1.GetProperties().Metadata;
      EXPECT_EQ(metadata1, result);
      result = client2.GetProperties().Metadata;
      EXPECT_EQ(metadata2, result);
    }
  }

  TEST_F(FileClientTest, PathProperties)
  {
    auto metadata1 = RandomMetadata();
    auto metadata2 = RandomMetadata();
    {
      // Get Metadata via properties works
      EXPECT_NO_THROW(m_fileClient->SetMetadata(metadata1));
      auto result = m_fileClient->GetProperties();
      EXPECT_EQ(metadata1, result.Metadata);
      EXPECT_NO_THROW(m_fileClient->SetMetadata(metadata2));
      result = m_fileClient->GetProperties();
      EXPECT_EQ(metadata2, result.Metadata);
    }

    {
      // Last modified Etag works.
      auto properties1 = m_fileClient->GetProperties();
      auto properties2 = m_fileClient->GetProperties();
      EXPECT_EQ(properties1.ETag, properties2.ETag);
      EXPECT_EQ(properties1.LastModified, properties2.LastModified);

      // This operation changes ETag/LastModified.
      EXPECT_NO_THROW(m_fileClient->SetMetadata(metadata1));

      auto properties3 = m_fileClient->GetProperties();
      EXPECT_NE(properties1.ETag, properties3.ETag);
      EXPECT_NE(properties1.LastModified, properties3.LastModified);
    }

    {
      // Http headers works.
      auto httpHeader = GetInterestingHttpHeaders();
      std::vector<Files::DataLake::FileClient> fileClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetFileClient(LowercaseRandomString());
        Files::DataLake::FileCreateOptions options;
        options.HttpHeaders = httpHeader;
        EXPECT_NO_THROW(client.Create(options));
        fileClient.emplace_back(std::move(client));
      }
      for (const auto& client : fileClient)
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

  TEST_F(FileClientTest, PathDataActions)
  {
    const int32_t bufferSize = 4 * 1024; // 4KB data size
    auto buffer = RandomBuffer(bufferSize);
    auto bufferStream = std::make_unique<Azure::Core::Http::MemoryBodyStream>(
        Azure::Core::Http::MemoryBodyStream(buffer));
    auto properties1 = m_fileClient->GetProperties();

    // Append
    m_fileClient->AppendData(bufferStream.get(), 0);
    auto properties2 = m_fileClient->GetProperties();
    // Append does not change etag because not committed yet.
    EXPECT_EQ(properties1.ETag, properties2.ETag);
    EXPECT_EQ(properties1.LastModified, properties2.LastModified);

    // Flush
    m_fileClient->FlushData(bufferSize);
    auto properties3 = m_fileClient->GetProperties();
    EXPECT_NE(properties2.ETag, properties3.ETag);
    EXPECT_NE(properties2.LastModified, properties3.LastModified);

    // Read
    auto result = m_fileClient->Read();
    auto downloaded = ReadBodyStream(result.Body);
    EXPECT_EQ(buffer, downloaded);
  }

  TEST_F(FileClientTest, PathReadReturns)
  {
    const int32_t bufferSize = 4 * 1024; // 4KB data size
    auto buffer = RandomBuffer(bufferSize);
    auto bufferStream = std::make_unique<Azure::Core::Http::MemoryBodyStream>(
        Azure::Core::Http::MemoryBodyStream(buffer));
    auto properties1 = m_fileClient->GetProperties();

    // Append
    m_fileClient->AppendData(bufferStream.get(), 0);
    auto properties2 = m_fileClient->GetProperties();
    // Append does not change etag because not committed yet.
    EXPECT_EQ(properties1.ETag, properties2.ETag);
    EXPECT_EQ(properties1.LastModified, properties2.LastModified);

    // Flush
    m_fileClient->FlushData(bufferSize);
    auto properties3 = m_fileClient->GetProperties();
    EXPECT_NE(properties2.ETag, properties3.ETag);
    EXPECT_NE(properties2.LastModified, properties3.LastModified);

    // Read
    auto result = m_fileClient->Read();
    auto downloaded = ReadBodyStream(result.Body);
    EXPECT_EQ(buffer, downloaded);

    // Read Range
    {
      auto firstHalf = std::vector<uint8_t>(buffer.begin(), buffer.begin() + (bufferSize / 2) - 1);
      Files::DataLake::FileReadOptions options;
      options.Offset = 0;
      options.Length = bufferSize / 2;
      result = m_fileClient->Read(options);
      downloaded = ReadBodyStream(result.Body);
      EXPECT_EQ(firstHalf, downloaded);
    }
    {
      auto secondHalf = std::vector<uint8_t>(buffer.begin() + bufferSize / 2, buffer.end());
      Files::DataLake::FileReadOptions options;
      options.Offset = bufferSize / 2;
      options.Length = bufferSize / 2;
      result = m_fileClient->Read(options);
      downloaded = ReadBodyStream(result.Body);
      EXPECT_EQ(secondHalf, downloaded);
    }
    {
      // Read with last modified access condition.
      auto response = m_fileClient->GetProperties();
      Files::DataLake::FileReadOptions options1;
      options1.AccessConditions.IfModifiedSince = response.LastModified;
      EXPECT_THROW(m_fileClient->Read(options1), StorageError);
      Files::DataLake::FileReadOptions options2;
      options2.AccessConditions.IfUnmodifiedSince = response.LastModified;
      EXPECT_NO_THROW(result = m_fileClient->Read(options2));
      downloaded = ReadBodyStream(result.Body);
      EXPECT_EQ(buffer, downloaded);
    }
    {
      // Read with if match access condition.
      auto response = m_fileClient->GetProperties();
      Files::DataLake::FileReadOptions options1;
      options1.AccessConditions.IfNoneMatch = response.ETag;
      m_fileClient->Read(options1);
      EXPECT_THROW(m_fileClient->Read(options1), StorageError);
      Files::DataLake::FileReadOptions options2;
      options2.AccessConditions.IfMatch = response.ETag;
      EXPECT_NO_THROW(result = m_fileClient->Read(options2));
      downloaded = ReadBodyStream(result.Body);
      EXPECT_EQ(buffer, downloaded);
    }
  }

}}} // namespace Azure::Storage::Test
