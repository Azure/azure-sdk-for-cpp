// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "page_blob_client_test.hpp"

#include <future>
#include <vector>

#include "azure/storage/common/crypt.hpp"
#include "azure/storage/common/file_io.hpp"

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
    m_blobUploadOptions.HttpHeaders.ContentMd5 = "";
    m_pageBlobClient->Create(m_blobContent.size(), m_blobUploadOptions);
    auto pageContent
        = Azure::Core::Http::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    m_pageBlobClient->UploadPages(&pageContent, 0);
    m_blobUploadOptions.HttpHeaders.ContentMd5
        = m_pageBlobClient->GetProperties()->HttpHeaders.ContentMd5;
  }

  void PageBlobClientTest::TearDownTestSuite() { BlobContainerClientTest::TearDownTestSuite(); }

  TEST_F(PageBlobClientTest, CreateDelete)
  {
    auto pageBlobClient = Azure::Storage::Blobs::PageBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    auto blobContentInfo = pageBlobClient.Create(0, m_blobUploadOptions);
    EXPECT_FALSE(blobContentInfo->ETag.empty());
    EXPECT_FALSE(blobContentInfo->LastModified.empty());
    EXPECT_TRUE(blobContentInfo->VersionId.HasValue());
    EXPECT_FALSE(blobContentInfo->VersionId.GetValue().empty());
    EXPECT_FALSE(blobContentInfo->EncryptionScope.HasValue());
    EXPECT_FALSE(blobContentInfo->EncryptionKeySha256.HasValue());

    pageBlobClient.Delete();
    EXPECT_THROW(pageBlobClient.Delete(), StorageError);
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
    pageBlobClient.UploadPages(&pageContent, 2_KB);
    // |_|_|x|x|  |x|x|_|_|
    blobContent.insert(blobContent.begin(), static_cast<std::size_t>(2_KB), '\x00');
    blobContent.resize(static_cast<std::size_t>(8_KB), '\x00');
    pageBlobClient.ClearPages(2_KB, 1_KB);
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
    EXPECT_EQ(static_cast<uint64_t>(pageRanges.PageRanges[0].Length), 3_KB);

    Azure::Storage::Blobs::GetPageBlobPageRangesOptions options;
    options.Offset = 4_KB;
    options.Length = 1_KB;
    pageRanges = *pageBlobClient.GetPageRanges(options);
    EXPECT_TRUE(pageRanges.ClearRanges.empty());
    ASSERT_FALSE(pageRanges.PageRanges.empty());
    EXPECT_EQ(static_cast<uint64_t>(pageRanges.PageRanges[0].Offset), 4_KB);
    EXPECT_EQ(static_cast<uint64_t>(pageRanges.PageRanges[0].Length), 1_KB);

    auto snapshot = pageBlobClient.CreateSnapshot()->Snapshot;
    // |_|_|_|x|  |x|x|_|_| This is what's in snapshot
    blobContent.resize(static_cast<std::size_t>(1_KB));
    auto pageClient = Azure::Core::Http::MemoryBodyStream(blobContent.data(), blobContent.size());
    pageBlobClient.UploadPages(&pageClient, 0);
    pageBlobClient.ClearPages(3_KB, 1_KB);
    // |x|_|_|_|  |x|x|_|_|

    options = Azure::Storage::Blobs::GetPageBlobPageRangesOptions();
    options.PreviousSnapshot = snapshot;
    pageRanges = *pageBlobClient.GetPageRanges(options);
    ASSERT_FALSE(pageRanges.ClearRanges.empty());
    ASSERT_FALSE(pageRanges.PageRanges.empty());
    EXPECT_EQ(pageRanges.PageRanges[0].Offset, 0);
    EXPECT_EQ(static_cast<uint64_t>(pageRanges.PageRanges[0].Length), 1_KB);
    EXPECT_EQ(static_cast<uint64_t>(pageRanges.ClearRanges[0].Offset), 3_KB);
    EXPECT_EQ(static_cast<uint64_t>(pageRanges.ClearRanges[0].Length), 1_KB);
  }

  TEST_F(PageBlobClientTest, UploadFromUri)
  {
    auto pageBlobClient = Azure::Storage::Blobs::PageBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    pageBlobClient.Create(m_blobContent.size(), m_blobUploadOptions);
    pageBlobClient.UploadPagesFromUri(
        m_pageBlobClient->GetUri() + GetSas(), 0, m_blobContent.size(), 0);
  }

  TEST_F(PageBlobClientTest, StartCopyIncremental)
  {
    auto pageBlobClient = Azure::Storage::Blobs::PageBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    std::string snapshot = m_pageBlobClient->CreateSnapshot()->Snapshot;
    UriBuilder sourceUri(m_pageBlobClient->WithSnapshot(snapshot).GetUri());
    sourceUri.AppendQueries(GetSas());
    auto copyInfo = pageBlobClient.StartCopyIncremental(sourceUri.ToString());
    EXPECT_FALSE(copyInfo->ETag.empty());
    EXPECT_FALSE(copyInfo->LastModified.empty());
    EXPECT_FALSE(copyInfo->CopyId.empty());
    EXPECT_NE(copyInfo->CopyStatus, Blobs::CopyStatus::Unknown);
    EXPECT_TRUE(copyInfo->VersionId.HasValue());
    EXPECT_FALSE(copyInfo->VersionId.GetValue().empty());
  }

  TEST_F(PageBlobClientTest, Lease)
  {
    std::string leaseId1 = CreateUniqueLeaseId();
    int32_t leaseDuration = 20;
    auto aLease = *m_pageBlobClient->AcquireLease(leaseId1, leaseDuration);
    EXPECT_FALSE(aLease.ETag.empty());
    EXPECT_FALSE(aLease.LastModified.empty());
    EXPECT_EQ(aLease.LeaseId, leaseId1);
    aLease = *m_pageBlobClient->AcquireLease(leaseId1, leaseDuration);
    EXPECT_FALSE(aLease.ETag.empty());
    EXPECT_FALSE(aLease.LastModified.empty());
    EXPECT_EQ(aLease.LeaseId, leaseId1);

    auto properties = *m_pageBlobClient->GetProperties();
    EXPECT_EQ(properties.LeaseState.GetValue(), Blobs::BlobLeaseState::Leased);
    EXPECT_EQ(properties.LeaseStatus.GetValue(), Blobs::BlobLeaseStatus::Locked);
    EXPECT_FALSE(properties.LeaseDuration.GetValue().empty());

    auto rLease = *m_pageBlobClient->RenewLease(leaseId1);
    EXPECT_FALSE(rLease.ETag.empty());
    EXPECT_FALSE(rLease.LastModified.empty());
    EXPECT_EQ(rLease.LeaseId, leaseId1);

    std::string leaseId2 = CreateUniqueLeaseId();
    EXPECT_NE(leaseId1, leaseId2);
    auto cLease = *m_pageBlobClient->ChangeLease(leaseId1, leaseId2);
    EXPECT_FALSE(cLease.ETag.empty());
    EXPECT_FALSE(cLease.LastModified.empty());
    EXPECT_EQ(cLease.LeaseId, leaseId2);

    auto blobInfo = *m_pageBlobClient->ReleaseLease(leaseId2);
    EXPECT_FALSE(blobInfo.ETag.empty());
    EXPECT_FALSE(blobInfo.LastModified.empty());

    aLease = *m_pageBlobClient->AcquireLease(CreateUniqueLeaseId(), c_InfiniteLeaseDuration);
    properties = *m_pageBlobClient->GetProperties();
    EXPECT_FALSE(properties.LeaseDuration.GetValue().empty());
    auto brokenLease = *m_pageBlobClient->BreakLease();
    EXPECT_FALSE(brokenLease.ETag.empty());
    EXPECT_FALSE(brokenLease.LastModified.empty());
    EXPECT_EQ(brokenLease.LeaseTime, 0);

    aLease = *m_pageBlobClient->AcquireLease(CreateUniqueLeaseId(), leaseDuration);
    brokenLease = *m_pageBlobClient->BreakLease();
    EXPECT_FALSE(brokenLease.ETag.empty());
    EXPECT_FALSE(brokenLease.LastModified.empty());
    EXPECT_NE(brokenLease.LeaseTime, 0);

    Blobs::BreakBlobLeaseOptions options;
    options.breakPeriod = 0;
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
    options.ContentMd5 = Base64Encode(Md5::Hash(blobContent.data(), blobContent.size()));
    EXPECT_NO_THROW(pageBlobClient.UploadPages(&pageContent, 0, options));

    pageContent.Rewind();
    options.ContentMd5 = c_dummyMd5;
    EXPECT_THROW(pageBlobClient.UploadPages(&pageContent, 0, options), StorageError);
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
    options.ContentCrc64 = Base64Encode(Crc64::Hash(blobContent.data(), blobContent.size()));
    EXPECT_NO_THROW(pageBlobClient.UploadPages(&pageContent, 0, options));

    pageContent.Rewind();
    options.ContentCrc64 = c_dummyCrc64;
    EXPECT_THROW(pageBlobClient.UploadPages(&pageContent, 0, options), StorageError);
  }

  TEST_F(PageBlobClientTest, ConcurrentUploadFromNonExistingFile)
  {
    auto pageBlobClient = m_blobContainerClient->GetPageBlobClient(RandomString());
    std::string emptyFilename = RandomString();
    EXPECT_THROW(pageBlobClient.UploadFrom(emptyFilename), std::runtime_error);
    EXPECT_THROW(pageBlobClient.Delete(), StorageError);
  }

  TEST_F(PageBlobClientTest, ConcurrentUploadEmptyBlob)
  {
    auto pageBlobClient = m_blobContainerClient->GetPageBlobClient(RandomString());

    std::vector<uint8_t> emptyContent;
    pageBlobClient.UploadFrom(emptyContent.data(), emptyContent.size());
    EXPECT_NO_THROW(pageBlobClient.Delete());

    std::string emptyFilename = RandomString();
    {
      Details::FileWriter writer(emptyFilename);
    }
    pageBlobClient.UploadFrom(emptyFilename);
    EXPECT_NO_THROW(pageBlobClient.Delete());

    DeleteFile(emptyFilename);
  }

  TEST_F(PageBlobClientTest, ConcurrentUpload)
  {
    std::vector<uint8_t> blobContent = RandomBuffer(static_cast<std::size_t>(8_MB));

    auto testUploadFromBuffer = [&](int concurrency, int64_t blobSize) {
      auto pageBlobClient = m_blobContainerClient->GetPageBlobClient(RandomString());

      Azure::Storage::Blobs::UploadPageBlobFromOptions options;
      options.ChunkSize = 512_KB;
      options.Concurrency = concurrency;
      options.HttpHeaders = m_blobUploadOptions.HttpHeaders;
      options.HttpHeaders.ContentMd5.clear();
      options.Metadata = m_blobUploadOptions.Metadata;

      auto res = pageBlobClient.UploadFrom(
          blobContent.data(), static_cast<std::size_t>(blobSize), options);
      EXPECT_TRUE(res->ServerEncrypted.HasValue());

      auto properties = *pageBlobClient.GetProperties();
      properties.HttpHeaders.ContentMd5.clear();
      EXPECT_EQ(properties.ContentLength, blobSize);
      EXPECT_EQ(properties.Metadata, options.Metadata);
      std::vector<uint8_t> downloadContent(static_cast<std::size_t>(blobSize), '\x00');
      pageBlobClient.DownloadTo(downloadContent.data(), static_cast<std::size_t>(blobSize));
      EXPECT_EQ(
          downloadContent,
          std::vector<uint8_t>(
              blobContent.begin(), blobContent.begin() + static_cast<std::size_t>(blobSize)));
    };

    auto testUploadFromFile = [&](int concurrency, int64_t blobSize) {
      auto pageBlobClient = m_blobContainerClient->GetPageBlobClient(RandomString());

      Azure::Storage::Blobs::UploadPageBlobFromOptions options;
      options.ChunkSize = 512_KB;
      options.Concurrency = concurrency;
      options.HttpHeaders = m_blobUploadOptions.HttpHeaders;
      options.HttpHeaders.ContentMd5.clear();
      options.Metadata = m_blobUploadOptions.Metadata;

      std::string tempFilename = RandomString();
      {
        Azure::Storage::Details::FileWriter fileWriter(tempFilename);
        fileWriter.Write(blobContent.data(), blobSize, 0);
      }

      auto res = pageBlobClient.UploadFrom(tempFilename, options);
      EXPECT_TRUE(res->ServerEncrypted.HasValue());

      auto properties = *pageBlobClient.GetProperties();
      properties.HttpHeaders.ContentMd5.clear();
      EXPECT_EQ(properties.ContentLength, blobSize);
      EXPECT_EQ(properties.Metadata, options.Metadata);
      std::vector<uint8_t> downloadContent(static_cast<std::size_t>(blobSize), '\x00');
      pageBlobClient.DownloadTo(downloadContent.data(), static_cast<std::size_t>(blobSize));
      EXPECT_EQ(
          downloadContent,
          std::vector<uint8_t>(
              blobContent.begin(), blobContent.begin() + static_cast<std::size_t>(blobSize)));

      DeleteFile(tempFilename);
    };

    std::vector<std::future<void>> futures;
    for (int c : {1, 2, 5})
    {
      for (int64_t l : {0ULL, 512ULL, 1_KB, 4_KB, 1_MB, 4_MB + 512})
      {
        ASSERT_GE(blobContent.size(), static_cast<std::size_t>(l));
        futures.emplace_back(std::async(std::launch::async, testUploadFromBuffer, c, l));
        futures.emplace_back(std::async(std::launch::async, testUploadFromFile, c, l));
      }
    }
    for (auto& f : futures)
    {
      f.get();
    }
  }

}}} // namespace Azure::Storage::Test
