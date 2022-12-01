// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <thread>

#include <azure/identity/client_secret_credential.hpp>
#include <azure/storage/blobs.hpp>

#include "test/ut/test_base.hpp"

namespace Azure { namespace Storage { namespace Blobs { namespace Models {

  bool operator==(const RetentionPolicy& lhs, const RetentionPolicy& rhs)
  {
    if (lhs.IsEnabled != rhs.IsEnabled)
    {
      return false;
    }
    if (lhs.Days.HasValue() != rhs.Days.HasValue())
    {
      return false;
    }
    if (lhs.Days.HasValue() && rhs.Days.HasValue() && lhs.Days.Value() != rhs.Days.Value())
    {
      return false;
    }
    return true;
  }

  bool operator==(const CorsRule& lhs, const CorsRule& rhs)
  {
    return lhs.AllowedHeaders == rhs.AllowedHeaders && lhs.AllowedMethods == rhs.AllowedMethods
        && lhs.AllowedOrigins == rhs.AllowedOrigins && lhs.ExposedHeaders == rhs.ExposedHeaders
        && lhs.MaxAgeInSeconds == rhs.MaxAgeInSeconds;
  }

  bool operator==(const StaticWebsite& lhs, const StaticWebsite& rhs)
  {
    if (lhs.IsEnabled != rhs.IsEnabled)
    {
      return false;
    }
    if (lhs.DefaultIndexDocumentPath.HasValue() != rhs.DefaultIndexDocumentPath.HasValue())
    {
      return false;
    }
    if (lhs.DefaultIndexDocumentPath.HasValue() && rhs.DefaultIndexDocumentPath.HasValue()
        && lhs.DefaultIndexDocumentPath.Value() != rhs.DefaultIndexDocumentPath.Value())
    {
      return false;
    }
    if (lhs.DefaultIndexDocumentPath.HasValue() != rhs.DefaultIndexDocumentPath.HasValue())
    {
      return false;
    }
    if (lhs.ErrorDocument404Path.HasValue() && rhs.ErrorDocument404Path.HasValue()
        && lhs.ErrorDocument404Path.Value() != rhs.ErrorDocument404Path.Value())
    {
      return false;
    }
    if (lhs.IndexDocument.HasValue() && rhs.IndexDocument.HasValue()
        && lhs.IndexDocument.Value() != rhs.IndexDocument.Value())
    {
      return false;
    }
    return true;
  }

}}}} // namespace Azure::Storage::Blobs::Models

namespace Azure { namespace Storage { namespace Test {

  class BlobServiceClientTest : public Azure::Storage::Test::StorageTest {

  private:
    std::unique_ptr<Azure::Storage::Blobs::BlobServiceClient> m_client;

  protected:
    // Required to rename the test propertly once the test is started.
    // We can only know the test instance name until the test instance is run.
    Azure::Storage::Blobs::BlobServiceClient const& GetClientForTest(std::string const& testName)
    {
      // set the interceptor for the current test
      m_testContext.RenameTest(testName);
      return *m_client;
    }

    Azure::Storage::Blobs::BlobServiceClient const& GetDataLakeClientService()
    {
      // set the interceptor for the current test
      auto options = InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>();
      m_client = std::make_unique<Azure::Storage::Blobs::BlobServiceClient>(
          Azure::Storage::Blobs::BlobServiceClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(), options));
      return *m_client;
    }

    virtual void SetUp() override
    {
      StorageTest::SetUp();

      auto options = InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>();
      m_client = std::make_unique<Azure::Storage::Blobs::BlobServiceClient>(
          Azure::Storage::Blobs::BlobServiceClient::CreateFromConnectionString(
              StandardStorageConnectionString(), options));
    }
  };

  TEST_F(BlobServiceClientTest, ListContainers)
  {
    const std::string testName = GetTestNameLowerCase();
    auto client = GetClientForTest(testName);

    const std::string prefix1 = testName + "-prefix1-";
    const std::string prefix2 = testName + "-prefix2-";

    std::set<std::string> p1Containers;
    std::set<std::string> p2Containers;
    std::set<std::string> p1p2Containers;

    for (int i = 0; i < 5; ++i)
    {
      std::string containerName = prefix1 + std::to_string(i);
      auto containerClient = client.GetBlobContainerClient(containerName);
      containerClient.Create();
      p1Containers.insert(containerName);
      p1p2Containers.insert(containerName);
    }
    for (int i = 0; i < 5; ++i)
    {
      std::string containerName = prefix2 + std::to_string(i);
      auto containerClient = client.GetBlobContainerClient(containerName);
      containerClient.Create();
      p2Containers.insert(containerName);
      p1p2Containers.insert(containerName);
    }

    Azure::Storage::Blobs::ListBlobContainersOptions options;
    options.PageSizeHint = 4;
    std::set<std::string> listContainers;
    for (auto pageResult = client.ListBlobContainers(options); pageResult.HasPage();
         pageResult.MoveToNextPage())
    {
      EXPECT_FALSE(pageResult.RawResponse->GetHeaders().at(_internal::HttpHeaderRequestId).empty());
      EXPECT_FALSE(pageResult.RawResponse->GetHeaders().at(_internal::HttpHeaderDate).empty());
      EXPECT_FALSE(
          pageResult.RawResponse->GetHeaders().at(_internal::HttpHeaderXMsVersion).empty());
      EXPECT_FALSE(pageResult.ServiceEndpoint.empty());
      for (const auto& container : pageResult.BlobContainers)
      {
        listContainers.insert(container.Name);
      }
    }
    EXPECT_TRUE(std::includes(
        listContainers.begin(),
        listContainers.end(),
        p1p2Containers.begin(),
        p1p2Containers.end()));

    // List with prefix
    options.Prefix = prefix1;
    listContainers.clear();
    for (auto pageResult = client.ListBlobContainers(options); pageResult.HasPage();
         pageResult.MoveToNextPage())
    {
      EXPECT_FALSE(pageResult.RawResponse->GetHeaders().at(_internal::HttpHeaderRequestId).empty());
      EXPECT_FALSE(pageResult.RawResponse->GetHeaders().at(_internal::HttpHeaderDate).empty());
      EXPECT_FALSE(
          pageResult.RawResponse->GetHeaders().at(_internal::HttpHeaderXMsVersion).empty());
      EXPECT_FALSE(pageResult.ServiceEndpoint.empty());
      for (const auto& container : pageResult.BlobContainers)
      {
        EXPECT_FALSE(container.Name.empty());
        EXPECT_TRUE(container.Details.ETag.HasValue());
        EXPECT_TRUE(IsValidTime(container.Details.LastModified));
        EXPECT_FALSE(container.IsDeleted);
        EXPECT_FALSE(container.VersionId.HasValue());
        EXPECT_FALSE(container.Details.DeletedOn.HasValue());
        EXPECT_FALSE(container.Details.RemainingRetentionDays.HasValue());
        EXPECT_EQ(container.Details.DefaultEncryptionScope, AccountEncryptionKey);
        EXPECT_FALSE(container.Details.PreventEncryptionScopeOverride);
        listContainers.insert(container.Name);
      }
    }
    EXPECT_TRUE(std::includes(
        listContainers.begin(), listContainers.end(), p1Containers.begin(), p1Containers.end()));

    // Remove all containers
    for (const auto& container : p1p2Containers)
    {
      auto container_client = client.GetBlobContainerClient(container);
      container_client.Delete();
    }
  }

  TEST_F(BlobServiceClientTest, ListSystemContainers)
  {
    const std::string testName = GetTestNameLowerCase();
    auto client = GetClientForTest(testName);
    Azure::Storage::Blobs::ListBlobContainersOptions options;
    options.Include = Blobs::Models::ListBlobContainersIncludeFlags::System;
    std::vector<std::string> containers;
    for (auto pageResult = client.ListBlobContainers(options); pageResult.HasPage();
         pageResult.MoveToNextPage())
    {
      for (const auto& c : pageResult.BlobContainers)
      {
        if (c.Name[0] == '$')
        {
          containers.push_back(c.Name);
        }
      }
    }

    EXPECT_FALSE(containers.empty());
  }

  TEST_F(BlobServiceClientTest, GetProperties)
  {
    const std::string testName = GetTestName();
    auto client = GetClientForTest(testName);

    auto ret = client.GetProperties();
    auto properties = ret.Value;
    auto logging = properties.Logging;
    EXPECT_FALSE(logging.Version.empty());
    if (logging.RetentionPolicy.IsEnabled)
    {
      EXPECT_TRUE(logging.RetentionPolicy.Days.HasValue());
    }
    auto hourMetrics = properties.HourMetrics;
    if (hourMetrics.IsEnabled)
    {
      EXPECT_FALSE(hourMetrics.Version.empty());
      if (hourMetrics.RetentionPolicy.IsEnabled)
      {
        EXPECT_TRUE(hourMetrics.RetentionPolicy.Days.HasValue());
      }
    }
    auto minuteMetrics = properties.HourMetrics;
    if (minuteMetrics.IsEnabled)
    {
      EXPECT_FALSE(minuteMetrics.Version.empty());
      if (minuteMetrics.RetentionPolicy.IsEnabled)
      {
        EXPECT_TRUE(minuteMetrics.RetentionPolicy.Days.HasValue());
      }
    }
    auto deleteRetentionPolicy = properties.DeleteRetentionPolicy;
    if (deleteRetentionPolicy.IsEnabled)
    {
      EXPECT_TRUE(deleteRetentionPolicy.Days.HasValue());
    }
  }

  TEST_F(BlobServiceClientTest, SetProperties)
  {
    const std::string testName = GetTestName();
    auto client = GetClientForTest(testName);

    auto getServicePropertiesResult = client.GetProperties().Value;
    Blobs::Models::BlobServiceProperties properties;
    properties.Logging = getServicePropertiesResult.Logging;
    properties.HourMetrics = getServicePropertiesResult.HourMetrics;
    properties.MinuteMetrics = getServicePropertiesResult.MinuteMetrics;
    properties.Cors = getServicePropertiesResult.Cors;
    properties.DefaultServiceVersion = getServicePropertiesResult.DefaultServiceVersion;
    properties.DeleteRetentionPolicy = getServicePropertiesResult.DeleteRetentionPolicy;
    properties.StaticWebsite = getServicePropertiesResult.StaticWebsite;

    auto originalProperties = properties;

    properties.Logging.Delete = !properties.Logging.Delete;
    properties.Logging.Read = !properties.Logging.Read;
    properties.Logging.Write = !properties.Logging.Write;
    properties.Logging.RetentionPolicy.IsEnabled = true;
    properties.Logging.RetentionPolicy.Days = 3;

    properties.HourMetrics.IsEnabled = true;
    properties.HourMetrics.RetentionPolicy.IsEnabled = true;
    properties.HourMetrics.RetentionPolicy.Days = 4;
    properties.HourMetrics.IncludeApis = true;

    properties.MinuteMetrics.IsEnabled = true;
    properties.MinuteMetrics.RetentionPolicy.IsEnabled = true;
    properties.MinuteMetrics.RetentionPolicy.Days = 4;
    properties.MinuteMetrics.IncludeApis = true;

    properties.DefaultServiceVersion = Blobs::_detail::ApiVersion;

    properties.StaticWebsite.IsEnabled = true;
    properties.StaticWebsite.IndexDocument = "index.html";
    properties.StaticWebsite.ErrorDocument404Path = "404.html";
    properties.StaticWebsite.DefaultIndexDocumentPath.Reset();

    Blobs::Models::CorsRule corsRule;
    corsRule.AllowedOrigins = "http://www.example1.com";
    corsRule.AllowedMethods = "GET,PUT";
    corsRule.AllowedHeaders = "x-ms-header1,x-ms-header2";
    corsRule.ExposedHeaders = "x-ms-header3";
    corsRule.MaxAgeInSeconds = 10;
    properties.Cors.emplace_back(corsRule);

    corsRule.AllowedOrigins = "http://www.example2.com";
    corsRule.AllowedMethods = "DELETE";
    corsRule.AllowedHeaders = "x-ms-header1";
    corsRule.ExposedHeaders = "x-ms-header2,x-ms-header3";
    corsRule.MaxAgeInSeconds = 20;
    properties.Cors.emplace_back(corsRule);

    properties.DeleteRetentionPolicy.IsEnabled = true;
    properties.DeleteRetentionPolicy.Days = 7;

    EXPECT_NO_THROW(client.SetProperties(properties));

    // It takes some time before the new properties comes into effect.
    using namespace std::chrono_literals;
    TestSleep(10s);
    auto downloadedProperties = client.GetProperties().Value;
    EXPECT_EQ(downloadedProperties.Logging.Version, properties.Logging.Version);
    EXPECT_EQ(downloadedProperties.Logging.Delete, properties.Logging.Delete);
    EXPECT_EQ(downloadedProperties.Logging.Read, properties.Logging.Read);
    EXPECT_EQ(downloadedProperties.Logging.Write, properties.Logging.Write);
    EXPECT_EQ(downloadedProperties.Logging.RetentionPolicy, properties.Logging.RetentionPolicy);

    EXPECT_EQ(downloadedProperties.HourMetrics.Version, properties.HourMetrics.Version);
    EXPECT_EQ(downloadedProperties.HourMetrics.IsEnabled, properties.HourMetrics.IsEnabled);
    EXPECT_EQ(
        downloadedProperties.HourMetrics.IncludeApis.HasValue(),
        properties.HourMetrics.IncludeApis.HasValue());
    if (downloadedProperties.HourMetrics.IncludeApis.HasValue()
        == properties.HourMetrics.IncludeApis.HasValue())
    {
      EXPECT_EQ(
          downloadedProperties.HourMetrics.IncludeApis.Value(),
          properties.HourMetrics.IncludeApis.Value());
    }
    EXPECT_EQ(
        downloadedProperties.HourMetrics.RetentionPolicy, properties.HourMetrics.RetentionPolicy);

    EXPECT_EQ(downloadedProperties.MinuteMetrics.Version, properties.MinuteMetrics.Version);
    EXPECT_EQ(downloadedProperties.MinuteMetrics.IsEnabled, properties.MinuteMetrics.IsEnabled);
    EXPECT_EQ(
        downloadedProperties.MinuteMetrics.IncludeApis.HasValue(),
        properties.MinuteMetrics.IncludeApis.HasValue());
    if (downloadedProperties.MinuteMetrics.IncludeApis.HasValue()
        == properties.MinuteMetrics.IncludeApis.HasValue())
    {
      EXPECT_EQ(
          downloadedProperties.MinuteMetrics.IncludeApis.Value(),
          properties.MinuteMetrics.IncludeApis.Value());
    }
    EXPECT_EQ(
        downloadedProperties.MinuteMetrics.RetentionPolicy,
        properties.MinuteMetrics.RetentionPolicy);

    EXPECT_EQ(
        downloadedProperties.DefaultServiceVersion.HasValue(),
        properties.DefaultServiceVersion.HasValue());
    if (downloadedProperties.DefaultServiceVersion.HasValue())
    {
      EXPECT_EQ(
          downloadedProperties.DefaultServiceVersion.Value(),
          properties.DefaultServiceVersion.Value());
    }
    EXPECT_EQ(downloadedProperties.Cors, properties.Cors);

    EXPECT_EQ(downloadedProperties.StaticWebsite, properties.StaticWebsite);

    EXPECT_EQ(downloadedProperties.DeleteRetentionPolicy, properties.DeleteRetentionPolicy);

    auto res = client.SetProperties(originalProperties);
  }

  TEST_F(BlobServiceClientTest, AccountInfo)
  {
    const std::string testName = GetTestName();
    auto client = GetClientForTest(testName);

    auto accountInfo = client.GetAccountInfo().Value;
    EXPECT_FALSE(accountInfo.SkuName.ToString().empty());
    EXPECT_FALSE(accountInfo.AccountKind.ToString().empty());
    EXPECT_FALSE(accountInfo.IsHierarchicalNamespaceEnabled);

    auto dataLakeServiceClient = GetDataLakeClientService();
    accountInfo = dataLakeServiceClient.GetAccountInfo().Value;
    EXPECT_TRUE(accountInfo.IsHierarchicalNamespaceEnabled);
  }

  TEST_F(BlobServiceClientTest, Statistics)
  {
    const std::string testName = GetTestName();
    auto client = GetClientForTest(testName);

    EXPECT_THROW(client.GetStatistics(), StorageException);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    auto secondaryServiceClient = Blobs::BlobServiceClient(
        InferSecondaryUrl(client.GetUrl()),
        keyCredential,
        InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>());

    auto serviceStatistics = secondaryServiceClient.GetStatistics().Value;
    EXPECT_FALSE(serviceStatistics.GeoReplication.Status.ToString().empty());
    if (serviceStatistics.GeoReplication.LastSyncedOn.HasValue())
    {
      EXPECT_TRUE(IsValidTime(serviceStatistics.GeoReplication.LastSyncedOn.Value()));
    }
  }

  TEST_F(BlobServiceClientTest, CreateDeleteBlobContainer)
  {
    const std::string testName = GetTestNameLowerCase();
    auto client = GetClientForTest(testName);

    std::string containerName = testName;
    auto containerClient = client.CreateBlobContainer(containerName);
    EXPECT_NO_THROW(containerClient.Value.GetProperties());

    client.DeleteBlobContainer(containerName);
    EXPECT_THROW(containerClient.Value.GetProperties(), StorageException);
  }

  TEST_F(BlobServiceClientTest, UndeleteBlobContainer)
  {
    const std::string testName = GetTestNameLowerCase();
    auto client = GetClientForTest(testName);

    std::string containerName = testName;
    auto containerClient = client.GetBlobContainerClient(containerName);
    containerClient.Create();
    containerClient.Delete();

    Blobs::Models::BlobContainerItem deletedContainerItem;
    {
      Azure::Storage::Blobs::ListBlobContainersOptions options;
      options.Prefix = containerName;
      options.Include = Blobs::Models::ListBlobContainersIncludeFlags::Deleted;
      for (auto pageResult = client.ListBlobContainers(options); pageResult.HasPage();
           pageResult.MoveToNextPage())
      {
        for (const auto& container : pageResult.BlobContainers)
        {
          if (container.Name == containerName)
          {
            deletedContainerItem = container;
            break;
          }
        }
      }
    }
    EXPECT_EQ(deletedContainerItem.Name, containerName);
    EXPECT_TRUE(deletedContainerItem.IsDeleted);
    EXPECT_TRUE(deletedContainerItem.VersionId.HasValue());
    EXPECT_FALSE(deletedContainerItem.VersionId.Value().empty());
    EXPECT_TRUE(deletedContainerItem.Details.DeletedOn.HasValue());
    EXPECT_TRUE(IsValidTime(deletedContainerItem.Details.DeletedOn.Value()));
    EXPECT_TRUE(deletedContainerItem.Details.RemainingRetentionDays.HasValue());
    EXPECT_GE(deletedContainerItem.Details.RemainingRetentionDays.Value(), 0);

    for (int i = 0; i < 60; ++i)
    {
      try
      {
        client.UndeleteBlobContainer(
            deletedContainerItem.Name, deletedContainerItem.VersionId.Value());
        break;
      }
      catch (const StorageException& e)
      {
        if (e.StatusCode == Azure::Core::Http::HttpStatusCode::Conflict
            && e.ReasonPhrase == "The specified container is being deleted.")
        {
          TestSleep(1s);
        }
        else
        {
          throw;
        }
      }
    }
    EXPECT_NO_THROW(containerClient.GetProperties());
  }

  TEST_F(BlobServiceClientTest, UserDelegationKey_LIVEONLY_)
  {
    const std::string testName = GetTestName();
    auto client = GetClientForTest(testName);

    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential
        = std::make_shared<Azure::Identity::ClientSecretCredential>(
            AadTenantId(), AadClientId(), AadClientSecret());
    Blobs::BlobClientOptions options;

    auto blobServiceClient1 = InitTestClient<Blobs::BlobServiceClient, Blobs::BlobClientOptions>(
        client.GetUrl(), credential, options);

    auto userDelegationKey = blobServiceClient1->GetUserDelegationKey(sasExpiresOn).Value;

    EXPECT_FALSE(userDelegationKey.SignedObjectId.empty());
    EXPECT_FALSE(userDelegationKey.SignedTenantId.empty());
    EXPECT_TRUE(IsValidTime(userDelegationKey.SignedStartsOn));
    EXPECT_TRUE(IsValidTime(userDelegationKey.SignedExpiresOn));
    EXPECT_FALSE(userDelegationKey.SignedService.empty());
    EXPECT_FALSE(userDelegationKey.SignedVersion.empty());
    EXPECT_FALSE(userDelegationKey.Value.empty());
  }

  TEST_F(BlobServiceClientTest, DISABLED_RenameBlobContainer)
  {
    const std::string testName = GetTestNameLowerCase();
    auto serviceClient = GetClientForTest(testName);

    const std::string srcContainerName = testName + "src";
    auto srcContainerClient = serviceClient.CreateBlobContainer(srcContainerName).Value;

    const std::string destContainerName = testName + "dest1";
    auto destContainerClient
        = serviceClient.RenameBlobContainer(srcContainerName, destContainerName).Value;

    EXPECT_THROW(srcContainerClient.GetProperties(), StorageException);
    EXPECT_NO_THROW(destContainerClient.GetProperties());

    Blobs::BlobLeaseClient leaseClient(
        destContainerClient, Blobs::BlobLeaseClient::CreateUniqueLeaseId());
    leaseClient.Acquire(std::chrono::seconds(60));

    const std::string destContainerName2 = testName + "dest2";
    Blobs::RenameBlobContainerOptions renameOptions;
    renameOptions.SourceAccessConditions.LeaseId = Blobs::BlobLeaseClient::CreateUniqueLeaseId();
    EXPECT_THROW(
        serviceClient.RenameBlobContainer(destContainerName, destContainerName2, renameOptions),
        StorageException);
    renameOptions.SourceAccessConditions.LeaseId = leaseClient.GetLeaseId();
    EXPECT_NO_THROW(
        serviceClient.RenameBlobContainer(destContainerName, destContainerName2, renameOptions));

    auto destContainerClient2 = serviceClient.GetBlobContainerClient(destContainerName2);
    destContainerClient2.Delete();
  }
}}} // namespace Azure::Storage::Test
