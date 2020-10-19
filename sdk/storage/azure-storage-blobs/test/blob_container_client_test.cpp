// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <chrono>
#include <thread>

#include "azure/storage/blobs/blob_sas_builder.hpp"
#include "azure/storage/common/crypt.hpp"
#include "blob_container_client_test.hpp"

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
    Azure::Storage::Blobs::CreateContainerOptions options;
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
    {
      auto appendBlobClient
          = m_blobContainerClient->GetAppendBlobClient(RandomString() + "-appendblob");
      appendBlobClient.Create();
      auto pageBlobClient = m_blobContainerClient->GetPageBlobClient(RandomString() + "-pageblob");
      pageBlobClient.Create(4096);
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

    Azure::Storage::Blobs::ListBlobsSegmentOptions options;
    options.MaxResults = 4;
    std::set<std::string> listBlobs;
    do
    {
      auto res = m_blobContainerClient->ListBlobsFlatSegment(options);
      EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderRequestId).empty());
      EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderDate).empty());
      EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderXMsVersion).empty());
      EXPECT_FALSE(res->ServiceEndpoint.empty());
      EXPECT_EQ(res->Container, m_containerName);

      options.ContinuationToken = res->ContinuationToken;
      for (const auto& blob : res->Items)
      {
        EXPECT_FALSE(blob.Name.empty());
        EXPECT_FALSE(blob.CreationTime.empty());
        EXPECT_FALSE(blob.LastModified.empty());
        EXPECT_FALSE(blob.ETag.empty());
        EXPECT_NE(blob.BlobType, Azure::Storage::Blobs::BlobType::Unknown);
        if (blob.BlobType == Blobs::BlobType::BlockBlob)
        {
          EXPECT_TRUE(blob.Tier.HasValue());
          EXPECT_TRUE(blob.AccessTierInferred.HasValue());
        }
        if (blob.Tier.HasValue())
        {
          EXPECT_NE(blob.Tier.GetValue(), Azure::Storage::Blobs::AccessTier::Unknown);
        }
        if (blob.BlobType == Blobs::BlobType::AppendBlob)
        {
          if (blob.IsSealed.HasValue())
          {
            EXPECT_FALSE(blob.IsSealed.GetValue());
          }
        }
        else
        {
          EXPECT_FALSE(blob.IsSealed.HasValue());
        }
        if (blob.BlobType == Blobs::BlobType::PageBlob)
        {
          EXPECT_TRUE(blob.SequenceNumber.HasValue());
        }
        else
        {
          EXPECT_FALSE(blob.SequenceNumber.HasValue());
        }
        listBlobs.insert(blob.Name);
      }
    } while (!options.ContinuationToken.GetValue().empty());
    EXPECT_TRUE(
        std::includes(listBlobs.begin(), listBlobs.end(), p1p2Blobs.begin(), p1p2Blobs.end()));

    options.Prefix = prefix1;
    listBlobs.clear();
    do
    {
      auto res = m_blobContainerClient->ListBlobsFlatSegment(options);
      options.ContinuationToken = res->ContinuationToken;
      for (const auto& blob : res->Items)
      {
        listBlobs.insert(blob.Name);
      }
    } while (!options.ContinuationToken.GetValue().empty());
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

    Azure::Storage::Blobs::ListBlobsSegmentOptions options;
    options.Prefix = prefix;
    std::set<std::string> items;
    while (true)
    {
      auto res = m_blobContainerClient->ListBlobsByHierarchySegment(delimiter, options);
      EXPECT_EQ(res->Delimiter, delimiter);
      EXPECT_EQ(res->Prefix, options.Prefix.GetValue());
      EXPECT_TRUE(res->Items.empty());
      for (const auto& i : res->BlobPrefixes)
      {
        items.emplace(i.Name);
      }
      if (!res->ContinuationToken.empty())
      {
        options.ContinuationToken = res->ContinuationToken;
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
        auto res = m_blobContainerClient->ListBlobsByHierarchySegment(delimiter, options);
        EXPECT_EQ(res->Delimiter, delimiter);
        EXPECT_EQ(res->Prefix, options.Prefix.GetValue());
        EXPECT_TRUE(res->BlobPrefixes.empty());
        for (const auto& i : res->Items)
        {
          items.emplace(i.Name);
        }
        if (!res->ContinuationToken.empty())
        {
          options.ContinuationToken = res->ContinuationToken;
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

    Azure::Storage::Blobs::ListBlobsSegmentOptions options;
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
      auto res = m_blobContainerClient->ListBlobsFlatSegment(options);
      options.ContinuationToken = res->ContinuationToken;
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
    } while (!options.ContinuationToken.GetValue().empty());
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

    Blobs::SetContainerAccessPolicyOptions options;
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
    auto aLease = *m_blobContainerClient->AcquireLease(leaseId1, leaseDuration);
    EXPECT_FALSE(aLease.ETag.empty());
    EXPECT_FALSE(aLease.LastModified.empty());
    EXPECT_EQ(aLease.LeaseId, leaseId1);
    aLease = *m_blobContainerClient->AcquireLease(leaseId1, leaseDuration);
    EXPECT_FALSE(aLease.ETag.empty());
    EXPECT_FALSE(aLease.LastModified.empty());
    EXPECT_EQ(aLease.LeaseId, leaseId1);

    auto properties = *m_blobContainerClient->GetProperties();
    EXPECT_EQ(properties.LeaseState, Blobs::BlobLeaseState::Leased);
    EXPECT_EQ(properties.LeaseStatus, Blobs::BlobLeaseStatus::Locked);
    EXPECT_FALSE(properties.LeaseDuration.GetValue().empty());

    auto rLease = *m_blobContainerClient->RenewLease(leaseId1);
    EXPECT_FALSE(rLease.ETag.empty());
    EXPECT_FALSE(rLease.LastModified.empty());
    EXPECT_EQ(rLease.LeaseId, leaseId1);

    std::string leaseId2 = CreateUniqueLeaseId();
    EXPECT_NE(leaseId1, leaseId2);
    auto cLease = *m_blobContainerClient->ChangeLease(leaseId1, leaseId2);
    EXPECT_FALSE(cLease.ETag.empty());
    EXPECT_FALSE(cLease.LastModified.empty());
    EXPECT_EQ(cLease.LeaseId, leaseId2);

    auto containerInfo = *m_blobContainerClient->ReleaseLease(leaseId2);
    EXPECT_FALSE(containerInfo.ETag.empty());
    EXPECT_FALSE(containerInfo.LastModified.empty());

    aLease = *m_blobContainerClient->AcquireLease(CreateUniqueLeaseId(), c_InfiniteLeaseDuration);
    properties = *m_blobContainerClient->GetProperties();
    EXPECT_FALSE(properties.LeaseDuration.GetValue().empty());
    auto brokenLease = *m_blobContainerClient->BreakLease();
    EXPECT_FALSE(brokenLease.ETag.empty());
    EXPECT_FALSE(brokenLease.LastModified.empty());
    EXPECT_EQ(brokenLease.LeaseTime, 0);

    aLease = *m_blobContainerClient->AcquireLease(CreateUniqueLeaseId(), leaseDuration);
    brokenLease = *m_blobContainerClient->BreakLease();
    EXPECT_FALSE(brokenLease.ETag.empty());
    EXPECT_FALSE(brokenLease.LastModified.empty());
    EXPECT_NE(brokenLease.LeaseTime, 0);

    Blobs::BreakContainerLeaseOptions options;
    options.BreakPeriod = 0;
    m_blobContainerClient->BreakLease(options);
  }

  TEST_F(BlobContainerClientTest, EncryptionScope)
  {
    {
      auto properties = *m_blobContainerClient->GetProperties();
      EXPECT_EQ(properties.DefaultEncryptionScope, c_AccountEncryptionKey);
      EXPECT_EQ(properties.PreventEncryptionScopeOverride, false);
    }
    {
      std::string containerName = LowercaseRandomString();
      std::string blobName = RandomString();
      Blobs::BlobContainerClientOptions options;
      options.EncryptionScope = c_TestEncryptionScope;
      auto containerClient = Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
          StandardStorageConnectionString(), containerName, options);
      Blobs::CreateContainerOptions createOptions;
      createOptions.DefaultEncryptionScope = c_TestEncryptionScope;
      createOptions.PreventEncryptionScopeOverride = true;
      EXPECT_NO_THROW(containerClient.Create(createOptions));
      auto properties = *containerClient.GetProperties();
      EXPECT_EQ(properties.DefaultEncryptionScope, createOptions.DefaultEncryptionScope.GetValue());
      EXPECT_EQ(
          properties.PreventEncryptionScopeOverride,
          createOptions.PreventEncryptionScopeOverride.GetValue());
      auto appendBlobClient = containerClient.GetAppendBlobClient(blobName);
      auto blobContentInfo = appendBlobClient.Create();
      appendBlobClient.Delete();
      EXPECT_TRUE(blobContentInfo->EncryptionScope.HasValue());
      EXPECT_EQ(blobContentInfo->EncryptionScope.GetValue(), c_TestEncryptionScope);
      auto appendBlobClientWithoutEncryptionScope
          = Azure::Storage::Blobs::AppendBlobClient::CreateFromConnectionString(
              StandardStorageConnectionString(), containerName, blobName);
      blobContentInfo = appendBlobClientWithoutEncryptionScope.Create();
      appendBlobClientWithoutEncryptionScope.Delete();
      EXPECT_TRUE(blobContentInfo->EncryptionScope.HasValue());
      EXPECT_EQ(blobContentInfo->EncryptionScope.GetValue(), c_TestEncryptionScope);
      containerClient.Delete();
    }
    {
      std::string blobName = RandomString();
      Blobs::AppendBlobClientOptions options;
      options.EncryptionScope = c_TestEncryptionScope;
      auto appendBlobClient = Azure::Storage::Blobs::AppendBlobClient::CreateFromConnectionString(
          StandardStorageConnectionString(), m_containerName, blobName, options);
      auto blobContentInfo = appendBlobClient.Create();
      EXPECT_TRUE(blobContentInfo->EncryptionScope.HasValue());
      EXPECT_EQ(blobContentInfo->EncryptionScope.GetValue(), c_TestEncryptionScope);
      auto properties = *appendBlobClient.GetProperties();
      EXPECT_TRUE(properties.EncryptionScope.HasValue());
      EXPECT_EQ(properties.EncryptionScope.GetValue(), c_TestEncryptionScope);
      std::vector<uint8_t> appendContent(1);
      Azure::Core::Http::MemoryBodyStream bodyStream(appendContent.data(), appendContent.size());
      EXPECT_NO_THROW(appendBlobClient.AppendBlock(&bodyStream));

      bodyStream.Rewind();
      auto appendBlobClientWithoutEncryptionScope
          = Azure::Storage::Blobs::AppendBlobClient::CreateFromConnectionString(
              StandardStorageConnectionString(), m_containerName, blobName);
      EXPECT_THROW(appendBlobClientWithoutEncryptionScope.AppendBlock(&bodyStream), StorageError);
      EXPECT_THROW(appendBlobClientWithoutEncryptionScope.CreateSnapshot(), StorageError);
      appendBlobClient.Delete();
    }
  }

  TEST_F(BlobContainerClientTest, CustomerProvidedKey)
  {
    auto getRandomCustomerProvidedKey = []() {
      Blobs::EncryptionKey key;
      std::string aes256Key;
      aes256Key.resize(32);
      RandomBuffer(&aes256Key[0], aes256Key.size());
      key.Key = Base64Encode(aes256Key);
      key.KeyHash = Base64Encode(Details::Sha256(aes256Key));
      key.Algorithm = Blobs::EncryptionAlgorithmType::Aes256;
      return key;
    };

    Blobs::BlobContainerClientOptions options;
    options.CustomerProvidedKey = getRandomCustomerProvidedKey();
    auto containerClient = Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, options);

    std::vector<uint8_t> blobContent(512);
    Azure::Core::Http::MemoryBodyStream bodyStream(blobContent.data(), blobContent.size());
    auto copySourceBlob = m_blobContainerClient->GetBlockBlobClient(RandomString());
    copySourceBlob.UploadFrom(blobContent.data(), blobContent.size());

    {
      std::string blockBlobName = RandomString();
      auto blockBlob = containerClient.GetBlockBlobClient(blockBlobName);
      bodyStream.Rewind();
      EXPECT_NO_THROW(blockBlob.Upload(&bodyStream));
      std::string blockId1 = Base64Encode("1");
      std::string blockId2 = Base64Encode("2");
      bodyStream.Rewind();
      EXPECT_NO_THROW(blockBlob.StageBlock(blockId1, &bodyStream));
      EXPECT_NO_THROW(blockBlob.StageBlockFromUri(blockId2, copySourceBlob.GetUri() + GetSas()));
      EXPECT_NO_THROW(blockBlob.CommitBlockList(
          {{Blobs::BlockType::Uncommitted, blockId1}, {Blobs::BlockType::Uncommitted, blockId2}}));
      EXPECT_THROW(blockBlob.SetAccessTier(Blobs::AccessTier::Cool), StorageError);

      auto appendBlobClientWithoutEncryptionKey
          = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
              StandardStorageConnectionString(), m_containerName, blockBlobName);
      EXPECT_THROW(
          appendBlobClientWithoutEncryptionKey.SetAccessTier(Blobs::AccessTier::Cool),
          StorageError);
      EXPECT_NO_THROW(appendBlobClientWithoutEncryptionKey.GetBlockList());
    }

    {
      std::string appendBlobName = RandomString();
      auto appendBlob = containerClient.GetAppendBlobClient(appendBlobName);
      auto blobContentInfo = *appendBlob.Create();
      EXPECT_TRUE(blobContentInfo.ServerEncrypted.HasValue());
      EXPECT_TRUE(blobContentInfo.ServerEncrypted.GetValue());
      EXPECT_TRUE(blobContentInfo.EncryptionKeySha256.HasValue());
      EXPECT_EQ(
          blobContentInfo.EncryptionKeySha256.GetValue(),
          options.CustomerProvidedKey.GetValue().KeyHash);

      bodyStream.Rewind();
      EXPECT_NO_THROW(appendBlob.AppendBlock(&bodyStream));
      EXPECT_NO_THROW(appendBlob.AppendBlockFromUri(copySourceBlob.GetUri() + GetSas()));
      EXPECT_NO_THROW(appendBlob.Download());
      EXPECT_NO_THROW(appendBlob.GetProperties());
      EXPECT_NO_THROW(appendBlob.SetMetadata({}));
      EXPECT_NO_THROW(appendBlob.CreateSnapshot());

      auto appendBlobClientWithoutEncryptionKey
          = Azure::Storage::Blobs::AppendBlobClient::CreateFromConnectionString(
              StandardStorageConnectionString(), m_containerName, appendBlobName);
      bodyStream.Rewind();
      EXPECT_THROW(appendBlobClientWithoutEncryptionKey.AppendBlock(&bodyStream), StorageError);
      EXPECT_THROW(
          appendBlobClientWithoutEncryptionKey.AppendBlockFromUri(
              copySourceBlob.GetUri() + GetSas()),
          StorageError);
      EXPECT_THROW(appendBlobClientWithoutEncryptionKey.Download(), StorageError);
      EXPECT_THROW(appendBlobClientWithoutEncryptionKey.GetProperties(), StorageError);
      EXPECT_THROW(appendBlobClientWithoutEncryptionKey.SetMetadata({}), StorageError);
      EXPECT_THROW(appendBlobClientWithoutEncryptionKey.CreateSnapshot(), StorageError);
      EXPECT_NO_THROW(
          appendBlobClientWithoutEncryptionKey.SetHttpHeaders(Blobs::BlobHttpHeaders()));
      Blobs::DeleteBlobOptions deleteOptions;
      deleteOptions.DeleteSnapshots = Blobs::DeleteSnapshotsOption::IncludeSnapshots;
      EXPECT_NO_THROW(appendBlobClientWithoutEncryptionKey.Delete(deleteOptions));
    }

    {
      std::string pageBlobName = RandomString();
      auto pageBlob = containerClient.GetPageBlobClient(pageBlobName);
      auto blobContentInfo = *pageBlob.Create(0);
      EXPECT_TRUE(blobContentInfo.ServerEncrypted.HasValue());
      EXPECT_TRUE(blobContentInfo.ServerEncrypted.GetValue());
      EXPECT_TRUE(blobContentInfo.EncryptionKeySha256.HasValue());
      EXPECT_EQ(
          blobContentInfo.EncryptionKeySha256.GetValue(),
          options.CustomerProvidedKey.GetValue().KeyHash);
      bodyStream.Rewind();
      EXPECT_NO_THROW(pageBlob.Resize(blobContent.size()));
      EXPECT_NO_THROW(pageBlob.UploadPages(0, &bodyStream));
      EXPECT_NO_THROW(pageBlob.ClearPages(0, blobContent.size()));
      EXPECT_NO_THROW(pageBlob.UploadPagesFromUri(
          0, copySourceBlob.GetUri() + GetSas(), 0, blobContent.size()));

      auto pageBlobClientWithoutEncryptionKey
          = Azure::Storage::Blobs::PageBlobClient::CreateFromConnectionString(
              StandardStorageConnectionString(), m_containerName, pageBlobName);
      EXPECT_NO_THROW(pageBlobClientWithoutEncryptionKey.GetPageRanges());
    }
  }

  TEST_F(BlobContainerClientTest, AccessConditionLastModifiedTime)
  {
    auto containerClient = Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
        StandardStorageConnectionString(), LowercaseRandomString());
    containerClient.Create();

    enum class TimePoint
    {
      TimeBefore,
      TimeAfter,
      None,
    };

    enum class Condition
    {
      ModifiedSince,
      UnmodifiedSince,
    };

    for (auto condition : {Condition::ModifiedSince, Condition::UnmodifiedSince})
    {
      for (auto sinceTime : {TimePoint::TimeBefore, TimePoint::TimeAfter})
      {
        auto lastModifiedTime = FromRfc1123(containerClient.GetProperties()->LastModified);
        auto timeBeforeStr = ToRfc1123(lastModifiedTime - std::chrono::seconds(1));
        auto timeAfterStr = ToRfc1123(lastModifiedTime + std::chrono::seconds(1));

        Blobs::SetContainerAccessPolicyOptions options;
        options.AccessType = Blobs::PublicAccessType::Private;
        if (condition == Condition::ModifiedSince)
        {
          options.AccessConditions.IfModifiedSince
              = sinceTime == TimePoint::TimeBefore ? timeBeforeStr : timeAfterStr;
        }
        else if (condition == Condition::UnmodifiedSince)
        {
          options.AccessConditions.IfUnmodifiedSince
              = sinceTime == TimePoint::TimeBefore ? timeBeforeStr : timeAfterStr;
        }
        bool shouldThrow
            = (condition == Condition::ModifiedSince && sinceTime == TimePoint::TimeAfter)
            || (condition == Condition::UnmodifiedSince && sinceTime == TimePoint::TimeBefore);
        if (shouldThrow)
        {
          EXPECT_THROW(containerClient.SetAccessPolicy(options), StorageError);
        }
        else
        {
          EXPECT_NO_THROW(containerClient.SetAccessPolicy(options));
        }
      }
    }
    containerClient.Delete();
  }

  TEST_F(BlobContainerClientTest, AccessConditionLeaseId)
  {
    auto containerClient = Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
        StandardStorageConnectionString(), LowercaseRandomString());
    containerClient.Create();

    std::string leaseId = CreateUniqueLeaseId();
    containerClient.AcquireLease(leaseId, 30);
    EXPECT_THROW(containerClient.Delete(), StorageError);
    Blobs::DeleteContainerOptions options;
    options.AccessConditions.LeaseId = leaseId;
    EXPECT_NO_THROW(containerClient.Delete(options));
  }

  TEST_F(BlobContainerClientTest, Undelete)
  {
    auto serviceClient = Azure::Storage::Blobs::BlobServiceClient::CreateFromConnectionString(
        StandardStorageConnectionString());
    std::string containerName = LowercaseRandomString();
    auto containerClient = serviceClient.GetBlobContainerClient(containerName);
    containerClient.Create();
    containerClient.Delete();

    Blobs::BlobContainerItem deletedContainerItem;
    {
      Azure::Storage::Blobs::ListContainersSegmentOptions options;
      options.Prefix = containerName;
      options.Include = Blobs::ListBlobContainersIncludeItem::Deleted;
      do
      {
        auto res = serviceClient.ListBlobContainersSegment(options);
        options.ContinuationToken = res->ContinuationToken;
        for (const auto& container : res->Items)
        {
          if (container.Name == containerName)
          {
            deletedContainerItem = container;
            break;
          }
        }
      } while (!options.ContinuationToken.GetValue().empty());
    }
    EXPECT_EQ(deletedContainerItem.Name, containerName);
    EXPECT_TRUE(deletedContainerItem.IsDeleted);
    EXPECT_TRUE(deletedContainerItem.VersionId.HasValue());
    EXPECT_FALSE(deletedContainerItem.VersionId.GetValue().empty());
    EXPECT_TRUE(deletedContainerItem.DeletedTime.HasValue());
    EXPECT_FALSE(deletedContainerItem.DeletedTime.GetValue().empty());
    EXPECT_TRUE(deletedContainerItem.RemainingRetentionDays.HasValue());
    EXPECT_GE(deletedContainerItem.RemainingRetentionDays.GetValue(), 0);

    std::string containerName2 = LowercaseRandomString();
    auto containerClient2 = serviceClient.GetBlobContainerClient(containerName2);
    for (int i = 0; i < 60; ++i)
    {
      try
      {
        containerClient2.Undelete(
            deletedContainerItem.Name, deletedContainerItem.VersionId.GetValue());
        break;
      }
      catch (StorageError& e)
      {
        if (e.StatusCode == Azure::Core::Http::HttpStatusCode::Conflict
            && e.ReasonPhrase == "The specified container is being deleted.")
        {
          std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        else
        {
          throw;
        }
      }
    }
    EXPECT_NO_THROW(containerClient2.GetProperties());
  }

  TEST_F(BlobContainerClientTest, Tags)
  {
    std::string blobName = RandomString();
    auto blobClient = Azure::Storage::Blobs::AppendBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, blobName);
    blobClient.Create();

    auto properties = *blobClient.GetProperties();
    EXPECT_FALSE(properties.TagCount.HasValue());

    auto downloadRet = blobClient.Download();
    EXPECT_FALSE(downloadRet->TagCount.HasValue());

    std::map<std::string, std::string> tags;
    std::string c1 = "k" + RandomString();
    std::string v1 = RandomString();
    std::string c2 = "k" + RandomString();
    std::string v2 = RandomString();
    std::string c3 = "k" + RandomString();
    std::string v3 = RandomString();
    tags[c1] = v1;
    tags[c2] = v2;
    tags[c3] = v3;

    auto downloadedTags = blobClient.GetTags()->Tags;
    EXPECT_TRUE(downloadedTags.empty());
    blobClient.SetTags(tags);
    downloadedTags = blobClient.GetTags()->Tags;
    EXPECT_EQ(downloadedTags, tags);

    properties = *blobClient.GetProperties();
    EXPECT_TRUE(properties.TagCount.HasValue());
    EXPECT_EQ(properties.TagCount.GetValue(), static_cast<int64_t>(tags.size()));

    downloadRet = blobClient.Download();
    EXPECT_TRUE(downloadRet->TagCount.HasValue());
    EXPECT_EQ(downloadRet->TagCount.GetValue(), static_cast<int64_t>(tags.size()));

    auto blobServiceClient = Azure::Storage::Blobs::BlobServiceClient::CreateFromConnectionString(
        StandardStorageConnectionString());
    std::string whereExpression
        = c1 + " = '" + v1 + "' AND " + c2 + " >= '" + v2 + "' AND " + c3 + " <= '" + v3 + "'";
    std::vector<Blobs::FilterBlobItem> findResults;
    for (int i = 0; i < 30; ++i)
    {
      std::string marker;
      do
      {
        Blobs::FindBlobsByTagsOptions options;
        if (!marker.empty())
        {
          options.ContinuationToken = marker;
        }
        auto findBlobsRet = *blobServiceClient.FindBlobsByTags(whereExpression, options);
        EXPECT_FALSE(findBlobsRet.ServiceEndpoint.empty());
        EXPECT_EQ(findBlobsRet.Where, whereExpression);
        options.ContinuationToken = findBlobsRet.ContinuationToken;

        for (auto& item : findBlobsRet.Items)
        {
          EXPECT_FALSE(item.BlobName.empty());
          EXPECT_FALSE(item.ContainerName.empty());
          EXPECT_FALSE(item.TagValue.empty());
          findResults.emplace_back(std::move(item));
        }
      } while (!marker.empty());

      if (findResults.empty())
      {
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
      else
      {
        break;
      }
    }
    ASSERT_FALSE(findResults.empty());
    EXPECT_EQ(findResults[0].BlobName, blobName);
    EXPECT_EQ(findResults[0].ContainerName, m_containerName);
    EXPECT_FALSE(findResults[0].TagValue.empty());
  }

  TEST_F(BlobContainerClientTest, AccessConditionTags)
  {
    std::map<std::string, std::string> tags;
    std::string c1 = "k" + RandomString();
    std::string v1 = RandomString();
    tags[c1] = v1;

    std::string successWhereExpression = c1 + " = '" + v1 + "'";
    std::string failWhereExpression = c1 + " != '" + v1 + "'";

    std::vector<uint8_t> contentData(512);
    int64_t contentSize = static_cast<int64_t>(contentData.size());
    auto content = Azure::Core::Http::MemoryBodyStream(contentData.data(), contentSize);

    std::string blobName = RandomString();
    auto appendBlobClient = Azure::Storage::Blobs::AppendBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, blobName);
    appendBlobClient.Create();
    appendBlobClient.SetTags(tags);

    {
      Blobs::GetBlobPropertiesOptions options;
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(appendBlobClient.GetProperties(options));
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(appendBlobClient.GetProperties(options), StorageError);
    }

    {
      Blobs::SetBlobHttpHeadersOptions options;
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(appendBlobClient.SetHttpHeaders(Blobs::BlobHttpHeaders(), options));
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(
          appendBlobClient.SetHttpHeaders(Blobs::BlobHttpHeaders(), options), StorageError);
    }

    {
      Blobs::SetBlobMetadataOptions options;
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(appendBlobClient.SetMetadata({}, options));
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(appendBlobClient.SetMetadata({}, options), StorageError);
    }

    {
      Blobs::DownloadBlobOptions options;
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(appendBlobClient.Download(options));
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(appendBlobClient.Download(options), StorageError);
    }

    {
      Blobs::CreateBlobSnapshotOptions options;
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(appendBlobClient.CreateSnapshot(options));
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(appendBlobClient.CreateSnapshot(options), StorageError);
    }

    {
      Blobs::CreateAppendBlobOptions options;
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(appendBlobClient.Create(options), StorageError);
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(appendBlobClient.Create(options));
      appendBlobClient.SetTags(tags);
    }

    {
      Blobs::AppendBlockOptions options;
      options.AccessConditions.TagConditions = failWhereExpression;
      content.Rewind();
      EXPECT_THROW(appendBlobClient.AppendBlock(&content, options), StorageError);
      options.AccessConditions.TagConditions = successWhereExpression;
      content.Rewind();
      EXPECT_NO_THROW(appendBlobClient.AppendBlock(&content, options));

      std::string uri = appendBlobClient.GetUri() + GetSas();
      Blobs::AppendBlockFromUriOptions options2;
      options2.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(appendBlobClient.AppendBlockFromUri(uri, options2), StorageError);
      options2.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(appendBlobClient.AppendBlockFromUri(uri, options2));
    }

    {
      std::string uri = appendBlobClient.GetUri() + GetSas();

      Blobs::StartCopyBlobFromUriOptions options;
      auto blobClient2 = Azure::Storage::Blobs::AppendBlobClient::CreateFromConnectionString(
          StandardStorageConnectionString(), m_containerName, RandomString());
      options.SourceConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(blobClient2.StartCopyFromUri(uri, options), StorageError);
      options.SourceConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(blobClient2.StartCopyFromUri(uri, options));

      options.SourceConditions.TagConditions.Reset();
      blobClient2.SetTags(tags);

      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(blobClient2.StartCopyFromUri(uri, options), StorageError);
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(blobClient2.StartCopyFromUri(uri, options));
    }

    {
      std::string leaseId = CreateUniqueLeaseId();
      Blobs::AcquireBlobLeaseOptions options;
      options.TagConditions = failWhereExpression;
      EXPECT_THROW(appendBlobClient.AcquireLease(leaseId, 60, options), StorageError);
      options.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(appendBlobClient.AcquireLease(leaseId, 60, options));

      Blobs::BreakBlobLeaseOptions options2;
      options2.TagConditions = failWhereExpression;
      EXPECT_THROW(appendBlobClient.BreakLease(options2), StorageError);
      options2.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(appendBlobClient.BreakLease(options2));

      Blobs::DeleteBlobOptions options3;
      options3.DeleteSnapshots = Blobs::DeleteSnapshotsOption::IncludeSnapshots;
      options3.AccessConditions.LeaseId = leaseId;
      options3.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(appendBlobClient.Delete(options3));
      options3.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(appendBlobClient.Delete(options3), StorageError);
    }

    blobName = RandomString();
    auto pageBlobClient = Azure::Storage::Blobs::PageBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, blobName);
    pageBlobClient.Create(contentSize);
    pageBlobClient.SetTags(tags);

    {
      Blobs::CreatePageBlobOptions options;
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(pageBlobClient.Create(contentSize, options), StorageError);
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(pageBlobClient.Create(contentSize, options));

      pageBlobClient.SetTags(tags);
    }

    {
      Blobs::UploadPageBlobPagesOptions options;
      options.AccessConditions.TagConditions = failWhereExpression;
      content.Rewind();
      EXPECT_THROW(pageBlobClient.UploadPages(0, &content, options), StorageError);
      options.AccessConditions.TagConditions = successWhereExpression;
      content.Rewind();
      EXPECT_NO_THROW(pageBlobClient.UploadPages(0, &content, options));
    }

    {
      std::string uri = pageBlobClient.GetUri() + GetSas();
      Blobs::UploadPageBlobPagesFromUriOptions options;
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(
          pageBlobClient.UploadPagesFromUri(0, uri, 0, contentSize, options), StorageError);
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(pageBlobClient.UploadPagesFromUri(0, uri, 0, contentSize, options));
    }

    {
      Blobs::ClearPageBlobPagesOptions options;
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(pageBlobClient.ClearPages(0, contentSize, options), StorageError);
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(pageBlobClient.ClearPages(0, contentSize, options));
    }

    {
      Blobs::ResizePageBlobOptions options;
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(pageBlobClient.Resize(contentSize, options), StorageError);
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(pageBlobClient.Resize(contentSize, options));
    }

    {
      Blobs::GetPageBlobPageRangesOptions options;
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(pageBlobClient.GetPageRanges(options), StorageError);
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(pageBlobClient.GetPageRanges(options));
    }

    blobName = RandomString();
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, blobName);
    blockBlobClient.UploadFrom(contentData.data(), contentData.size());
    blockBlobClient.SetTags(tags);

    {
      Blobs::UploadBlockBlobOptions options;
      options.AccessConditions.TagConditions = failWhereExpression;
      content.Rewind();
      EXPECT_THROW(blockBlobClient.Upload(&content, options), StorageError);
      options.AccessConditions.TagConditions = successWhereExpression;
      content.Rewind();
      EXPECT_NO_THROW(blockBlobClient.Upload(&content, options));
      blockBlobClient.SetTags(tags);
    }

    {
      std::string blockId = Base64Encode("1");
      std::vector<std::pair<Blobs::BlockType, std::string>> blockIds
          = {{Blobs::BlockType::Uncommitted, blockId}};
      content.Rewind();
      blockBlobClient.StageBlock(blockId, &content);

      Blobs::CommitBlockListOptions options;
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(blockBlobClient.CommitBlockList(blockIds, options), StorageError);
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(blockBlobClient.CommitBlockList(blockIds, options));
      blockBlobClient.SetTags(tags);
    }

    {
      Blobs::GetBlockListOptions options;
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(blockBlobClient.GetBlockList(options), StorageError);
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(blockBlobClient.GetBlockList(options));
    }
  }

  TEST_F(BlobContainerClientTest, SpecialBlobName)
  {
    const std::string non_ascii_word = "\xE6\xB5\x8B\xE8\xAF\x95";
    const std::string encoded_non_ascii_word = "%E6%B5%8B%E8%AF%95";
    std::string baseBlobName = "a b c / !@#$%^&*(?/<>,.;:'\"[]{}|`~\\) def" + non_ascii_word;

    {
      std::string blobName = baseBlobName + RandomString();
      auto blobClient = m_blobContainerClient->GetAppendBlobClient(blobName);
      EXPECT_NO_THROW(blobClient.Create());
      auto blobUrl = blobClient.GetUri();
      EXPECT_EQ(
          blobUrl,
          m_blobContainerClient->GetUri() + "/" + Storage::Details::UrlEncodePath(blobName));
    }
    {
      std::string blobName = baseBlobName + RandomString();
      auto blobClient = m_blobContainerClient->GetPageBlobClient(blobName);
      EXPECT_NO_THROW(blobClient.Create(1024));
      auto blobUrl = blobClient.GetUri();
      EXPECT_EQ(
          blobUrl,
          m_blobContainerClient->GetUri() + "/" + Storage::Details::UrlEncodePath(blobName));
    }
    {
      std::string blobName = baseBlobName + RandomString();
      auto blobClient = m_blobContainerClient->GetBlockBlobClient(blobName);
      EXPECT_NO_THROW(blobClient.UploadFrom(nullptr, 0));
      auto blobUrl = blobClient.GetUri();
      EXPECT_EQ(
          blobUrl,
          m_blobContainerClient->GetUri() + "/" + Storage::Details::UrlEncodePath(blobName));
    }

    {
      std::string blobName = baseBlobName + RandomString();
      auto blobClient = Blobs::AppendBlobClient::CreateFromConnectionString(
          StandardStorageConnectionString(), m_containerName, blobName);
      EXPECT_NO_THROW(blobClient.Create());
      auto blobUrl = blobClient.GetUri();
      EXPECT_EQ(
          blobUrl,
          m_blobContainerClient->GetUri() + "/" + Storage::Details::UrlEncodePath(blobName));
    }
    {
      std::string blobName = baseBlobName + RandomString();
      auto blobClient = Blobs::PageBlobClient::CreateFromConnectionString(
          StandardStorageConnectionString(), m_containerName, blobName);
      EXPECT_NO_THROW(blobClient.Create(1024));
      auto blobUrl = blobClient.GetUri();
      EXPECT_EQ(
          blobUrl,
          m_blobContainerClient->GetUri() + "/" + Storage::Details::UrlEncodePath(blobName));
    }
    {
      std::string blobName = baseBlobName + RandomString();
      auto blobClient = Blobs::BlockBlobClient::CreateFromConnectionString(
          StandardStorageConnectionString(), m_containerName, blobName);
      EXPECT_NO_THROW(blobClient.UploadFrom(nullptr, 0));
      auto blobUrl = blobClient.GetUri();
      EXPECT_EQ(
          blobUrl,
          m_blobContainerClient->GetUri() + "/" + Storage::Details::UrlEncodePath(blobName));
    }
  }

  TEST_F(BlobContainerClientTest, QuestionMarkBlobName)
  {
    std::string blobName = "?";
    auto blobClient = m_blobContainerClient->GetAppendBlobClient(blobName);
    EXPECT_NO_THROW(blobClient.Create());
    auto blobUrl = blobClient.GetUri();
    EXPECT_EQ(
        blobUrl,
        m_blobContainerClient->GetUri() + "/" + Storage::Details::UrlEncodePath(blobName));
  }

}}} // namespace Azure::Storage::Test
