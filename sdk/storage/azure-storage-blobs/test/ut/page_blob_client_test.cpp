// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "page_blob_client_test.hpp"

#include <future>
#include <vector>

#include <azure/core/cryptography/hash.hpp>
#include <azure/storage/blobs/blob_lease_client.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/file_io.hpp>

namespace Azure { namespace Storage { namespace Test {

  void PageBlobClientTest::SetUp() { BlobContainerClientTest::SetUp(); }

  void PageBlobClientTest::TearDown() { BlobContainerClientTest::TearDown(); }

  TEST_F(PageBlobClientTest, CreateDelete)
  {
    auto const testName(GetTestName());
    auto pageBlobClient = GetPageBlobClient(testName);

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

  TEST_F(PageBlobClientTest, CreateWithTags)
  {
    auto const testName(GetTestName());
    auto pageBlobClient = GetPageBlobClient(testName);

    Blobs::CreatePageBlobOptions options;
    options.Tags["key1"] = "value1";
    options.Tags["key2"] = "value2";
    options.Tags["key3 +-./:=_"] = "v1 +-./:=_";
    pageBlobClient.Create(512, options);

    EXPECT_EQ(pageBlobClient.GetTags().Value, options.Tags);
  }

  TEST_F(PageBlobClientTest, Resize)
  {
    auto const testName(GetTestName());
    auto pageBlobClient = GetPageBlobClient(testName);

    pageBlobClient.Create(0, m_blobUploadOptions);

    EXPECT_EQ(pageBlobClient.GetProperties().Value.BlobSize, 0);
    pageBlobClient.Resize(static_cast<int64_t>(2_KB));
    EXPECT_EQ(static_cast<uint64_t>(pageBlobClient.GetProperties().Value.BlobSize), 2_KB);
    pageBlobClient.Resize(static_cast<int64_t>(1_KB));
    EXPECT_EQ(static_cast<uint64_t>(pageBlobClient.GetProperties().Value.BlobSize), 1_KB);
  }

  TEST_F(PageBlobClientTest, UploadClear_LIVEONLY_)
  {
    CHECK_SKIP_TEST();
    auto const testName(GetTestName());
    auto pageBlobClient = GetPageBlobClient(testName);

    std::vector<uint8_t> blobContent = std::vector<uint8_t>(static_cast<size_t>(4_KB), 'x');

    pageBlobClient.Create(8_KB, m_blobUploadOptions);
    auto pageContent = Azure::Core::IO::MemoryBodyStream(blobContent.data(), blobContent.size());
    pageBlobClient.UploadPages(2_KB, pageContent);
    // |_|_|x|x|  |x|x|_|_|
    blobContent.insert(blobContent.begin(), static_cast<size_t>(2_KB), '\x00');
    blobContent.resize(static_cast<size_t>(8_KB), '\x00');
    pageBlobClient.ClearPages({2_KB, 1_KB});
    // |_|_|_|x|  |x|x|_|_|
    std::fill(
        blobContent.begin() + static_cast<size_t>(2_KB),
        blobContent.begin() + static_cast<size_t>(2_KB + 1_KB),
        '\x00');

    auto downloadContent = pageBlobClient.Download();
    EXPECT_EQ(ReadBodyStream(downloadContent.Value.BodyStream), blobContent);

    std::vector<Core::Http::HttpRange> pageRanges;
    for (auto pageResult = pageBlobClient.GetPageRanges(); pageResult.HasPage();
         pageResult.MoveToNextPage())
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
    for (auto pageResult = pageBlobClient.GetPageRanges(options); pageResult.HasPage();
         pageResult.MoveToNextPage())
    {
      pageRanges.insert(
          pageRanges.end(), pageResult.PageRanges.begin(), pageResult.PageRanges.end());
    }
    ASSERT_FALSE(pageRanges.empty());
    EXPECT_EQ(static_cast<uint64_t>(pageRanges[0].Offset), 4_KB);
    EXPECT_EQ(static_cast<uint64_t>(pageRanges[0].Length.Value()), 1_KB);

    auto snapshot = pageBlobClient.CreateSnapshot().Value.Snapshot;
    // |_|_|_|x|  |x|x|_|_| This is what's in snapshot
    blobContent.resize(static_cast<size_t>(1_KB));
    auto pageClient = Azure::Core::IO::MemoryBodyStream(blobContent.data(), blobContent.size());
    pageBlobClient.UploadPages(0, pageClient);
    pageBlobClient.ClearPages({3_KB, 1_KB});
    // |x|_|_|_|  |x|x|_|_|

    pageRanges.clear();
    std::vector<Core::Http::HttpRange> clearRanges;
    for (auto pageResult = pageBlobClient.GetPageRangesDiff(snapshot); pageResult.HasPage();
         pageResult.MoveToNextPage())
    {
      pageRanges.insert(
          pageRanges.end(), pageResult.PageRanges.begin(), pageResult.PageRanges.end());
      clearRanges.insert(
          clearRanges.end(), pageResult.ClearRanges.begin(), pageResult.ClearRanges.end());
    }
    ASSERT_FALSE(pageRanges.empty());
    ASSERT_FALSE(clearRanges.empty());
    EXPECT_EQ(pageRanges[0].Offset, 0);
    EXPECT_EQ(static_cast<uint64_t>(pageRanges[0].Length.Value()), 1_KB);
    EXPECT_EQ(static_cast<uint64_t>(clearRanges[0].Offset), 3_KB);
    EXPECT_EQ(static_cast<uint64_t>(clearRanges[0].Length.Value()), 1_KB);
  }

  TEST_F(PageBlobClientTest, GetPageRangesContinuation)
  {
    auto const testName(GetTestName());
    auto pageBlobClient = GetPageBlobClient(testName);

    std::vector<uint8_t> blobContent = std::vector<uint8_t>(static_cast<size_t>(512), 'x');

    pageBlobClient.Create(8_KB, m_blobUploadOptions);
    auto pageContent = Azure::Core::IO::MemoryBodyStream(blobContent.data(), blobContent.size());
    pageBlobClient.UploadPages(0, pageContent);
    pageContent.Rewind();
    pageBlobClient.UploadPages(1024, pageContent);
    pageContent.Rewind();
    pageBlobClient.UploadPages(4096, pageContent);

    Blobs::GetPageRangesOptions options;
    options.PageSizeHint = 1;
    size_t numRanges = 0;
    for (auto pageResult = pageBlobClient.GetPageRanges(options); pageResult.HasPage();
         pageResult.MoveToNextPage())
    {
      EXPECT_EQ(pageResult.PageRanges.size(), static_cast<size_t>(1));
      numRanges += pageResult.PageRanges.size();
    }
    EXPECT_EQ(numRanges, static_cast<size_t>(3));
  }

  TEST_F(PageBlobClientTest, UploadFromUri)
  {
    auto const testName(GetTestName());
    auto testPageBlobClient = GetPageBlobClient(testName);
    UploadPage();

    auto pageBlobClient = Azure::Storage::Blobs::PageBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(),
        m_containerName,
        testName + "2",
        InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>());
    pageBlobClient.Create(m_blobContent.size(), m_blobUploadOptions);
    pageBlobClient.UploadPagesFromUri(
        0, testPageBlobClient.GetUrl() + GetSas(), {0, static_cast<int64_t>(m_blobContent.size())});
  }

  TEST_F(PageBlobClientTest, StartCopyIncremental)
  {
    auto const testName(GetTestName());
    auto testPageBlobClient = GetPageBlobClient(testName);
    UploadPage();

    const std::string blobName(testName + "2");
    auto pageBlobClient = Azure::Storage::Blobs::PageBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(),
        m_containerName,
        blobName,
        InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>());

    std::string snapshot = testPageBlobClient.CreateSnapshot().Value.Snapshot;
    Azure::Core::Url sourceUri(testPageBlobClient.WithSnapshot(snapshot).GetUrl());
    auto copyInfo = pageBlobClient.StartCopyIncremental(AppendQueryParameters(sourceUri, GetSas()));
    EXPECT_EQ(
        copyInfo.GetRawResponse().GetStatusCode(), Azure::Core::Http::HttpStatusCode::Accepted);
    auto getPropertiesResult = copyInfo.PollUntilDone(PollInterval());
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

    auto blobItem = GetBlobItem(blobName, Blobs::Models::ListBlobsIncludeFlags::Copy);
    ASSERT_TRUE(blobItem.Details.IsIncrementalCopy.HasValue());
    EXPECT_TRUE(blobItem.Details.IsIncrementalCopy.Value());
    ASSERT_TRUE(blobItem.Details.IncrementalCopyDestinationSnapshot.HasValue());
    EXPECT_FALSE(blobItem.Details.IncrementalCopyDestinationSnapshot.Value().empty());
  }

  TEST_F(PageBlobClientTest, Lease)
  {
    auto const testName(GetTestName());
    auto testPageBlobClient = GetPageBlobClient(testName);
    UploadPage();

    {
      std::string leaseId1 = Blobs::BlobLeaseClient::CreateUniqueLeaseId();
      auto leaseDuration = std::chrono::seconds(20);
      Blobs::BlobLeaseClient leaseClient(testPageBlobClient, leaseId1);
      auto aLease = leaseClient.Acquire(leaseDuration).Value;
      EXPECT_TRUE(aLease.ETag.HasValue());
      EXPECT_TRUE(IsValidTime(aLease.LastModified));
      EXPECT_EQ(aLease.LeaseId, leaseId1);
      EXPECT_EQ(leaseClient.GetLeaseId(), leaseId1);
      aLease = leaseClient.Acquire(leaseDuration).Value;
      EXPECT_TRUE(aLease.ETag.HasValue());
      EXPECT_TRUE(IsValidTime(aLease.LastModified));
      EXPECT_EQ(aLease.LeaseId, leaseId1);

      auto properties = testPageBlobClient.GetProperties().Value;
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
      EXPECT_EQ(leaseClient.GetLeaseId(), leaseId2);

      auto blobInfo = leaseClient.Release().Value;
      EXPECT_TRUE(blobInfo.ETag.HasValue());
      EXPECT_TRUE(IsValidTime(blobInfo.LastModified));
    }

    {
      Blobs::BlobLeaseClient leaseClient(
          testPageBlobClient, Blobs::BlobLeaseClient::CreateUniqueLeaseId());
      auto aLease = leaseClient.Acquire(Blobs::BlobLeaseClient::InfiniteLeaseDuration).Value;
      auto properties = testPageBlobClient.GetProperties().Value;
      EXPECT_EQ(properties.LeaseDuration.Value(), Blobs::Models::LeaseDurationType::Infinite);
      auto brokenLease = leaseClient.Break().Value;
      EXPECT_TRUE(brokenLease.ETag.HasValue());
      EXPECT_TRUE(IsValidTime(brokenLease.LastModified));
    }

    {
      Blobs::BlobLeaseClient leaseClient(
          testPageBlobClient, Blobs::BlobLeaseClient::CreateUniqueLeaseId());
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

  TEST_F(PageBlobClientTest, ContentHash)
  {
    auto const testName(GetTestName());
    auto pageBlobClient = GetPageBlobClient(testName);

    std::vector<uint8_t> blobContent = RandomBuffer(static_cast<size_t>(4_KB));
    const std::vector<uint8_t> contentMd5
        = Azure::Core::Cryptography::Md5Hash().Final(blobContent.data(), blobContent.size());
    const std::vector<uint8_t> contentCrc64
        = Azure::Storage::Crc64Hash().Final(blobContent.data(), blobContent.size());

    pageBlobClient.Create(blobContent.size());
    auto contentStream = Azure::Core::IO::MemoryBodyStream(blobContent.data(), blobContent.size());
    pageBlobClient.UploadPages(0, contentStream);

    auto pageBlobClient2 = GetPageBlobClient(testName + "2");
    pageBlobClient2.Create(blobContent.size());

    Blobs::UploadPagesOptions options1;
    options1.TransactionalContentHash = ContentHash();
    options1.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Md5;
    options1.TransactionalContentHash.Value().Value = Azure::Core::Convert::Base64Decode(DummyMd5);
    contentStream.Rewind();
    EXPECT_THROW(pageBlobClient2.UploadPages(0, contentStream, options1), StorageException);
    options1.TransactionalContentHash.Value().Value = contentMd5;
    contentStream.Rewind();
    EXPECT_NO_THROW(pageBlobClient2.UploadPages(0, contentStream, options1));
    options1.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Crc64;
    options1.TransactionalContentHash.Value().Value
        = Azure::Core::Convert::Base64Decode(DummyCrc64);
    contentStream.Rewind();
    EXPECT_THROW(pageBlobClient2.UploadPages(0, contentStream, options1), StorageException);
    options1.TransactionalContentHash.Value().Value = contentCrc64;
    contentStream.Rewind();
    EXPECT_NO_THROW(pageBlobClient2.UploadPages(0, contentStream, options1));

    Blobs::UploadPagesFromUriOptions options2;
    Azure::Core::Http::HttpRange sourceRange;
    sourceRange.Offset = 0;
    sourceRange.Length = blobContent.size();
    options2.TransactionalContentHash = ContentHash();
    options2.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Md5;
    options2.TransactionalContentHash.Value().Value = Azure::Core::Convert::Base64Decode(DummyMd5);
    EXPECT_THROW(
        pageBlobClient2.UploadPagesFromUri(
            0, pageBlobClient.GetUrl() + GetSas(), sourceRange, options2),
        StorageException);
    options2.TransactionalContentHash.Value().Value = contentMd5;
    EXPECT_NO_THROW(pageBlobClient2.UploadPagesFromUri(
        0, pageBlobClient.GetUrl() + GetSas(), sourceRange, options2));
    options2.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Crc64;
    options2.TransactionalContentHash.Value().Value
        = Azure::Core::Convert::Base64Decode(DummyCrc64);
    // EXPECT_THROW(
    //    pageBlobClient2.UploadPagesFromUri(
    //        0, pageBlobClient.GetUrl() + GetSas(), sourceRange, options2),
    //    StorageException);
    options2.TransactionalContentHash.Value().Value = contentCrc64;
    EXPECT_NO_THROW(pageBlobClient2.UploadPagesFromUri(
        0, pageBlobClient.GetUrl() + GetSas(), sourceRange, options2));
  }

  TEST_F(PageBlobClientTest, CreateIfNotExists)
  {
    auto const testName(GetTestName());
    auto blobClient = GetPageBlobClient(testName);

    auto blobClientWithoutAuth = Azure::Storage::Blobs::PageBlobClient(
        blobClient.GetUrl(), InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>());
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

  TEST_F(PageBlobClientTest, SourceBlobAccessConditions)
  {
    auto const testName(GetTestName());
    auto sourceBlobClient = GetPageBlobClient(testName);

    const std::string url = sourceBlobClient.GetUrl() + GetSas();

    const int64_t blobSize = 512;
    auto createResponse = sourceBlobClient.Create(blobSize);
    Azure::ETag eTag = createResponse.Value.ETag;
    auto lastModifiedTime = createResponse.Value.LastModified;
    auto timeBeforeStr = lastModifiedTime - std::chrono::seconds(1);
    auto timeAfterStr = lastModifiedTime + std::chrono::seconds(1);

    auto destBlobClient = Azure::Storage::Blobs::PageBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(),
        m_containerName,
        testName + "2",
        InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>());
    destBlobClient.Create(blobSize);

    {
      Blobs::UploadPagesFromUriOptions options;
      options.SourceAccessConditions.IfMatch = eTag;
      EXPECT_NO_THROW(destBlobClient.UploadPagesFromUri(0, url, {0, blobSize}, options));
      options.SourceAccessConditions.IfMatch = DummyETag;
      EXPECT_THROW(
          destBlobClient.UploadPagesFromUri(0, url, {0, blobSize}, options), StorageException);
    }
    {
      Blobs::UploadPagesFromUriOptions options;
      options.SourceAccessConditions.IfNoneMatch = DummyETag;
      EXPECT_NO_THROW(destBlobClient.UploadPagesFromUri(0, url, {0, blobSize}, options));
      options.SourceAccessConditions.IfNoneMatch = eTag;
      EXPECT_THROW(
          destBlobClient.UploadPagesFromUri(0, url, {0, blobSize}, options), StorageException);
    }
    {
      Blobs::UploadPagesFromUriOptions options;
      options.SourceAccessConditions.IfModifiedSince = timeBeforeStr;
      EXPECT_NO_THROW(destBlobClient.UploadPagesFromUri(0, url, {0, blobSize}, options));
      options.SourceAccessConditions.IfModifiedSince = timeAfterStr;
      EXPECT_THROW(
          destBlobClient.UploadPagesFromUri(0, url, {0, blobSize}, options), StorageException);
    }
    {
      Blobs::UploadPagesFromUriOptions options;
      options.SourceAccessConditions.IfUnmodifiedSince = timeAfterStr;
      EXPECT_NO_THROW(destBlobClient.UploadPagesFromUri(0, url, {0, blobSize}, options));
      options.SourceAccessConditions.IfUnmodifiedSince = timeBeforeStr;
      EXPECT_THROW(
          destBlobClient.UploadPagesFromUri(0, url, {0, blobSize}, options), StorageException);
    }
  }

  TEST_F(PageBlobClientTest, UpdateSequenceNumber)
  {
    auto const testName(GetTestName());
    auto blobClient = GetPageBlobClient(testName);

    blobClient.Create(512);

    Blobs::Models::BlobHttpHeaders headers;
    headers.ContentType = "text/plain";
    blobClient.SetHttpHeaders(headers);

    Blobs::UpdatePageBlobSequenceNumberOptions options;
    options.SequenceNumber = 100;
    auto res
        = blobClient.UpdateSequenceNumber(Blobs::Models::SequenceNumberAction::Update, options);
    EXPECT_TRUE(res.Value.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(res.Value.LastModified));
    EXPECT_EQ(res.Value.SequenceNumber, 100);
    EXPECT_EQ(blobClient.GetProperties().Value.SequenceNumber.Value(), 100);

    options.SequenceNumber = 200;
    res = blobClient.UpdateSequenceNumber(Blobs::Models::SequenceNumberAction::Update, options);
    EXPECT_EQ(res.Value.SequenceNumber, 200);
    EXPECT_EQ(blobClient.GetProperties().Value.SequenceNumber.Value(), 200);

    options.SequenceNumber = 50;
    res = blobClient.UpdateSequenceNumber(Blobs::Models::SequenceNumberAction::Max, options);
    EXPECT_EQ(res.Value.SequenceNumber, 200);
    EXPECT_EQ(blobClient.GetProperties().Value.SequenceNumber.Value(), 200);
    options.SequenceNumber = 300;
    res = blobClient.UpdateSequenceNumber(Blobs::Models::SequenceNumberAction::Max, options);
    EXPECT_EQ(res.Value.SequenceNumber, 300);
    EXPECT_EQ(blobClient.GetProperties().Value.SequenceNumber.Value(), 300);

    options.SequenceNumber.Reset();
    res = blobClient.UpdateSequenceNumber(Blobs::Models::SequenceNumberAction::Increment, options);
    EXPECT_EQ(res.Value.SequenceNumber, 301);
    EXPECT_EQ(blobClient.GetProperties().Value.SequenceNumber.Value(), 301);

    EXPECT_EQ(blobClient.GetProperties().Value.HttpHeaders.ContentType, headers.ContentType);
  }

  TEST_F(PageBlobClientTest, PageBlobAccessConditions)
  {
    auto const testName(GetTestName());
    auto blobClient = GetPageBlobClient(testName);

    blobClient.Create(1024);
    Blobs::UpdatePageBlobSequenceNumberOptions updateSequenceNumberOptions;
    updateSequenceNumberOptions.SequenceNumber = 100;
    blobClient.UpdateSequenceNumber(
        Blobs::Models::SequenceNumberAction::Update, updateSequenceNumberOptions);

    enum class AccessConditionType
    {
      Eq,
      Lt,
      LtOrEq,
    };
    enum class Operation
    {
      Upload,
      UploadFromUri,
      Clear,
    };
    for (auto o : {Operation::Upload, Operation::UploadFromUri, Operation::Clear})
    {

      for (auto willSuccess : {true, false})
      {
        for (auto t :
             {AccessConditionType::Eq, AccessConditionType::Lt, AccessConditionType::LtOrEq})
        {
          Blobs::PageBlobAccessConditions accessConditions;
          if (t == AccessConditionType::Eq)
          {
            accessConditions.IfSequenceNumberEqual
                = blobClient.GetProperties().Value.SequenceNumber.Value();
            if (!willSuccess)
            {
              accessConditions.IfSequenceNumberEqual.Value()++;
            }
          }
          else if (t == AccessConditionType::Lt)
          {
            accessConditions.IfSequenceNumberLessThan
                = blobClient.GetProperties().Value.SequenceNumber.Value();
            if (willSuccess)
            {
              accessConditions.IfSequenceNumberLessThan.Value()++;
            }
          }
          else if (t == AccessConditionType::LtOrEq)
          {
            accessConditions.IfSequenceNumberLessThanOrEqual
                = blobClient.GetProperties().Value.SequenceNumber.Value();
            if (!willSuccess)
            {
              accessConditions.IfSequenceNumberLessThanOrEqual.Value()--;
            }
          }

          if (o == Operation::Upload)
          {
            std::vector<uint8_t> pageContent(512);
            auto pageContentStream
                = Azure::Core::IO::MemoryBodyStream(pageContent.data(), pageContent.size());

            Blobs::UploadPagesOptions options;
            options.AccessConditions = accessConditions;
            if (willSuccess)
            {
              EXPECT_NO_THROW(blobClient.UploadPages(0, pageContentStream, options));
            }
            else
            {
              EXPECT_THROW(blobClient.UploadPages(0, pageContentStream, options), StorageException);
            }
          }
          else if (o == Operation::UploadFromUri)
          {
            Blobs::UploadPagesFromUriOptions options;
            options.AccessConditions = accessConditions;
            if (willSuccess)
            {
              EXPECT_NO_THROW(blobClient.UploadPagesFromUri(
                  512, blobClient.GetUrl() + GetSas(), {0, 512}, options));
            }
            else
            {
              EXPECT_THROW(
                  blobClient.UploadPagesFromUri(
                      512, blobClient.GetUrl() + GetSas(), {0, 512}, options),
                  StorageException);
            }
          }
          else if (o == Operation::Clear)
          {
            Blobs::ClearPagesOptions options;
            options.AccessConditions = accessConditions;
            if (willSuccess)
            {
              EXPECT_NO_THROW(blobClient.ClearPages({0, 512}, options));
            }
            else
            {
              EXPECT_THROW(blobClient.ClearPages({0, 512}, options), StorageException);
            }
          }
        }
      }
    }
  }

}}} // namespace Azure::Storage::Test
