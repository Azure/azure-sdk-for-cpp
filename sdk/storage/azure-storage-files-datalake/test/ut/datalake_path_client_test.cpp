// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake_path_client_test.hpp"

#include <algorithm>
#include <thread>

#include <azure/identity/client_secret_credential.hpp>

namespace Azure { namespace Storage { namespace Test {

  void DataLakePathClientTest::SetUp()
  {
    DataLakeFileSystemClientTest::SetUp();

    m_pathName = RandomString();
    m_pathClient = std::make_shared<Files::DataLake::DataLakePathClient>(
        m_fileSystemClient->GetFileClient(m_pathName));
    m_fileSystemClient->GetFileClient(m_pathName).Create();
  }

  std::vector<Files::DataLake::Models::Acl> DataLakePathClientTest::GetAclsForTesting()
  {
    static std::vector<Files::DataLake::Models::Acl> result = []() {
      std::vector<Files::DataLake::Models::Acl> ret;
      Files::DataLake::Models::Acl acl1;
      acl1.Type = "user";
      acl1.Id = "72a3f86f-271f-439e-b031-25678907d381";
      acl1.Permissions = "rwx";
      Files::DataLake::Models::Acl acl2;
      acl2.Type = "user";
      acl2.Id = "";
      acl2.Permissions = "rwx";
      Files::DataLake::Models::Acl acl3;
      acl3.Type = "group";
      acl3.Id = "";
      acl3.Permissions = "r--";
      Files::DataLake::Models::Acl acl4;
      acl4.Type = "other";
      acl4.Id = "";
      acl4.Permissions = "---";
      ret.emplace_back(std::move(acl1));
      ret.emplace_back(std::move(acl2));
      ret.emplace_back(std::move(acl3));
      ret.emplace_back(std::move(acl4));
      return ret;
    }();
    return result;
  }

  TEST_F(DataLakePathClientTest, CreateWithOptions)
  {
    // owner&group
    {
      auto client = m_fileSystemClient->GetFileClient(RandomString() + "owner_group");
      Files::DataLake::CreateFileOptions options;
      options.Group = "$superuser";
      options.Owner = "$superuser";
      EXPECT_NO_THROW(client.Create(options));
      auto response = client.GetAccessControlList();
      EXPECT_EQ(options.Group.Value(), response.Value.Group);
      EXPECT_EQ(options.Owner.Value(), response.Value.Owner);
    }
    // acl
    {
      auto client = m_fileSystemClient->GetFileClient(RandomString() + "_acl");
      Files::DataLake::CreateFileOptions options;
      auto acls = GetAclsForTesting();
      options.Acls = acls;
      EXPECT_NO_THROW(client.Create(options));
      auto resultAcls = client.GetAccessControlList().Value.Acls;
      EXPECT_EQ(resultAcls.size(), acls.size() + 1); // Always append mask::rwx
      for (const auto& acl : acls)
      {
        auto iter = std::find_if(
            resultAcls.begin(),
            resultAcls.end(),
            [&acl](const Files::DataLake::Models::Acl& targetAcl) {
              return (targetAcl.Type == acl.Type) && (targetAcl.Id == acl.Id)
                  && (targetAcl.Scope == acl.Scope);
            });
        EXPECT_NE(iter, resultAcls.end());
        EXPECT_EQ(iter->Permissions, acl.Permissions);
      }
    }
    // lease
    {
      auto client = m_fileSystemClient->GetFileClient(RandomString() + "_lease");
      Files::DataLake::CreateFileOptions options;
      options.LeaseId = RandomUUID();
      options.LeaseDuration = std::chrono::seconds(20);
      EXPECT_NO_THROW(client.Create(options));
      auto response = client.GetProperties();
      EXPECT_TRUE(response.Value.LeaseStatus.HasValue());
      EXPECT_EQ(Files::DataLake::Models::LeaseStatus::Locked, response.Value.LeaseStatus.Value());
      EXPECT_TRUE(response.Value.LeaseState.HasValue());
      EXPECT_EQ(Files::DataLake::Models::LeaseState::Leased, response.Value.LeaseState.Value());
      EXPECT_TRUE(response.Value.LeaseDuration.HasValue());
      EXPECT_EQ(
          Files::DataLake::Models::LeaseDurationType::Fixed, response.Value.LeaseDuration.Value());
    }
    // relative expiry
    {
      auto client = m_fileSystemClient->GetFileClient(RandomString() + "_relative_expiry");
      Files::DataLake::CreateFileOptions options;
      options.ScheduleDeletionOptions.TimeToExpire = std::chrono::hours(1);
      EXPECT_NO_THROW(client.Create(options));
      auto response = client.GetProperties();
      DateTime leftExpiryTime = response.Value.CreatedOn
          + options.ScheduleDeletionOptions.TimeToExpire.Value() - std::chrono::seconds(5);
      DateTime rightExpiryTime = response.Value.CreatedOn
          + options.ScheduleDeletionOptions.TimeToExpire.Value() + std::chrono::seconds(5);
      EXPECT_TRUE(response.Value.ExpiresOn.HasValue());
      EXPECT_TRUE(
          response.Value.ExpiresOn.Value() > leftExpiryTime
          && response.Value.ExpiresOn.Value() < rightExpiryTime);
    }
    // absolute expiry
    {
      auto client = m_fileSystemClient->GetFileClient(RandomString() + "_absolute_expiry");
      Files::DataLake::CreateFileOptions options;
      options.ScheduleDeletionOptions.ExpiresOn = Azure::DateTime::Parse(
          "Wed, 29 Sep 2100 09:53:03 GMT", Azure::DateTime::DateFormat::Rfc1123);
      EXPECT_NO_THROW(client.Create(options));
      auto response = client.GetProperties();
      EXPECT_TRUE(response.Value.ExpiresOn.HasValue());
      EXPECT_EQ(
          options.ScheduleDeletionOptions.ExpiresOn.Value(), response.Value.ExpiresOn.Value());
    }
  }

  TEST_F(DataLakePathClientTest, PathMetadata)
  {
    auto metadata1 = RandomMetadata();
    auto metadata2 = RandomMetadata();
    {
      // Set/Get Metadata works
      EXPECT_NO_THROW(m_pathClient->SetMetadata(metadata1));
      auto result = m_pathClient->GetProperties().Value.Metadata;
      EXPECT_EQ(metadata1, result);
      EXPECT_NO_THROW(m_pathClient->SetMetadata(metadata2));
      result = m_pathClient->GetProperties().Value.Metadata;
      EXPECT_EQ(metadata2, result);
    }

    {
      // Create path with metadata works
      std::string const baseName = RandomString();
      auto client1 = m_fileSystemClient->GetFileClient(baseName + "1");
      auto client2 = m_fileSystemClient->GetFileClient(baseName + "2");
      Files::DataLake::CreatePathOptions options1;
      Files::DataLake::CreatePathOptions options2;
      options1.Metadata = metadata1;
      options2.Metadata = metadata2;

      EXPECT_NO_THROW(client1.Create(options1));
      EXPECT_NO_THROW(client2.Create(options2));
      auto result = client1.GetProperties().Value.Metadata;
      EXPECT_EQ(metadata1, result);
      result = client2.GetProperties().Value.Metadata;
      EXPECT_EQ(metadata2, result);
    }
  }

  TEST_F(DataLakePathClientTest, GetDataLakePathPropertiesResult)
  {
    auto metadata1 = RandomMetadata();
    auto metadata2 = RandomMetadata();
    {
      // Get Metadata via properties works
      EXPECT_NO_THROW(m_pathClient->SetMetadata(metadata1));
      auto result = m_pathClient->GetProperties();
      EXPECT_EQ(metadata1, result.Value.Metadata);
      EXPECT_NO_THROW(m_pathClient->SetMetadata(metadata2));
      result = m_pathClient->GetProperties();
      EXPECT_EQ(metadata2, result.Value.Metadata);
    }

    {
      // Last modified Etag works.
      auto properties1 = m_pathClient->GetProperties();
      auto properties2 = m_pathClient->GetProperties();
      EXPECT_FALSE(properties1.Value.IsDirectory);
      EXPECT_EQ(properties1.Value.ETag, properties2.Value.ETag);
      EXPECT_EQ(properties1.Value.LastModified, properties2.Value.LastModified);

      // This operation changes ETag/LastModified.
      EXPECT_NO_THROW(m_pathClient->SetMetadata(metadata1));

      auto properties3 = m_pathClient->GetProperties();
      EXPECT_NE(properties1.Value.ETag, properties3.Value.ETag);
    }
  }

  TEST_F(DataLakePathClientTest, PathHttpHeaders)
  {
    auto httpHeaders = Files::DataLake::Models::PathHttpHeaders();
    httpHeaders.ContentType = "application/x-binary";
    httpHeaders.ContentLanguage = "en-US";
    httpHeaders.ContentDisposition = "attachment";
    httpHeaders.CacheControl = "no-cache";
    httpHeaders.ContentEncoding = "identity";

    const std::string baseName = RandomString();
    {
      // HTTP headers works with create.
      std::vector<Files::DataLake::DataLakePathClient> pathClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetFileClient(baseName + std::to_string(i));
        Files::DataLake::CreatePathOptions options;
        options.HttpHeaders = httpHeaders;
        EXPECT_NO_THROW(client.Create(options));
        pathClient.emplace_back(std::move(client));
      }
      for (const auto& client : pathClient)
      {
        auto result = client.GetProperties();
        EXPECT_EQ(httpHeaders.CacheControl, result.Value.HttpHeaders.CacheControl);
        EXPECT_EQ(httpHeaders.ContentDisposition, result.Value.HttpHeaders.ContentDisposition);
        EXPECT_EQ(httpHeaders.ContentLanguage, result.Value.HttpHeaders.ContentLanguage);
        EXPECT_EQ(httpHeaders.ContentType, result.Value.HttpHeaders.ContentType);
        EXPECT_EQ(httpHeaders.ContentEncoding, result.Value.HttpHeaders.ContentEncoding);
        client.Delete();
      }
    }

    {
      // HTTP headers works with SetHttpHeaders.
      std::vector<Files::DataLake::DataLakePathClient> pathClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetFileClient(baseName + "2" + std::to_string(i));
        EXPECT_NO_THROW(client.Create());
        EXPECT_NO_THROW(client.SetHttpHeaders(httpHeaders));
        pathClient.emplace_back(std::move(client));
      }
      for (const auto& client : pathClient)
      {
        auto result = client.GetProperties();
        EXPECT_EQ(httpHeaders.CacheControl, result.Value.HttpHeaders.CacheControl);
        EXPECT_EQ(httpHeaders.ContentDisposition, result.Value.HttpHeaders.ContentDisposition);
        EXPECT_EQ(httpHeaders.ContentLanguage, result.Value.HttpHeaders.ContentLanguage);
        EXPECT_EQ(httpHeaders.ContentType, result.Value.HttpHeaders.ContentType);
        EXPECT_EQ(httpHeaders.ContentEncoding, result.Value.HttpHeaders.ContentEncoding);
        client.Delete();
      }
    }

    {
      // Set HTTP headers work with last modified access condition.
      auto response = m_pathClient->GetProperties();
      Files::DataLake::SetPathHttpHeadersOptions options1;
      options1.AccessConditions.IfModifiedSince = response.Value.LastModified;
      EXPECT_THROW(m_pathClient->SetHttpHeaders(httpHeaders, options1), StorageException);
      Files::DataLake::SetPathHttpHeadersOptions options2;
      options2.AccessConditions.IfUnmodifiedSince = response.Value.LastModified;
      EXPECT_NO_THROW(m_pathClient->SetHttpHeaders(httpHeaders, options2));
    }

    {
      // Set HTTP headers work with last modified access condition.
      auto response = m_pathClient->GetProperties();
      Files::DataLake::SetPathHttpHeadersOptions options1;
      options1.AccessConditions.IfNoneMatch = response.Value.ETag;
      EXPECT_THROW(m_pathClient->SetHttpHeaders(httpHeaders, options1), StorageException);
      Files::DataLake::SetPathHttpHeadersOptions options2;
      options2.AccessConditions.IfMatch = response.Value.ETag;
      EXPECT_NO_THROW(m_pathClient->SetHttpHeaders(httpHeaders, options2));
    }
  }

  TEST_F(DataLakePathClientTest, PathAccessControls)
  {
    {
      // Set/Get Acls works.
      std::vector<Files::DataLake::Models::Acl> acls = GetAclsForTesting();
      EXPECT_NO_THROW(m_pathClient->SetAccessControlList(acls));
      std::vector<Files::DataLake::Models::Acl> resultAcls;
      EXPECT_NO_THROW(resultAcls = m_pathClient->GetAccessControlList().Value.Acls);
      EXPECT_EQ(resultAcls.size(), acls.size() + 1); // Always append mask::rwx
      for (const auto& acl : acls)
      {
        auto iter = std::find_if(
            resultAcls.begin(),
            resultAcls.end(),
            [&acl](const Files::DataLake::Models::Acl& targetAcl) {
              return (targetAcl.Type == acl.Type) && (targetAcl.Id == acl.Id)
                  && (targetAcl.Scope == acl.Scope);
            });
        EXPECT_NE(iter, resultAcls.end());
        EXPECT_EQ(iter->Permissions, acl.Permissions);
      }
    }

    {
      // Set/Get Acls works with last modified access condition.
      std::vector<Files::DataLake::Models::Acl> acls = GetAclsForTesting();

      auto response = m_pathClient->GetProperties();
      Files::DataLake::SetPathAccessControlListOptions options1;
      options1.AccessConditions.IfModifiedSince = response.Value.LastModified;
      EXPECT_THROW(m_pathClient->SetAccessControlList(acls, options1), StorageException);
      Files::DataLake::SetPathAccessControlListOptions options2;
      options2.AccessConditions.IfUnmodifiedSince = response.Value.LastModified;
      EXPECT_NO_THROW(m_pathClient->SetAccessControlList(acls, options2));
    }

    {
      // Set/Get Acls works with if match access condition.
      std::vector<Files::DataLake::Models::Acl> acls = GetAclsForTesting();
      auto response = m_pathClient->GetProperties();
      Files::DataLake::SetPathAccessControlListOptions options1;
      options1.AccessConditions.IfNoneMatch = response.Value.ETag;
      EXPECT_THROW(m_pathClient->SetAccessControlList(acls, options1), StorageException);
      Files::DataLake::SetPathAccessControlListOptions options2;
      options2.AccessConditions.IfMatch = response.Value.ETag;
      EXPECT_NO_THROW(m_pathClient->SetAccessControlList(acls, options2));
    }
  }

  TEST_F(DataLakePathClientTest, PathSetPermissions)
  {
    std::string const baseName = RandomString();
    {
      auto pathClient = Files::DataLake::DataLakePathClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(),
          m_fileSystemName,
          baseName + "1",
          InitClientOptions<Files::DataLake::DataLakeClientOptions>());
      pathClient.Create(Files::DataLake::Models::PathResourceType::File);
      std::string pathPermissions = "rwxrw-rw-";
      EXPECT_NO_THROW(pathClient.SetPermissions(pathPermissions));
      auto result = pathClient.GetAccessControlList();
      EXPECT_EQ(pathPermissions, result.Value.Permissions);

      pathPermissions = "rw-rw-rw-";
      EXPECT_NO_THROW(pathClient.SetPermissions(pathPermissions));
      result = pathClient.GetAccessControlList();
      EXPECT_EQ(pathPermissions, result.Value.Permissions);

      EXPECT_NO_THROW(pathClient.SetPermissions("0766"));
      result = pathClient.GetAccessControlList();
      EXPECT_EQ("rwxrw-rw-", result.Value.Permissions);
    }
    {
      // Set/Get Permissions works with last modified access condition.
      auto pathClient = Files::DataLake::DataLakePathClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(),
          m_fileSystemName,
          baseName + "2",
          InitClientOptions<Files::DataLake::DataLakeClientOptions>());
      auto response = pathClient.Create(Files::DataLake::Models::PathResourceType::File);
      Files::DataLake::SetPathPermissionsOptions options1, options2;
      options1.AccessConditions.IfUnmodifiedSince = response.Value.LastModified;
      options2.AccessConditions.IfModifiedSince = response.Value.LastModified;
      std::string pathPermissions = "rwxrw-rw-";
      EXPECT_THROW(pathClient.SetPermissions(pathPermissions, options2), StorageException);
      EXPECT_NO_THROW(pathClient.SetPermissions(pathPermissions, options1));
    }
    {
      // Set/Get Permissions works with if match access condition.
      auto pathClient = Files::DataLake::DataLakePathClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(),
          m_fileSystemName,
          baseName + "3",
          InitClientOptions<Files::DataLake::DataLakeClientOptions>());
      auto response = pathClient.Create(Files::DataLake::Models::PathResourceType::File);
      Files::DataLake::SetPathPermissionsOptions options1, options2;
      options1.AccessConditions.IfMatch = response.Value.ETag;
      options2.AccessConditions.IfNoneMatch = response.Value.ETag;
      std::string pathPermissions = "rwxrw-rw-";
      EXPECT_THROW(pathClient.SetPermissions(pathPermissions, options2), StorageException);
      EXPECT_NO_THROW(pathClient.SetPermissions(pathPermissions, options1));
    }
  }

}}} // namespace Azure::Storage::Test
