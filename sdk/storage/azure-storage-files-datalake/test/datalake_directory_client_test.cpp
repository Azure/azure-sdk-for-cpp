// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake_directory_client_test.hpp"

#include <algorithm>

namespace Azure { namespace Storage { namespace Test {

  std::shared_ptr<Files::DataLake::DirectoryClient> DataLakeDirectoryClientTest::m_directoryClient;
  std::string DataLakeDirectoryClientTest::m_directoryName;

  void DataLakeDirectoryClientTest::SetUpTestSuite()
  {
    DataLakeFileSystemClientTest::SetUpTestSuite();
    m_directoryName = LowercaseRandomString(10);
    m_directoryClient = std::make_shared<Files::DataLake::DirectoryClient>(
        m_fileSystemClient->GetDirectoryClient(m_directoryName));
    m_fileSystemClient->GetFileClient(m_directoryName).Create();
  }

  void DataLakeDirectoryClientTest::TearDownTestSuite()
  {
    m_fileSystemClient->GetFileClient(m_directoryName).Delete();
    DataLakeFileSystemClientTest::TearDownTestSuite();
  }

  TEST_F(DataLakeDirectoryClientTest, CreateDeleteDirectory)
  {
    {
      // Normal create/delete.
      std::vector<Files::DataLake::DirectoryClient> directoryClient;
      for (int32_t i = 0; i < 5; ++i)
      {
        auto client = m_fileSystemClient->GetDirectoryClient(LowercaseRandomString());
        EXPECT_NO_THROW(client.Create());
        directoryClient.emplace_back(std::move(client));
      }
      for (const auto& client : directoryClient)
      {
        EXPECT_NO_THROW(client.Delete(false));
      }
    }
    {
      // Normal delete with last modified access condition.
      std::vector<Files::DataLake::DirectoryClient> directoryClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetDirectoryClient(LowercaseRandomString());
        EXPECT_NO_THROW(client.Create());
        directoryClient.emplace_back(std::move(client));
      }
      for (const auto& client : directoryClient)
      {
        auto response = client.GetProperties();
        Files::DataLake::DeleteDirectoryOptions options1;
        options1.AccessConditions.IfModifiedSince = response->LastModified;
        EXPECT_THROW(client.Delete(false, options1), StorageError);
        Files::DataLake::DeleteDirectoryOptions options2;
        options2.AccessConditions.IfUnmodifiedSince = response->LastModified;
        EXPECT_NO_THROW(client.Delete(false, options2));
      }
    }
    {
      // Normal delete with if match access condition.
      std::vector<Files::DataLake::DirectoryClient> directoryClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetDirectoryClient(LowercaseRandomString());
        EXPECT_NO_THROW(client.Create());
        directoryClient.emplace_back(std::move(client));
      }
      for (const auto& client : directoryClient)
      {
        auto response = client.GetProperties();
        Files::DataLake::DeleteDirectoryOptions options1;
        options1.AccessConditions.IfNoneMatch = response->ETag;
        EXPECT_THROW(client.Delete(false, options1), StorageError);
        Files::DataLake::DeleteDirectoryOptions options2;
        options2.AccessConditions.IfMatch = response->ETag;
        EXPECT_NO_THROW(client.Delete(false, options2));
      }
    }

    {
      // Recursive delete works.
      std::vector<Files::DataLake::DirectoryClient> directoryClient;
      auto rootDir = LowercaseRandomString();
      auto rootDirClient = m_fileSystemClient->GetDirectoryClient(rootDir);
      EXPECT_NO_THROW(rootDirClient.Create());
      for (int32_t i = 0; i < 5; ++i)
      {
        auto client
            = m_fileSystemClient->GetDirectoryClient(rootDir + "/" + LowercaseRandomString());
        EXPECT_NO_THROW(client.Create());
        directoryClient.emplace_back(std::move(client));
      }
      EXPECT_THROW(rootDirClient.Delete(false), StorageError);
      EXPECT_NO_THROW(rootDirClient.Delete(true));
    }
  }

  TEST_F(DataLakeDirectoryClientTest, RenameDirectory)
  {
    {
      // Normal create/rename/delete.
      std::vector<Files::DataLake::DirectoryClient> directoryClients;
      for (int32_t i = 0; i < 5; ++i)
      {
        auto client = m_fileSystemClient->GetDirectoryClient(LowercaseRandomString());
        EXPECT_NO_THROW(client.Create());
        directoryClients.emplace_back(std::move(client));
      }
      std::vector<std::string> newPaths;
      for (auto& client : directoryClients)
      {
        auto newPath = LowercaseRandomString();
        EXPECT_NO_THROW(client.Rename(newPath));
        newPaths.push_back(newPath);
      }
      for (const auto& client : directoryClients)
      {
        EXPECT_THROW(client.Delete(false), StorageError);
      }
      for (const auto& newPath : newPaths)
      {
        EXPECT_NO_THROW(m_fileSystemClient->GetDirectoryClient(newPath).Delete(false));
      }
    }
    {
      // Normal rename with last modified access condition.
      std::vector<Files::DataLake::DirectoryClient> directoryClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetDirectoryClient(LowercaseRandomString());
        EXPECT_NO_THROW(client.Create());
        directoryClient.emplace_back(std::move(client));
      }
      for (auto& client : directoryClient)
      {
        auto response = client.GetProperties();
        Files::DataLake::RenameDirectoryOptions options1;
        options1.SourceAccessConditions.IfModifiedSince = response->LastModified;
        EXPECT_THROW(client.Rename(LowercaseRandomString(), options1), StorageError);
        Files::DataLake::RenameDirectoryOptions options2;
        options2.SourceAccessConditions.IfUnmodifiedSince = response->LastModified;
        auto newPath = LowercaseRandomString();
        EXPECT_NO_THROW(client.Rename(newPath, options2));
        EXPECT_NO_THROW(m_fileSystemClient->GetDirectoryClient(newPath).Delete(false));
      }
    }
    {
      // Normal rename with if match access condition.
      std::vector<Files::DataLake::DirectoryClient> directoryClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetDirectoryClient(LowercaseRandomString());
        EXPECT_NO_THROW(client.Create());
        directoryClient.emplace_back(std::move(client));
      }
      for (auto& client : directoryClient)
      {
        auto response = client.GetProperties();
        Files::DataLake::RenameDirectoryOptions options1;
        options1.SourceAccessConditions.IfNoneMatch = response->ETag;
        EXPECT_THROW(client.Rename(LowercaseRandomString(), options1), StorageError);
        Files::DataLake::RenameDirectoryOptions options2;
        options2.SourceAccessConditions.IfMatch = response->ETag;
        auto newPath = LowercaseRandomString();
        EXPECT_NO_THROW(client.Rename(newPath, options2));
        EXPECT_NO_THROW(m_fileSystemClient->GetDirectoryClient(newPath).Delete(false));
      }
    }
    {
      // Rename to a destination file system.
      std::vector<Files::DataLake::DirectoryClient> directoryClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetDirectoryClient(LowercaseRandomString());
        EXPECT_NO_THROW(client.Create());
        directoryClient.emplace_back(std::move(client));
      }
      {
        // Rename to a non-existing file system will fail and source is not changed.
        Files::DataLake::RenameDirectoryOptions options;
        options.DestinationFileSystem = LowercaseRandomString();
        for (auto& client : directoryClient)
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
        Files::DataLake::RenameDirectoryOptions options;
        options.DestinationFileSystem = newfileSystemName;
        for (auto& client : directoryClient)
        {
          auto newPath = LowercaseRandomString();
          EXPECT_NO_THROW(client.Rename(newPath, options));
          EXPECT_NO_THROW(newfileSystemClient->GetDirectoryClient(newPath).Delete(false));
        }
      }
    }
  }

  TEST_F(DataLakeDirectoryClientTest, DirectoryMetadata)
  {
    auto metadata1 = RandomMetadata();
    auto metadata2 = RandomMetadata();
    {
      // Set/Get Metadata works
      EXPECT_NO_THROW(m_directoryClient->SetMetadata(metadata1));
      auto result = m_directoryClient->GetProperties()->Metadata;
      EXPECT_EQ(metadata1, result);
      EXPECT_NO_THROW(m_directoryClient->SetMetadata(metadata2));
      result = m_directoryClient->GetProperties()->Metadata;
      EXPECT_EQ(metadata2, result);
    }

    {
      // Create path with metadata works
      auto client1 = m_fileSystemClient->GetDirectoryClient(LowercaseRandomString());
      auto client2 = m_fileSystemClient->GetDirectoryClient(LowercaseRandomString());
      Files::DataLake::CreatePathOptions options1;
      Files::DataLake::CreatePathOptions options2;
      options1.Metadata = metadata1;
      options2.Metadata = metadata2;

      EXPECT_NO_THROW(client1.Create(options1));
      EXPECT_NO_THROW(client2.Create(options2));
      auto result = client1.GetProperties()->Metadata;
      metadata1["hdi_isfolder"] = "true";
      metadata2["hdi_isfolder"] = "true";
      EXPECT_EQ(metadata1, result);
      result = client2.GetProperties()->Metadata;
      EXPECT_EQ(metadata2, result);
    }
  }

  TEST_F(DataLakeDirectoryClientTest, DirectoryProperties)
  {
    auto metadata1 = RandomMetadata();
    auto metadata2 = RandomMetadata();
    {
      // Get Metadata via properties works
      EXPECT_NO_THROW(m_directoryClient->SetMetadata(metadata1));
      auto result = m_directoryClient->GetProperties();
      EXPECT_EQ(metadata1, result->Metadata);
      EXPECT_NO_THROW(m_directoryClient->SetMetadata(metadata2));
      result = m_directoryClient->GetProperties();
      EXPECT_EQ(metadata2, result->Metadata);
    }

    {
      // Last modified Etag works.
      auto properties1 = m_directoryClient->GetProperties();
      auto properties2 = m_directoryClient->GetProperties();
      EXPECT_EQ(properties1->ETag, properties2->ETag);
      EXPECT_EQ(properties1->LastModified, properties2->LastModified);

      // This operation changes ETag/LastModified.
      EXPECT_NO_THROW(m_directoryClient->SetMetadata(metadata1));

      auto properties3 = m_directoryClient->GetProperties();
      EXPECT_NE(properties1->ETag, properties3->ETag);
    }

    {
      // Http headers works.
      auto httpHeader = GetInterestingHttpHeaders();
      std::vector<Files::DataLake::DirectoryClient> directoryClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetDirectoryClient(LowercaseRandomString());
        Files::DataLake::CreatePathOptions options;
        options.HttpHeaders = httpHeader;
        EXPECT_NO_THROW(client.Create(options));
        directoryClient.emplace_back(std::move(client));
      }
      for (const auto& client : directoryClient)
      {
        auto result = client.GetProperties();
        EXPECT_EQ(httpHeader.CacheControl, result->HttpHeaders.CacheControl);
        EXPECT_EQ(httpHeader.ContentDisposition, result->HttpHeaders.ContentDisposition);
        EXPECT_EQ(httpHeader.ContentLanguage, result->HttpHeaders.ContentLanguage);
        EXPECT_EQ(httpHeader.ContentType, result->HttpHeaders.ContentType);
        EXPECT_NO_THROW(client.Delete(false));
      }
    }
  }

}}} // namespace Azure::Storage::Test
