// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake_file_client_test.hpp"

#include <algorithm>

namespace Azure { namespace Storage { namespace Test {

  std::shared_ptr<Files::DataLake::FileClient> DataLakeFileClientTest::m_fileClient;
  std::string DataLakeFileClientTest::m_fileName;

  void DataLakeFileClientTest::SetUpTestSuite()
  {
    DataLakeFileSystemClientTest::SetUpTestSuite();
    m_fileName = LowercaseRandomString(10);
    m_fileClient = std::make_shared<Files::DataLake::FileClient>(
        m_fileSystemClient->GetFileClient(m_fileName));
    m_fileClient->Create();
  }

  void DataLakeFileClientTest::TearDownTestSuite()
  {
    m_fileSystemClient->GetFileClient(m_fileName).Delete();
    DataLakeFileSystemClientTest::TearDownTestSuite();
  }

  TEST_F(DataLakeFileClientTest, CreateDeleteFiles)
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
        options1.AccessConditions.IfModifiedSince = response->LastModified;
        EXPECT_THROW(client.Delete(options1), StorageError);
        Files::DataLake::FileDeleteOptions options2;
        options2.AccessConditions.IfUnmodifiedSince = response->LastModified;
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
        options1.AccessConditions.IfNoneMatch = response->ETag;
        EXPECT_THROW(client.Delete(options1), StorageError);
        Files::DataLake::FileDeleteOptions options2;
        options2.AccessConditions.IfMatch = response->ETag;
        EXPECT_NO_THROW(client.Delete(options2));
      }
    }
  }

  TEST_F(DataLakeFileClientTest, RenameFiles)
  {
    {
      // Normal create/rename/delete.
      std::vector<Files::DataLake::FileClient> fileClients;
      for (int32_t i = 0; i < 5; ++i)
      {
        auto client = m_fileSystemClient->GetFileClient(LowercaseRandomString());
        EXPECT_NO_THROW(client.Create());
        fileClients.emplace_back(std::move(client));
      }
      std::vector<std::string> newPaths;
      for (auto& client : fileClients)
      {
        auto newPath = LowercaseRandomString();
        EXPECT_NO_THROW(client.Rename(newPath));
        newPaths.push_back(newPath);
      }
      for (const auto& client : fileClients)
      {
        EXPECT_THROW(client.Delete(), StorageError);
      }
      for (const auto& newPath : newPaths)
      {
        EXPECT_NO_THROW(m_fileSystemClient->GetDirectoryClient(newPath).Delete(false));
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
        Files::DataLake::RenameFileOptions options1;
        options1.SourceAccessConditions.IfModifiedSince = response->LastModified;
        EXPECT_THROW(client.Rename(LowercaseRandomString(), options1), StorageError);
        Files::DataLake::RenameFileOptions options2;
        options2.SourceAccessConditions.IfUnmodifiedSince = response->LastModified;
        auto newPath = LowercaseRandomString();
        EXPECT_NO_THROW(client.Rename(newPath, options2));
        EXPECT_NO_THROW(m_fileSystemClient->GetDirectoryClient(newPath).Delete(false));
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
        Files::DataLake::RenameFileOptions options1;
        options1.SourceAccessConditions.IfNoneMatch = response->ETag;
        EXPECT_THROW(client.Rename(LowercaseRandomString(), options1), StorageError);
        Files::DataLake::RenameFileOptions options2;
        options2.SourceAccessConditions.IfMatch = response->ETag;
        auto newPath = LowercaseRandomString();
        EXPECT_NO_THROW(client.Rename(newPath, options2));
        EXPECT_NO_THROW(m_fileSystemClient->GetDirectoryClient(newPath).Delete(false));
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
        Files::DataLake::RenameFileOptions options;
        options.DestinationFileSystem = LowercaseRandomString();
        for (auto& client : fileClient)
        {
          EXPECT_THROW(client.Rename(LowercaseRandomString(), options), StorageError);
          EXPECT_NO_THROW(client.GetProperties());
        }
      }
      {
        // Rename to an existing file system will succeed and changes URI.
        auto newfileSystemName = LowercaseRandomString(10);
        auto newfileSystemClient = std::make_shared<Files::DataLake::FileSystemClient>(
            Files::DataLake::FileSystemClient::CreateFromConnectionString(
                AdlsGen2ConnectionString(), newfileSystemName));
        newfileSystemClient->Create();
        Files::DataLake::RenameFileOptions options;
        options.DestinationFileSystem = newfileSystemName;
        for (auto& client : fileClient)
        {
          auto newPath = LowercaseRandomString();
          EXPECT_NO_THROW(client.Rename(newPath, options));
          EXPECT_NO_THROW(newfileSystemClient->GetDirectoryClient(newPath).Delete(false));
        }
      }
    }
  }

  TEST_F(DataLakeFileClientTest, FileMetadata)
  {
    auto metadata1 = RandomMetadata();
    auto metadata2 = RandomMetadata();
    {
      // Set/Get Metadata works
      EXPECT_NO_THROW(m_fileClient->SetMetadata(metadata1));
      auto result = m_fileClient->GetProperties()->Metadata;
      EXPECT_EQ(metadata1, result);
      EXPECT_NO_THROW(m_fileClient->SetMetadata(metadata2));
      result = m_fileClient->GetProperties()->Metadata;
      EXPECT_EQ(metadata2, result);
    }

    {
      // Create path with metadata works
      auto client1 = m_fileSystemClient->GetFileClient(LowercaseRandomString());
      auto client2 = m_fileSystemClient->GetFileClient(LowercaseRandomString());
      Files::DataLake::CreateFileOptions options1;
      Files::DataLake::CreateFileOptions options2;
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

  TEST_F(DataLakeFileClientTest, FileProperties)
  {
    auto metadata1 = RandomMetadata();
    auto metadata2 = RandomMetadata();
    {
      // Get Metadata via properties works
      EXPECT_NO_THROW(m_fileClient->SetMetadata(metadata1));
      auto result = m_fileClient->GetProperties();
      EXPECT_EQ(metadata1, result->Metadata);
      EXPECT_NO_THROW(m_fileClient->SetMetadata(metadata2));
      result = m_fileClient->GetProperties();
      EXPECT_EQ(metadata2, result->Metadata);
    }

    {
      // Last modified Etag works.
      auto properties1 = m_fileClient->GetProperties();
      auto properties2 = m_fileClient->GetProperties();
      EXPECT_EQ(properties1->ETag, properties2->ETag);
      EXPECT_EQ(properties1->LastModified, properties2->LastModified);

      // This operation changes ETag/LastModified.
      EXPECT_NO_THROW(m_fileClient->SetMetadata(metadata1));

      auto properties3 = m_fileClient->GetProperties();
      EXPECT_NE(properties1->ETag, properties3->ETag);
    }

    {
      // Http headers works.
      auto httpHeader = GetInterestingHttpHeaders();
      std::vector<Files::DataLake::FileClient> fileClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetFileClient(LowercaseRandomString());
        Files::DataLake::CreateFileOptions options;
        options.HttpHeaders = httpHeader;
        EXPECT_NO_THROW(client.Create(options));
        fileClient.emplace_back(std::move(client));
      }
      for (const auto& client : fileClient)
      {
        auto result = client.GetProperties();
        EXPECT_EQ(httpHeader.CacheControl, result->HttpHeaders.CacheControl);
        EXPECT_EQ(httpHeader.ContentDisposition, result->HttpHeaders.ContentDisposition);
        EXPECT_EQ(httpHeader.ContentLanguage, result->HttpHeaders.ContentLanguage);
        EXPECT_EQ(httpHeader.ContentType, result->HttpHeaders.ContentType);
        EXPECT_NO_THROW(client.Delete());
      }
    }
  }

  TEST_F(DataLakeFileClientTest, FileDataActions)
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
    EXPECT_EQ(properties1->ETag, properties2->ETag);
    EXPECT_EQ(properties1->LastModified, properties2->LastModified);

    // Flush
    m_fileClient->FlushData(bufferSize);
    auto properties3 = m_fileClient->GetProperties();
    EXPECT_NE(properties2->ETag, properties3->ETag);
    EXPECT_NE(properties2->LastModified, properties3->LastModified);

    // Read
    auto result = m_fileClient->Read();
    auto downloaded = ReadBodyStream(result->Body);
    EXPECT_EQ(buffer, downloaded);
  }

  TEST_F(DataLakeFileClientTest, FileReadReturns)
  {
    const int32_t bufferSize = 4 * 1024; // 4KB data size
    auto buffer = RandomBuffer(bufferSize);
    auto bufferStream = std::make_unique<Azure::Core::Http::MemoryBodyStream>(
        Azure::Core::Http::MemoryBodyStream(buffer));
    auto newFileName = LowercaseRandomString(10);
    auto newFileClient = std::make_shared<Files::DataLake::FileClient>(
        m_fileSystemClient->GetFileClient(newFileName));
    newFileClient->Create();
    auto properties1 = newFileClient->GetProperties();

    // Append
    newFileClient->AppendData(bufferStream.get(), 0);
    auto properties2 = newFileClient->GetProperties();
    // Append does not change etag because not committed yet.
    EXPECT_EQ(properties1->ETag, properties2->ETag);
    EXPECT_EQ(properties1->LastModified, properties2->LastModified);

    // Flush
    newFileClient->FlushData(bufferSize);
    auto properties3 = newFileClient->GetProperties();
    EXPECT_NE(properties2->ETag, properties3->ETag);
    EXPECT_NE(properties2->LastModified, properties3->LastModified);

    // Read
    auto result = newFileClient->Read();
    auto downloaded = ReadBodyStream(result->Body);
    EXPECT_EQ(buffer, downloaded);

    // Read Range
    {
      auto firstHalf = std::vector<uint8_t>(buffer.begin(), buffer.begin() + (bufferSize / 2));
      Files::DataLake::ReadFileOptions options;
      options.Offset = 0;
      options.Length = bufferSize / 2;
      result = newFileClient->Read(options);
      downloaded = ReadBodyStream(result->Body);
      EXPECT_EQ(firstHalf.size(), downloaded.size());
      EXPECT_EQ(firstHalf, downloaded);
    }
    {
      auto secondHalf = std::vector<uint8_t>(buffer.begin() + bufferSize / 2, buffer.end());
      Files::DataLake::ReadFileOptions options;
      options.Offset = bufferSize / 2;
      options.Length = bufferSize / 2;
      result = newFileClient->Read(options);
      downloaded = ReadBodyStream(result->Body);
      EXPECT_EQ(secondHalf, downloaded);
    }
    {
      // Read with last modified access condition.
      auto response = newFileClient->GetProperties();
      Files::DataLake::ReadFileOptions options1;
      options1.AccessConditions.IfModifiedSince = response->LastModified;
      EXPECT_THROW(newFileClient->Read(options1), StorageError);
      Files::DataLake::ReadFileOptions options2;
      options2.AccessConditions.IfUnmodifiedSince = response->LastModified;
      EXPECT_NO_THROW(result = newFileClient->Read(options2));
      downloaded = ReadBodyStream(result->Body);
      EXPECT_EQ(buffer, downloaded);
    }
    {
      // Read with if match access condition.
      auto response = newFileClient->GetProperties();
      Files::DataLake::ReadFileOptions options1;
      options1.AccessConditions.IfNoneMatch = response->ETag;
      EXPECT_THROW(newFileClient->Read(options1), StorageError);
      Files::DataLake::ReadFileOptions options2;
      options2.AccessConditions.IfMatch = response->ETag;
      EXPECT_NO_THROW(result = newFileClient->Read(options2));
      downloaded = ReadBodyStream(result->Body);
      EXPECT_EQ(buffer, downloaded);
    }
  }

}}} // namespace Azure::Storage::Test
