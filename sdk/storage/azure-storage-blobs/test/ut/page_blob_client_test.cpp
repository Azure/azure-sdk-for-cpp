// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "page_blob_client_test.hpp"

#include <future>
#include <vector>

#include <azure/core/cryptography/hash.hpp>
#include <azure/storage/blobs/blob_lease_client.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/file_io.hpp>

namespace Azure { namespace Storage { namespace Test {

  std::shared_ptr<Azure::Storage::Blobs::PageBlobClient> PageBlobClientTest::m_pageBlobClient;
  std::string PageBlobClientTest::m_blobName;
  Azure::Storage::Blobs::CreatePageBlobOptions PageBlobClientTest::m_blobUploadOptions;
  std::vector<uint8_t> PageBlobClientTest::m_blobContent;

  void PageBlobClientTest::SetUpTestSuite()
  {
    BlobContainerClientTest::SetUpTestSuite();

    m_blobName = RandomString();
    auto pageBlobClient = Azure::Storage::Blobs::PageBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, m_blobName);
    m_pageBlobClient
        = std::make_shared<Azure::Storage::Blobs::PageBlobClient>(std::move(pageBlobClient));
    m_blobContent.resize(static_cast<std::size_t>(1_KB));
    RandomBuffer(reinterpret_cast<char*>(&m_blobContent[0]), m_blobContent.size());
    m_blobUploadOptions.Metadata = {{"key1", "V1"}, {"key2", "Value2"}};
    m_blobUploadOptions.HttpHeaders.ContentType = "application/x-binary";
    m_blobUploadOptions.HttpHeaders.ContentLanguage = "en-US";
    m_blobUploadOptions.HttpHeaders.ContentDisposition = "attachment";
    m_blobUploadOptions.HttpHeaders.CacheControl = "no-cache";
    m_blobUploadOptions.HttpHeaders.ContentEncoding = "identity";
    m_blobUploadOptions.HttpHeaders.ContentHash.Value.clear();
    m_pageBlobClient->Create(m_blobContent.size(), m_blobUploadOptions);
    auto pageContent
        = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    m_pageBlobClient->UploadPages(0, pageContent);
    m_blobUploadOptions.HttpHeaders.ContentHash
        = m_pageBlobClient->GetProperties().Value.HttpHeaders.ContentHash;
  }

  void PageBlobClientTest::TearDownTestSuite() { BlobContainerClientTest::TearDownTestSuite(); }

  TEST_F(PageBlobClientTest, CreateDelete)
  {
    auto pageBlobClient = Azure::Storage::Blobs::PageBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    auto blobContentInfo = pageBlobClient.Create(0, m_blobUploadOptions);
    EXPECT_TRUE(blobContentInfo.Value.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(blobContentInfo.Value.LastModified));
    EXPECT_TRUE(blobContentInfo.Value.VersionId.HasValue());
    EXPECT_FALSE(blobContentInfo.Value.VersionId.Value().empty());
    EXPECT_FALSE(blobContentInfo.Value.EncryptionScope.HasValue());
    EXPECT_FALSE(blobContentInfo.Value.EncryptionKeySha256.HasValue());

    pageBlobClient.Delete();
    EXPECT_THROW(pageBlobClient.Delete(), StorageException);
  }

  TEST_F(PageBlobClientTest, Resize)
  {
    auto pageBlobClient = Azure::Storage::Blobs::PageBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    pageBlobClient.Create(0, m_blobUploadOptions);

    EXPECT_EQ(pageBlobClient.GetProperties().Value.BlobSize, 0);
    pageBlobClient.Resize(static_cast<int64_t>(2_KB));
    EXPECT_EQ(static_cast<uint64_t>(pageBlobClient.GetProperties().Value.BlobSize), 2_KB);
    pageBlobClient.Resize(static_cast<int64_t>(1_KB));
    EXPECT_EQ(static_cast<uint64_t>(pageBlobClient.GetProperties().Value.BlobSize), 1_KB);
  }

  TEST_F(PageBlobClientTest, UploadClear)
  {
    std::vector<uint8_t> blobContent;
    blobContent.resize(static_cast<std::size_t>(4_KB));
    RandomBuffer(reinterpret_cast<char*>(&blobContent[0]), blobContent.size());

    auto pageBlobClient = Azure::Storage::Blobs::PageBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    pageBlobClient.Create(8_KB, m_blobUploadOptions);
    auto pageContent = Azure::Core::IO::MemoryBodyStream(blobContent.data(), blobContent.size());
    pageBlobClient.UploadPages(2_KB, pageContent);
    // |_|_|x|x|  |x|x|_|_|
    blobContent.insert(blobContent.begin(), static_cast<std::size_t>(2_KB), '\x00');
    blobContent.resize(static_cast<std::size_t>(8_KB), '\x00');
    pageBlobClient.ClearPages({2_KB, 1_KB});
    // |_|_|_|x|  |x|x|_|_|
    std::fill(
        blobContent.begin() + static_cast<std::size_t>(2_KB),
        blobContent.begin() + static_cast<std::size_t>(2_KB + 1_KB),
        '\x00');

    auto downloadContent = pageBlobClient.Download();
    EXPECT_EQ(ReadBodyStream(downloadContent.Value.BodyStream), blobContent);

    std::vector<Core::Http::HttpRange> pageRanges;
    for (auto pageResult = pageBlobClient.GetPageRanges(); pageResult.HasMore();
         pageResult.NextPage(Azure::Core::Context()))
    {
      pageRanges.insert(
          pageRanges.end(), pageResult.PageRanges.begin(), pageResult.PageRanges.end());
    }
    ASSERT_FALSE(pageRanges.empty());
    EXPECT_EQ(static_cast<uint64_t>(pageRanges[0].Offset), 3_KB);
    EXPECT_EQ(static_cast<uint64_t>(pageRanges[0].Length.Value()), 3_KB);

    Azure::Storage::Blobs::GetPageRangesOptions options;
    options.Range = Core::Http::HttpRange();
    options.Range.Value().Offset = 4_KB;
    options.Range.Value().Length = 1_KB;
    pageRanges.clear();
    for (auto pageResult = pageBlobClient.GetPageRanges(options); pageResult.HasMore();
         pageResult.NextPage(Azure::Core::Context()))
    {
      pageRanges.insert(
          pageRanges.end(), pageResult.PageRanges.begin(), pageResult.PageRanges.end());
    }
    ASSERT_FALSE(pageRanges.empty());
    EXPECT_EQ(static_cast<uint64_t>(pageRanges[0].Offset), 4_KB);
    EXPECT_EQ(static_cast<uint64_t>(pageRanges[0].Length.Value()), 1_KB);

    auto snapshot = pageBlobClient.CreateSnapshot().Value.Snapshot;
    // |_|_|_|x|  |x|x|_|_| This is what's in snapshot
    blobContent.resize(static_cast<std::size_t>(1_KB));
    auto pageClient = Azure::Core::IO::MemoryBodyStream(blobContent.data(), blobContent.size());
    pageBlobClient.UploadPages(0, pageClient);
    pageBlobClient.ClearPages({3_KB, 1_KB});
    // |x|_|_|_|  |x|x|_|_|

    pageRanges.clear();
    std::vector<Core::Http::HttpRange> clearRanges;
    for (auto pageResult = pageBlobClient.GetPageRangesDiff(snapshot); pageResult.HasMore();
         pageResult.NextPage(Azure::Core::Context()))
    {
      pageRanges.insert(
          pageRanges.end(), pageResult.PageRanges.begin(), pageResult.PageRanges.end());
      clearRanges.insert(
          clearRanges.end(), pageResult.ClearRanges.begin(), pageResult.ClearRanges.end());
    }
    ASSERT_FALSE(pageRanges.empty());
    ASSERT_FALSE(pageRanges.empty());
    EXPECT_EQ(pageRanges[0].Offset, 0);
    EXPECT_EQ(static_cast<uint64_t>(pageRanges[0].Length.Value()), 1_KB);
    EXPECT_EQ(static_cast<uint64_t>(clearRanges[0].Offset), 3_KB);
    EXPECT_EQ(static_cast<uint64_t>(clearRanges[0].Length.Value()), 1_KB);
  }

  TEST_F(PageBlobClientTest, UploadFromUri)
  {
    auto pageBlobClient = Azure::Storage::Blobs::PageBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    pageBlobClient.Create(m_blobContent.size(), m_blobUploadOptions);
    pageBlobClient.UploadPagesFromUri(
        0, m_pageBlobClient->GetUrl() + GetSas(), {0, static_cast<int64_t>(m_blobContent.size())});
  }

  TEST_F(PageBlobClientTest, StartCopyIncremental)
  {
    auto pageBlobClient = Azure::Storage::Blobs::PageBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    std::string snapshot = m_pageBlobClient->CreateSnapshot().Value.Snapshot;
    Azure::Core::Url sourceUri(m_pageBlobClient->WithSnapshot(snapshot).GetUrl());
    auto copyInfo = pageBlobClient.StartCopyIncremental(AppendQueryParameters(sourceUri, GetSas()));
    EXPECT_EQ(
        copyInfo.GetRawResponse().GetStatusCode(), Azure::Core::Http::HttpStatusCode::Accepted);
    auto getPropertiesResult = copyInfo.PollUntilDone(std::chrono::seconds(1));
    ASSERT_TRUE(getPropertiesResult.Value.CopyStatus.HasValue());
    EXPECT_EQ(getPropertiesResult.Value.CopyStatus.Value(), Blobs::Models::CopyStatus::Success);
    ASSERT_TRUE(getPropertiesResult.Value.CopyId.HasValue());
    EXPECT_FALSE(getPropertiesResult.Value.CopyId.Value().empty());
    ASSERT_TRUE(getPropertiesResult.Value.CopySource.HasValue());
    EXPECT_FALSE(getPropertiesResult.Value.CopySource.Value().empty());
    ASSERT_TRUE(getPropertiesResult.Value.IsIncrementalCopy.HasValue());
    EXPECT_TRUE(getPropertiesResult.Value.IsIncrementalCopy.Value());
    ASSERT_TRUE(getPropertiesResult.Value.IncrementalCopyDestinationSnapshot.HasValue());
    EXPECT_FALSE(getPropertiesResult.Value.IncrementalCopyDestinationSnapshot.Value().empty());
    ASSERT_TRUE(getPropertiesResult.Value.CopyCompletedOn.HasValue());
    EXPECT_TRUE(IsValidTime(getPropertiesResult.Value.CopyCompletedOn.Value()));
    ASSERT_TRUE(getPropertiesResult.Value.CopyProgress.HasValue());
    EXPECT_FALSE(getPropertiesResult.Value.CopyProgress.Value().empty());
  }

  TEST_F(PageBlobClientTest, Lease)
  {
    std::string leaseId1 = Blobs::BlobLeaseClient::CreateUniqueLeaseId();
    auto leaseDuration = std::chrono::seconds(20);
    auto leaseClient = Blobs::BlobLeaseClient(*m_pageBlobClient, leaseId1);
    auto aLease = leaseClient.Acquire(leaseDuration).Value;
    EXPECT_TRUE(aLease.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(aLease.LastModified));
    EXPECT_EQ(aLease.LeaseId, leaseId1);
    EXPECT_EQ(leaseClient.GetLeaseId(), leaseId1);
    aLease = leaseClient.Acquire(leaseDuration).Value;
    EXPECT_TRUE(aLease.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(aLease.LastModified));
    EXPECT_EQ(aLease.LeaseId, leaseId1);

    auto properties = m_pageBlobClient->GetProperties().Value;
    EXPECT_EQ(properties.LeaseState.Value(), Blobs::Models::LeaseState::Leased);
    EXPECT_EQ(properties.LeaseStatus.Value(), Blobs::Models::LeaseStatus::Locked);
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
    leaseClient = Blobs::BlobLeaseClient(*m_pageBlobClient, cLease.LeaseId);
    EXPECT_EQ(leaseClient.GetLeaseId(), leaseId2);

    auto blobInfo = leaseClient.Release().Value;
    EXPECT_TRUE(blobInfo.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(blobInfo.LastModified));

    leaseClient
        = Blobs::BlobLeaseClient(*m_pageBlobClient, Blobs::BlobLeaseClient::CreateUniqueLeaseId());
    aLease = leaseClient.Acquire(Blobs::BlobLeaseClient::InfiniteLeaseDuration).Value;
    properties = m_pageBlobClient->GetProperties().Value;
    EXPECT_EQ(properties.LeaseDuration.Value(), Blobs::Models::LeaseDurationType::Infinite);
    auto brokenLease = leaseClient.Break().Value;
    EXPECT_TRUE(brokenLease.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(brokenLease.LastModified));

    leaseClient
        = Blobs::BlobLeaseClient(*m_pageBlobClient, Blobs::BlobLeaseClient::CreateUniqueLeaseId());
    aLease = leaseClient.Acquire(leaseDuration).Value;
    brokenLease = leaseClient.Break().Value;
    EXPECT_TRUE(brokenLease.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(brokenLease.LastModified));

    Blobs::BreakLeaseOptions options;
    options.BreakPeriod = std::chrono::seconds(0);
    leaseClient.Break(options);
  }

  TEST_F(PageBlobClientTest, ContentMd5)
  {
    std::vector<uint8_t> blobContent;
    blobContent.resize(static_cast<std::size_t>(4_KB));
    RandomBuffer(reinterpret_cast<char*>(&blobContent[0]), blobContent.size());

    auto pageBlobClient = Azure::Storage::Blobs::PageBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    pageBlobClient.Create(blobContent.size(), m_blobUploadOptions);
    auto pageContent = Azure::Core::IO::MemoryBodyStream(blobContent.data(), blobContent.size());

    Blobs::UploadPagesOptions options;
    ContentHash hash;
    hash.Algorithm = HashAlgorithm::Md5;

    {
      Azure::Core::Cryptography::Md5Hash instance;
      hash.Value = instance.Final(blobContent.data(), blobContent.size());
    }
    options.TransactionalContentHash = hash;
    EXPECT_NO_THROW(pageBlobClient.UploadPages(0, pageContent, options));

    pageContent.Rewind();
    hash.Value = Azure::Core::Convert::Base64Decode(DummyMd5);
    options.TransactionalContentHash = hash;
    EXPECT_THROW(pageBlobClient.UploadPages(0, pageContent, options), StorageException);
  }

  TEST_F(PageBlobClientTest, ContentCrc64)
  {
    std::vector<uint8_t> blobContent;
    blobContent.resize(static_cast<std::size_t>(4_KB));
    RandomBuffer(reinterpret_cast<char*>(&blobContent[0]), blobContent.size());

    auto pageBlobClient = Azure::Storage::Blobs::PageBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    pageBlobClient.Create(blobContent.size(), m_blobUploadOptions);
    auto pageContent = Azure::Core::IO::MemoryBodyStream(blobContent.data(), blobContent.size());

    Blobs::UploadPagesOptions options;
    ContentHash hash;
    hash.Algorithm = HashAlgorithm::Crc64;

    {
      Crc64Hash instance;
      hash.Value = instance.Final(blobContent.data(), blobContent.size());
    }
    options.TransactionalContentHash = hash;
    EXPECT_NO_THROW(pageBlobClient.UploadPages(0, pageContent, options));

    pageContent.Rewind();
    hash.Value = Azure::Core::Convert::Base64Decode(DummyCrc64);
    options.TransactionalContentHash = hash;
    EXPECT_THROW(pageBlobClient.UploadPages(0, pageContent, options), StorageException);
  }

  TEST_F(PageBlobClientTest, CreateIfNotExists)
  {
    auto blobClient = Azure::Storage::Blobs::PageBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    auto blobClientWithoutAuth = Azure::Storage::Blobs::PageBlobClient(blobClient.GetUrl());
    EXPECT_THROW(blobClientWithoutAuth.CreateIfNotExists(m_blobContent.size()), StorageException);
    {
      auto response = blobClient.CreateIfNotExists(m_blobContent.size());
      EXPECT_TRUE(response.Value.Created);
    }

    auto blobContent
        = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    blobClient.UploadPages(0, blobContent);
    {
      auto response = blobClient.CreateIfNotExists(m_blobContent.size());
      EXPECT_FALSE(response.Value.Created);
    }
    auto downloadStream = std::move(blobClient.Download().Value.BodyStream);
    EXPECT_EQ(downloadStream->ReadToEnd(Azure::Core::Context()), m_blobContent);
  }

}}} // namespace Azure::Storage::Test
