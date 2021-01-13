// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "page_blob_client_test.hpp"

#include <future>
#include <vector>

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
        = Azure::Core::Http::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    m_pageBlobClient->UploadPages(0, &pageContent);
    m_blobUploadOptions.HttpHeaders.ContentHash
        = m_pageBlobClient->GetProperties()->HttpHeaders.ContentHash;
  }

  void PageBlobClientTest::TearDownTestSuite() { BlobContainerClientTest::TearDownTestSuite(); }

  TEST_F(PageBlobClientTest, CreateDelete)
  {
    auto pageBlobClient = Azure::Storage::Blobs::PageBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    auto blobContentInfo = pageBlobClient.Create(0, m_blobUploadOptions);
    EXPECT_FALSE(blobContentInfo->ETag.empty());
    EXPECT_TRUE(IsValidTime(blobContentInfo->LastModified));
    EXPECT_TRUE(blobContentInfo->VersionId.HasValue());
    EXPECT_FALSE(blobContentInfo->VersionId.GetValue().empty());
    EXPECT_FALSE(blobContentInfo->EncryptionScope.HasValue());
    EXPECT_FALSE(blobContentInfo->EncryptionKeySha256.HasValue());

    pageBlobClient.Delete();
    EXPECT_THROW(pageBlobClient.Delete(), StorageException);
  }

  TEST_F(PageBlobClientTest, Resize)
  {
    auto pageBlobClient = Azure::Storage::Blobs::PageBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    pageBlobClient.Create(0, m_blobUploadOptions);

    EXPECT_EQ(pageBlobClient.GetProperties()->ContentLength, 0);
    pageBlobClient.Resize(static_cast<int64_t>(2_KB));
    EXPECT_EQ(static_cast<uint64_t>(pageBlobClient.GetProperties()->ContentLength), 2_KB);
    pageBlobClient.Resize(static_cast<int64_t>(1_KB));
    EXPECT_EQ(static_cast<uint64_t>(pageBlobClient.GetProperties()->ContentLength), 1_KB);
  }

  TEST_F(PageBlobClientTest, UploadClear)
  {
    std::vector<uint8_t> blobContent;
    blobContent.resize(static_cast<std::size_t>(4_KB));
    RandomBuffer(reinterpret_cast<char*>(&blobContent[0]), blobContent.size());

    auto pageBlobClient = Azure::Storage::Blobs::PageBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    pageBlobClient.Create(8_KB, m_blobUploadOptions);
    auto pageContent = Azure::Core::Http::MemoryBodyStream(blobContent.data(), blobContent.size());
    pageBlobClient.UploadPages(2_KB, &pageContent);
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
    EXPECT_EQ(ReadBodyStream(downloadContent->BodyStream), blobContent);

    auto pageRanges = *pageBlobClient.GetPageRanges();
    EXPECT_TRUE(pageRanges.ClearRanges.empty());
    ASSERT_FALSE(pageRanges.PageRanges.empty());
    EXPECT_EQ(static_cast<uint64_t>(pageRanges.PageRanges[0].Offset), 3_KB);
    EXPECT_EQ(static_cast<uint64_t>(pageRanges.PageRanges[0].Length.GetValue()), 3_KB);

    Azure::Storage::Blobs::GetPageBlobPageRangesOptions options;
    options.Range = Core::Http::Range();
    options.Range.GetValue().Offset = 4_KB;
    options.Range.GetValue().Length = 1_KB;
    pageRanges = *pageBlobClient.GetPageRanges(options);
    EXPECT_TRUE(pageRanges.ClearRanges.empty());
    ASSERT_FALSE(pageRanges.PageRanges.empty());
    EXPECT_EQ(static_cast<uint64_t>(pageRanges.PageRanges[0].Offset), 4_KB);
    EXPECT_EQ(static_cast<uint64_t>(pageRanges.PageRanges[0].Length.GetValue()), 1_KB);

    auto snapshot = pageBlobClient.CreateSnapshot()->Snapshot;
    // |_|_|_|x|  |x|x|_|_| This is what's in snapshot
    blobContent.resize(static_cast<std::size_t>(1_KB));
    auto pageClient = Azure::Core::Http::MemoryBodyStream(blobContent.data(), blobContent.size());
    pageBlobClient.UploadPages(0, &pageClient);
    pageBlobClient.ClearPages({3_KB, 1_KB});
    // |x|_|_|_|  |x|x|_|_|

    pageRanges = *pageBlobClient.GetPageRangesDiff(snapshot);
    ASSERT_FALSE(pageRanges.ClearRanges.empty());
    ASSERT_FALSE(pageRanges.PageRanges.empty());
    EXPECT_EQ(pageRanges.PageRanges[0].Offset, 0);
    EXPECT_EQ(static_cast<uint64_t>(pageRanges.PageRanges[0].Length.GetValue()), 1_KB);
    EXPECT_EQ(static_cast<uint64_t>(pageRanges.ClearRanges[0].Offset), 3_KB);
    EXPECT_EQ(static_cast<uint64_t>(pageRanges.ClearRanges[0].Length.GetValue()), 1_KB);
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
    std::string snapshot = m_pageBlobClient->CreateSnapshot()->Snapshot;
    Azure::Core::Http::Url sourceUri(m_pageBlobClient->WithSnapshot(snapshot).GetUrl());
    sourceUri.AppendQueryParameters(GetSas());
    auto copyInfo = pageBlobClient.StartCopyIncremental(sourceUri.GetAbsoluteUrl());
    EXPECT_FALSE(copyInfo->ETag.empty());
    EXPECT_TRUE(IsValidTime(copyInfo->LastModified));
    EXPECT_FALSE(copyInfo->CopyId.empty());
    EXPECT_FALSE(copyInfo->CopyStatus.Get().empty());
    EXPECT_TRUE(copyInfo->VersionId.HasValue());
    EXPECT_FALSE(copyInfo->VersionId.GetValue().empty());
  }

  TEST_F(PageBlobClientTest, Lease)
  {
    std::string leaseId1 = CreateUniqueLeaseId();
    int32_t leaseDuration = 20;
    auto aLease = *m_pageBlobClient->AcquireLease(leaseId1, leaseDuration);
    EXPECT_FALSE(aLease.ETag.empty());
    EXPECT_TRUE(IsValidTime(aLease.LastModified));
    EXPECT_EQ(aLease.LeaseId, leaseId1);
    aLease = *m_pageBlobClient->AcquireLease(leaseId1, leaseDuration);
    EXPECT_FALSE(aLease.ETag.empty());
    EXPECT_TRUE(IsValidTime(aLease.LastModified));
    EXPECT_EQ(aLease.LeaseId, leaseId1);

    auto properties = *m_pageBlobClient->GetProperties();
    EXPECT_EQ(properties.LeaseState.GetValue(), Blobs::Models::BlobLeaseState::Leased);
    EXPECT_EQ(properties.LeaseStatus.GetValue(), Blobs::Models::BlobLeaseStatus::Locked);
    EXPECT_FALSE(properties.LeaseDuration.GetValue().empty());

    auto rLease = *m_pageBlobClient->RenewLease(leaseId1);
    EXPECT_FALSE(rLease.ETag.empty());
    EXPECT_TRUE(IsValidTime(rLease.LastModified));
    EXPECT_EQ(rLease.LeaseId, leaseId1);

    std::string leaseId2 = CreateUniqueLeaseId();
    EXPECT_NE(leaseId1, leaseId2);
    auto cLease = *m_pageBlobClient->ChangeLease(leaseId1, leaseId2);
    EXPECT_FALSE(cLease.ETag.empty());
    EXPECT_TRUE(IsValidTime(cLease.LastModified));
    EXPECT_EQ(cLease.LeaseId, leaseId2);

    auto blobInfo = *m_pageBlobClient->ReleaseLease(leaseId2);
    EXPECT_FALSE(blobInfo.ETag.empty());
    EXPECT_TRUE(IsValidTime(blobInfo.LastModified));

    aLease = *m_pageBlobClient->AcquireLease(CreateUniqueLeaseId(), InfiniteLeaseDuration);
    properties = *m_pageBlobClient->GetProperties();
    EXPECT_FALSE(properties.LeaseDuration.GetValue().empty());
    auto brokenLease = *m_pageBlobClient->BreakLease();
    EXPECT_FALSE(brokenLease.ETag.empty());
    EXPECT_TRUE(IsValidTime(brokenLease.LastModified));
    EXPECT_EQ(brokenLease.LeaseTime, 0);

    aLease = *m_pageBlobClient->AcquireLease(CreateUniqueLeaseId(), leaseDuration);
    brokenLease = *m_pageBlobClient->BreakLease();
    EXPECT_FALSE(brokenLease.ETag.empty());
    EXPECT_TRUE(IsValidTime(brokenLease.LastModified));
    EXPECT_NE(brokenLease.LeaseTime, 0);

    Blobs::BreakBlobLeaseOptions options;
    options.BreakPeriod = 0;
    m_pageBlobClient->BreakLease(options);
  }

  TEST_F(PageBlobClientTest, ContentMd5)
  {
    std::vector<uint8_t> blobContent;
    blobContent.resize(static_cast<std::size_t>(4_KB));
    RandomBuffer(reinterpret_cast<char*>(&blobContent[0]), blobContent.size());

    auto pageBlobClient = Azure::Storage::Blobs::PageBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    pageBlobClient.Create(blobContent.size(), m_blobUploadOptions);
    auto pageContent = Azure::Core::Http::MemoryBodyStream(blobContent.data(), blobContent.size());

    Blobs::UploadPageBlobPagesOptions options;
    ContentHash hash;
    hash.Algorithm = HashAlgorithm::Md5;
    hash.Value = Md5::Hash(blobContent.data(), blobContent.size());
    options.TransactionalContentHash = hash;
    EXPECT_NO_THROW(pageBlobClient.UploadPages(0, &pageContent, options));

    pageContent.Rewind();
    hash.Value = Azure::Core::Base64Decode(DummyMd5);
    options.TransactionalContentHash = hash;
    EXPECT_THROW(pageBlobClient.UploadPages(0, &pageContent, options), StorageException);
  }

  TEST_F(PageBlobClientTest, ContentCrc64)
  {
    std::vector<uint8_t> blobContent;
    blobContent.resize(static_cast<std::size_t>(4_KB));
    RandomBuffer(reinterpret_cast<char*>(&blobContent[0]), blobContent.size());

    auto pageBlobClient = Azure::Storage::Blobs::PageBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    pageBlobClient.Create(blobContent.size(), m_blobUploadOptions);
    auto pageContent = Azure::Core::Http::MemoryBodyStream(blobContent.data(), blobContent.size());

    Blobs::UploadPageBlobPagesOptions options;
    ContentHash hash;
    hash.Algorithm = HashAlgorithm::Crc64;
    hash.Value = Crc64::Hash(blobContent.data(), blobContent.size());
    options.TransactionalContentHash = hash;
    EXPECT_NO_THROW(pageBlobClient.UploadPages(0, &pageContent, options));

    pageContent.Rewind();
    hash.Value = Azure::Core::Base64Decode(DummyCrc64);
    options.TransactionalContentHash = hash;
    EXPECT_THROW(pageBlobClient.UploadPages(0, &pageContent, options), StorageException);
  }

  TEST_F(PageBlobClientTest, CreateIfNotExists)
  {
    auto blobClient = Azure::Storage::Blobs::PageBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    auto blobClientWithoutAuth = Azure::Storage::Blobs::PageBlobClient(blobClient.GetUrl());
    EXPECT_THROW(blobClientWithoutAuth.CreateIfNotExists(m_blobContent.size()), StorageException);
    {
      auto response = blobClient.CreateIfNotExists(m_blobContent.size());
      EXPECT_TRUE(response->Created);
    }

    auto blobContent
        = Azure::Core::Http::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    blobClient.UploadPages(0, &blobContent);
    {
      auto response = blobClient.CreateIfNotExists(m_blobContent.size());
      EXPECT_FALSE(response->Created);
    }
    auto downloadStream = std::move(blobClient.Download()->BodyStream);
    EXPECT_EQ(
        Azure::Core::Http::BodyStream::ReadToEnd(Azure::Core::Context(), *downloadStream),
        m_blobContent);
  }

}}} // namespace Azure::Storage::Test
