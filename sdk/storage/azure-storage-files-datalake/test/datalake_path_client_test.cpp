﻿// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake_path_client_test.hpp"

#include <algorithm>
#include <thread>

#include <azure/identity/client_secret_credential.hpp>

namespace Azure { namespace Storage { namespace Test {

  std::shared_ptr<Files::DataLake::DataLakePathClient> DataLakePathClientTest::m_pathClient;
  std::string DataLakePathClientTest::m_pathName;

  void DataLakePathClientTest::SetUpTestSuite()
  {
    DataLakeFileSystemClientTest::SetUpTestSuite();
    m_pathName = RandomString();
    m_pathClient = std::make_shared<Files::DataLake::DataLakePathClient>(
        m_fileSystemClient->GetFileClient(m_pathName));
    m_fileSystemClient->GetFileClient(m_pathName).Create();
  }

  void DataLakePathClientTest::TearDownTestSuite()
  {
    m_pathClient->Delete();
    DataLakeFileSystemClientTest::TearDownTestSuite();
  }

  std::vector<Files::DataLake::Models::Acl> DataLakePathClientTest::GetValidAcls()
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
      auto client1 = m_fileSystemClient->GetFileClient(RandomString());
      auto client2 = m_fileSystemClient->GetFileClient(RandomString());
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
    {
      // HTTP headers works with create.
      auto httpHeader = GetInterestingHttpHeaders();
      std::vector<Files::DataLake::DataLakePathClient> pathClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetFileClient(RandomString());
        Files::DataLake::CreatePathOptions options;
        options.HttpHeaders = httpHeader;
        EXPECT_NO_THROW(client.Create(options));
        pathClient.emplace_back(std::move(client));
      }
      for (const auto& client : pathClient)
      {
        auto result = client.GetProperties();
        EXPECT_EQ(httpHeader.CacheControl, result.Value.HttpHeaders.CacheControl);
        EXPECT_EQ(httpHeader.ContentDisposition, result.Value.HttpHeaders.ContentDisposition);
        EXPECT_EQ(httpHeader.ContentLanguage, result.Value.HttpHeaders.ContentLanguage);
        EXPECT_EQ(httpHeader.ContentType, result.Value.HttpHeaders.ContentType);
      }
    }

    {
      // HTTP headers works with SetHttpHeaders.
      auto httpHeader = GetInterestingHttpHeaders();
      std::vector<Files::DataLake::DataLakePathClient> pathClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetFileClient(RandomString());
        EXPECT_NO_THROW(client.Create());
        EXPECT_NO_THROW(client.SetHttpHeaders(httpHeader));
        pathClient.emplace_back(std::move(client));
      }
      for (const auto& client : pathClient)
      {
        auto result = client.GetProperties();
        EXPECT_EQ(httpHeader.CacheControl, result.Value.HttpHeaders.CacheControl);
        EXPECT_EQ(httpHeader.ContentDisposition, result.Value.HttpHeaders.ContentDisposition);
        EXPECT_EQ(httpHeader.ContentLanguage, result.Value.HttpHeaders.ContentLanguage);
        EXPECT_EQ(httpHeader.ContentType, result.Value.HttpHeaders.ContentType);
      }
    }

    {
      // Set HTTP headers work with last modified access condition.
      auto response = m_pathClient->GetProperties();
      Files::DataLake::SetPathHttpHeadersOptions options1;
      options1.AccessConditions.IfModifiedSince = response.Value.LastModified;
      EXPECT_THROW(
          m_pathClient->SetHttpHeaders(GetInterestingHttpHeaders(), options1), StorageException);
      Files::DataLake::SetPathHttpHeadersOptions options2;
      options2.AccessConditions.IfUnmodifiedSince = response.Value.LastModified;
      EXPECT_NO_THROW(m_pathClient->SetHttpHeaders(GetInterestingHttpHeaders(), options2));
    }

    {
      // Set HTTP headers work with last modified access condition.
      auto response = m_pathClient->GetProperties();
      Files::DataLake::SetPathHttpHeadersOptions options1;
      options1.AccessConditions.IfNoneMatch = response.Value.ETag;
      EXPECT_THROW(
          m_pathClient->SetHttpHeaders(GetInterestingHttpHeaders(), options1), StorageException);
      Files::DataLake::SetPathHttpHeadersOptions options2;
      options2.AccessConditions.IfMatch = response.Value.ETag;
      EXPECT_NO_THROW(m_pathClient->SetHttpHeaders(GetInterestingHttpHeaders(), options2));
    }
  }

  TEST_F(DataLakePathClientTest, PathAccessControls)
  {
    {
      // Set/Get Acls works.
      std::vector<Files::DataLake::Models::Acl> acls = GetValidAcls();
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
      std::vector<Files::DataLake::Models::Acl> acls = GetValidAcls();

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
      std::vector<Files::DataLake::Models::Acl> acls = GetValidAcls();
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
    {
      auto pathClient = Files::DataLake::DataLakePathClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(), m_fileSystemName, RandomString());
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
          AdlsGen2ConnectionString(), m_fileSystemName, RandomString());
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
          AdlsGen2ConnectionString(), m_fileSystemName, RandomString());
      auto response = pathClient.Create(Files::DataLake::Models::PathResourceType::File);
      Files::DataLake::SetPathPermissionsOptions options1, options2;
      options1.AccessConditions.IfMatch = response.Value.ETag;
      options2.AccessConditions.IfNoneMatch = response.Value.ETag;
      std::string pathPermissions = "rwxrw-rw-";
      EXPECT_THROW(pathClient.SetPermissions(pathPermissions, options2), StorageException);
      EXPECT_NO_THROW(pathClient.SetPermissions(pathPermissions, options1));
    }
  }
#if 0
  TEST_F(DataLakePathClientTest, LeaseRelated)
  {
    std::string leaseId1 = CreateUniqueLeaseId();
    int32_t leaseDuration = 20;
    auto lastModified = m_pathClient->GetProperties().Value.LastModified;
    auto aLease = *m_pathClient->AcquireLease(leaseId1, leaseDuration);
    EXPECT_FALSE(aLease.ETag.empty());
    EXPECT_FALSE(aLease.LastModified > lastModified);
    EXPECT_EQ(aLease.LeaseId, leaseId1);
    aLease = *m_pathClient->AcquireLease(leaseId1, leaseDuration);
    EXPECT_FALSE(aLease.ETag.empty());
    EXPECT_FALSE(aLease.LastModified > lastModified);
    EXPECT_EQ(aLease.LeaseId, leaseId1);

    auto properties = *m_pathClient->GetProperties();
    EXPECT_EQ(properties.LeaseState.Value(), Files::DataLake::Models::LeaseStateType::Leased);
    EXPECT_EQ(properties.LeaseStatus.Value(), Files::DataLake::Models::LeaseStatusType::Locked);
    EXPECT_FALSE(properties.LeaseDuration.Value().empty());

    lastModified = properties.LastModified;
    auto rLease = *m_pathClient->RenewLease(leaseId1);
    EXPECT_FALSE(rLease.ETag.empty());
    EXPECT_FALSE(rLease.LastModified > lastModified);
    EXPECT_EQ(rLease.LeaseId, leaseId1);

    std::string leaseId2 = CreateUniqueLeaseId();
    EXPECT_NE(leaseId1, leaseId2);
    lastModified = m_pathClient->GetProperties().Value.LastModified;
    auto cLease = *m_pathClient->ChangeLease(leaseId1, leaseId2);
    EXPECT_FALSE(cLease.ETag.empty());
    EXPECT_FALSE(cLease.LastModified > lastModified);
    EXPECT_EQ(cLease.LeaseId, leaseId2);

    lastModified = m_pathClient->GetProperties().Value.LastModified;
    auto pathInfo = *m_pathClient->ReleaseLease(leaseId2);
    EXPECT_FALSE(pathInfo.ETag.empty());
    EXPECT_FALSE(pathInfo.LastModified > lastModified);

    lastModified = m_pathClient->GetProperties().Value.LastModified;
    aLease = *m_pathClient->AcquireLease(CreateUniqueLeaseId(), InfiniteLeaseDuration);
    properties = *m_pathClient->GetProperties();
    EXPECT_FALSE(properties.LeaseDuration.Value().empty());
    auto brokenLease = *m_pathClient->BreakLease();
    EXPECT_FALSE(brokenLease.ETag.empty());
    EXPECT_FALSE(brokenLease.LastModified > lastModified);
    EXPECT_EQ(brokenLease.LeaseTime, 0);

    aLease = *m_pathClient->AcquireLease(CreateUniqueLeaseId(), leaseDuration);
    Files::DataLake::BreakDataLakePathLeaseOptions breakOptions;
    breakOptions.BreakPeriod = 30;
    lastModified = m_pathClient->GetProperties().Value.LastModified;
    brokenLease = *m_pathClient->BreakLease(breakOptions);
    EXPECT_FALSE(brokenLease.ETag.empty());
    EXPECT_FALSE(brokenLease.LastModified > lastModified);
    EXPECT_NE(brokenLease.LeaseTime, 0);

    Files::DataLake::BreakDataLakePathLeaseOptions options;
    options.BreakPeriod = 0;
    m_pathClient->BreakLease(options);
  }
#endif
  TEST_F(DataLakePathClientTest, ConstructorsWorks)
  {
    {
      // Create from connection string validates static creator function and shared key constructor.
      auto pathName = RandomString();
      auto connectionStringClient
          = Azure::Storage::Files::DataLake::DataLakePathClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(), m_fileSystemName, pathName);
      EXPECT_NO_THROW(
          connectionStringClient.Create(Files::DataLake::Models::PathResourceType::File));
      EXPECT_NO_THROW(connectionStringClient.Delete());
    }

    {
      // Create from client secret credential.
      auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
          AadTenantId(), AadClientId(), AadClientSecret());

      auto clientSecretClient = Azure::Storage::Files::DataLake::DataLakePathClient(
          Files::DataLake::_detail::GetDfsUrlFromUrl(
              Azure::Storage::Files::DataLake::DataLakePathClient::CreateFromConnectionString(
                  AdlsGen2ConnectionString(), m_fileSystemName, RandomString())
                  .GetUrl()),
          credential);

      EXPECT_NO_THROW(clientSecretClient.Create(Files::DataLake::Models::PathResourceType::File));
      EXPECT_NO_THROW(clientSecretClient.Delete());
    }

    {
      // Create from Anonymous credential.
      auto objectName = RandomString();
      auto containerClient = Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(), m_fileSystemName);
      Azure::Storage::Blobs::SetBlobContainerAccessPolicyOptions options;
      options.AccessType = Azure::Storage::Blobs::Models::PublicAccessType::BlobContainer;
      containerClient.SetAccessPolicy(options);

      auto pathClient
          = Azure::Storage::Files::DataLake::DataLakePathClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(), m_fileSystemName, objectName);
      EXPECT_NO_THROW(pathClient.Create(Files::DataLake::Models::PathResourceType::File));

      auto anonymousClient
          = Azure::Storage::Files::DataLake::DataLakePathClient(pathClient.GetUrl());

      std::this_thread::sleep_for(std::chrono::seconds(30));

      EXPECT_NO_THROW(anonymousClient.GetProperties());
    }
  }
}}} // namespace Azure::Storage::Test
