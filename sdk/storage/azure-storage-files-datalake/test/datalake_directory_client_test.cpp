// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake_directory_client_test.hpp"

#include <algorithm>
#include <thread>

#include <azure/identity/client_secret_credential.hpp>
#include <azure/storage/common/shared_key_policy.hpp>
#include <azure/storage/files/datalake/datalake_utilities.hpp>

namespace Azure { namespace Storage { namespace Test {

  std::shared_ptr<Files::DataLake::DataLakeDirectoryClient>
      DataLakeDirectoryClientTest::m_directoryClient;
  std::string DataLakeDirectoryClientTest::m_directoryName;

  void DataLakeDirectoryClientTest::SetUpTestSuite()
  {
    DataLakeFileSystemClientTest::SetUpTestSuite();
    m_directoryName = RandomString(10);
    m_directoryClient = std::make_shared<Files::DataLake::DataLakeDirectoryClient>(
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
      std::vector<Files::DataLake::DataLakeDirectoryClient> directoryClient;
      for (int32_t i = 0; i < 5; ++i)
      {
        auto client = m_fileSystemClient->GetDirectoryClient(RandomString());
        EXPECT_NO_THROW(client.Create());
        directoryClient.emplace_back(std::move(client));
      }
      for (const auto& client : directoryClient)
      {
        EXPECT_NO_THROW(client.DeleteEmpty());
      }
    }
    {
      // Normal delete with last modified access condition.
      std::vector<Files::DataLake::DataLakeDirectoryClient> directoryClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetDirectoryClient(RandomString());
        EXPECT_NO_THROW(client.Create());
        directoryClient.emplace_back(std::move(client));
      }
      for (const auto& client : directoryClient)
      {
        auto response = client.GetProperties();
        Files::DataLake::DeleteDataLakeDirectoryOptions options1;
        options1.AccessConditions.IfModifiedSince = response->LastModified;
        EXPECT_TRUE(IsValidTime(response->LastModified));
        EXPECT_THROW(client.DeleteEmpty(options1), StorageException);
        Files::DataLake::DeleteDataLakeDirectoryOptions options2;
        options2.AccessConditions.IfUnmodifiedSince = response->LastModified;
        EXPECT_NO_THROW(client.DeleteEmpty(options2));
      }
    }
    {
      // Normal delete with if match access condition.
      std::vector<Files::DataLake::DataLakeDirectoryClient> directoryClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetDirectoryClient(RandomString());
        EXPECT_NO_THROW(client.Create());
        directoryClient.emplace_back(std::move(client));
      }
      for (const auto& client : directoryClient)
      {
        auto response = client.GetProperties();
        EXPECT_TRUE(response->IsDirectory);
        Files::DataLake::DeleteDataLakeDirectoryOptions options1;
        options1.AccessConditions.IfNoneMatch = response->ETag;
        EXPECT_THROW(client.DeleteEmpty(options1), StorageException);
        Files::DataLake::DeleteDataLakeDirectoryOptions options2;
        options2.AccessConditions.IfMatch = response->ETag;
        EXPECT_NO_THROW(client.DeleteEmpty(options2));
      }
    }

    {
      // Recursive delete works.
      std::vector<Files::DataLake::DataLakeDirectoryClient> directoryClient;
      auto rootDir = RandomString();
      auto rootDirClient = m_fileSystemClient->GetDirectoryClient(rootDir);
      EXPECT_NO_THROW(rootDirClient.Create());
      for (int32_t i = 0; i < 5; ++i)
      {
        auto client = m_fileSystemClient->GetDirectoryClient(rootDir + "/" + RandomString());
        EXPECT_NO_THROW(client.Create());
        directoryClient.emplace_back(std::move(client));
      }
      EXPECT_THROW(rootDirClient.DeleteEmpty(), StorageException);
      EXPECT_NO_THROW(rootDirClient.DeleteRecursive());
    }
  }

  TEST_F(DataLakeDirectoryClientTest, CreateDeleteIfExistsDirectory)
  {
    {
      auto client = m_fileSystemClient->GetDirectoryClient(RandomString());
      bool created = false;
      bool deleted = false;
      EXPECT_NO_THROW(created = client.Create()->Created);
      EXPECT_TRUE(created);
      EXPECT_NO_THROW(created = client.CreateIfNotExists()->Created);
      EXPECT_FALSE(created);
      EXPECT_NO_THROW(deleted = client.DeleteEmpty()->Deleted);
      EXPECT_TRUE(deleted);
      EXPECT_NO_THROW(deleted = client.DeleteEmptyIfExists()->Deleted);
      EXPECT_FALSE(deleted);
    }
    {
      auto client = Files::DataLake::DataLakeDirectoryClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(), LowercaseRandomString(), RandomString());
      bool deleted = false;
      EXPECT_NO_THROW(deleted = client.DeleteEmptyIfExists()->Deleted);
      EXPECT_FALSE(deleted);
    }
  }

  TEST_F(DataLakeDirectoryClientTest, RenameDirectory)
  {
    {
      // Normal create/rename/delete.
      std::vector<Files::DataLake::DataLakeDirectoryClient> directoryClients;
      for (int32_t i = 0; i < 5; ++i)
      {
        auto client = m_fileSystemClient->GetDirectoryClient(RandomString());
        EXPECT_NO_THROW(client.Create());
        directoryClients.emplace_back(std::move(client));
      }
      std::vector<Files::DataLake::DataLakeDirectoryClient> newDirectoryClients;
      for (auto& client : directoryClients)
      {
        auto newPath = RandomString();
        EXPECT_NO_THROW(newDirectoryClients.emplace_back(
            client.RenameSubdirectory("", newPath).ExtractValue()));
      }
      for (const auto& client : directoryClients)
      {
        EXPECT_THROW(client.DeleteEmpty(), StorageException);
      }
      for (const auto& client : newDirectoryClients)
      {
        EXPECT_NO_THROW(client.DeleteEmpty());
      }
    }
    {
      // Normal rename with last modified access condition.
      std::vector<Files::DataLake::DataLakeDirectoryClient> directoryClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetDirectoryClient(RandomString());
        EXPECT_NO_THROW(client.Create());
        directoryClient.emplace_back(std::move(client));
      }
      for (auto& client : directoryClient)
      {
        auto response = client.GetProperties();
        Files::DataLake::RenameDataLakeDirectoryOptions options1;
        options1.SourceAccessConditions.IfModifiedSince = response->LastModified;
        EXPECT_TRUE(IsValidTime(response->LastModified));
        EXPECT_THROW(client.RenameSubdirectory("", RandomString(), options1), StorageException);
        Files::DataLake::RenameDataLakeDirectoryOptions options2;
        options2.SourceAccessConditions.IfUnmodifiedSince = response->LastModified;
        auto newPath = RandomString();
        EXPECT_NO_THROW(
            client.RenameSubdirectory("", newPath, options2).ExtractValue().DeleteEmpty());
      }
    }
    {
      // Normal rename with if match access condition.
      std::vector<Files::DataLake::DataLakeDirectoryClient> directoryClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetDirectoryClient(RandomString());
        EXPECT_NO_THROW(client.Create());
        directoryClient.emplace_back(std::move(client));
      }
      for (auto& client : directoryClient)
      {
        auto response = client.GetProperties();
        Files::DataLake::RenameDataLakeDirectoryOptions options1;
        options1.SourceAccessConditions.IfNoneMatch = response->ETag;
        EXPECT_THROW(client.RenameSubdirectory("", RandomString(), options1), StorageException);
        Files::DataLake::RenameDataLakeDirectoryOptions options2;
        options2.SourceAccessConditions.IfMatch = response->ETag;
        auto newPath = RandomString();
        EXPECT_NO_THROW(
            client.RenameSubdirectory("", newPath, options2).ExtractValue().DeleteEmpty());
      }
    }
    {
      // Rename to a destination file system.
      std::vector<Files::DataLake::DataLakeDirectoryClient> directoryClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetDirectoryClient(RandomString());
        EXPECT_NO_THROW(client.Create());
        directoryClient.emplace_back(std::move(client));
      }
      {
        // Rename to a non-existing file system will fail and source is not changed.
        Files::DataLake::RenameDataLakeDirectoryOptions options;
        options.DestinationFileSystem = LowercaseRandomString();
        for (auto& client : directoryClient)
        {
          EXPECT_THROW(client.RenameSubdirectory("", RandomString(), options), StorageException);
          EXPECT_NO_THROW(client.GetProperties());
        }
      }
      {
        // Rename to an existing file system will succeed and changes URI.
        auto newfileSystemName = LowercaseRandomString(10);
        auto newfileSystemClient = std::make_shared<Files::DataLake::DataLakeFileSystemClient>(
            Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
                AdlsGen2ConnectionString(), newfileSystemName));
        newfileSystemClient->Create();
        Files::DataLake::RenameDataLakeDirectoryOptions options;
        options.DestinationFileSystem = newfileSystemName;
        for (auto& client : directoryClient)
        {
          auto newPath = RandomString();
          EXPECT_NO_THROW(
              client.RenameSubdirectory("", newPath, options).ExtractValue().DeleteEmpty());
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
      auto client1 = m_fileSystemClient->GetDirectoryClient(RandomString());
      auto client2 = m_fileSystemClient->GetDirectoryClient(RandomString());
      Files::DataLake::CreateDataLakePathOptions options1;
      Files::DataLake::CreateDataLakePathOptions options2;
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
      EXPECT_TRUE(IsValidTime(properties1->LastModified));
      EXPECT_EQ(properties1->LastModified, properties2->LastModified);

      // This operation changes ETag/LastModified.
      EXPECT_NO_THROW(m_directoryClient->SetMetadata(metadata1));

      auto properties3 = m_directoryClient->GetProperties();
      EXPECT_NE(properties1->ETag, properties3->ETag);
    }

    {
      // Http headers works.
      auto httpHeader = GetInterestingHttpHeaders();
      std::vector<Files::DataLake::DataLakeDirectoryClient> directoryClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetDirectoryClient(RandomString());
        Files::DataLake::CreateDataLakePathOptions options;
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
        EXPECT_NO_THROW(client.DeleteEmpty());
      }
    }
  }

  TEST_F(DataLakeDirectoryClientTest, DirectoryAccessControlRecursive)
  {
    // Setup directories.
    auto rootDirectoryName = RandomString();
    auto directoryName1 = RandomString();
    auto directoryName2 = RandomString();
    auto rootDirectoryClient = m_fileSystemClient->GetDirectoryClient(rootDirectoryName);
    rootDirectoryClient.Create();
    auto directoryClient1
        = m_fileSystemClient->GetDirectoryClient(rootDirectoryName + "/" + directoryName1);
    directoryClient1.Create();
    auto directoryClient2
        = m_fileSystemClient->GetDirectoryClient(rootDirectoryName + "/" + directoryName2);
    directoryClient2.Create();

    {
      // Set Acls recursive.
      std::vector<Files::DataLake::Models::Acl> acls = GetValidAcls();
      EXPECT_NO_THROW(rootDirectoryClient.SetAccessControlListRecursiveSinglePage(acls));
      std::vector<Files::DataLake::Models::Acl> resultAcls1;
      std::vector<Files::DataLake::Models::Acl> resultAcls2;
      EXPECT_NO_THROW(resultAcls1 = directoryClient1.GetAccessControlList()->Acls);
      EXPECT_NO_THROW(resultAcls2 = directoryClient2.GetAccessControlList()->Acls);
      for (const auto& acl : resultAcls2)
      {
        auto iter = std::find_if(
            resultAcls1.begin(),
            resultAcls1.end(),
            [&acl](const Files::DataLake::Models::Acl& targetAcl) {
              return (targetAcl.Type == acl.Type) && (targetAcl.Id == acl.Id)
                  && (targetAcl.Scope == acl.Scope);
            });
        EXPECT_TRUE(iter != resultAcls1.end());
        EXPECT_EQ(iter->Permissions, acl.Permissions);
      }
    }
    {
      // Update Acls recursive.
      std::vector<Files::DataLake::Models::Acl> originalAcls = GetValidAcls();
      Files::DataLake::Models::Acl newAcl;
      newAcl.Type = "group";
      newAcl.Id = "";
      newAcl.Permissions = "rw-";
      std::vector<Files::DataLake::Models::Acl> acls;
      acls.emplace_back(std::move(newAcl));
      EXPECT_NO_THROW(rootDirectoryClient.UpdateAccessControlListRecursiveSinglePage(acls));
      std::vector<Files::DataLake::Models::Acl> resultAcls1;
      std::vector<Files::DataLake::Models::Acl> resultAcls2;
      EXPECT_NO_THROW(resultAcls1 = directoryClient1.GetAccessControlList()->Acls);
      EXPECT_NO_THROW(resultAcls2 = directoryClient2.GetAccessControlList()->Acls);
      for (const auto& acl : resultAcls2)
      {
        auto iter = std::find_if(
            resultAcls1.begin(),
            resultAcls1.end(),
            [&acl](const Files::DataLake::Models::Acl& targetAcl) {
              return (targetAcl.Type == acl.Type) && (targetAcl.Id == acl.Id)
                  && (targetAcl.Scope == acl.Scope);
            });
        EXPECT_TRUE(iter != resultAcls1.end());
        EXPECT_EQ(iter->Permissions, acl.Permissions);
      }
      {
        // verify group has changed
        auto groupFinder = [](const Files::DataLake::Models::Acl& targetAcl) {
          return targetAcl.Type == "group";
        };
        auto iter = std::find_if(resultAcls1.begin(), resultAcls1.end(), groupFinder);
        EXPECT_TRUE(iter != resultAcls1.end());
        EXPECT_EQ("rw-", iter->Permissions);
        iter = std::find_if(resultAcls2.begin(), resultAcls2.end(), groupFinder);
        EXPECT_TRUE(iter != resultAcls2.end());
        EXPECT_EQ("rw-", iter->Permissions);
      }
      {
        // verify other has not changed
        {
          auto otherFinder = [](const Files::DataLake::Models::Acl& targetAcl) {
            return targetAcl.Type == "other";
          };
          auto iter = std::find_if(resultAcls1.begin(), resultAcls1.end(), otherFinder);
          EXPECT_TRUE(iter != resultAcls1.end());
          EXPECT_EQ(originalAcls[3].Permissions, iter->Permissions);
          iter = std::find_if(resultAcls2.begin(), resultAcls2.end(), otherFinder);
          EXPECT_TRUE(iter != resultAcls2.end());
          EXPECT_EQ(originalAcls[3].Permissions, iter->Permissions);
        }
        {
          auto userFinder = [](const Files::DataLake::Models::Acl& targetAcl) {
            return targetAcl.Type == "user";
          };
          auto iter = std::find_if(resultAcls1.begin(), resultAcls1.end(), userFinder);
          EXPECT_TRUE(iter != resultAcls1.end());
          if (iter->Id == originalAcls[0].Id)
          {
            EXPECT_EQ(originalAcls[0].Permissions, iter->Permissions);
          }
          else
          {
            EXPECT_EQ(originalAcls[1].Permissions, iter->Permissions);
          }
          iter = std::find_if(resultAcls2.begin(), resultAcls2.end(), userFinder);
          EXPECT_TRUE(iter != resultAcls2.end());
          if (iter->Id == originalAcls[0].Id)
          {
            EXPECT_EQ(originalAcls[0].Permissions, iter->Permissions);
          }
          else
          {
            EXPECT_EQ(originalAcls[1].Permissions, iter->Permissions);
          }
        }
      }
    }
    {
      // Remove Acls recursive.
      std::vector<Files::DataLake::Models::Acl> originalAcls = GetValidAcls();
      Files::DataLake::Models::Acl removeAcl;
      removeAcl.Type = "user";
      removeAcl.Id = "72a3f86f-271f-439e-b031-25678907d381";
      std::vector<Files::DataLake::Models::Acl> acls;
      acls.emplace_back(std::move(removeAcl));
      EXPECT_NO_THROW(rootDirectoryClient.RemoveAccessControlListRecursiveSinglePage(acls));
      std::vector<Files::DataLake::Models::Acl> resultAcls1;
      std::vector<Files::DataLake::Models::Acl> resultAcls2;
      EXPECT_NO_THROW(resultAcls1 = directoryClient1.GetAccessControlList()->Acls);
      EXPECT_NO_THROW(resultAcls2 = directoryClient2.GetAccessControlList()->Acls);
      for (const auto& acl : resultAcls2)
      {
        auto iter = std::find_if(
            resultAcls1.begin(),
            resultAcls1.end(),
            [&acl](const Files::DataLake::Models::Acl& targetAcl) {
              return (targetAcl.Type == acl.Type) && (targetAcl.Id == acl.Id)
                  && (targetAcl.Scope == acl.Scope);
            });
        EXPECT_TRUE(iter != resultAcls1.end());
        EXPECT_EQ(iter->Permissions, acl.Permissions);
      }
      {
        // verify group policy has been removed.
        auto userFinder = [](const Files::DataLake::Models::Acl& targetAcl) {
          return targetAcl.Type == "user" && targetAcl.Id == "72a3f86f-271f-439e-b031-25678907d381";
        };
        auto iter = std::find_if(resultAcls1.begin(), resultAcls1.end(), userFinder);
        EXPECT_TRUE(iter == resultAcls1.end());
        iter = std::find_if(resultAcls2.begin(), resultAcls2.end(), userFinder);
        EXPECT_TRUE(iter == resultAcls2.end());
      }
      {
        // verify other has not changed
        {
          auto otherFinder = [](const Files::DataLake::Models::Acl& targetAcl) {
            return targetAcl.Type == "other";
          };
          auto iter = std::find_if(resultAcls1.begin(), resultAcls1.end(), otherFinder);
          EXPECT_TRUE(iter != resultAcls1.end());
          EXPECT_EQ(originalAcls[3].Permissions, iter->Permissions);
          iter = std::find_if(resultAcls2.begin(), resultAcls2.end(), otherFinder);
          EXPECT_TRUE(iter != resultAcls2.end());
          EXPECT_EQ(originalAcls[3].Permissions, iter->Permissions);
        }
        {
          auto userFinder = [](const Files::DataLake::Models::Acl& targetAcl) {
            return targetAcl.Type == "user";
          };
          auto iter = std::find_if(resultAcls1.begin(), resultAcls1.end(), userFinder);
          EXPECT_TRUE(iter != resultAcls1.end());
          EXPECT_EQ(originalAcls[1].Id, iter->Id);
          EXPECT_EQ(originalAcls[1].Permissions, iter->Permissions);
          iter = std::find_if(resultAcls2.begin(), resultAcls2.end(), userFinder);
          EXPECT_TRUE(iter != resultAcls2.end());
          EXPECT_EQ(originalAcls[1].Id, iter->Id);
          EXPECT_EQ(originalAcls[1].Permissions, iter->Permissions);
        }
      }
    }
    {
      // Set Acls recursive, with new set of acls
      std::vector<Files::DataLake::Models::Acl> acls;
      {
        Files::DataLake::Models::Acl newAcl;
        newAcl.Type = "user";
        newAcl.Permissions = "rw-";
        acls.emplace_back(std::move(newAcl));
      }
      {
        Files::DataLake::Models::Acl newAcl;
        newAcl.Type = "group";
        newAcl.Permissions = "rw-";
        acls.emplace_back(std::move(newAcl));
      }
      {
        Files::DataLake::Models::Acl newAcl;
        newAcl.Type = "other";
        newAcl.Permissions = "rw-";
        acls.emplace_back(std::move(newAcl));
      }
      (rootDirectoryClient.SetAccessControlListRecursiveSinglePage(acls));
      std::vector<Files::DataLake::Models::Acl> resultAcls1;
      std::vector<Files::DataLake::Models::Acl> resultAcls2;
      EXPECT_NO_THROW(resultAcls1 = directoryClient1.GetAccessControlList()->Acls);
      EXPECT_NO_THROW(resultAcls2 = directoryClient2.GetAccessControlList()->Acls);
      for (const auto& acl : resultAcls2)
      {
        auto iter = std::find_if(
            resultAcls1.begin(),
            resultAcls1.end(),
            [&acl](const Files::DataLake::Models::Acl& targetAcl) {
              return (targetAcl.Type == acl.Type) && (targetAcl.Id == acl.Id)
                  && (targetAcl.Scope == acl.Scope);
            });
        EXPECT_TRUE(iter != resultAcls1.end());
        EXPECT_EQ(iter->Permissions, acl.Permissions);
      }
      {
        // verify group has changed
        auto groupFinder = [](const Files::DataLake::Models::Acl& targetAcl) {
          return targetAcl.Type == "group";
        };
        auto iter = std::find_if(resultAcls1.begin(), resultAcls1.end(), groupFinder);
        EXPECT_TRUE(iter != resultAcls1.end());
        EXPECT_EQ("rw-", iter->Permissions);
        EXPECT_EQ("", iter->Id);
        iter = std::find_if(resultAcls2.begin(), resultAcls2.end(), groupFinder);
        EXPECT_EQ("rw-", iter->Permissions);
        EXPECT_EQ("", iter->Id);
      }
      {
        // verify other has changed
        auto otherFinder = [](const Files::DataLake::Models::Acl& targetAcl) {
          return targetAcl.Type == "other";
        };
        auto iter = std::find_if(resultAcls1.begin(), resultAcls1.end(), otherFinder);
        EXPECT_TRUE(iter != resultAcls1.end());
        EXPECT_EQ("rw-", iter->Permissions);
        EXPECT_EQ("", iter->Id);
        iter = std::find_if(resultAcls2.begin(), resultAcls2.end(), otherFinder);
        EXPECT_EQ("rw-", iter->Permissions);
        EXPECT_EQ("", iter->Id);
      }
      {
        // verify user has only one entry
        std::vector<Files::DataLake::Models::Acl> originalAcls = GetValidAcls();
        auto userFinder = [&originalAcls](const Files::DataLake::Models::Acl& targetAcl) {
          return targetAcl.Type == "user" && targetAcl.Id == originalAcls[0].Id;
        };
        auto iter = std::find_if(resultAcls1.begin(), resultAcls1.end(), userFinder);
        EXPECT_TRUE(iter == resultAcls1.end());
        iter = std::find_if(resultAcls2.begin(), resultAcls2.end(), userFinder);
        EXPECT_TRUE(iter == resultAcls2.end());
      }
      {
        // verify user has changed
        auto userFinder = [](const Files::DataLake::Models::Acl& targetAcl) {
          return targetAcl.Type == "user";
        };
        auto iter = std::find_if(resultAcls1.begin(), resultAcls1.end(), userFinder);
        EXPECT_TRUE(iter != resultAcls1.end());
        EXPECT_EQ("rw-", iter->Permissions);
        EXPECT_EQ("", iter->Id);
        iter = std::find_if(resultAcls2.begin(), resultAcls2.end(), userFinder);
        EXPECT_EQ("rw-", iter->Permissions);
        EXPECT_EQ("", iter->Id);
      }
    }
  }

  TEST_F(DataLakeDirectoryClientTest, ConstructorsWorks)
  {
    {
      // Create from connection string validates static creator function and shared key
      // constructor.
      auto directoryName = RandomString(10);
      auto connectionStringClient
          = Azure::Storage::Files::DataLake::DataLakeDirectoryClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(), m_fileSystemName, directoryName);
      EXPECT_NO_THROW(connectionStringClient.Create());
      EXPECT_NO_THROW(connectionStringClient.DeleteRecursive());
    }

    {
      // Create from client secret credential.
      auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
          AadTenantId(), AadClientId(), AadClientSecret());

      auto clientSecretClient = Azure::Storage::Files::DataLake::DataLakeDirectoryClient(
          Azure::Storage::Files::DataLake::Details::GetDfsUrlFromUrl(
              Azure::Storage::Files::DataLake::DataLakeDirectoryClient::CreateFromConnectionString(
                  AdlsGen2ConnectionString(), m_fileSystemName, RandomString(10))
                  .GetUrl()),
          credential);

      EXPECT_NO_THROW(clientSecretClient.Create());
      EXPECT_NO_THROW(clientSecretClient.DeleteRecursive());
    }

    {
      // Create from Anonymous credential.
      auto objectName = RandomString(10);
      auto containerClient = Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(), m_fileSystemName);
      Azure::Storage::Blobs::SetBlobContainerAccessPolicyOptions options;
      options.AccessType = Azure::Storage::Blobs::Models::PublicAccessType::BlobContainer;
      containerClient.SetAccessPolicy(options);

      auto directoryClient
          = Azure::Storage::Files::DataLake::DataLakeDirectoryClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(), m_fileSystemName, objectName);
      EXPECT_NO_THROW(directoryClient.Create());

      auto anonymousClient
          = Azure::Storage::Files::DataLake::DataLakeDirectoryClient(directoryClient.GetUrl());

      std::this_thread::sleep_for(std::chrono::seconds(30));

      EXPECT_NO_THROW(anonymousClient.GetProperties());
    }
  }
}}} // namespace Azure::Storage::Test
