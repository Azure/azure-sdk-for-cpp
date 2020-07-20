// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "directory_client_test.hpp"

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
        EXPECT_NO_THROW(client.Delete());
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
        Files::DataLake::DirectoryDeleteOptions options1;
        options1.AccessConditions.IfModifiedSince = response->LastModified;
        EXPECT_THROW(client.Delete(options1), StorageError);
        Files::DataLake::DirectoryDeleteOptions options2;
        options2.AccessConditions.IfUnmodifiedSince = response->LastModified;
        EXPECT_NO_THROW(client.Delete(options2));
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
        Files::DataLake::DirectoryDeleteOptions options1;
        options1.AccessConditions.IfNoneMatch = response->ETag;
        EXPECT_THROW(client.Delete(options1), StorageError);
        Files::DataLake::DirectoryDeleteOptions options2;
        options2.AccessConditions.IfMatch = response->ETag;
        EXPECT_NO_THROW(client.Delete(options2));
      }
    }
  }

  TEST_F(DataLakeDirectoryClientTest, RenameDirectory)
  {
    {
      // Normal create/rename/delete.
      std::vector<Files::DataLake::DirectoryClient> directoryClient;
      for (int32_t i = 0; i < 5; ++i)
      {
        auto client = m_fileSystemClient->GetDirectoryClient(LowercaseRandomString());
        EXPECT_NO_THROW(client.Create());
        directoryClient.emplace_back(std::move(client));
      }
      auto directoryClientClone = directoryClient;
      for (auto& client : directoryClient)
      {
        EXPECT_NO_THROW(client.Rename(LowercaseRandomString()));
      }
      for (const auto& client : directoryClientClone)
      {
        EXPECT_THROW(client.Delete(), StorageError);
      }
      for (const auto& client : directoryClient)
      {
        EXPECT_NO_THROW(client.Delete());
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
        Files::DataLake::DirectoryRenameOptions options1;
        options1.SourceAccessConditions.IfModifiedSince = response->LastModified;
        EXPECT_THROW(client.Rename(LowercaseRandomString(), options1), StorageError);
        Files::DataLake::DirectoryRenameOptions options2;
        options2.SourceAccessConditions.IfUnmodifiedSince = response->LastModified;
        EXPECT_NO_THROW(client.Rename(LowercaseRandomString(), options2));
        EXPECT_NO_THROW(client.Delete());
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
        Files::DataLake::DirectoryRenameOptions options1;
        options1.SourceAccessConditions.IfNoneMatch = response->ETag;
        EXPECT_THROW(client.Rename(LowercaseRandomString(), options1), StorageError);
        Files::DataLake::DirectoryRenameOptions options2;
        options2.SourceAccessConditions.IfMatch = response->ETag;
        EXPECT_NO_THROW(client.Rename(LowercaseRandomString(), options2));
        EXPECT_NO_THROW(client.Delete());
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
        // Rename to a non-existing file system will fail but will not change URI.
        Files::DataLake::DirectoryRenameOptions options;
        options.DestinationFileSystem = LowercaseRandomString();
        for (auto& client : directoryClient)
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
        Files::DataLake::DirectoryRenameOptions options;
        options.DestinationFileSystem = newfileSystemName;
        for (auto& client : directoryClient)
        {
          EXPECT_NO_THROW(client.Rename(LowercaseRandomString(), options));
          EXPECT_NO_THROW(client.GetProperties());
          EXPECT_NO_THROW(client.Delete());
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
      Files::DataLake::PathCreateOptions options1;
      Files::DataLake::PathCreateOptions options2;
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
      EXPECT_NE(properties1->LastModified, properties3->LastModified);
    }

    {
      // Http headers works.
      auto httpHeader = GetInterestingHttpHeaders();
      std::vector<Files::DataLake::DirectoryClient> directoryClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetDirectoryClient(LowercaseRandomString());
        Files::DataLake::PathCreateOptions options;
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
        EXPECT_NO_THROW(client.Delete());
      }
    }
  }

  // TEST_F(DataLakeDirectoryClientTest, DirectorySetAccessControlRecursive)
  //{
  //  // Setup directories.
  //  auto rootDirectoryName = LowercaseRandomString() + "/";
  //  auto directoryName1 = LowercaseRandomString() + "/";
  //  auto directoryName2 = LowercaseRandomString() + "/";
  //  auto rootDirectoryClient = m_fileSystemClient->GetDirectoryClient(rootDirectoryName);
  //  rootDirectoryClient.Create();
  //  auto directoryClient1
  //      = m_fileSystemClient->GetDirectoryClient(rootDirectoryName + directoryName1);
  //  directoryClient1.Create();
  //  auto directoryClient2
  //      = m_fileSystemClient->GetDirectoryClient(rootDirectoryName + directoryName2);
  //  directoryClient2.Create();

  //  {
  //    // Set/Get Acls recursive works.
  //    std::vector<Files::DataLake::Acl> acls = GetInterestingAcls();
  //    EXPECT_NO_THROW(directoryClient1.SetAccessControl(acls));
  //    EXPECT_NO_THROW(rootDirectoryClient.SetAccessControlRecursive(
  //        Files::DataLake::PathSetAccessControlRecursiveMode::Set, acls));
  //    std::vector<Files::DataLake::Acl> resultAcls1;
  //    std::vector<Files::DataLake::Acl> resultAcls2;
  //    EXPECT_NO_THROW(resultAcls1 = directoryClient1.GetAccessControls()->Acls);
  //    EXPECT_NO_THROW(resultAcls2 = directoryClient2.GetAccessControls()->Acls);
  //    for (const auto& acl : resultAcls2)
  //    {
  //      auto iter = std::find_if(
  //          resultAcls1.begin(), resultAcls1.end(), [&acl](const Files::DataLake::Acl& targetAcl)
  //          {
  //            return (targetAcl.Type == acl.Type) && (targetAcl.Id == acl.Id)
  //                && (targetAcl.Scope == acl.Scope);
  //          });
  //      EXPECT_TRUE(iter != resultAcls1.end());
  //      EXPECT_EQ(iter->Permissions, acl.Permissions);
  //    }
  //  }
  //}
}}} // namespace Azure::Storage::Test
