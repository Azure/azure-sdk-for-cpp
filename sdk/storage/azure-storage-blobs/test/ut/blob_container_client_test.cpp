// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blob_container_client_test.hpp"

#include <chrono>
#include <thread>

#include <azure/core/internal/cryptography/sha_hash.hpp>
#include <azure/storage/blobs/blob_lease_client.hpp>
#include <azure/storage/blobs/blob_sas_builder.hpp>
#include <azure/storage/common/crypt.hpp>

namespace Azure { namespace Storage { namespace Blobs { namespace Models {

  bool operator==(const SignedIdentifier& lhs, const SignedIdentifier& rhs);

}}}} // namespace Azure::Storage::Blobs::Models

namespace Azure { namespace Storage { namespace Test {

  std::string BlobContainerClientTest::GetSas()
  {
    Sas::BlobSasBuilder sasBuilder;
    sasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    sasBuilder.ExpiresOn = std::chrono::system_clock::now() + std::chrono::hours(72);
    sasBuilder.BlobContainerName = m_containerName;
    sasBuilder.Resource = Sas::BlobSasResource::BlobContainer;
    sasBuilder.SetPermissions(Sas::BlobContainerSasPermissions::All);
    return sasBuilder.GenerateSasToken(
        *_internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential);
  }

  Blobs::Models::BlobItem BlobContainerClientTest::GetBlobItem(
      const std::string& blobName,
      Blobs::Models::ListBlobsIncludeFlags include)
  {
    Blobs::ListBlobsOptions options;
    options.Prefix = blobName;
    options.Include = include;
    for (auto page = m_blobContainerClient->ListBlobs(options); page.HasPage();
         page.MoveToNextPage())
    {
      for (auto& blob : page.Blobs)
      {
        if (blob.Name == blobName)
        {
          return std::move(blob);
        }
      }
    }
    std::abort();
  }

  TEST_F(BlobContainerClientTest, CreateDelete)
  {
    auto container_client = GetBlobContainerTestClient();
    Azure::Storage::Blobs::CreateBlobContainerOptions options;
    Azure::Storage::Metadata metadata;
    metadata["key1"] = "one";
    metadata["key2"] = "TWO";
    options.Metadata = metadata;
    auto res = container_client.Create(options);
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderRequestId).empty());
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderDate).empty());
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderXMsVersion).empty());
    EXPECT_TRUE(res.Value.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(res.Value.LastModified));
    EXPECT_THROW(container_client.Create(), StorageException);

    auto res2 = container_client.Delete();
    EXPECT_FALSE(res2.RawResponse->GetHeaders().at(_internal::HttpHeaderRequestId).empty());
    EXPECT_FALSE(res2.RawResponse->GetHeaders().at(_internal::HttpHeaderDate).empty());
    EXPECT_FALSE(res2.RawResponse->GetHeaders().at(_internal::HttpHeaderXMsVersion).empty());

    container_client = GetBlobContainerTestClient("UPPERCASE");
    EXPECT_THROW(container_client.CreateIfNotExists(), StorageException);
    container_client = GetBlobContainerTestClient("2");
    {
      auto response = container_client.DeleteIfExists();
      EXPECT_FALSE(response.Value.Deleted);
    }
    {
      auto response = container_client.CreateIfNotExists();
      EXPECT_TRUE(response.Value.Created);
    }
    {
      auto response = container_client.CreateIfNotExists();
      EXPECT_FALSE(response.Value.Created);
    }
    {
      auto response = container_client.DeleteIfExists();
      EXPECT_TRUE(response.Value.Deleted);
    }
  }

  TEST_F(BlobContainerClientTest, Metadata)
  {
    auto client = GetBlobContainerTestClient();
    client.Create();
    Azure::Storage::Metadata metadata;
    metadata["key1"] = "one";
    metadata["key2"] = "TWO";
    auto res = client.SetMetadata(metadata);
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderRequestId).empty());
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderDate).empty());
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderXMsVersion).empty());
    EXPECT_TRUE(res.Value.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(res.Value.LastModified));

    auto res2 = client.GetProperties();
    EXPECT_FALSE(res2.RawResponse->GetHeaders().at(_internal::HttpHeaderRequestId).empty());
    EXPECT_FALSE(res2.RawResponse->GetHeaders().at(_internal::HttpHeaderDate).empty());
    EXPECT_FALSE(res2.RawResponse->GetHeaders().at(_internal::HttpHeaderXMsVersion).empty());
    auto properties = res2.Value;
    EXPECT_TRUE(properties.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(properties.LastModified));
    EXPECT_EQ(properties.Metadata, metadata);

    metadata.clear();
    client.SetMetadata(metadata);
    properties = client.GetProperties().Value;
    EXPECT_TRUE(properties.Metadata.empty());
  }

  TEST_F(BlobContainerClientTest, ListBlobsFlat)
  {
    auto client = GetBlobContainerTestClient();
    client.Create();

    const std::string prefix1 = "prefix1-";
    const std::string prefix2 = "prefix2-";
    const std::string baseName = "blob";

    std::set<std::string> p1Blobs;
    std::set<std::string> p2Blobs;
    std::set<std::string> p1p2Blobs;

    for (int i = 0; i < 5; ++i)
    {
      std::string blobName = prefix1 + baseName + std::to_string(i);
      auto blobClient = client.GetBlockBlobClient(blobName);
      auto emptyContent = Azure::Core::IO::MemoryBodyStream(nullptr, 0);
      blobClient.Upload(emptyContent);
      p1Blobs.insert(blobName);
      p1p2Blobs.insert(blobName);
    }
    {
      auto appendBlobClient = client.GetAppendBlobClient(m_containerName + "-appendblob");
      appendBlobClient.Create();
      auto pageBlobClient = client.GetPageBlobClient(m_containerName + "-pageblob");
      pageBlobClient.Create(4096);
    }
    for (int i = 0; i < 5; ++i)
    {
      std::string blobName = prefix2 + baseName + std::to_string(i);
      auto blobClient = client.GetBlockBlobClient(blobName);
      auto emptyContent = Azure::Core::IO::MemoryBodyStream(nullptr, 0);
      blobClient.Upload(emptyContent);
      p2Blobs.insert(blobName);
      p1p2Blobs.insert(blobName);
    }

    Azure::Storage::Blobs::ListBlobsOptions options;
    options.PageSizeHint = 4;
    std::set<std::string> listBlobs;
    for (auto pageResult = client.ListBlobs(options); pageResult.HasPage();
         pageResult.MoveToNextPage())
    {
      EXPECT_FALSE(pageResult.RawResponse->GetHeaders().at(_internal::HttpHeaderRequestId).empty());
      EXPECT_FALSE(pageResult.RawResponse->GetHeaders().at(_internal::HttpHeaderDate).empty());
      EXPECT_FALSE(
          pageResult.RawResponse->GetHeaders().at(_internal::HttpHeaderXMsVersion).empty());
      EXPECT_FALSE(pageResult.ServiceEndpoint.empty());
      EXPECT_EQ(pageResult.BlobContainerName, m_containerName);
      for (const auto& blob : pageResult.Blobs)
      {
        EXPECT_FALSE(blob.Name.empty());
        EXPECT_TRUE(IsValidTime(blob.Details.CreatedOn));
        EXPECT_TRUE(IsValidTime(blob.Details.LastModified));
        EXPECT_TRUE(blob.Details.ETag.HasValue());
        EXPECT_FALSE(blob.BlobType.ToString().empty());
        if (blob.BlobType == Blobs::Models::BlobType::BlockBlob)
        {
          EXPECT_TRUE(blob.Details.AccessTier.HasValue());
          EXPECT_TRUE(blob.Details.IsAccessTierInferred.HasValue());
        }
        if (blob.Details.AccessTier.HasValue())
        {
          EXPECT_FALSE(blob.Details.AccessTier.Value().ToString().empty());
        }
        if (blob.BlobType == Blobs::Models::BlobType::AppendBlob)
        {
          if (blob.Details.IsSealed.HasValue())
          {
            EXPECT_FALSE(blob.Details.IsSealed.Value());
          }
        }
        else
        {
          EXPECT_FALSE(blob.Details.IsSealed.HasValue());
        }
        if (blob.BlobType == Blobs::Models::BlobType::PageBlob)
        {
          EXPECT_TRUE(blob.Details.SequenceNumber.HasValue());
        }
        else
        {
          EXPECT_FALSE(blob.Details.SequenceNumber.HasValue());
        }
        listBlobs.insert(blob.Name);
      }
    }
    EXPECT_TRUE(
        std::includes(listBlobs.begin(), listBlobs.end(), p1p2Blobs.begin(), p1p2Blobs.end()));

    options.Prefix = prefix1;
    listBlobs.clear();
    for (auto pageResult = client.ListBlobs(options); pageResult.HasPage();
         pageResult.MoveToNextPage())
    {
      for (const auto& blob : pageResult.Blobs)
      {
        listBlobs.insert(blob.Name);
      }
    }
    EXPECT_TRUE(std::includes(listBlobs.begin(), listBlobs.end(), p1Blobs.begin(), p1Blobs.end()));
  }

  TEST_F(BlobContainerClientTest, ListBlobsByHierarchy)
  {
    auto client = GetBlobContainerTestClient();
    client.Create();

    const std::string delimiter = "/";
    const std::string prefix = m_containerName;
    const std::string prefix1 = prefix + "-" + m_containerName;
    const std::string prefix2 = prefix + "-" + m_containerName;
    std::set<std::string> blobs;
    for (const auto& blobNamePrefix : {prefix1, prefix2})
    {
      for (int i = 0; i < 3; ++i)
      {
        std::string blobName = blobNamePrefix + delimiter + m_containerName + std::to_string(i);
        auto blobClient = client.GetBlockBlobClient(blobName);
        auto emptyContent = Azure::Core::IO::MemoryBodyStream(nullptr, 0);
        blobClient.Upload(emptyContent);
        blobs.insert(blobName);
      }
    }

    Azure::Storage::Blobs::ListBlobsOptions options;
    options.Prefix = prefix;
    std::set<std::string> items;
    for (auto pageResult = client.ListBlobsByHierarchy(delimiter, options); pageResult.HasPage();
         pageResult.MoveToNextPage())
    {
      EXPECT_EQ(pageResult.Delimiter, delimiter);
      EXPECT_EQ(pageResult.Prefix, options.Prefix.Value());
      EXPECT_TRUE(pageResult.Blobs.empty());
      for (const auto& p : pageResult.BlobPrefixes)
      {
        items.emplace(p);
      }
    }
    EXPECT_EQ(items, (std::set<std::string>{prefix1 + delimiter, prefix2 + delimiter}));

    items.clear();
    for (const auto& p : {prefix1, prefix2})
    {
      options.Prefix = p + delimiter;
      for (auto pageResult = client.ListBlobsByHierarchy(delimiter, options); pageResult.HasPage();
           pageResult.MoveToNextPage())
      {
        EXPECT_EQ(pageResult.Delimiter, delimiter);
        EXPECT_EQ(pageResult.Prefix, options.Prefix.Value());
        EXPECT_TRUE(pageResult.BlobPrefixes.empty());
        for (const auto& i : pageResult.Blobs)
        {
          items.emplace(i.Name);
        }
      }
    }
    EXPECT_EQ(items, blobs);
  }

  // NOTE: This test Requires storage account with versioning enabled!
  // https://docs.microsoft.com/en-us/azure/storage/blobs/versioning-enable?tabs=portal
  TEST_F(BlobContainerClientTest, ListBlobsOtherStuff)
  {
    auto client = GetBlobContainerTestClient();
    client.Create();

    std::string blobName = "blob" + m_containerName;
    auto blobClient = client.GetAppendBlobClient(blobName);
    blobClient.Create();
    blobClient.Delete();
    blobClient.Create();
    blobClient.CreateSnapshot();
    blobClient.SetMetadata({{"k1", "v1"}});
    std::vector<uint8_t> content(1);
    auto contentStream = Azure::Core::IO::MemoryBodyStream(content.data(), 1);
    blobClient.AppendBlock(contentStream);

    Azure::Storage::Blobs::ListBlobsOptions options;
    options.Prefix = blobName;
    options.Include = Blobs::Models::ListBlobsIncludeFlags::Snapshots
        | Blobs::Models::ListBlobsIncludeFlags::Versions
        | Blobs::Models::ListBlobsIncludeFlags::Deleted
        | Blobs::Models::ListBlobsIncludeFlags::Metadata;
    bool foundSnapshot = false;
    bool foundVersions = false;
    bool foundCurrentVersion = false;
    bool foundNotCurrentVersion = false;
    bool foundDeleted = false;
    bool foundMetadata = false;

    for (auto pageResult = client.ListBlobs(options); pageResult.HasPage();
         pageResult.MoveToNextPage())
    {
      for (const auto& blob : pageResult.Blobs)
      {
        if (!blob.Snapshot.empty())
        {
          foundSnapshot = true;
        }
        if (blob.VersionId.HasValue())
        {
          EXPECT_FALSE(blob.VersionId.Value().empty());
          foundVersions = true;
        }
        if (blob.IsCurrentVersion.HasValue())
        {
          if (blob.IsCurrentVersion.Value())
          {
            foundCurrentVersion = true;
          }
          else
          {
            foundNotCurrentVersion = true;
          }
        }
        if (blob.IsDeleted)
        {
          foundDeleted = true;
        }
        if (!blob.Details.Metadata.empty())
        {
          foundMetadata = true;
        }
      }
    }
    EXPECT_TRUE(foundSnapshot);
    EXPECT_TRUE(foundVersions);
    EXPECT_TRUE(foundCurrentVersion);
    EXPECT_TRUE(foundNotCurrentVersion);
    // Blobs won't be listed as deleted once versioning is enabled
    EXPECT_FALSE(foundDeleted);
    EXPECT_TRUE(foundMetadata);
  }

  // Test uses StartsOn and ExpiresOn values based on time now() + minutes
  // Hence, the test can't be recorded and need to run on live mode always.
  TEST_F(BlobContainerClientTest, AccessControlList_LIVEONLY_)
  {
    // will skip test under some cased where test can't run (usually LIVE only tests)
    CHECK_SKIP_TEST()

    auto client = GetBlobContainerTestClient();
    client.Create();

    Blobs::SetBlobContainerAccessPolicyOptions options;
    options.AccessType = Blobs::Models::PublicAccessType::Blob;
    {
      Blobs::Models::SignedIdentifier identifier;
      identifier.Id = GetStringOfSize(63) + "1";
      identifier.StartsOn = std::chrono::system_clock::now() - std::chrono::minutes(1);
      identifier.ExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(1);
      identifier.Permissions = "r";
      options.SignedIdentifiers.emplace_back(identifier);
    }
    {
      Blobs::Models::SignedIdentifier identifier;
      identifier.Id = GetStringOfSize(63) + "2";
      identifier.StartsOn = std::chrono::system_clock::now() - std::chrono::minutes(2);
      identifier.ExpiresOn.Reset();
      /* cspell:disable-next-line */
      identifier.Permissions = "racwdxlt";
      options.SignedIdentifiers.emplace_back(identifier);
    }
    {
      Blobs::Models::SignedIdentifier identifier;
      identifier.Id = GetStringOfSize(63) + "3";
      identifier.Permissions = "r";
      options.SignedIdentifiers.emplace_back(identifier);
    }
    {
      Blobs::Models::SignedIdentifier identifier;
      identifier.Id = GetStringOfSize(63) + "4";
      identifier.StartsOn = std::chrono::system_clock::now() - std::chrono::minutes(1);
      identifier.ExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(1);
      options.SignedIdentifiers.emplace_back(identifier);
    }

    auto ret = client.SetAccessPolicy(options);
    EXPECT_TRUE(ret.Value.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(ret.Value.LastModified));

    auto ret2 = client.GetAccessPolicy();
    EXPECT_EQ(ret2.Value.AccessType, options.AccessType);
    EXPECT_EQ(ret2.Value.SignedIdentifiers, options.SignedIdentifiers);
  }

  TEST_F(BlobContainerClientTest, Lease_LIVEONLY_)
  {
    auto client = GetBlobContainerTestClient();
    client.Create();

    {
      std::string leaseId1 = Blobs::BlobLeaseClient::CreateUniqueLeaseId();
      auto leaseDuration = std::chrono::seconds(20);
      Blobs::BlobLeaseClient leaseClient(client, leaseId1);
      auto aLease = leaseClient.Acquire(leaseDuration).Value;
      EXPECT_TRUE(aLease.ETag.HasValue());
      EXPECT_TRUE(IsValidTime(aLease.LastModified));
      EXPECT_EQ(aLease.LeaseId, leaseId1);
      EXPECT_EQ(leaseClient.GetLeaseId(), leaseId1);
      aLease = leaseClient.Acquire(leaseDuration).Value;
      EXPECT_TRUE(aLease.ETag.HasValue());
      EXPECT_TRUE(IsValidTime(aLease.LastModified));
      EXPECT_EQ(aLease.LeaseId, leaseId1);

      auto properties = client.GetProperties().Value;
      EXPECT_EQ(properties.LeaseState, Blobs::Models::LeaseState::Leased);
      EXPECT_EQ(properties.LeaseStatus, Blobs::Models::LeaseStatus::Locked);
      EXPECT_EQ(properties.LeaseDuration.Value(), Blobs::Models::LeaseDurationType::Fixed);

      auto rLease = leaseClient.Renew().Value;
      EXPECT_TRUE(rLease.ETag.HasValue());
      EXPECT_TRUE(IsValidTime(rLease.LastModified));
      EXPECT_EQ(rLease.LeaseId, leaseId1);

      std::string leaseId2 = Blobs::BlobLeaseClient::CreateUniqueLeaseId();
      EXPECT_NE(leaseId1, leaseId2);
      auto cLease = leaseClient.Change(leaseId2).Value;
      EXPECT_TRUE(cLease.ETag.HasValue());
      EXPECT_TRUE(IsValidTime(cLease.LastModified));
      EXPECT_EQ(cLease.LeaseId, leaseId2);
      EXPECT_EQ(leaseClient.GetLeaseId(), leaseId2);

      auto containerInfo = leaseClient.Release().Value;
      EXPECT_TRUE(containerInfo.ETag.HasValue());
      EXPECT_TRUE(IsValidTime(containerInfo.LastModified));
    }

    {
      Blobs::BlobLeaseClient leaseClient(client, Blobs::BlobLeaseClient::CreateUniqueLeaseId());
      auto aLease = leaseClient.Acquire(Blobs::BlobLeaseClient::InfiniteLeaseDuration).Value;
      auto properties = client.GetProperties().Value;
      EXPECT_EQ(properties.LeaseDuration.Value(), Blobs::Models::LeaseDurationType::Infinite);
      auto brokenLease = leaseClient.Break().Value;
      EXPECT_TRUE(brokenLease.ETag.HasValue());
      EXPECT_TRUE(IsValidTime(brokenLease.LastModified));
    }

    {
      Blobs::BlobLeaseClient leaseClient(client, Blobs::BlobLeaseClient::CreateUniqueLeaseId());
      auto leaseDuration = std::chrono::seconds(20);
      auto aLease = leaseClient.Acquire(leaseDuration).Value;
      auto brokenLease = leaseClient.Break().Value;
      EXPECT_TRUE(brokenLease.ETag.HasValue());
      EXPECT_TRUE(IsValidTime(brokenLease.LastModified));

      Blobs::BreakLeaseOptions options;
      options.BreakPeriod = std::chrono::seconds(0);
      leaseClient.Break(options);
    }
  }

  TEST_F(BlobContainerClientTest, EncryptionScope)
  {
    auto blobContainerClient = GetBlobContainerTestClient();
    blobContainerClient.CreateIfNotExists();
    auto const& testEncryptionScope = GetTestEncryptionScope();

    {
      auto properties = blobContainerClient.GetProperties().Value;
      EXPECT_EQ(properties.DefaultEncryptionScope, AccountEncryptionKey);
      EXPECT_EQ(properties.PreventEncryptionScopeOverride, false);
    }
    {
      std::string containerName = GetContainerValidName() + "1";
      std::string blobName = GetTestName() + "1";
      Blobs::BlobClientOptions options = InitClientOptions<Blobs::BlobClientOptions>();
      options.EncryptionScope = testEncryptionScope;
      auto containerClient = Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
          StandardStorageConnectionString(), containerName, options);
      Blobs::CreateBlobContainerOptions createOptions;
      createOptions.DefaultEncryptionScope = testEncryptionScope;
      createOptions.PreventEncryptionScopeOverride = true;
      EXPECT_NO_THROW(containerClient.Create(createOptions));
      auto properties = containerClient.GetProperties().Value;
      EXPECT_EQ(properties.DefaultEncryptionScope, createOptions.DefaultEncryptionScope.Value());
      EXPECT_EQ(
          properties.PreventEncryptionScopeOverride,
          createOptions.PreventEncryptionScopeOverride.Value());
      auto appendBlobClient = containerClient.GetAppendBlobClient(blobName);
      auto blobContentInfo = appendBlobClient.Create();
      {
        Blobs::ListBlobsOptions listOptions;
        listOptions.Prefix = blobName;
        for (auto page = containerClient.ListBlobs(listOptions); page.HasPage();
             page.MoveToNextPage())
        {
          for (auto& blob : page.Blobs)
          {
            if (blob.Name == blobName)
            {
              EXPECT_TRUE(blob.Details.IsServerEncrypted);
              EXPECT_TRUE(blob.Details.EncryptionScope.HasValue());
              EXPECT_EQ(blob.Details.EncryptionScope.Value(), testEncryptionScope);
            }
          }
        }
      }
      appendBlobClient.Delete();
      EXPECT_TRUE(blobContentInfo.Value.EncryptionScope.HasValue());
      EXPECT_EQ(blobContentInfo.Value.EncryptionScope.Value(), testEncryptionScope);
      auto appendBlobClientWithoutEncryptionScope = containerClient.GetAppendBlobClient(blobName);
      blobContentInfo = appendBlobClientWithoutEncryptionScope.Create();
      appendBlobClientWithoutEncryptionScope.Delete();
      EXPECT_TRUE(blobContentInfo.Value.EncryptionScope.HasValue());
      EXPECT_EQ(blobContentInfo.Value.EncryptionScope.Value(), testEncryptionScope);
      containerClient.Delete();
    }
    {
      std::string blobName = GetTestName() + "2";
      Blobs::BlobClientOptions options = InitClientOptions<Blobs::BlobClientOptions>();
      options.EncryptionScope = testEncryptionScope;
      auto appendBlobClient = Azure::Storage::Blobs::AppendBlobClient::CreateFromConnectionString(
          StandardStorageConnectionString(), m_containerName, blobName, options);
      auto blobContentInfo = appendBlobClient.Create();
      EXPECT_TRUE(blobContentInfo.Value.EncryptionScope.HasValue());
      EXPECT_EQ(blobContentInfo.Value.EncryptionScope.Value(), testEncryptionScope);
      auto setMetadataRes = appendBlobClient.SetMetadata({});
      EXPECT_TRUE(setMetadataRes.Value.IsServerEncrypted);
      ASSERT_TRUE(setMetadataRes.Value.EncryptionScope.HasValue());
      EXPECT_EQ(setMetadataRes.Value.EncryptionScope.Value(), testEncryptionScope);
      auto properties = appendBlobClient.GetProperties().Value;
      EXPECT_TRUE(properties.EncryptionScope.HasValue());
      EXPECT_EQ(properties.EncryptionScope.Value(), testEncryptionScope);
      std::vector<uint8_t> appendContent(1);
      Azure::Core::IO::MemoryBodyStream bodyStream(appendContent.data(), appendContent.size());
      EXPECT_NO_THROW(appendBlobClient.AppendBlock(bodyStream));

      bodyStream.Rewind();
      auto appendBlobClientWithoutEncryptionScope
          = blobContainerClient.GetAppendBlobClient(blobName);
      EXPECT_THROW(
          appendBlobClientWithoutEncryptionScope.AppendBlock(bodyStream), StorageException);
      EXPECT_THROW(appendBlobClientWithoutEncryptionScope.CreateSnapshot(), StorageException);
      appendBlobClient.Delete();
    }
  }

  TEST_F(BlobContainerClientTest, CustomerProvidedKey_LIVEONLY_)
  {
    // will skip test under some cased where test can't run (usually LIVE only tests)
    CHECK_SKIP_TEST()

    auto client = GetBlobContainerTestClient();
    client.Create();

    auto getRandomCustomerProvidedKey = [&]() {
      Blobs::EncryptionKey key;
      std::vector<uint8_t> aes256Key;
      aes256Key.resize(32);
      RandomBuffer(&aes256Key[0], aes256Key.size());
      key.Key = Azure::Core::Convert::Base64Encode(aes256Key);
      key.KeyHash = Azure::Core::Cryptography::_internal::Sha256Hash().Final(
          aes256Key.data(), aes256Key.size());
      key.Algorithm = Blobs::Models::EncryptionAlgorithmType::Aes256;
      return key;
    };

    Blobs::BlobClientOptions options;
    options.CustomerProvidedKey = getRandomCustomerProvidedKey();
    auto containerClient = Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, options);

    std::vector<uint8_t> blobContent(512);
    Azure::Core::IO::MemoryBodyStream bodyStream(blobContent.data(), blobContent.size());
    auto copySourceBlob = client.GetBlockBlobClient(RandomString());
    copySourceBlob.UploadFrom(blobContent.data(), blobContent.size());

    {
      std::string blockBlobName = RandomString();
      auto blockBlob = containerClient.GetBlockBlobClient(blockBlobName);
      bodyStream.Rewind();
      EXPECT_NO_THROW(blockBlob.Upload(bodyStream));
      std::string blockId1 = Base64EncodeText("1");
      std::string blockId2 = Base64EncodeText("2");
      bodyStream.Rewind();
      EXPECT_NO_THROW(blockBlob.StageBlock(blockId1, bodyStream));
      EXPECT_NO_THROW(blockBlob.StageBlockFromUri(blockId2, copySourceBlob.GetUrl() + GetSas()));
      EXPECT_NO_THROW(blockBlob.CommitBlockList({blockId1, blockId2}));
      EXPECT_THROW(blockBlob.SetAccessTier(Blobs::Models::AccessTier::Cool), StorageException);

      auto appendBlobClientWithoutEncryptionKey
          = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
              StandardStorageConnectionString(), m_containerName, blockBlobName);
      EXPECT_THROW(
          appendBlobClientWithoutEncryptionKey.SetAccessTier(Blobs::Models::AccessTier::Cool),
          StorageException);
      EXPECT_NO_THROW(appendBlobClientWithoutEncryptionKey.GetBlockList());
    }

    {
      std::string appendBlobName = RandomString();
      auto appendBlob = containerClient.GetAppendBlobClient(appendBlobName);
      auto blobContentInfo = appendBlob.Create().Value;
      EXPECT_TRUE(blobContentInfo.IsServerEncrypted);
      EXPECT_TRUE(blobContentInfo.EncryptionKeySha256.HasValue());
      EXPECT_EQ(
          blobContentInfo.EncryptionKeySha256.Value(), options.CustomerProvidedKey.Value().KeyHash);
      auto blobItem = GetBlobItem(appendBlobName);
      EXPECT_TRUE(blobItem.Details.IsServerEncrypted);
      EXPECT_TRUE(blobItem.Details.EncryptionKeySha256.HasValue());
      EXPECT_EQ(
          blobItem.Details.EncryptionKeySha256.Value(),
          options.CustomerProvidedKey.Value().KeyHash);

      bodyStream.Rewind();
      EXPECT_NO_THROW(appendBlob.AppendBlock(bodyStream));
      EXPECT_NO_THROW(appendBlob.AppendBlockFromUri(copySourceBlob.GetUrl() + GetSas()));
      EXPECT_NO_THROW(appendBlob.Download());
      EXPECT_NO_THROW(appendBlob.GetProperties());
      auto setMetadataRes = appendBlob.SetMetadata({});
      EXPECT_TRUE(setMetadataRes.Value.IsServerEncrypted);
      ASSERT_TRUE(setMetadataRes.Value.EncryptionKeySha256.HasValue());
      EXPECT_EQ(
          setMetadataRes.Value.EncryptionKeySha256.Value(),
          options.CustomerProvidedKey.Value().KeyHash);
      EXPECT_NO_THROW(appendBlob.CreateSnapshot());

      auto appendBlobClientWithoutEncryptionKey
          = Azure::Storage::Blobs::AppendBlobClient::CreateFromConnectionString(
              StandardStorageConnectionString(), m_containerName, appendBlobName);
      bodyStream.Rewind();
      EXPECT_THROW(appendBlobClientWithoutEncryptionKey.AppendBlock(bodyStream), StorageException);
      EXPECT_THROW(
          appendBlobClientWithoutEncryptionKey.AppendBlockFromUri(
              copySourceBlob.GetUrl() + GetSas()),
          StorageException);
      EXPECT_THROW(appendBlobClientWithoutEncryptionKey.Download(), StorageException);
      EXPECT_THROW(appendBlobClientWithoutEncryptionKey.GetProperties(), StorageException);
      EXPECT_THROW(appendBlobClientWithoutEncryptionKey.SetMetadata({}), StorageException);
      EXPECT_THROW(appendBlobClientWithoutEncryptionKey.CreateSnapshot(), StorageException);
      EXPECT_NO_THROW(
          appendBlobClientWithoutEncryptionKey.SetHttpHeaders(Blobs::Models::BlobHttpHeaders()));
      Blobs::DeleteBlobOptions deleteOptions;
      deleteOptions.DeleteSnapshots = Blobs::Models::DeleteSnapshotsOption::IncludeSnapshots;
      EXPECT_NO_THROW(appendBlobClientWithoutEncryptionKey.Delete(deleteOptions));
    }

    {
      std::string pageBlobName = RandomString();
      auto pageBlob = containerClient.GetPageBlobClient(pageBlobName);
      auto blobContentInfo = pageBlob.Create(0).Value;
      EXPECT_TRUE(blobContentInfo.IsServerEncrypted);
      EXPECT_TRUE(blobContentInfo.EncryptionKeySha256.HasValue());
      EXPECT_EQ(
          blobContentInfo.EncryptionKeySha256.Value(), options.CustomerProvidedKey.Value().KeyHash);
      bodyStream.Rewind();
      EXPECT_NO_THROW(pageBlob.Resize(blobContent.size()));
      EXPECT_NO_THROW(pageBlob.UploadPages(0, bodyStream));
      EXPECT_NO_THROW(pageBlob.ClearPages({0, static_cast<int64_t>(blobContent.size())}));
      EXPECT_NO_THROW(pageBlob.UploadPagesFromUri(
          0, copySourceBlob.GetUrl() + GetSas(), {0, static_cast<int64_t>(blobContent.size())}));

      auto pageBlobClientWithoutEncryptionKey
          = Azure::Storage::Blobs::PageBlobClient::CreateFromConnectionString(
              StandardStorageConnectionString(), m_containerName, pageBlobName);
      EXPECT_NO_THROW(pageBlobClientWithoutEncryptionKey.GetPageRanges());
      EXPECT_NO_THROW(pageBlobClientWithoutEncryptionKey.Resize(blobContent.size() + 512));
    }
  }

  TEST_F(BlobContainerClientTest, AccessConditionLastModifiedTime)
  {
    auto client = GetBlobContainerTestClient();
    client.Create();

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
        auto lastModifiedTime = client.GetProperties().Value.LastModified;
        auto timeBefore = lastModifiedTime - std::chrono::seconds(1);
        auto timeAfter = lastModifiedTime + std::chrono::seconds(1);

        Blobs::SetBlobContainerAccessPolicyOptions options;
        options.AccessType = Blobs::Models::PublicAccessType::None;
        if (condition == Condition::ModifiedSince)
        {
          options.AccessConditions.IfModifiedSince
              = sinceTime == TimePoint::TimeBefore ? timeBefore : timeAfter;
        }
        else if (condition == Condition::UnmodifiedSince)
        {
          options.AccessConditions.IfUnmodifiedSince
              = sinceTime == TimePoint::TimeBefore ? timeBefore : timeAfter;
        }
        bool shouldThrow
            = (condition == Condition::ModifiedSince && sinceTime == TimePoint::TimeAfter)
            || (condition == Condition::UnmodifiedSince && sinceTime == TimePoint::TimeBefore);
        if (shouldThrow)
        {
          EXPECT_THROW(client.SetAccessPolicy(options), StorageException);
        }
        else
        {
          EXPECT_NO_THROW(client.SetAccessPolicy(options));
        }
      }
    }
    client.Delete();
  }

  TEST_F(BlobContainerClientTest, AccessConditionLeaseId)
  {
    auto client = GetBlobContainerTestClient();
    client.Create();

    const std::string leaseId = Blobs::BlobLeaseClient::CreateUniqueLeaseId();
    const std::string dummyLeaseId = Blobs::BlobLeaseClient::CreateUniqueLeaseId();
    Blobs::BlobLeaseClient leaseClient(client, leaseId);
    leaseClient.Acquire(std::chrono::seconds(30));
    {
      Blobs::GetBlobContainerPropertiesOptions options;
      options.AccessConditions.LeaseId = dummyLeaseId;
      EXPECT_THROW(client.GetProperties(options), StorageException);
      options.AccessConditions.LeaseId = leaseId;
      EXPECT_NO_THROW(client.GetProperties(options));
    }
    {
      Blobs::SetBlobContainerMetadataOptions options;
      options.AccessConditions.LeaseId = dummyLeaseId;
      EXPECT_THROW(client.SetMetadata({}, options), StorageException);
      options.AccessConditions.LeaseId = leaseId;
      EXPECT_NO_THROW(client.SetMetadata({}, options));
    }
    {
      Blobs::GetBlobContainerAccessPolicyOptions options;
      options.AccessConditions.LeaseId = dummyLeaseId;
      EXPECT_THROW(client.GetAccessPolicy(options), StorageException);
      options.AccessConditions.LeaseId = leaseId;
      EXPECT_NO_THROW(client.GetAccessPolicy(options));
    }
    {
      Blobs::SetBlobContainerAccessPolicyOptions options;
      options.AccessConditions.LeaseId = dummyLeaseId;
      EXPECT_THROW(client.SetAccessPolicy(options), StorageException);
      options.AccessConditions.LeaseId = leaseId;
      EXPECT_NO_THROW(client.SetAccessPolicy(options));
    }
    {
      EXPECT_THROW(client.Delete(), StorageException);
      Blobs::DeleteBlobContainerOptions options;
      options.AccessConditions.LeaseId = leaseId;
      EXPECT_NO_THROW(client.Delete(options));
    }
  }

  TEST_F(BlobContainerClientTest, Tags)
  {
    auto containerClient = GetBlobContainerTestClient();
    containerClient.Create();

    std::string blobName = "blob" + m_containerName;
    auto blobClient = containerClient.GetAppendBlobClient(blobName);
    blobClient.Create();

    auto properties = blobClient.GetProperties().Value;
    EXPECT_FALSE(properties.TagCount.HasValue());

    auto downloadRet = blobClient.Download();
    EXPECT_FALSE(downloadRet.Value.Details.TagCount.HasValue());

    std::map<std::string, std::string> tags;
    std::string c1 = "k" + m_containerName + "1";
    std::string v1 = m_containerName + "2";
    std::string c2 = "k" + m_containerName + "3";
    std::string v2 = m_containerName + "4";
    std::string c3 = "k" + m_containerName + "5";
    std::string v3 = m_containerName + "6";
    std::string c4 = "key3 +-./:=_";
    std::string v4 = "v1 +-./:=_";
    tags[c1] = v1;
    tags[c2] = v2;
    tags[c3] = v3;

    auto downloadedTags = blobClient.GetTags().Value;
    EXPECT_TRUE(downloadedTags.empty());
    blobClient.SetTags(tags);
    downloadedTags = blobClient.GetTags().Value;
    EXPECT_EQ(downloadedTags, tags);

    properties = blobClient.GetProperties().Value;
    EXPECT_TRUE(properties.TagCount.HasValue());
    EXPECT_EQ(properties.TagCount.Value(), static_cast<int32_t>(tags.size()));

    downloadRet = blobClient.Download();
    ASSERT_TRUE(downloadRet.Value.Details.TagCount.HasValue());
    EXPECT_EQ(downloadRet.Value.Details.TagCount.Value(), static_cast<int32_t>(tags.size()));

    auto blobItem = GetBlobItem(blobName, Blobs::Models::ListBlobsIncludeFlags::Tags);
    EXPECT_EQ(blobItem.Details.Tags, tags);

    std::vector<std::string> blobNames;
    blobNames.push_back(blobName);
    for (int i = 0; i < 5; ++i)
    {
      const auto blobName1 = blobName + std::to_string(i);
      blobNames.push_back(blobName1);
      auto blobClient1 = containerClient.GetAppendBlobClient(blobName1);
      blobClient1.Create();
      blobClient1.SetTags(tags);
    }

    auto blobServiceClient = Azure::Storage::Blobs::BlobServiceClient::CreateFromConnectionString(
        StandardStorageConnectionString(),
        InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>());
    std::string whereExpression
        = c1 + " = '" + v1 + "' AND " + c2 + " >= '" + v2 + "' AND " + c3 + " <= '" + v3 + "'";
    std::vector<std::string> findResults;
    std::vector<std::string> findResults2;
    for (int i = 0; i < 30; ++i)
    {
      findResults.clear();
      findResults2.clear();
      Blobs::FindBlobsByTagsOptions findOptions;
      findOptions.PageSizeHint = 2;

      for (auto pageResult = containerClient.FindBlobsByTags(whereExpression); pageResult.HasPage();
           pageResult.MoveToNextPage())
      {
        EXPECT_FALSE(pageResult.ServiceEndpoint.empty());
        for (auto& item : pageResult.TaggedBlobs)
        {
          EXPECT_FALSE(item.BlobName.empty());
          EXPECT_EQ(item.BlobContainerName, m_containerName);
          EXPECT_FALSE(item.Tags.empty());
          findResults2.emplace_back(item.BlobName);
        }
      }
      for (auto pageResult = blobServiceClient.FindBlobsByTags(whereExpression);
           pageResult.HasPage();
           pageResult.MoveToNextPage())
      {
        EXPECT_FALSE(pageResult.ServiceEndpoint.empty());
        for (auto& item : pageResult.TaggedBlobs)
        {
          EXPECT_FALSE(item.BlobName.empty());
          EXPECT_EQ(item.BlobContainerName, m_containerName);
          EXPECT_FALSE(item.Tags.empty());
          findResults.emplace_back(item.BlobName);
        }
      }

      if (findResults.size() != blobNames.size() || findResults2.size() != blobNames.size())
      {
        TestSleep(1s);
      }
      else
      {
        break;
      }
    }
    EXPECT_EQ(findResults.size(), blobNames.size());
    EXPECT_EQ(findResults2.size(), blobNames.size());
    std::sort(blobNames.begin(), blobNames.end());
    std::sort(findResults.begin(), findResults.end());
    std::sort(findResults2.begin(), findResults2.end());
    EXPECT_EQ(blobNames, findResults);
    EXPECT_EQ(blobNames, findResults2);
  }

  TEST_F(BlobContainerClientTest, AccessConditionTags)
  {
    auto client = GetBlobContainerTestClient();
    client.Create();

    std::map<std::string, std::string> tags;
    std::string c1 = "k" + m_containerName + "1";
    std::string v1 = m_containerName + "2";
    tags[c1] = v1;

    std::string successWhereExpression = c1 + " = '" + v1 + "'";
    std::string failWhereExpression = c1 + " != '" + v1 + "'";

    std::vector<uint8_t> contentData(512);
    int64_t contentSize = static_cast<int64_t>(contentData.size());
    auto content = Azure::Core::IO::MemoryBodyStream(contentData.data(), contentData.size());

    std::string blobName = m_containerName;
    auto appendBlobClient = Azure::Storage::Blobs::AppendBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(),
        m_containerName,
        blobName,
        InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>());
    appendBlobClient.Create();
    appendBlobClient.SetTags(tags);

    {
      Blobs::GetBlobPropertiesOptions options;
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(appendBlobClient.GetProperties(options));
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(appendBlobClient.GetProperties(options), StorageException);
    }

    {
      Blobs::SetBlobHttpHeadersOptions options;
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(appendBlobClient.SetHttpHeaders(Blobs::Models::BlobHttpHeaders(), options));
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(
          appendBlobClient.SetHttpHeaders(Blobs::Models::BlobHttpHeaders(), options),
          StorageException);
    }

    {
      Blobs::SetBlobMetadataOptions options;
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(appendBlobClient.SetMetadata({}, options));
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(appendBlobClient.SetMetadata({}, options), StorageException);
    }

    {
      Blobs::DownloadBlobOptions options;
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(appendBlobClient.Download(options));
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(appendBlobClient.Download(options), StorageException);
    }

    {
      Blobs::CreateBlobSnapshotOptions options;
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(appendBlobClient.CreateSnapshot(options));
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(appendBlobClient.CreateSnapshot(options), StorageException);
    }

    {
      Blobs::CreateAppendBlobOptions options;
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(appendBlobClient.Create(options), StorageException);
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(appendBlobClient.Create(options));
      appendBlobClient.SetTags(tags);
    }

    {
      Blobs::AppendBlockOptions options;
      options.AccessConditions.TagConditions = failWhereExpression;
      content.Rewind();
      EXPECT_THROW(appendBlobClient.AppendBlock(content, options), StorageException);
      options.AccessConditions.TagConditions = successWhereExpression;
      content.Rewind();
      EXPECT_NO_THROW(appendBlobClient.AppendBlock(content, options));

      std::string url = appendBlobClient.GetUrl() + GetSas();
      Blobs::AppendBlockFromUriOptions options2;
      options2.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(appendBlobClient.AppendBlockFromUri(url, options2), StorageException);
      options2.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(appendBlobClient.AppendBlockFromUri(url, options2));
    }

    {
      std::string url = appendBlobClient.GetUrl() + GetSas();

      Blobs::StartBlobCopyFromUriOptions options;
      auto blobClient2 = Azure::Storage::Blobs::AppendBlobClient::CreateFromConnectionString(
          StandardStorageConnectionString(),
          m_containerName,
          m_containerName + "blobClient2",
          InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>());
      options.SourceAccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(blobClient2.StartCopyFromUri(url, options), StorageException);
      options.SourceAccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(blobClient2.StartCopyFromUri(url, options));

      options.SourceAccessConditions.TagConditions.Reset();
      blobClient2.SetTags(tags);

      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(blobClient2.StartCopyFromUri(url, options), StorageException);
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(blobClient2.StartCopyFromUri(url, options));
    }

    {
      std::string leaseId = Blobs::BlobLeaseClient::CreateUniqueLeaseId();
      Blobs::AcquireLeaseOptions options;
      options.AccessConditions.TagConditions = failWhereExpression;
      Blobs::BlobLeaseClient leaseClient(appendBlobClient, leaseId);
      EXPECT_THROW(leaseClient.Acquire(std::chrono::seconds(60), options), StorageException);
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(leaseClient.Acquire(std::chrono::seconds(60), options));

      Blobs::BreakLeaseOptions options2;
      options2.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(leaseClient.Break(options2), StorageException);
      options2.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(leaseClient.Break(options2));

      Blobs::DeleteBlobOptions options3;
      options3.DeleteSnapshots = Blobs::Models::DeleteSnapshotsOption::IncludeSnapshots;
      options3.AccessConditions.LeaseId = leaseId;
      options3.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(appendBlobClient.Delete(options3));
      options3.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(appendBlobClient.Delete(options3), StorageException);
    }

    blobName = m_containerName;
    auto pageBlobClient = Azure::Storage::Blobs::PageBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(),
        m_containerName,
        blobName + "blob",
        InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>());
    pageBlobClient.Create(contentSize);
    pageBlobClient.SetTags(tags);

    {
      Blobs::CreatePageBlobOptions options;
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(pageBlobClient.Create(contentSize, options), StorageException);
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(pageBlobClient.Create(contentSize, options));

      pageBlobClient.SetTags(tags);
    }

    {
      Blobs::UploadPagesOptions options;
      options.AccessConditions.TagConditions = failWhereExpression;
      content.Rewind();
      EXPECT_THROW(pageBlobClient.UploadPages(0, content, options), StorageException);
      options.AccessConditions.TagConditions = successWhereExpression;
      content.Rewind();
      EXPECT_NO_THROW(pageBlobClient.UploadPages(0, content, options));
    }

    {
      std::string url = pageBlobClient.GetUrl() + GetSas();
      Blobs::UploadPagesFromUriOptions options;
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(
          pageBlobClient.UploadPagesFromUri(0, url, {0, contentSize}, options), StorageException);
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(pageBlobClient.UploadPagesFromUri(0, url, {0, contentSize}, options));
    }

    {
      Blobs::ClearPagesOptions options;
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(pageBlobClient.ClearPages({0, contentSize}, options), StorageException);
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(pageBlobClient.ClearPages({0, contentSize}, options));
    }

    {
      Blobs::ResizePageBlobOptions options;
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(pageBlobClient.Resize(contentSize, options), StorageException);
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(pageBlobClient.Resize(contentSize, options));
    }

    {
      Blobs::UpdatePageBlobSequenceNumberOptions options;
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(
          pageBlobClient.UpdateSequenceNumber(
              Blobs::Models::SequenceNumberAction::Increment, options),
          StorageException);
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(pageBlobClient.UpdateSequenceNumber(
          Blobs::Models::SequenceNumberAction::Increment, options));
    }

    {
      Blobs::GetPageRangesOptions options;
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(pageBlobClient.GetPageRanges(options), StorageException);
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(pageBlobClient.GetPageRanges(options));
    }

    blobName = m_containerName;
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(),
        m_containerName,
        blobName + "blockBlobClient",
        InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>());
    blockBlobClient.UploadFrom(contentData.data(), contentData.size());
    blockBlobClient.SetTags(tags);

    {
      Blobs::SetBlobAccessTierOptions options;
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(blockBlobClient.SetAccessTier(Blobs::Models::AccessTier::Hot, options));
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(
          blockBlobClient.SetAccessTier(Blobs::Models::AccessTier::Hot, options), StorageException);
    }

    {
      Blobs::UploadBlockBlobOptions options;
      options.AccessConditions.TagConditions = failWhereExpression;
      content.Rewind();
      EXPECT_THROW(blockBlobClient.Upload(content, options), StorageException);
      options.AccessConditions.TagConditions = successWhereExpression;
      content.Rewind();
      EXPECT_NO_THROW(blockBlobClient.Upload(content, options));
      blockBlobClient.SetTags(tags);
    }

    {
      std::string blockId = Base64EncodeText("1");
      std::vector<std::string> blockIds = {blockId};
      content.Rewind();
      blockBlobClient.StageBlock(blockId, content);

      Blobs::CommitBlockListOptions options;
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(blockBlobClient.CommitBlockList(blockIds, options), StorageException);
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(blockBlobClient.CommitBlockList(blockIds, options));
      blockBlobClient.SetTags(tags);
    }

    {
      Blobs::GetBlockListOptions options;
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(blockBlobClient.GetBlockList(options), StorageException);
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(blockBlobClient.GetBlockList(options));
    }

    {
      auto sourceBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
          StandardStorageConnectionString(),
          m_containerName,
          m_containerName + "sourceBlobClient",
          InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>());
      std::vector<uint8_t> buffer;
      buffer.resize(1024);
      sourceBlobClient.UploadFrom(buffer.data(), buffer.size());

      Blobs::CopyBlobFromUriOptions options;
      options.AccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(
          blockBlobClient.CopyFromUri(sourceBlobClient.GetUrl() + GetSas(), options),
          StorageException);
      options.AccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(blockBlobClient.CopyFromUri(sourceBlobClient.GetUrl() + GetSas(), options));
    }

    {
      auto sourceBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
          StandardStorageConnectionString(),
          m_containerName,
          m_containerName + "sourceBlobClient2",
          InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>());
      std::vector<uint8_t> buffer;
      buffer.resize(1024);
      sourceBlobClient.UploadFrom(buffer.data(), buffer.size());
      sourceBlobClient.SetTags(tags);

      Blobs::StartBlobCopyFromUriOptions options;
      options.SourceAccessConditions.TagConditions = failWhereExpression;
      EXPECT_THROW(
          blockBlobClient.StartCopyFromUri(sourceBlobClient.GetUrl(), options), StorageException);
      options.SourceAccessConditions.TagConditions = successWhereExpression;
      EXPECT_NO_THROW(blockBlobClient.StartCopyFromUri(sourceBlobClient.GetUrl(), options));
    }
  }

  TEST_F(BlobContainerClientTest, SpecialBlobName)
  {
    auto client = GetBlobContainerTestClient();
    client.CreateIfNotExists();

    const std::string non_ascii_word = "\xE6\xB5\x8B\xE8\xAF\x95";
    const std::string encoded_non_ascii_word = "%E6%B5%8B%E8%AF%95";
    ASSERT_EQ(_internal::UrlEncodePath(non_ascii_word), encoded_non_ascii_word);
    // blobName cannot contain backslash '\'
    const std::string baseBlobName = "a b c / !@#$%^&*(?/<>,.;:'\"[]{}|`~) def" + non_ascii_word;

    {
      const std::string blobName = baseBlobName + m_containerName;
      auto blobClient = client.GetAppendBlobClient(blobName);
      EXPECT_NO_THROW(blobClient.Create());
      auto blobUrl = blobClient.GetUrl();
      EXPECT_EQ(blobUrl, client.GetUrl() + "/" + _internal::UrlEncodePath(blobName));
      auto blobItem = GetBlobItem(blobName);
      EXPECT_EQ(blobItem.Name, blobName);
    }
    {
      const std::string blobName = baseBlobName + m_containerName + "1";
      auto blobClient = client.GetPageBlobClient(blobName);
      EXPECT_NO_THROW(blobClient.Create(1024));
      auto blobUrl = blobClient.GetUrl();
      EXPECT_EQ(blobUrl, client.GetUrl() + "/" + _internal::UrlEncodePath(blobName));
      auto blobItem = GetBlobItem(blobName);
      EXPECT_EQ(blobItem.Name, blobName);
    }
    {
      const std::string blobName = baseBlobName + m_containerName + "2";
      auto blobClient = client.GetBlockBlobClient(blobName);
      EXPECT_NO_THROW(blobClient.UploadFrom(nullptr, 0));
      auto blobUrl = blobClient.GetUrl();
      EXPECT_EQ(blobUrl, client.GetUrl() + "/" + _internal::UrlEncodePath(blobName));
      auto blobItem = GetBlobItem(blobName);
      EXPECT_EQ(blobItem.Name, blobName);
    }
    auto clientOptions = InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>();
    {
      const std::string blobName = baseBlobName + m_containerName + "3";
      auto blobClient = Blobs::AppendBlobClient::CreateFromConnectionString(
          StandardStorageConnectionString(), m_containerName, blobName, clientOptions);
      EXPECT_NO_THROW(blobClient.Create());
      auto blobUrl = blobClient.GetUrl();
      EXPECT_EQ(blobUrl, client.GetUrl() + "/" + _internal::UrlEncodePath(blobName));
      auto blobItem = GetBlobItem(blobName);
      EXPECT_EQ(blobItem.Name, blobName);
    }
    {
      const std::string blobName = baseBlobName + m_containerName + "4";
      auto blobClient = Blobs::PageBlobClient::CreateFromConnectionString(
          StandardStorageConnectionString(), m_containerName, blobName, clientOptions);
      EXPECT_NO_THROW(blobClient.Create(1024));
      auto blobUrl = blobClient.GetUrl();
      EXPECT_EQ(blobUrl, client.GetUrl() + "/" + _internal::UrlEncodePath(blobName));
      auto blobItem = GetBlobItem(blobName);
      EXPECT_EQ(blobItem.Name, blobName);
    }
    {
      const std::string blobName = baseBlobName + m_containerName + "5";
      auto blobClient = Blobs::BlockBlobClient::CreateFromConnectionString(
          StandardStorageConnectionString(), m_containerName, blobName, clientOptions);
      EXPECT_NO_THROW(blobClient.UploadFrom(nullptr, 0));
      auto blobUrl = blobClient.GetUrl();
      EXPECT_EQ(blobUrl, client.GetUrl() + "/" + _internal::UrlEncodePath(blobName));
      auto blobItem = GetBlobItem(blobName);
      EXPECT_EQ(blobItem.Name, blobName);
    }
    {
      /*
       * UTF-16 0xFFFF and 0xFFFE are not valid in XML, we'll need to encode if blob name contains
       * these two characters.
       */
      const std::string blobPrefix
          = std::string("aaaaa\xEF\xBF\xBF") + "bbb/"; // UTF-8 0xEF, 0xBF, 0xBF is UTF-16 0xFFFF
      const std::string blobName = blobPrefix + "ccc";
      auto blobClient = Blobs::BlockBlobClient::CreateFromConnectionString(
          StandardStorageConnectionString(), m_containerName, blobName, clientOptions);
      EXPECT_NO_THROW(blobClient.UploadFrom(nullptr, 0));
      auto blobUrl = blobClient.GetUrl();
      EXPECT_EQ(blobUrl, client.GetUrl() + "/" + _internal::UrlEncodePath(blobName));
      Blobs::Models::BlobItem blobItem;
      Blobs::ListBlobsOptions options;
      options.Prefix = "aaaaa";
      for (auto page = client.ListBlobs(options); page.HasPage(); page.MoveToNextPage())
      {
        for (auto& blob : page.Blobs)
        {
          if (blob.Name == blobName)
          {
            blobItem = std::move(blob);
          }
        }
      }
      EXPECT_EQ(blobItem.Name, blobName);
      bool found = false;
      for (auto page = client.ListBlobsByHierarchy("/"); page.HasPage(); page.MoveToNextPage())
      {
        for (auto& p : page.BlobPrefixes)
        {
          if (p == blobPrefix)
          {
            found = true;
          }
        }
      }
      EXPECT_TRUE(found);
    }
  }

  TEST_F(BlobContainerClientTest, QuestionMarkBlobName)
  {
    auto client = GetBlobContainerTestClient();
    client.Create();

    std::string blobName = "?";
    auto blobClient = client.GetAppendBlobClient(blobName);
    EXPECT_NO_THROW(blobClient.Create());
    auto blobUrl = blobClient.GetUrl();
    EXPECT_EQ(blobUrl, client.GetUrl() + "/" + _internal::UrlEncodePath(blobName));
  }

  TEST_F(BlobContainerClientTest, DeleteBlob)
  {
    auto client = GetBlobContainerTestClient();
    client.Create();

    std::string blobName = m_containerName;
    auto blobClient = client.GetAppendBlobClient(blobName);
    blobClient.Create();
    EXPECT_NO_THROW(blobClient.GetProperties());
    blobClient.Delete();
    EXPECT_THROW(blobClient.GetProperties(), StorageException);
  }

  TEST_F(BlobContainerClientTest, ListBlobsDeletedWithActiveVersions)
  {
    auto client = GetBlobContainerTestClient();
    client.Create();

    std::string blobName = "blob" + m_containerName;
    auto blobClient = client.GetAppendBlobClient(blobName);
    blobClient.Create();

    auto blobItem
        = GetBlobItem(blobName, Blobs::Models::ListBlobsIncludeFlags::DeletedWithVersions);
    ASSERT_TRUE(blobItem.HasVersionsOnly.HasValue());
    EXPECT_FALSE(blobItem.HasVersionsOnly.Value());

    blobClient.Delete();

    blobItem = GetBlobItem(blobName, Blobs::Models::ListBlobsIncludeFlags::DeletedWithVersions);
    ASSERT_TRUE(blobItem.HasVersionsOnly.HasValue());
    EXPECT_TRUE(blobItem.HasVersionsOnly.Value());
  }
}}} // namespace Azure::Storage::Test
