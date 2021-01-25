// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake_file_system_client_test.hpp"

#include <algorithm>

#include <azure/identity/client_secret_credential.hpp>
#include <azure/storage/common/crypt.hpp>

namespace Azure { namespace Storage { namespace Test {

  const size_t c_PATH_TEST_SIZE = 5;

  std::shared_ptr<Files::DataLake::DataLakeFileSystemClient>
      DataLakeFileSystemClientTest::m_fileSystemClient;
  std::string DataLakeFileSystemClientTest::m_fileSystemName;

  std::vector<std::string> DataLakeFileSystemClientTest::m_pathNameSetA;
  std::vector<std::string> DataLakeFileSystemClientTest::m_pathNameSetB;
  std::string DataLakeFileSystemClientTest::m_directoryA;
  std::string DataLakeFileSystemClientTest::m_directoryB;

  void DataLakeFileSystemClientTest::SetUpTestSuite()
  {
    m_fileSystemName = LowercaseRandomString();
    m_fileSystemClient = std::make_shared<Files::DataLake::DataLakeFileSystemClient>(
        Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
            AdlsGen2ConnectionString(), m_fileSystemName));
    m_fileSystemClient->Create();

    m_directoryA = LowercaseRandomString();
    m_directoryB = LowercaseRandomString();
    m_pathNameSetA.clear();
    m_pathNameSetB.clear();
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

  std::vector<Files::DataLake::Models::PathItem> DataLakeFileSystemClientTest::ListAllPaths(
      bool recursive,
      const std::string& directory)
  {
    std::vector<Files::DataLake::Models::PathItem> result;
    std::string continuation;
    Files::DataLake::ListPathsSinglePageOptions options;
    if (directory.empty())
    {
      do
      {
        auto response = m_fileSystemClient->ListPathsSinglePage(recursive, options);
        result.insert(result.end(), response->Items.begin(), response->Items.end());
        if (response->ContinuationToken.HasValue())
        {
          continuation = response->ContinuationToken.GetValue();
          options.ContinuationToken = continuation;
        }
        else
        {
          continuation.clear();
        }
      } while (!continuation.empty());
    }
    else
    {
      auto directoryClient = m_fileSystemClient->GetDirectoryClient(directory);
      do
      {
        auto response = directoryClient.ListPathsSinglePage(recursive, options);
        result.insert(result.end(), response->Items.begin(), response->Items.end());
        if (response->ContinuationToken.HasValue())
        {
          continuation = response->ContinuationToken.GetValue();
          options.ContinuationToken = continuation;
        }
        else
        {
          continuation.clear();
        }
      } while (!continuation.empty());
    }

    return result;
  }

  Files::DataLake::Models::PathHttpHeaders DataLakeFileSystemClientTest::GetInterestingHttpHeaders()
  {
    static Files::DataLake::Models::PathHttpHeaders result = []() {
      Files::DataLake::Models::PathHttpHeaders ret;
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
      std::vector<Files::DataLake::DataLakeFileSystemClient> fileSystemClient;
      for (int32_t i = 0; i < 5; ++i)
      {
        auto client = Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
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
      std::vector<Files::DataLake::DataLakeFileSystemClient> fileSystemClient;
      for (int32_t i = 0; i < 5; ++i)
      {
        auto client = Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
            AdlsGen2ConnectionString(), LowercaseRandomString());
        EXPECT_NO_THROW(client.Create());
        fileSystemClient.emplace_back(std::move(client));
      }
      for (const auto& client : fileSystemClient)
      {
        auto response = client.GetProperties();
        Files::DataLake::DeleteDataLakeFileSystemOptions options1;
        options1.AccessConditions.IfModifiedSince = response->LastModified;
        EXPECT_THROW(client.Delete(options1), StorageException);
        Files::DataLake::DeleteDataLakeFileSystemOptions options2;
        options2.AccessConditions.IfUnmodifiedSince = response->LastModified;
        EXPECT_NO_THROW(client.Delete(options2));
      }
    }
    {
      // CreateIfNotExists & DeleteIfExists.
      {
        auto client = Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
            AdlsGen2ConnectionString(), LowercaseRandomString());
        EXPECT_NO_THROW(client.Create());
        EXPECT_NO_THROW(client.CreateIfNotExists());
        EXPECT_NO_THROW(client.Delete());
        EXPECT_NO_THROW(client.DeleteIfExists());
      }
      {
        auto client = Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
            AdlsGen2ConnectionString(), LowercaseRandomString());
        EXPECT_NO_THROW(client.CreateIfNotExists());
        EXPECT_THROW(client.Create(), StorageException);
        EXPECT_NO_THROW(client.DeleteIfExists());
      }
      {
        auto client = Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
            AdlsGen2ConnectionString(), LowercaseRandomString());
        auto created = client.Create()->Created;
        EXPECT_TRUE(created);
        auto createResult = client.CreateIfNotExists();
        EXPECT_FALSE(createResult->Created);
        EXPECT_TRUE(createResult->ETag.empty());
        EXPECT_EQ(Core::DateTime(), createResult->LastModified);
        auto deleted = client.Delete()->Deleted;
        EXPECT_TRUE(deleted);
      }
      {
        auto client = Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
            AdlsGen2ConnectionString(), LowercaseRandomString());
        auto deleteResult = client.DeleteIfExists();
        EXPECT_FALSE(deleteResult->Deleted);
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
      auto client1 = Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(), LowercaseRandomString());
      auto client2 = Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(), LowercaseRandomString());
      Files::DataLake::CreateDataLakeFileSystemOptions options1;
      Files::DataLake::CreateDataLakeFileSystemOptions options2;
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

  TEST_F(DataLakeFileSystemClientTest, GetDataLakeFileSystemPropertiesResult)
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
            result.begin(), result.end(), [&name](const Files::DataLake::Models::PathItem& path) {
              return path.Name == name;
            });
        EXPECT_NE(result.end(), iter);
        EXPECT_EQ(iter->Name, name);
        EXPECT_EQ(iter->Name.substr(0U, m_directoryA.size()), m_directoryA);
      }
      for (const auto& name : m_pathNameSetB)
      {
        auto iter = std::find_if(
            result.begin(), result.end(), [&name](const Files::DataLake::Models::PathItem& path) {
              return path.Name == name;
            });
        EXPECT_NE(result.end(), iter);
        EXPECT_EQ(iter->Name, name);
        EXPECT_EQ(iter->Name.substr(0U, m_directoryB.size()), m_directoryB);
      }
    }
    {
      // List with directory.
      auto result = ListAllPaths(true, m_directoryA);
      for (const auto& name : m_pathNameSetA)
      {
        auto iter = std::find_if(
            result.begin(), result.end(), [&name](const Files::DataLake::Models::PathItem& path) {
              return path.Name == name;
            });
        EXPECT_NE(result.end(), iter);
        EXPECT_EQ(iter->Name, name);
        EXPECT_EQ(iter->Name.substr(0U, m_directoryA.size()), m_directoryA);
      }
      for (const auto& name : m_pathNameSetB)
      {
        auto iter = std::find_if(
            result.begin(), result.end(), [&name](const Files::DataLake::Models::PathItem& path) {
              return path.Name == name;
            });
        EXPECT_EQ(result.end(), iter);
      }
    }
    {
      // List max result
      Files::DataLake::ListPathsSinglePageOptions options;
      options.PageSizeHint = 2;
      auto response = m_fileSystemClient->ListPathsSinglePage(true, options);
      EXPECT_LE(2U, response->Items.size());
    }
  }

  TEST_F(DataLakeFileSystemClientTest, UnencodedPathDirectoryFileNameWorks)
  {
    const std::string non_ascii_word = "\xE6\xB5\x8B\xE8\xAF\x95";
    const std::string encoded_non_ascii_word = "%E6%B5%8B%E8%AF%95";
    std::string baseName = "a b c / !@#$%^&*(?/<>,.;:'\"[]{}|`~\\) def" + non_ascii_word;
    {
      std::string pathName = baseName + RandomString();
      auto pathClient = m_fileSystemClient->GetPathClient(pathName);
      EXPECT_NO_THROW(pathClient.Create(Files::DataLake::Models::PathResourceType::File));
      auto pathUrl = pathClient.GetUrl();
      EXPECT_EQ(
          pathUrl, m_fileSystemClient->GetUrl() + "/" + Storage::Details::UrlEncodePath(pathName));
    }
    {
      std::string directoryName = baseName + RandomString();
      auto directoryClient = m_fileSystemClient->GetDirectoryClient(directoryName);
      EXPECT_NO_THROW(directoryClient.Create());
      auto directoryUrl = directoryClient.GetUrl();
      EXPECT_EQ(
          directoryUrl,
          m_fileSystemClient->GetUrl() + "/" + Storage::Details::UrlEncodePath(directoryName));
    }
    {
      std::string fileName = baseName + RandomString();
      auto fileClient = m_fileSystemClient->GetFileClient(fileName);
      EXPECT_NO_THROW(fileClient.Create());
      auto fileUrl = fileClient.GetUrl();
      EXPECT_EQ(
          fileUrl, m_fileSystemClient->GetUrl() + "/" + Storage::Details::UrlEncodePath(fileName));
    }
  }

  TEST_F(DataLakeFileSystemClientTest, ConstructorsWorks)
  {
    {
      // Create from connection string validates static creator function and shared key constructor.
      auto fileSystemName = LowercaseRandomString(10);
      auto connectionStringClient
          = Azure::Storage::Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(), fileSystemName);
      EXPECT_NO_THROW(connectionStringClient.Create());
      EXPECT_NO_THROW(connectionStringClient.Delete());
    }

    {
      // Create from client secret credential.
      auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
          AadTenantId(), AadClientId(), AadClientSecret());

      auto clientSecretClient = Azure::Storage::Files::DataLake::DataLakeFileSystemClient(
          Azure::Storage::Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(), LowercaseRandomString(10))
              .GetUrl(),
          credential);

      EXPECT_NO_THROW(clientSecretClient.Create());
      EXPECT_NO_THROW(clientSecretClient.Delete());
    }
  }

  TEST_F(DataLakeFileSystemClientTest, GetSetAccessPolicy)
  {
    {
      auto fileSystem = Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(), LowercaseRandomString());
      fileSystem.Create();

      Files::DataLake::SetDataLakeFileSystemAccessPolicyOptions options;
      options.AccessType = Files::DataLake::Models::PublicAccessType::Path;
      Files::DataLake::Models::DataLakeSignedIdentifier identifier;
      identifier.Id = RandomString(64);
      identifier.StartsOn = std::chrono::system_clock::now() - std::chrono::minutes(1);
      identifier.ExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(1);
      identifier.Permissions = "r";
      options.SignedIdentifiers.emplace_back(identifier);
      identifier.Id = RandomString(64);
      identifier.StartsOn = std::chrono::system_clock::now() - std::chrono::minutes(2);
      identifier.ExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(2);
      identifier.Permissions = "racwdxlt";
      options.SignedIdentifiers.emplace_back(identifier);

      auto ret = fileSystem.SetAccessPolicy(options);
      EXPECT_FALSE(ret->ETag.empty());
      EXPECT_TRUE(IsValidTime(ret->LastModified));

      auto ret2 = fileSystem.GetAccessPolicy();
      EXPECT_EQ(ret2->ETag, ret->ETag);
      EXPECT_EQ(ret2->LastModified, ret->LastModified);
      EXPECT_EQ(ret2->AccessType, options.AccessType);
      for (size_t i = 0; i < ret2->SignedIdentifiers.size(); ++i)
      {
        EXPECT_EQ(ret2->SignedIdentifiers[i].StartsOn, options.SignedIdentifiers[i].StartsOn);
        EXPECT_EQ(ret2->SignedIdentifiers[i].ExpiresOn, options.SignedIdentifiers[i].ExpiresOn);
        EXPECT_EQ(ret2->SignedIdentifiers[i].Id, options.SignedIdentifiers[i].Id);
        EXPECT_EQ(ret2->SignedIdentifiers[i].Permissions, options.SignedIdentifiers[i].Permissions);
      }

      options.AccessType = Files::DataLake::Models::PublicAccessType::FileSystem;
      EXPECT_NO_THROW(fileSystem.SetAccessPolicy(options));
      ret2 = fileSystem.GetAccessPolicy();
      EXPECT_EQ(ret2->AccessType, options.AccessType);

      options.AccessType = Files::DataLake::Models::PublicAccessType::None;
      EXPECT_NO_THROW(fileSystem.SetAccessPolicy(options));
      ret2 = fileSystem.GetAccessPolicy();
      EXPECT_EQ(ret2->AccessType, options.AccessType);

      fileSystem.Delete();
    }
    {
      auto fileSystem = Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(), LowercaseRandomString());
      Files::DataLake::CreateDataLakeFileSystemOptions options;
      options.AccessType = Files::DataLake::Models::PublicAccessType::FileSystem;
      fileSystem.Create(options);
      auto ret = fileSystem.GetAccessPolicy();
      EXPECT_EQ(Files::DataLake::Models::PublicAccessType::FileSystem, ret->AccessType);
    }
    {
      auto fileSystem = Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(), LowercaseRandomString());
      Files::DataLake::CreateDataLakeFileSystemOptions options;
      options.AccessType = Files::DataLake::Models::PublicAccessType::Path;
      fileSystem.Create(options);
      auto ret = fileSystem.GetAccessPolicy();
      EXPECT_EQ(Files::DataLake::Models::PublicAccessType::Path, ret->AccessType);
    }
    {
      auto fileSystem = Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(), LowercaseRandomString());
      Files::DataLake::CreateDataLakeFileSystemOptions options;
      options.AccessType = Files::DataLake::Models::PublicAccessType::Path;
      fileSystem.Create(options);
      auto ret = fileSystem.GetAccessPolicy();
      EXPECT_EQ(Files::DataLake::Models::PublicAccessType::Path, ret->AccessType);
    }
  }
}}} // namespace Azure::Storage::Test
