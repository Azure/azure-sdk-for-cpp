// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blob_container_client_test.hpp"
#include "blobs/blob_sas_builder.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  bool operator==(
      const Azure::Storage::Blobs::BlobSignedIdentifier& lhs,
      const Azure::Storage::Blobs::BlobSignedIdentifier& rhs)
  {
    return lhs.Id == rhs.Id && lhs.StartsOn == rhs.StartsOn && lhs.ExpiresOn == rhs.ExpiresOn
        && lhs.Permissions == rhs.Permissions;
  }

}}} // namespace Azure::Storage::Blobs

namespace Azure { namespace Storage { namespace Test {

  std::shared_ptr<Azure::Storage::Blobs::BlobContainerClient>
      BlobContainerClientTest::m_blobContainerClient;
  std::string BlobContainerClientTest::m_containerName;

  void BlobContainerClientTest::SetUpTestSuite()
  {
    m_containerName = LowercaseRandomString();
    auto blobContainerClient
        = Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
            StandardStorageConnectionString(), m_containerName);
    m_blobContainerClient = std::make_shared<Azure::Storage::Blobs::BlobContainerClient>(
        std::move(blobContainerClient));
    m_blobContainerClient->Create();
  }

  void BlobContainerClientTest::TearDownTestSuite() { m_blobContainerClient->Delete(); }

  std::string BlobContainerClientTest::GetSas()
  {
    Blobs::BlobSasBuilder sasBuilder;
    sasBuilder.Protocol = SasProtocol::HttpsAndHtttp;
    sasBuilder.ExpiresOn = ToIso8601(std::chrono::system_clock::now() + std::chrono::hours(72));
    sasBuilder.ContainerName = m_containerName;
    sasBuilder.Resource = Blobs::BlobSasResource::Container;
    sasBuilder.SetPermissions(Blobs::BlobContainerSasPermissions::All);
    return sasBuilder.ToSasQueryParameters(
        *Details::ParseConnectionString(StandardStorageConnectionString()).KeyCredential);
  }

  TEST_F(BlobContainerClientTest, CreateDelete)
  {
    auto container_client = Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
        StandardStorageConnectionString(), LowercaseRandomString());
    Azure::Storage::Blobs::CreateBlobContainerOptions options;
    std::map<std::string, std::string> metadata;
    metadata["key1"] = "one";
    metadata["key2"] = "TWO";
    options.Metadata = metadata;
    auto res = container_client.Create(options);
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderRequestId).empty());
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderDate).empty());
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderXMsVersion).empty());
    EXPECT_FALSE(res->ETag.empty());
    EXPECT_FALSE(res->LastModified.empty());
    EXPECT_THROW(container_client.Create(), StorageError);

    auto res2 = container_client.Delete();
    EXPECT_FALSE(res2.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderRequestId).empty());
    EXPECT_FALSE(res2.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderDate).empty());
    EXPECT_FALSE(res2.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderXMsVersion).empty());
  }

  TEST_F(BlobContainerClientTest, Metadata)
  {
    std::map<std::string, std::string> metadata;
    metadata["key1"] = "one";
    metadata["key2"] = "TWO";
    auto res = m_blobContainerClient->SetMetadata(metadata);
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderRequestId).empty());
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderDate).empty());
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderXMsVersion).empty());
    EXPECT_FALSE(res->ETag.empty());
    EXPECT_FALSE(res->LastModified.empty());

    auto res2 = m_blobContainerClient->GetProperties();
    EXPECT_FALSE(res2.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderRequestId).empty());
    EXPECT_FALSE(res2.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderDate).empty());
    EXPECT_FALSE(res2.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderXMsVersion).empty());
    auto properties = *res2;
    EXPECT_FALSE(properties.ETag.empty());
    EXPECT_FALSE(properties.LastModified.empty());
    EXPECT_EQ(properties.Metadata, metadata);

    metadata.clear();
    m_blobContainerClient->SetMetadata(metadata);
    properties = *m_blobContainerClient->GetProperties();
    EXPECT_TRUE(properties.Metadata.empty());
  }

  TEST_F(BlobContainerClientTest, ListBlobsFlat)
  {
    const std::string prefix1 = "prefix1-";
    const std::string prefix2 = "prefix2-";
    const std::string baseName = "blob";

    std::set<std::string> p1Blobs;
    std::set<std::string> p2Blobs;
    std::set<std::string> p1p2Blobs;

    for (int i = 0; i < 5; ++i)
    {
      std::string blobName = prefix1 + baseName + std::to_string(i);
      auto blobClient = m_blobContainerClient->GetBlockBlobClient(blobName);
      auto emptyContent = Azure::Core::Http::MemoryBodyStream(nullptr, 0);
      blobClient.Upload(&emptyContent);
      p1Blobs.insert(blobName);
      p1p2Blobs.insert(blobName);
    }
    for (int i = 0; i < 5; ++i)
    {
      std::string blobName = prefix2 + baseName + std::to_string(i);
      auto blobClient = m_blobContainerClient->GetBlockBlobClient(blobName);
      auto emptyContent = Azure::Core::Http::MemoryBodyStream(nullptr, 0);
      blobClient.Upload(&emptyContent);
      p2Blobs.insert(blobName);
      p1p2Blobs.insert(blobName);
    }

    Azure::Storage::Blobs::ListBlobsOptions options;
    options.MaxResults = 4;
    std::set<std::string> listBlobs;
    do
    {
      auto res = m_blobContainerClient->ListBlobsFlat(options);
      EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderRequestId).empty());
      EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderDate).empty());
      EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderXMsVersion).empty());
      EXPECT_FALSE(res->ServiceEndpoint.empty());
      EXPECT_EQ(res->Container, m_containerName);

      options.Marker = res->NextMarker;
      for (const auto& blob : res->Items)
      {
        EXPECT_FALSE(blob.Name.empty());
        EXPECT_FALSE(blob.CreationTime.empty());
        EXPECT_FALSE(blob.LastModified.empty());
        EXPECT_FALSE(blob.ETag.empty());
        EXPECT_NE(blob.BlobType, Azure::Storage::Blobs::BlobType::Unknown);
        EXPECT_NE(blob.Tier, Azure::Storage::Blobs::AccessTier::Unknown);
        listBlobs.insert(blob.Name);
      }
    } while (!options.Marker.GetValue().empty());
    EXPECT_TRUE(
        std::includes(listBlobs.begin(), listBlobs.end(), p1p2Blobs.begin(), p1p2Blobs.end()));

    options.Prefix = prefix1;
    listBlobs.clear();
    do
    {
      auto res = m_blobContainerClient->ListBlobsFlat(options);
      options.Marker = res->NextMarker;
      for (const auto& blob : res->Items)
      {
        listBlobs.insert(blob.Name);
      }
    } while (!options.Marker.GetValue().empty());
    EXPECT_TRUE(std::includes(listBlobs.begin(), listBlobs.end(), p1Blobs.begin(), p1Blobs.end()));
  }

  TEST_F(BlobContainerClientTest, ListBlobsHierarchy)
  {
    const std::string delimiter = "/";
    const std::string prefix = RandomString();
    const std::string prefix1 = prefix + "-" + RandomString();
    const std::string prefix2 = prefix + "-" + RandomString();
    std::set<std::string> blobs;
    for (const auto& blobNamePrefix : {prefix1, prefix2})
    {
      for (int i = 0; i < 3; ++i)
      {
        std::string blobName = blobNamePrefix + delimiter + RandomString();
        auto blobClient = m_blobContainerClient->GetBlockBlobClient(blobName);
        auto emptyContent = Azure::Core::Http::MemoryBodyStream(nullptr, 0);
        blobClient.Upload(&emptyContent);
        blobs.insert(blobName);
      }
    }

    Azure::Storage::Blobs::ListBlobsOptions options;
    options.Prefix = prefix;
    std::set<std::string> items;
    while (true)
    {
      auto res = m_blobContainerClient->ListBlobsByHierarchy(delimiter, options);
      EXPECT_EQ(res->Delimiter, delimiter);
      EXPECT_EQ(res->Prefix, options.Prefix.GetValue());
      EXPECT_TRUE(res->Items.empty());
      for (const auto& i : res->BlobPrefixes)
      {
        items.emplace(i.Name);
      }
      if (!res->NextMarker.empty())
      {
        options.Marker = res->NextMarker;
      }
      else
      {
        break;
      }
    }
    EXPECT_EQ(items, (std::set<std::string>{prefix1 + delimiter, prefix2 + delimiter}));

    items.clear();
    for (const auto& p : {prefix1, prefix2})
    {
      options.Prefix = p + delimiter;
      while (true)
      {
        auto res = m_blobContainerClient->ListBlobsByHierarchy(delimiter, options);
        EXPECT_EQ(res->Delimiter, delimiter);
        EXPECT_EQ(res->Prefix, options.Prefix.GetValue());
        EXPECT_TRUE(res->BlobPrefixes.empty());
        for (const auto& i : res->Items)
        {
          items.emplace(i.Name);
        }
        if (!res->NextMarker.empty())
        {
          options.Marker = res->NextMarker;
        }
        else
        {
          break;
        }
      }
    }
    EXPECT_EQ(items, blobs);
  }

  TEST_F(BlobContainerClientTest, ListBlobsOtherStuff)
  {
    std::string blobName = RandomString();
    auto blobClient = m_blobContainerClient->GetAppendBlobClient(blobName);
    blobClient.Create();
    blobClient.Delete();
    blobClient.Create();
    blobClient.CreateSnapshot();
    blobClient.SetMetadata({{"k1", "v1"}});
    std::vector<uint8_t> content(1);
    auto contentStream = Azure::Core::Http::MemoryBodyStream(content.data(), 1);
    blobClient.AppendBlock(&contentStream);

    Azure::Storage::Blobs::ListBlobsOptions options;
    options.Prefix = blobName;
    options.Include = Blobs::ListBlobsIncludeItem::Snapshots | Blobs::ListBlobsIncludeItem::Versions
        | Blobs::ListBlobsIncludeItem::Deleted | Blobs::ListBlobsIncludeItem::Metadata;
    bool foundSnapshot = false;
    bool foundVersions = false;
    bool foundCurrentVersion = false;
    bool foundNotCurrentVersion = false;
    bool foundDeleted = false;
    bool foundMetadata = false;
    do
    {
      auto res = m_blobContainerClient->ListBlobsFlat(options);
      options.Marker = res->NextMarker;
      for (const auto& blob : res->Items)
      {
        if (!blob.Snapshot.empty())
        {
          foundSnapshot = true;
        }
        if (blob.VersionId.HasValue())
        {
          EXPECT_FALSE(blob.VersionId.GetValue().empty());
          foundVersions = true;
        }
        if (blob.IsCurrentVersion.HasValue())
        {
          if (blob.IsCurrentVersion.GetValue())
          {
            foundCurrentVersion = true;
          }
          else
          {
            foundNotCurrentVersion = true;
          }
        }
        if (blob.Deleted)
        {
          foundDeleted = true;
        }
        if (!blob.Metadata.empty())
        {
          foundMetadata = true;
        }
      }
    } while (!options.Marker.GetValue().empty());
    EXPECT_TRUE(foundSnapshot);
    EXPECT_TRUE(foundVersions);
    EXPECT_TRUE(foundCurrentVersion);
    EXPECT_TRUE(foundNotCurrentVersion);
    // Blobs won't be listed as deleted once versioning is enabled
    EXPECT_FALSE(foundDeleted);
    EXPECT_TRUE(foundMetadata);
  }

  TEST_F(BlobContainerClientTest, AccessControlList)
  {
    auto container_client = Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
        StandardStorageConnectionString(), LowercaseRandomString());
    container_client.Create();

    Blobs::SetBlobContainerAccessPolicyOptions options;
    options.AccessType = Blobs::PublicAccessType::Blob;
    Blobs::BlobSignedIdentifier identifier;
    identifier.Id = RandomString(64);
    identifier.StartsOn = ToIso8601(std::chrono::system_clock::now() - std::chrono::minutes(1), 7);
    identifier.ExpiresOn = ToIso8601(std::chrono::system_clock::now() + std::chrono::minutes(1), 7);
    identifier.Permissions
        = Blobs::BlobContainerSasPermissionsToString(Blobs::BlobContainerSasPermissions::Read);
    options.SignedIdentifiers.emplace_back(identifier);
    identifier.Id = RandomString(64);
    identifier.StartsOn = ToIso8601(std::chrono::system_clock::now() - std::chrono::minutes(2), 7);
    identifier.ExpiresOn = ToIso8601(std::chrono::system_clock::now() + std::chrono::minutes(2), 7);
    identifier.Permissions
        = Blobs::BlobContainerSasPermissionsToString(Blobs::BlobContainerSasPermissions::All);
    options.SignedIdentifiers.emplace_back(identifier);

    auto ret = container_client.SetAccessPolicy(options);
    EXPECT_FALSE(ret->ETag.empty());
    EXPECT_FALSE(ret->LastModified.empty());

    auto ret2 = container_client.GetAccessPolicy();
    EXPECT_EQ(ret2->ETag, ret->ETag);
    EXPECT_EQ(ret2->LastModified, ret->LastModified);
    EXPECT_EQ(ret2->AccessType, options.AccessType.GetValue());
    EXPECT_EQ(ret2->SignedIdentifiers, options.SignedIdentifiers);

    container_client.Delete();
  }

  TEST_F(BlobContainerClientTest, Lease)
  {
    std::string leaseId1 = CreateUniqueLeaseId();
    int32_t leaseDuration = 20;
    auto lease = *m_blobContainerClient->AcquireLease(leaseId1, leaseDuration);
    EXPECT_FALSE(lease.ETag.empty());
    EXPECT_FALSE(lease.LastModified.empty());
    EXPECT_EQ(lease.LeaseId, leaseId1);
    lease = *m_blobContainerClient->AcquireLease(leaseId1, leaseDuration);
    EXPECT_FALSE(lease.ETag.empty());
    EXPECT_FALSE(lease.LastModified.empty());
    EXPECT_EQ(lease.LeaseId, leaseId1);

    auto properties = *m_blobContainerClient->GetProperties();
    EXPECT_EQ(properties.LeaseState, Blobs::BlobLeaseState::Leased);
    EXPECT_EQ(properties.LeaseStatus, Blobs::BlobLeaseStatus::Locked);
    EXPECT_FALSE(properties.LeaseDuration.GetValue().empty());

    lease = *m_blobContainerClient->RenewLease(leaseId1);
    EXPECT_FALSE(lease.ETag.empty());
    EXPECT_FALSE(lease.LastModified.empty());
    EXPECT_EQ(lease.LeaseId, leaseId1);

    std::string leaseId2 = CreateUniqueLeaseId();
    EXPECT_NE(leaseId1, leaseId2);
    lease = *m_blobContainerClient->ChangeLease(leaseId1, leaseId2);
    EXPECT_FALSE(lease.ETag.empty());
    EXPECT_FALSE(lease.LastModified.empty());
    EXPECT_EQ(lease.LeaseId, leaseId2);

    auto containerInfo = *m_blobContainerClient->ReleaseLease(leaseId2);
    EXPECT_FALSE(containerInfo.ETag.empty());
    EXPECT_FALSE(containerInfo.LastModified.empty());

    lease = *m_blobContainerClient->AcquireLease(CreateUniqueLeaseId(), c_InfiniteLeaseDuration);
    properties = *m_blobContainerClient->GetProperties();
    EXPECT_FALSE(properties.LeaseDuration.GetValue().empty());
    auto brokenLease = *m_blobContainerClient->BreakLease();
    EXPECT_FALSE(brokenLease.ETag.empty());
    EXPECT_FALSE(brokenLease.LastModified.empty());
    EXPECT_EQ(brokenLease.LeaseTime, 0);

    lease = *m_blobContainerClient->AcquireLease(CreateUniqueLeaseId(), leaseDuration);
    brokenLease = *m_blobContainerClient->BreakLease();
    EXPECT_FALSE(brokenLease.ETag.empty());
    EXPECT_FALSE(brokenLease.LastModified.empty());
    EXPECT_NE(brokenLease.LeaseTime, 0);

    Blobs::BreakBlobContainerLeaseOptions options;
    options.breakPeriod = 0;
    m_blobContainerClient->BreakLease(options);
  }

}}} // namespace Azure::Storage::Test
