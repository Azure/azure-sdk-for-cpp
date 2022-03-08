// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "block_blob_client_test.hpp"

#include <future>
#include <random>
#include <vector>

#include <azure/core/cryptography/hash.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/file_io.hpp>

namespace Azure { namespace Storage { namespace Blobs { namespace Models {

  bool operator==(const BlobHttpHeaders& lhs, const BlobHttpHeaders& rhs)
  {
    return lhs.ContentType == rhs.ContentType && lhs.ContentEncoding == rhs.ContentEncoding
        && lhs.ContentLanguage == rhs.ContentLanguage
        && lhs.ContentHash.Value == rhs.ContentHash.Value
        && lhs.ContentHash.Algorithm == rhs.ContentHash.Algorithm
        && lhs.CacheControl == rhs.CacheControl && lhs.ContentDisposition == rhs.ContentDisposition;
  }

  bool operator==(const BlobImmutabilityPolicy& lhs, const BlobImmutabilityPolicy& rhs)
  {
    return lhs.ExpiresOn == rhs.ExpiresOn && lhs.PolicyMode == rhs.PolicyMode;
  }

}}}} // namespace Azure::Storage::Blobs::Models

namespace Azure { namespace Storage { namespace Test {

  void BlockBlobClientTest::SetUp() { BlobContainerClientTest::SetUp(); }

  void BlockBlobClientTest::TearDown()
  {
    // Deleting the container with any blobs in it
    BlobContainerClientTest::TearDown();
  }

  TEST_F(BlockBlobClientTest, CreateDelete)
  {
    auto const testName(GetTestName());
    auto client = GetBlockBlobClient(testName);

    auto blobContent
        = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    auto blobContentInfo = client.Upload(blobContent, m_blobUploadOptions);
    EXPECT_TRUE(blobContentInfo.Value.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(blobContentInfo.Value.LastModified));
    EXPECT_TRUE(blobContentInfo.Value.VersionId.HasValue());
    EXPECT_FALSE(blobContentInfo.Value.VersionId.Value().empty());
    EXPECT_FALSE(blobContentInfo.Value.EncryptionScope.HasValue());
    EXPECT_FALSE(blobContentInfo.Value.EncryptionKeySha256.HasValue());

    client.Delete();
    EXPECT_THROW(client.Delete(), StorageException);
  }

  TEST_F(BlockBlobClientTest, SoftDelete)
  {
    auto const testName(GetTestName());
    auto client = GetBlockBlobClient(testName);

    const std::string blobName(testName);
    std::vector<uint8_t> emptyContent;
    auto blobContent = Azure::Core::IO::MemoryBodyStream(emptyContent.data(), emptyContent.size());
    client.Upload(blobContent);

    auto blobItem = GetBlobItem(blobName);
    EXPECT_FALSE(blobItem.IsDeleted);
    EXPECT_FALSE(blobItem.Details.DeletedOn.HasValue());
    EXPECT_FALSE(blobItem.Details.RemainingRetentionDays.HasValue());

    client.Delete();

    /*
    // Soft delete doesn't work in storage account with versioning enabled.
    blobItem = GetBlobItem(blobName, Blobs::Models::ListBlobsIncludeFlags::Deleted);
    EXPECT_TRUE(blobItem.IsDeleted);
    ASSERT_TRUE(blobItem.Details.DeletedOn.HasValue());
    EXPECT_TRUE(IsValidTime(blobItem.Details.DeletedOn.Value()));
    EXPECT_TRUE(blobItem.Details.RemainingRetentionDays.HasValue());
    */
  }

  // small default 1Kb upload/download
  TEST_F(BlockBlobClientTest, SmallUploadDownload)
  {
    auto const testName(GetTestName());
    auto client = GetBlockBlobClient(testName);
    UploadBlockBlob();

    auto res = client.Download();
    EXPECT_EQ(res.Value.BlobSize, static_cast<int64_t>(m_blobContent.size()));
    EXPECT_EQ(res.Value.ContentRange.Offset, 0);
    EXPECT_EQ(res.Value.ContentRange.Length.Value(), static_cast<int64_t>(m_blobContent.size()));
    EXPECT_EQ(ReadBodyStream(res.Value.BodyStream), m_blobContent);
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderRequestId).empty());
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderDate).empty());
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderXMsVersion).empty());
    EXPECT_TRUE(res.Value.Details.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(res.Value.Details.LastModified));
    EXPECT_TRUE(IsValidTime(res.Value.Details.CreatedOn));
    EXPECT_EQ(res.Value.Details.HttpHeaders, m_blobUploadOptions.HttpHeaders);
    EXPECT_EQ(res.Value.Details.Metadata, m_blobUploadOptions.Metadata);
    EXPECT_EQ(res.Value.BlobType, Azure::Storage::Blobs::Models::BlobType::BlockBlob);
    Azure::Storage::Blobs::DownloadBlobOptions options;
    options.Range = {100, 200};
    res = client.Download(options);
    EXPECT_EQ(
        ReadBodyStream(res.Value.BodyStream),
        std::vector<uint8_t>(
            m_blobContent.begin() + static_cast<size_t>(options.Range.Value().Offset),
            m_blobContent.begin()
                + static_cast<size_t>(
                    options.Range.Value().Offset + options.Range.Value().Length.Value())));
    EXPECT_EQ(res.Value.ContentRange.Offset, options.Range.Value().Offset);
    EXPECT_EQ(res.Value.ContentRange.Length.Value(), options.Range.Value().Length.Value());
    EXPECT_EQ(res.Value.BlobSize, static_cast<int64_t>(m_blobContent.size()));
  }

  // big 8Mb upload/download should be LIVE only to avoid big recording files
  TEST_F(BlockBlobClientTest, UploadDownload_LIVEONLY_)
  {
    CHECK_SKIP_TEST();
    auto const testName(GetTestName());
    auto client = GetBlockBlobClient(testName);
    UploadBlockBlob(8_MB);

    auto res = client.Download();
    EXPECT_EQ(res.Value.BlobSize, static_cast<int64_t>(m_blobContent.size()));
    EXPECT_EQ(res.Value.ContentRange.Offset, 0);
    EXPECT_EQ(res.Value.ContentRange.Length.Value(), static_cast<int64_t>(m_blobContent.size()));
    EXPECT_EQ(ReadBodyStream(res.Value.BodyStream), m_blobContent);
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderRequestId).empty());
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderDate).empty());
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderXMsVersion).empty());
    EXPECT_TRUE(res.Value.Details.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(res.Value.Details.LastModified));
    EXPECT_TRUE(IsValidTime(res.Value.Details.CreatedOn));
    EXPECT_EQ(res.Value.Details.HttpHeaders, m_blobUploadOptions.HttpHeaders);
    EXPECT_EQ(res.Value.Details.Metadata, m_blobUploadOptions.Metadata);
    EXPECT_EQ(res.Value.BlobType, Azure::Storage::Blobs::Models::BlobType::BlockBlob);
    Azure::Storage::Blobs::DownloadBlobOptions options;
    options.Range = {1_MB, 2_MB};
    res = client.Download(options);
    EXPECT_EQ(
        ReadBodyStream(res.Value.BodyStream),
        std::vector<uint8_t>(
            m_blobContent.begin() + static_cast<size_t>(options.Range.Value().Offset),
            m_blobContent.begin()
                + static_cast<size_t>(
                    options.Range.Value().Offset + options.Range.Value().Length.Value())));
    EXPECT_EQ(res.Value.ContentRange.Offset, options.Range.Value().Offset);
    EXPECT_EQ(res.Value.ContentRange.Length.Value(), options.Range.Value().Length.Value());
    EXPECT_EQ(res.Value.BlobSize, static_cast<int64_t>(m_blobContent.size()));
  }

  TEST_F(BlockBlobClientTest, UploadWithTags)
  {
    auto const testName(GetTestName());
    auto client = GetBlockBlobClient(testName);

    std::map<std::string, std::string> tags;
    tags["key1"] = "value1";
    tags["key2"] = "value2";
    tags["key3 +-./:=_"] = "v1 +-./:=_";

    std::vector<uint8_t> blobContent(100, 'a');
    {
      Blobs::UploadBlockBlobOptions options;
      options.Tags = tags;
      auto stream = Azure::Core::IO::MemoryBodyStream(blobContent.data(), blobContent.size());
      client.Upload(stream, options);
      EXPECT_EQ(client.GetTags().Value, tags);
      client.Delete();
    }

    {
      Blobs::UploadBlockBlobFromOptions options;
      options.TransferOptions.SingleUploadThreshold = 0;
      options.TransferOptions.ChunkSize = blobContent.size() / 2;
      options.Tags = tags;

      {
        client.UploadFrom(blobContent.data(), blobContent.size(), options);
        EXPECT_EQ(client.GetTags().Value, tags);
        client.Delete();
      }
      {
        const std::string tempFilename = "file" + testName;
        {
          Azure::Storage::_internal::FileWriter fileWriter(tempFilename);
          fileWriter.Write(blobContent.data(), blobContent.size(), 0);
        }
        client.UploadFrom(tempFilename, options);
        EXPECT_EQ(client.GetTags().Value, tags);
        client.Delete();
      }
    }
  }

  TEST_F(BlockBlobClientTest, DownloadTransactionalHash)
  {
    auto const testName(GetTestName());
    auto blobClient = GetBlockBlobClient(testName);

    const std::vector<uint8_t> dataPart1(static_cast<size_t>(4_MB + 1), 'a');
    const std::vector<uint8_t> dataPart2(static_cast<size_t>(4_MB + 1), 'b');

    const std::string blockId1 = Base64EncodeText("0");
    const std::string blockId2 = Base64EncodeText("1");

    auto blockContent = Azure::Core::IO::MemoryBodyStream(dataPart1.data(), dataPart1.size());
    blobClient.StageBlock(blockId1, blockContent);
    blockContent = Azure::Core::IO::MemoryBodyStream(dataPart2.data(), dataPart2.size());
    blobClient.StageBlock(blockId2, blockContent);
    blobClient.CommitBlockList({blockId1, blockId2});

    std::vector<uint8_t> blobMd5;
    {
      Azure::Core::Cryptography::Md5Hash instance;
      instance.Append(dataPart1.data(), dataPart1.size());
      blobMd5 = instance.Final(dataPart2.data(), dataPart2.size());
    }

    for (bool blobHasMd5 : {true, false})
    {
      if (blobHasMd5)
      {
        Blobs::Models::BlobHttpHeaders headers;
        headers.ContentHash.Algorithm = HashAlgorithm::Md5;
        headers.ContentHash.Value = blobMd5;
        blobClient.SetHttpHeaders(headers);
        ASSERT_FALSE(blobClient.GetProperties().Value.HttpHeaders.ContentHash.Value.empty());
        EXPECT_EQ(blobClient.Download().Value.Details.HttpHeaders.ContentHash.Value, blobMd5);
      }
      else
      {
        blobClient.SetHttpHeaders(Blobs::Models::BlobHttpHeaders());
        ASSERT_TRUE(blobClient.GetProperties().Value.HttpHeaders.ContentHash.Value.empty());
        ASSERT_TRUE(blobClient.Download().Value.Details.HttpHeaders.ContentHash.Value.empty());
      }
      const int64_t downloadLength = 1;
      Blobs::DownloadBlobOptions options;
      options.Range = Azure::Core::Http::HttpRange();
      options.Range.Value().Offset = 0;
      options.Range.Value().Length = downloadLength;
      options.RangeHashAlgorithm = HashAlgorithm::Md5;
      auto res = blobClient.Download(options);
      if (blobHasMd5)
      {
        EXPECT_EQ(res.Value.Details.HttpHeaders.ContentHash.Value, blobMd5);
      }
      else
      {
        EXPECT_TRUE(res.Value.Details.HttpHeaders.ContentHash.Value.empty());
      }
      ASSERT_TRUE(res.Value.TransactionalContentHash.HasValue());
      EXPECT_EQ(res.Value.TransactionalContentHash.Value().Algorithm, HashAlgorithm::Md5);
      {
        Azure::Core::Cryptography::Md5Hash instance;
        EXPECT_EQ(
            res.Value.TransactionalContentHash.Value().Value,
            instance.Final(dataPart1.data(), downloadLength));
      }
      options.RangeHashAlgorithm = HashAlgorithm::Crc64;
      res = blobClient.Download(options);
      if (blobHasMd5)
      {
        EXPECT_EQ(res.Value.Details.HttpHeaders.ContentHash.Value, blobMd5);
      }
      else
      {
        EXPECT_TRUE(res.Value.Details.HttpHeaders.ContentHash.Value.empty());
      }
      ASSERT_TRUE(res.Value.TransactionalContentHash.HasValue());
      EXPECT_EQ(res.Value.TransactionalContentHash.Value().Algorithm, HashAlgorithm::Crc64);
      {
        Crc64Hash instance;
        EXPECT_EQ(
            res.Value.TransactionalContentHash.Value().Value,
            instance.Final(dataPart1.data(), downloadLength));
      }
    }
  }

  TEST_F(BlockBlobClientTest, DISABLED_LastAccessTime)
  {
    auto const testName(GetTestName());
    auto blobClient = GetBlockBlobClient(testName);

    {
      auto res = blobClient.Download();
      ASSERT_TRUE(res.Value.Details.LastAccessedOn.HasValue());
      EXPECT_TRUE(IsValidTime(res.Value.Details.LastAccessedOn.Value()));
    }
    {
      auto res = blobClient.GetProperties();
      ASSERT_TRUE(res.Value.LastAccessedOn.HasValue());
      EXPECT_TRUE(IsValidTime(res.Value.LastAccessedOn.Value()));
    }
    {
      EXPECT_TRUE(IsValidTime(GetBlobItem(testName).Details.LastAccessedOn.Value()));
    }
  }

  TEST_F(BlockBlobClientTest, DownloadEmpty)
  {
    auto const testName(GetTestName());
    auto blockBlobClient = GetBlockBlobClient(testName);

    std::vector<uint8_t> emptyContent;
    auto blobContent = Azure::Core::IO::MemoryBodyStream(emptyContent.data(), emptyContent.size());
    blockBlobClient.Upload(blobContent);
    blockBlobClient.SetHttpHeaders(m_blobUploadOptions.HttpHeaders);
    blockBlobClient.SetMetadata(m_blobUploadOptions.Metadata);

    auto res = blockBlobClient.Download();
    EXPECT_EQ(res.Value.BodyStream->Length(), 0);
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderRequestId).empty());
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderDate).empty());
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderXMsVersion).empty());
    EXPECT_TRUE(res.Value.Details.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(res.Value.Details.LastModified));
    EXPECT_EQ(res.Value.Details.HttpHeaders, m_blobUploadOptions.HttpHeaders);
    EXPECT_EQ(res.Value.Details.Metadata, m_blobUploadOptions.Metadata);
    EXPECT_EQ(res.Value.BlobType, Azure::Storage::Blobs::Models::BlobType::BlockBlob);

    std::string tempFilename = testName;
    EXPECT_NO_THROW(blockBlobClient.DownloadTo(tempFilename));
    EXPECT_TRUE(ReadFile(tempFilename).empty());
    DeleteFile(tempFilename);

    std::vector<uint8_t> buff;
    EXPECT_NO_THROW(blockBlobClient.DownloadTo(buff.data(), 0));

    Azure::Storage::Blobs::DownloadBlobOptions options;
    options.Range = Core::Http::HttpRange();
    options.Range.Value().Offset = 0;
    EXPECT_THROW(blockBlobClient.Download(options), StorageException);
    options.Range.Value().Length = 1;
    EXPECT_THROW(blockBlobClient.Download(options), StorageException);
  }

  TEST_F(BlockBlobClientTest, SyncCopyFromUri)
  {
    auto const testName(GetTestName());
    auto blockBlobClient = GetBlockBlobClient(testName);
    UploadBlockBlob(8_MB);

    const std::string blobName = testName + "blob";
    auto blobClient = GetBlobClient(blobName);

    auto res = blobClient->CopyFromUri(blockBlobClient.GetUrl() + GetSas());
    EXPECT_EQ(res.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::Accepted);
    EXPECT_TRUE(res.Value.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(res.Value.LastModified));
    EXPECT_FALSE(res.Value.CopyId.empty());
    EXPECT_EQ(res.Value.CopyStatus, Azure::Storage::Blobs::Models::CopyStatus::Success);

    auto downloadResult = blobClient->Download();
    EXPECT_FALSE(downloadResult.Value.Details.CopyId.Value().empty());
    EXPECT_FALSE(downloadResult.Value.Details.CopySource.Value().empty());
    EXPECT_TRUE(
        downloadResult.Value.Details.CopyStatus.Value()
        == Azure::Storage::Blobs::Models::CopyStatus::Success);
    EXPECT_FALSE(downloadResult.Value.Details.CopyProgress.Value().empty());
    EXPECT_TRUE(IsValidTime(downloadResult.Value.Details.CopyCompletedOn.Value()));

    auto blobItem = GetBlobItem(blobName, Blobs::Models::ListBlobsIncludeFlags::Copy);
    EXPECT_FALSE(blobItem.Details.CopyId.Value().empty());
    EXPECT_FALSE(blobItem.Details.CopySource.Value().empty());
    EXPECT_TRUE(
        blobItem.Details.CopyStatus.Value() == Azure::Storage::Blobs::Models::CopyStatus::Success);
    EXPECT_FALSE(blobItem.Details.CopyProgress.Value().empty());
    EXPECT_TRUE(IsValidTime(blobItem.Details.CopyCompletedOn.Value()));
    ASSERT_TRUE(blobItem.Details.IsIncrementalCopy.HasValue());
    EXPECT_FALSE(blobItem.Details.IsIncrementalCopy.Value());
    EXPECT_FALSE(blobItem.Details.IncrementalCopyDestinationSnapshot.HasValue());
  }

  TEST_F(BlockBlobClientTest, AsyncCopyFromUri)
  {

    auto const testName(GetTestName());
    auto blockBlobClient = GetBlockBlobClient(testName);
    UploadBlockBlob(8_MB);

    const std::string blobName = testName + "blob";
    auto blobClient = GetBlobClient(blobName);

    auto res = blobClient->StartCopyFromUri(blockBlobClient.GetUrl());
    EXPECT_EQ(res.GetRawResponse().GetStatusCode(), Azure::Core::Http::HttpStatusCode::Accepted);
    res.PollUntilDone(PollInterval());
    auto properties = blobClient->GetProperties().Value;
    EXPECT_FALSE(properties.CopyId.Value().empty());
    EXPECT_FALSE(properties.CopySource.Value().empty());
    EXPECT_TRUE(
        properties.CopyStatus.Value() == Azure::Storage::Blobs::Models::CopyStatus::Success);
    EXPECT_FALSE(properties.CopyProgress.Value().empty());
    EXPECT_TRUE(IsValidTime(properties.CopyCompletedOn.Value()));
    ASSERT_TRUE(properties.IsIncrementalCopy.HasValue());
    EXPECT_FALSE(properties.IsIncrementalCopy.Value());
    EXPECT_FALSE(properties.IncrementalCopyDestinationSnapshot.HasValue());

    auto downloadResult = blobClient->Download();
    EXPECT_FALSE(downloadResult.Value.Details.CopyId.Value().empty());
    EXPECT_FALSE(downloadResult.Value.Details.CopySource.Value().empty());
    EXPECT_TRUE(
        downloadResult.Value.Details.CopyStatus.Value()
        == Azure::Storage::Blobs::Models::CopyStatus::Success);
    EXPECT_FALSE(downloadResult.Value.Details.CopyProgress.Value().empty());
    EXPECT_TRUE(IsValidTime(downloadResult.Value.Details.CopyCompletedOn.Value()));

    auto blobItem = GetBlobItem(blobName, Blobs::Models::ListBlobsIncludeFlags::Copy);
    EXPECT_FALSE(blobItem.Details.CopyId.Value().empty());
    EXPECT_FALSE(blobItem.Details.CopySource.Value().empty());
    EXPECT_TRUE(
        blobItem.Details.CopyStatus.Value() == Azure::Storage::Blobs::Models::CopyStatus::Success);
    EXPECT_FALSE(blobItem.Details.CopyProgress.Value().empty());
    EXPECT_TRUE(IsValidTime(blobItem.Details.CopyCompletedOn.Value()));
    ASSERT_TRUE(blobItem.Details.IsIncrementalCopy.HasValue());
    EXPECT_FALSE(blobItem.Details.IsIncrementalCopy.Value());
    EXPECT_FALSE(blobItem.Details.IncrementalCopyDestinationSnapshot.HasValue());
  }

  TEST_F(BlockBlobClientTest, CopyWithTagsMetadataTier)
  {
    auto const testName(GetTestName());
    auto blockBlobClient = GetBlockBlobClient(testName);
    UploadBlockBlob(8_MB);

    const std::string blobName = testName + "blob";
    auto blobClient = GetBlobClient(blobName);

    Blobs::StartBlobCopyFromUriOptions options;
    options.Tags["key1"] = "value1";
    options.Tags["key2"] = "value2";
    options.Tags["key3 +-./:=_"] = "v1 +-./:=_";
    options.Metadata["key1"] = "value1";
    options.Metadata["key2"] = "value2";
    options.AccessTier = Blobs::Models::AccessTier::Cool;
    auto operation = blobClient->StartCopyFromUri(blockBlobClient.GetUrl(), options);
    operation.PollUntilDone(std::chrono::seconds(1));
    EXPECT_EQ(blobClient->GetTags().Value, options.Tags);
    auto properties = blobClient->GetProperties().Value;
    EXPECT_EQ(properties.Metadata, options.Metadata);
    EXPECT_EQ(properties.AccessTier.Value(), options.AccessTier.Value());

    Blobs::CopyBlobFromUriOptions options2;
    options2.Tags = options.Tags;
    options2.Metadata = options.Metadata;
    options2.AccessTier = options.AccessTier;
    blobClient->CopyFromUri(blockBlobClient.GetUrl() + GetSas(), options2);
    EXPECT_EQ(blobClient->GetTags().Value, options2.Tags);
    properties = blobClient->GetProperties().Value;
    EXPECT_EQ(properties.Metadata, options2.Metadata);
    EXPECT_EQ(properties.AccessTier.Value(), options2.AccessTier.Value());
  }

  TEST_F(BlockBlobClientTest, SnapShotVersions)
  {

    auto const testName(GetTestName());
    auto blockBlobClient = GetBlockBlobClient(testName);
    UploadBlockBlob(8_MB);

    auto res = blockBlobClient.CreateSnapshot();
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderRequestId).empty());
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderDate).empty());
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderXMsVersion).empty());
    EXPECT_TRUE(res.Value.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(res.Value.LastModified));
    EXPECT_FALSE(res.Value.Snapshot.empty());
    EXPECT_TRUE(res.Value.VersionId.HasValue());
    EXPECT_FALSE(res.Value.VersionId.Value().empty());
    auto snapshotClient = blockBlobClient.WithSnapshot(res.Value.Snapshot);
    EXPECT_EQ(ReadBodyStream(snapshotClient.Download().Value.BodyStream), m_blobContent);
    EXPECT_EQ(snapshotClient.GetProperties().Value.Metadata, m_blobUploadOptions.Metadata);
    EXPECT_TRUE(snapshotClient.GetProperties().Value.IsServerEncrypted);
    auto versionClient = blockBlobClient.WithVersionId(res.Value.VersionId.Value());
    EXPECT_EQ(ReadBodyStream(versionClient.Download().Value.BodyStream), m_blobContent);
    EXPECT_EQ(versionClient.GetProperties().Value.Metadata, m_blobUploadOptions.Metadata);
    EXPECT_TRUE(versionClient.GetProperties().Value.IsServerEncrypted);
    auto emptyContent = Azure::Core::IO::MemoryBodyStream(nullptr, 0);
    EXPECT_THROW(snapshotClient.Upload(emptyContent), StorageException);
    EXPECT_THROW(snapshotClient.SetMetadata({}), StorageException);
    EXPECT_NO_THROW(snapshotClient.SetAccessTier(Azure::Storage::Blobs::Models::AccessTier::Cool));
    EXPECT_THROW(
        snapshotClient.SetHttpHeaders(Azure::Storage::Blobs::Models::BlobHttpHeaders()),
        StorageException);
    EXPECT_THROW(versionClient.Upload(emptyContent), StorageException);
    EXPECT_THROW(versionClient.SetMetadata({}), StorageException);
    EXPECT_NO_THROW(versionClient.SetAccessTier(Azure::Storage::Blobs::Models::AccessTier::Cool));
    EXPECT_THROW(
        versionClient.SetHttpHeaders(Azure::Storage::Blobs::Models::BlobHttpHeaders()),
        StorageException);

    Azure::Storage::Blobs::CreateBlobSnapshotOptions options;
    options.Metadata = {{"snapshotkey1", "snapshotvalue1"}, {"snapshotkey2", "SNAPSHOTVALUE2"}};
    res = blockBlobClient.CreateSnapshot(options);
    EXPECT_FALSE(res.Value.Snapshot.empty());
    auto snapshotClient2 = blockBlobClient.WithSnapshot(res.Value.Snapshot);
    EXPECT_EQ(snapshotClient2.GetProperties().Value.Metadata, options.Metadata);

    EXPECT_NO_THROW(snapshotClient.Delete());
    EXPECT_NO_THROW(snapshotClient2.Delete());
    EXPECT_NO_THROW(versionClient.Delete());
    EXPECT_NO_THROW(blockBlobClient.GetProperties());
  }

  TEST_F(BlockBlobClientTest, IsCurrentVersion)
  {
    auto const testName(GetTestName());
    auto blobClient = GetBlockBlobClient(testName);

    std::vector<uint8_t> emptyContent;
    blobClient.UploadFrom(emptyContent.data(), emptyContent.size());

    auto properties = blobClient.GetProperties().Value;
    ASSERT_TRUE(properties.VersionId.HasValue());
    ASSERT_TRUE(properties.IsCurrentVersion.HasValue());
    EXPECT_TRUE(properties.IsCurrentVersion.Value());

    auto downloadResponse = blobClient.Download();
    ASSERT_TRUE(downloadResponse.Value.Details.VersionId.HasValue());
    ASSERT_TRUE(downloadResponse.Value.Details.IsCurrentVersion.HasValue());
    EXPECT_TRUE(downloadResponse.Value.Details.IsCurrentVersion.Value());

    std::string version1 = properties.VersionId.Value();

    blobClient.CreateSnapshot();

    properties = blobClient.GetProperties().Value;
    ASSERT_TRUE(properties.VersionId.HasValue());
    ASSERT_TRUE(properties.IsCurrentVersion.HasValue());
    EXPECT_TRUE(properties.IsCurrentVersion.Value());
    std::string latestVersion = properties.VersionId.Value();
    EXPECT_NE(version1, properties.VersionId.Value());

    auto versionClient = blobClient.WithVersionId(version1);
    properties = versionClient.GetProperties().Value;
    ASSERT_TRUE(properties.VersionId.HasValue());
    ASSERT_TRUE(properties.IsCurrentVersion.HasValue());
    EXPECT_FALSE(properties.IsCurrentVersion.Value());
    EXPECT_EQ(version1, properties.VersionId.Value());
    downloadResponse = versionClient.Download();
    ASSERT_TRUE(downloadResponse.Value.Details.VersionId.HasValue());
    ASSERT_TRUE(downloadResponse.Value.Details.IsCurrentVersion.HasValue());
    EXPECT_FALSE(downloadResponse.Value.Details.IsCurrentVersion.Value());
    EXPECT_EQ(version1, downloadResponse.Value.Details.VersionId.Value());

    auto blobItem = GetBlobItem(testName, Blobs::Models::ListBlobsIncludeFlags::Versions);
    ASSERT_TRUE(blobItem.VersionId.HasValue());
    ASSERT_TRUE(blobItem.IsCurrentVersion.HasValue());
    if (blobItem.VersionId.Value() == latestVersion)
    {
      EXPECT_TRUE(blobItem.IsCurrentVersion.Value());
    }
    else
    {
      EXPECT_FALSE(blobItem.IsCurrentVersion.Value());
    }
  }

  TEST_F(BlockBlobClientTest, Properties)
  {
    auto const testName(GetTestName());
    auto blockBlobClient = GetBlockBlobClient(testName);

    auto blobContent
        = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    blockBlobClient.Upload(blobContent);
    blockBlobClient.SetMetadata(m_blobUploadOptions.Metadata);
    blockBlobClient.SetAccessTier(Azure::Storage::Blobs::Models::AccessTier::Cool);
    blockBlobClient.SetHttpHeaders(m_blobUploadOptions.HttpHeaders);

    auto res = blockBlobClient.GetProperties();
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderRequestId).empty());
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderDate).empty());
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderXMsVersion).empty());
    EXPECT_TRUE(res.Value.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(res.Value.LastModified));
    EXPECT_TRUE(IsValidTime(res.Value.CreatedOn));
    EXPECT_EQ(res.Value.Metadata, m_blobUploadOptions.Metadata);
    EXPECT_EQ(res.Value.BlobSize, static_cast<int64_t>(m_blobContent.size()));
    EXPECT_EQ(res.Value.HttpHeaders, m_blobUploadOptions.HttpHeaders);
    EXPECT_EQ(res.Value.HttpHeaders.ContentHash.Algorithm, Storage::HashAlgorithm::Md5);
    EXPECT_EQ(res.Value.AccessTier.Value(), Azure::Storage::Blobs::Models::AccessTier::Cool);
    EXPECT_TRUE(IsValidTime(res.Value.AccessTierChangedOn.Value()));
  }

  TEST_F(BlockBlobClientTest, StageBlock)
  {
    auto const testName(GetTestName());
    auto client = GetBlockBlobClient(testName);
    UploadBlockBlob(8_MB);

    const std::string blockId1 = Base64EncodeText("0");
    const std::string blockId2 = Base64EncodeText("1");
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(),
        m_containerName,
        testName + "extra",
        InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>());
    std::vector<uint8_t> block1Content;
    block1Content.resize(100);
    RandomBuffer(reinterpret_cast<char*>(&block1Content[0]), block1Content.size());
    auto blockContent
        = Azure::Core::IO::MemoryBodyStream(block1Content.data(), block1Content.size());
    blockBlobClient.StageBlock(blockId1, blockContent);
    Azure::Storage::Blobs::CommitBlockListOptions options;
    options.HttpHeaders = m_blobUploadOptions.HttpHeaders;
    options.Metadata = m_blobUploadOptions.Metadata;
    auto blobContentInfo = blockBlobClient.CommitBlockList({blockId1}, options);
    EXPECT_TRUE(blobContentInfo.Value.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(blobContentInfo.Value.LastModified));
    EXPECT_TRUE(blobContentInfo.Value.VersionId.HasValue());
    EXPECT_FALSE(blobContentInfo.Value.VersionId.Value().empty());
    auto res = blockBlobClient.GetBlockList();
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderRequestId).empty());
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderDate).empty());
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderXMsVersion).empty());
    EXPECT_TRUE(res.Value.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(res.Value.LastModified));
    EXPECT_EQ(res.Value.BlobSize, static_cast<int64_t>(block1Content.size()));
    ASSERT_FALSE(res.Value.CommittedBlocks.empty());
    EXPECT_EQ(res.Value.CommittedBlocks[0].Name, blockId1);
    EXPECT_EQ(res.Value.CommittedBlocks[0].Size, static_cast<int64_t>(block1Content.size()));
    EXPECT_TRUE(res.Value.UncommittedBlocks.empty());

    blockBlobClient.StageBlockFromUri(blockId2, client.GetUrl() + GetSas());
    Blobs::GetBlockListOptions options2;
    options2.ListType = Blobs::Models::BlockListType::All;
    res = blockBlobClient.GetBlockList(options2);
    EXPECT_EQ(res.Value.BlobSize, static_cast<int64_t>(block1Content.size()));
    ASSERT_FALSE(res.Value.UncommittedBlocks.empty());
    EXPECT_EQ(res.Value.UncommittedBlocks[0].Name, blockId2);
    EXPECT_EQ(res.Value.UncommittedBlocks[0].Size, static_cast<int64_t>(m_blobContent.size()));

    blockBlobClient.CommitBlockList({blockId1, blockId2});
    res = blockBlobClient.GetBlockList(options2);
    EXPECT_EQ(
        res.Value.BlobSize, static_cast<int64_t>(block1Content.size() + m_blobContent.size()));
    EXPECT_TRUE(res.Value.UncommittedBlocks.empty());
  }

  namespace {

    struct BlobConcurrentDownloadParameter
    {
      int Concurrency;
      int64_t DownloadSize;
      Azure::Nullable<int64_t> Offset = {};
      Azure::Nullable<int64_t> Length = {};
      Azure::Nullable<int64_t> InitialChunkSize = {};
      Azure::Nullable<int64_t> ChunkSize = {};
    };

    struct BlobConcurrentUploadParameter
    {
      int Concurrency;
      int64_t Size;
    };

    class DownloadBlockBlob
        : public BlockBlobClientTest,
          public ::testing::WithParamInterface<BlobConcurrentDownloadParameter> {
    };

    class UploadBlockBlob : public BlockBlobClientTest,
                            public ::testing::WithParamInterface<BlobConcurrentUploadParameter> {
    };

#define APPEND_IF_NOT_NULL(value, suffix, destination) \
  if (value) \
  { \
    destination.append(suffix + std::to_string(value.Value())); \
  }

    std::string GetDownloadSuffix(const testing::TestParamInfo<DownloadBlockBlob::ParamType>& info)
    {
      // Can't use empty spaces or underscores (_) as per google test documentation
      // http://google.github.io/googletest/advanced.html#specifying-names-for-value-parameterized-test-parameters
      auto const& p = info.param;
      std::string suffix(
          "c" + std::to_string(p.Concurrency) + "s" + std::to_string(p.DownloadSize));
      APPEND_IF_NOT_NULL(p.Offset, "o", suffix)
      APPEND_IF_NOT_NULL(p.Length, "l", suffix)
      APPEND_IF_NOT_NULL(p.InitialChunkSize, "ics", suffix)
      APPEND_IF_NOT_NULL(p.ChunkSize, "cs", suffix)
      return suffix;
    }

    std::string GetUploadSuffix(const testing::TestParamInfo<UploadBlockBlob::ParamType>& info)
    {
      // Can't use empty spaces or underscores (_) as per google test documentation
      // http://google.github.io/googletest/advanced.html#specifying-names-for-value-parameterized-test-parameters
      auto const& p = info.param;
      std::string suffix("c" + std::to_string(p.Concurrency) + "s" + std::to_string(p.Size));
      return suffix;
    }

    std::vector<BlobConcurrentDownloadParameter> GetDownloadParameters(int64_t const blobSize)
    {
      std::vector<BlobConcurrentDownloadParameter> testParametes;
      for (int c : {1, 2, 4})
      {
        // download whole blob
        testParametes.emplace_back(BlobConcurrentDownloadParameter({c, blobSize}));
        testParametes.emplace_back(BlobConcurrentDownloadParameter({c, blobSize, 0}));
        testParametes.emplace_back(BlobConcurrentDownloadParameter({c, blobSize, 0, blobSize}));
        testParametes.emplace_back(BlobConcurrentDownloadParameter({c, blobSize, 0, blobSize * 2}));
        testParametes.emplace_back(BlobConcurrentDownloadParameter({c, blobSize * 2}));

        // Do offset
        testParametes.emplace_back(BlobConcurrentDownloadParameter({c, blobSize, 0, 1}));
        testParametes.emplace_back(BlobConcurrentDownloadParameter({c, blobSize, 1, 1}));
        testParametes.emplace_back(BlobConcurrentDownloadParameter({c, blobSize, blobSize - 1, 1}));
        testParametes.emplace_back(BlobConcurrentDownloadParameter({c, blobSize, blobSize - 1, 2}));
        testParametes.emplace_back(BlobConcurrentDownloadParameter({c, blobSize, blobSize, 1}));
        testParametes.emplace_back(BlobConcurrentDownloadParameter({c, blobSize, blobSize + 1, 2}));

        // // initial chunk size
        testParametes.emplace_back(
            BlobConcurrentDownloadParameter({c, blobSize, 0, 1024, 512, 1024}));
        testParametes.emplace_back(
            BlobConcurrentDownloadParameter({c, blobSize, 0, 1024, 1024, 1024}));
        testParametes.emplace_back(
            BlobConcurrentDownloadParameter({c, blobSize, 0, 1024, 2048, 1024}));
      }
      return testParametes;
    }

    std::vector<BlobConcurrentUploadParameter> GetUploadParameters()
    {
      std::vector<BlobConcurrentUploadParameter> testParametes;
      for (int c : {1, 2, 4})
      {
        for (int64_t l :
             {0ULL, 1ULL, 2ULL, 2_KB, 4_KB, 999_KB, 1_MB, 2_MB - 1, 3_MB, 5_MB, 8_MB - 1234, 8_MB})
        {
          testParametes.emplace_back(BlobConcurrentUploadParameter({c, l}));
        }
      }
      return testParametes;
    }

  } // namespace

  TEST_P(DownloadBlockBlob, downloadToBuffer)
  {
    BlobConcurrentDownloadParameter const& p(GetParam());
    auto const testName(GetTestName(true));
    auto client = GetBlockBlobClient(testName);
    UploadBlockBlob(8_MB);

    std::vector<uint8_t> downloadBuffer;
    std::vector<uint8_t> expectedData = m_blobContent;
    int64_t blobSize = m_blobContent.size();
    int64_t actualDownloadSize = std::min(p.DownloadSize, blobSize);
    if (p.Offset.HasValue() && p.Length.HasValue())
    {
      actualDownloadSize = std::min(p.Length.Value(), blobSize - p.Offset.Value());
      if (actualDownloadSize >= 0)
      {
        expectedData.assign(
            m_blobContent.begin() + static_cast<ptrdiff_t>(p.Offset.Value()),
            m_blobContent.begin() + static_cast<ptrdiff_t>(p.Offset.Value() + actualDownloadSize));
      }
      else
      {
        expectedData.clear();
      }
    }
    else if (p.Offset.HasValue())
    {
      actualDownloadSize = blobSize - p.Offset.Value();
      if (actualDownloadSize >= 0)
      {
        expectedData.assign(
            m_blobContent.begin() + static_cast<ptrdiff_t>(p.Offset.Value()), m_blobContent.end());
      }
      else
      {
        expectedData.clear();
      }
    }
    downloadBuffer.resize(static_cast<size_t>(p.DownloadSize), '\x00');
    Blobs::DownloadBlobToOptions options;
    options.TransferOptions.Concurrency = p.Concurrency;
    if (p.Offset.HasValue() || p.Length.HasValue())
    {
      options.Range = Core::Http::HttpRange();
      options.Range.Value().Offset = p.Offset.Value();
      options.Range.Value().Length = p.Length;
    }
    if (p.InitialChunkSize.HasValue())
    {
      options.TransferOptions.InitialChunkSize = p.InitialChunkSize.Value();
    }
    if (p.ChunkSize.HasValue())
    {
      options.TransferOptions.ChunkSize = p.ChunkSize.Value();
    }
    if (actualDownloadSize > 0)
    {
      auto res = client.DownloadTo(downloadBuffer.data(), downloadBuffer.size(), options);
      EXPECT_EQ(res.Value.BlobSize, blobSize);
      EXPECT_EQ(res.Value.ContentRange.Length.Value(), actualDownloadSize);
      EXPECT_EQ(res.Value.ContentRange.Offset, p.Offset.HasValue() ? p.Offset.Value() : 0);
      downloadBuffer.resize(static_cast<size_t>(res.Value.ContentRange.Length.Value()));
      EXPECT_EQ(downloadBuffer, expectedData);
    }
    else
    {
      EXPECT_THROW(
          client.DownloadTo(downloadBuffer.data(), downloadBuffer.size(), options),
          StorageException);
    }
  }

  TEST_P(DownloadBlockBlob, downloadToFile)
  {
    BlobConcurrentDownloadParameter const& p(GetParam());
    auto const testName(GetTestName(true));
    auto client = GetBlockBlobClient(testName);
    UploadBlockBlob(8_MB);

    std::string tempFilename(testName);
    std::vector<uint8_t> expectedData = m_blobContent;
    int64_t blobSize = m_blobContent.size();
    int64_t actualDownloadSize = std::min(p.DownloadSize, blobSize);
    if (p.Offset.HasValue() && p.Length.HasValue())
    {
      actualDownloadSize = std::min(p.Length.Value(), blobSize - p.Offset.Value());
      if (actualDownloadSize >= 0)
      {
        expectedData.assign(
            m_blobContent.begin() + static_cast<ptrdiff_t>(p.Offset.Value()),
            m_blobContent.begin() + static_cast<ptrdiff_t>(p.Offset.Value() + actualDownloadSize));
      }
      else
      {
        expectedData.clear();
      }
    }
    else if (p.Offset.HasValue())
    {
      actualDownloadSize = blobSize - p.Offset.Value();
      if (actualDownloadSize >= 0)
      {
        expectedData.assign(
            m_blobContent.begin() + static_cast<ptrdiff_t>(p.Offset.Value()), m_blobContent.end());
      }
      else
      {
        expectedData.clear();
      }
    }
    Blobs::DownloadBlobToOptions options;
    options.TransferOptions.Concurrency = p.Concurrency;
    if (p.Offset.HasValue() || p.Length.HasValue())
    {
      options.Range = Core::Http::HttpRange();
      options.Range.Value().Offset = p.Offset.Value();
      options.Range.Value().Length = p.Length;
    }
    if (p.InitialChunkSize.HasValue())
    {
      options.TransferOptions.InitialChunkSize = p.InitialChunkSize.Value();
    }
    if (p.ChunkSize.HasValue())
    {
      options.TransferOptions.ChunkSize = p.ChunkSize.Value();
    }
    if (actualDownloadSize > 0)
    {
      auto res = client.DownloadTo(tempFilename, options);
      EXPECT_EQ(res.Value.BlobSize, blobSize);
      EXPECT_EQ(res.Value.ContentRange.Length.Value(), actualDownloadSize);
      EXPECT_EQ(res.Value.ContentRange.Offset, p.Offset.HasValue() ? p.Offset.Value() : 0);
      EXPECT_EQ(ReadFile(tempFilename), expectedData);
    }
    else
    {
      EXPECT_THROW(client.DownloadTo(tempFilename, options), StorageException);
    }
    DeleteFile(tempFilename);
  }

  INSTANTIATE_TEST_SUITE_P(
      withParam,
      DownloadBlockBlob,
      testing::ValuesIn(GetDownloadParameters(8_MB)),
      GetDownloadSuffix);

  TEST_F(BlockBlobClientTest, ConcurrentDownload_LIVEONLY_)
  {
    CHECK_SKIP_TEST();
    auto const testName(GetTestName());
    auto client = GetBlockBlobClient(testName);
    UploadBlockBlob(8_MB);

    auto testDownloadToBuffer = [&](int concurrency,
                                    int64_t downloadSize,
                                    Azure::Nullable<int64_t> offset = {},
                                    Azure::Nullable<int64_t> length = {},
                                    Azure::Nullable<int64_t> initialChunkSize = {},
                                    Azure::Nullable<int64_t> chunkSize = {}) {
      std::vector<uint8_t> downloadBuffer;
      std::vector<uint8_t> expectedData = m_blobContent;
      int64_t blobSize = m_blobContent.size();
      int64_t actualDownloadSize = std::min(downloadSize, blobSize);
      if (offset.HasValue() && length.HasValue())
      {
        actualDownloadSize = std::min(length.Value(), blobSize - offset.Value());
        if (actualDownloadSize >= 0)
        {
          expectedData.assign(
              m_blobContent.begin() + static_cast<ptrdiff_t>(offset.Value()),
              m_blobContent.begin() + static_cast<ptrdiff_t>(offset.Value() + actualDownloadSize));
        }
        else
        {
          expectedData.clear();
        }
      }
      else if (offset.HasValue())
      {
        actualDownloadSize = blobSize - offset.Value();
        if (actualDownloadSize >= 0)
        {
          expectedData.assign(
              m_blobContent.begin() + static_cast<ptrdiff_t>(offset.Value()), m_blobContent.end());
        }
        else
        {
          expectedData.clear();
        }
      }
      downloadBuffer.resize(static_cast<size_t>(downloadSize), '\x00');
      Blobs::DownloadBlobToOptions options;
      options.TransferOptions.Concurrency = concurrency;
      if (offset.HasValue() || length.HasValue())
      {
        options.Range = Core::Http::HttpRange();
        options.Range.Value().Offset = offset.Value();
        options.Range.Value().Length = length;
      }
      if (initialChunkSize.HasValue())
      {
        options.TransferOptions.InitialChunkSize = initialChunkSize.Value();
      }
      if (chunkSize.HasValue())
      {
        options.TransferOptions.ChunkSize = chunkSize.Value();
      }
      if (actualDownloadSize > 0)
      {
        auto res = client.DownloadTo(downloadBuffer.data(), downloadBuffer.size(), options);
        EXPECT_EQ(res.Value.BlobSize, blobSize);
        EXPECT_EQ(res.Value.ContentRange.Length.Value(), actualDownloadSize);
        EXPECT_EQ(res.Value.ContentRange.Offset, offset.HasValue() ? offset.Value() : 0);
        downloadBuffer.resize(static_cast<size_t>(res.Value.ContentRange.Length.Value()));
        EXPECT_EQ(downloadBuffer, expectedData);
      }
      else
      {
        EXPECT_THROW(
            client.DownloadTo(downloadBuffer.data(), downloadBuffer.size(), options),
            StorageException);
      }
    };

    auto testDownloadToFile = [&](int concurrency,
                                  int64_t downloadSize,
                                  Azure::Nullable<int64_t> offset = {},
                                  Azure::Nullable<int64_t> length = {},
                                  Azure::Nullable<int64_t> initialChunkSize = {},
                                  Azure::Nullable<int64_t> chunkSize = {}) {
      std::string tempFilename = testName + "file" + std::to_string(concurrency);
      if (offset)
      {
        tempFilename.append(std::to_string(offset.Value()));
      }
      std::vector<uint8_t> expectedData = m_blobContent;
      int64_t blobSize = m_blobContent.size();
      int64_t actualDownloadSize = std::min(downloadSize, blobSize);
      if (offset.HasValue() && length.HasValue())
      {
        actualDownloadSize = std::min(length.Value(), blobSize - offset.Value());
        if (actualDownloadSize >= 0)
        {
          expectedData.assign(
              m_blobContent.begin() + static_cast<ptrdiff_t>(offset.Value()),
              m_blobContent.begin() + static_cast<ptrdiff_t>(offset.Value() + actualDownloadSize));
        }
        else
        {
          expectedData.clear();
        }
      }
      else if (offset.HasValue())
      {
        actualDownloadSize = blobSize - offset.Value();
        if (actualDownloadSize >= 0)
        {
          expectedData.assign(
              m_blobContent.begin() + static_cast<ptrdiff_t>(offset.Value()), m_blobContent.end());
        }
        else
        {
          expectedData.clear();
        }
      }
      Blobs::DownloadBlobToOptions options;
      options.TransferOptions.Concurrency = concurrency;
      if (offset.HasValue() || length.HasValue())
      {
        options.Range = Core::Http::HttpRange();
        options.Range.Value().Offset = offset.Value();
        options.Range.Value().Length = length;
      }
      if (initialChunkSize.HasValue())
      {
        options.TransferOptions.InitialChunkSize = initialChunkSize.Value();
      }
      if (chunkSize.HasValue())
      {
        options.TransferOptions.ChunkSize = chunkSize.Value();
      }
      if (actualDownloadSize > 0)
      {
        auto res = client.DownloadTo(tempFilename, options);
        EXPECT_EQ(res.Value.BlobSize, blobSize);
        EXPECT_EQ(res.Value.ContentRange.Length.Value(), actualDownloadSize);
        EXPECT_EQ(res.Value.ContentRange.Offset, offset.HasValue() ? offset.Value() : 0);
        EXPECT_EQ(ReadFile(tempFilename), expectedData);
      }
      else
      {
        EXPECT_THROW(client.DownloadTo(tempFilename, options), StorageException);
      }
      DeleteFile(tempFilename);
    };

    const int64_t blobSize = m_blobContent.size();
    std::vector<std::future<void>> futures;
    for (int c : {1, 2, 4})
    {
      // random range
      std::mt19937_64 random_generator(std::random_device{}());
      for (int i = 0; i < 16; ++i)
      {
        std::uniform_int_distribution<int64_t> offsetDistribution(0, m_blobContent.size() - 1);
        int64_t offset = offsetDistribution(random_generator);
        std::uniform_int_distribution<int64_t> lengthDistribution(1, 64_KB);
        int64_t length = lengthDistribution(random_generator);
        futures.emplace_back(std::async(
            std::launch::async, testDownloadToBuffer, c, blobSize, offset, length, 4_KB, 4_KB));
        futures.emplace_back(std::async(
            std::launch::async, testDownloadToFile, c, blobSize, offset, length, 4_KB, 4_KB));
      }

      // // buffer not big enough
      Blobs::DownloadBlobToOptions options;
      options.TransferOptions.Concurrency = c;
      options.Range = Core::Http::HttpRange();
      options.Range.Value().Offset = 1;
      for (int64_t length : {1ULL, 2ULL, 4_KB, 5_KB, 8_KB, 11_KB, 20_KB})
      {
        std::vector<uint8_t> downloadBuffer;
        downloadBuffer.resize(static_cast<size_t>(length - 1));
        options.Range.Value().Length = length;
        EXPECT_THROW(
            client.DownloadTo(downloadBuffer.data(), static_cast<size_t>(length - 1), options),
            std::runtime_error);
      }
    }
    for (auto& f : futures)
    {
      f.get();
    }
  }

  TEST_F(BlockBlobClientTest, ConcurrentUploadFromNonExistingFile)
  {
    auto const testName(GetTestName());
    auto blockBlobClient = GetBlockBlobClient(testName);

    std::string emptyFilename(testName);
    EXPECT_THROW(blockBlobClient.UploadFrom(emptyFilename), std::runtime_error);
    EXPECT_THROW(blockBlobClient.Delete(), StorageException);
  }

  TEST_F(BlockBlobClientTest, ConcurrentDownloadNonExistingBlob)
  {
    auto const testName(GetTestName());
    auto blockBlobClient = GetBlockBlobClient(testName);

    std::vector<uint8_t> blobContent(100);
    std::string tempFilename(testName);

    EXPECT_THROW(
        blockBlobClient.DownloadTo(blobContent.data(), blobContent.size()), StorageException);
    EXPECT_THROW(blockBlobClient.DownloadTo(tempFilename), StorageException);
    DeleteFile(tempFilename);
  }

  TEST_F(BlockBlobClientTest, ConcurrentUploadEmptyBlob)
  {
    auto const testName(GetTestName());
    auto blockBlobClient = GetBlockBlobClient(testName);

    std::vector<uint8_t> emptyContent;

    blockBlobClient.UploadFrom(emptyContent.data(), emptyContent.size());
    EXPECT_NO_THROW(blockBlobClient.Delete());

    std::string emptyFilename(testName);
    {
      _internal::FileWriter writer(emptyFilename);
    }
    blockBlobClient.UploadFrom(emptyFilename);
    EXPECT_NO_THROW(blockBlobClient.Delete());

    DeleteFile(emptyFilename);
  }

  TEST_F(BlockBlobClientTest, ConcurrentDownloadEmptyBlob)
  {
    auto const testName(GetTestName());
    auto blockBlobClient = GetBlockBlobClient(testName);
    std::string tempFilename(testName);

    std::vector<uint8_t> emptyContent;

    auto blobContent = Azure::Core::IO::MemoryBodyStream(emptyContent.data(), emptyContent.size());
    blockBlobClient.Upload(blobContent);
    blockBlobClient.SetHttpHeaders(m_blobUploadOptions.HttpHeaders);
    blockBlobClient.SetMetadata(m_blobUploadOptions.Metadata);

    auto res = blockBlobClient.DownloadTo(emptyContent.data(), 0);
    EXPECT_EQ(res.Value.BlobSize, 0);
    EXPECT_EQ(res.Value.ContentRange.Length.Value(), 0);
    EXPECT_TRUE(res.Value.Details.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(res.Value.Details.LastModified));
    EXPECT_EQ(res.Value.Details.HttpHeaders, m_blobUploadOptions.HttpHeaders);
    EXPECT_EQ(res.Value.Details.Metadata, m_blobUploadOptions.Metadata);
    EXPECT_EQ(res.Value.BlobType, Azure::Storage::Blobs::Models::BlobType::BlockBlob);
    res = blockBlobClient.DownloadTo(tempFilename);
    EXPECT_EQ(res.Value.BlobSize, 0);
    EXPECT_EQ(res.Value.ContentRange.Length.Value(), 0);
    EXPECT_TRUE(res.Value.Details.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(res.Value.Details.LastModified));
    EXPECT_EQ(res.Value.Details.HttpHeaders, m_blobUploadOptions.HttpHeaders);
    EXPECT_EQ(res.Value.Details.Metadata, m_blobUploadOptions.Metadata);
    EXPECT_EQ(res.Value.BlobType, Azure::Storage::Blobs::Models::BlobType::BlockBlob);
    EXPECT_TRUE(ReadFile(tempFilename).empty());
    DeleteFile(tempFilename);

    res = blockBlobClient.DownloadTo(emptyContent.data(), static_cast<size_t>(8_MB));
    EXPECT_EQ(res.Value.BlobSize, 0);
    EXPECT_EQ(res.Value.ContentRange.Length.Value(), 0);
    EXPECT_TRUE(res.Value.Details.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(res.Value.Details.LastModified));
    EXPECT_EQ(res.Value.Details.HttpHeaders, m_blobUploadOptions.HttpHeaders);
    EXPECT_EQ(res.Value.Details.Metadata, m_blobUploadOptions.Metadata);
    EXPECT_EQ(res.Value.BlobType, Azure::Storage::Blobs::Models::BlobType::BlockBlob);
    res = blockBlobClient.DownloadTo(tempFilename);
    EXPECT_EQ(res.Value.BlobSize, 0);
    EXPECT_EQ(res.Value.ContentRange.Length.Value(), 0);
    EXPECT_TRUE(res.Value.Details.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(res.Value.Details.LastModified));
    EXPECT_EQ(res.Value.Details.HttpHeaders, m_blobUploadOptions.HttpHeaders);
    EXPECT_EQ(res.Value.Details.Metadata, m_blobUploadOptions.Metadata);
    EXPECT_EQ(res.Value.BlobType, Azure::Storage::Blobs::Models::BlobType::BlockBlob);
    EXPECT_TRUE(ReadFile(tempFilename).empty());
    DeleteFile(tempFilename);

    for (int c : {1, 2})
    {
      Azure::Storage::Blobs::DownloadBlobToOptions options;
      options.TransferOptions.InitialChunkSize = 10;
      options.TransferOptions.ChunkSize = 10;
      options.TransferOptions.Concurrency = c;

      res = blockBlobClient.DownloadTo(emptyContent.data(), static_cast<size_t>(8_MB), options);
      EXPECT_EQ(res.Value.BlobSize, 0);
      EXPECT_EQ(res.Value.ContentRange.Length.Value(), 0);
      EXPECT_TRUE(res.Value.Details.ETag.HasValue());
      EXPECT_TRUE(IsValidTime(res.Value.Details.LastModified));
      EXPECT_EQ(res.Value.Details.HttpHeaders, m_blobUploadOptions.HttpHeaders);
      EXPECT_EQ(res.Value.Details.Metadata, m_blobUploadOptions.Metadata);
      EXPECT_EQ(res.Value.BlobType, Azure::Storage::Blobs::Models::BlobType::BlockBlob);
      res = blockBlobClient.DownloadTo(tempFilename, options);
      EXPECT_EQ(res.Value.BlobSize, 0);
      EXPECT_EQ(res.Value.ContentRange.Length.Value(), 0);
      EXPECT_TRUE(res.Value.Details.ETag.HasValue());
      EXPECT_TRUE(IsValidTime(res.Value.Details.LastModified));
      EXPECT_EQ(res.Value.Details.HttpHeaders, m_blobUploadOptions.HttpHeaders);
      EXPECT_EQ(res.Value.Details.Metadata, m_blobUploadOptions.Metadata);
      EXPECT_EQ(res.Value.BlobType, Azure::Storage::Blobs::Models::BlobType::BlockBlob);
      EXPECT_TRUE(ReadFile(tempFilename).empty());
      DeleteFile(tempFilename);

      options.Range = Core::Http::HttpRange();
      options.Range.Value().Offset = 0;
      EXPECT_THROW(
          blockBlobClient.DownloadTo(emptyContent.data(), static_cast<size_t>(8_MB), options),
          StorageException);
      EXPECT_THROW(blockBlobClient.DownloadTo(tempFilename, options), StorageException);

      options.Range.Value().Offset = 1;
      EXPECT_THROW(
          blockBlobClient.DownloadTo(emptyContent.data(), static_cast<size_t>(8_MB), options),
          StorageException);
      EXPECT_THROW(blockBlobClient.DownloadTo(tempFilename, options), StorageException);

      options.Range.Value().Offset = 0;
      options.Range.Value().Length = 1;
      EXPECT_THROW(
          blockBlobClient.DownloadTo(emptyContent.data(), static_cast<size_t>(8_MB), options),
          StorageException);
      EXPECT_THROW(blockBlobClient.DownloadTo(tempFilename, options), StorageException);

      options.Range.Value().Offset = 100;
      options.Range.Value().Length = 100;
      EXPECT_THROW(
          blockBlobClient.DownloadTo(emptyContent.data(), static_cast<size_t>(8_MB), options),
          StorageException);
      EXPECT_THROW(blockBlobClient.DownloadTo(tempFilename, options), StorageException);
      DeleteFile(tempFilename);
    }
  }

  TEST_P(UploadBlockBlob, fromBuffer)
  {
    auto const testName(GetTestName());
    auto blockBlobClient = GetBlockBlobClient(testName);
    SetOptions();
    UploadBlockBlob::ParamType const& p(GetParam());
    auto const blobSize = p.Size;
    std::vector<uint8_t> blobContent(static_cast<size_t>(8_MB), 'x');

    Azure::Storage::Blobs::UploadBlockBlobFromOptions options;
    options.TransferOptions.ChunkSize = 1_MB;
    options.TransferOptions.Concurrency = p.Concurrency;
    options.HttpHeaders = m_blobUploadOptions.HttpHeaders;
    options.HttpHeaders.ContentHash.Value.clear();
    options.Metadata = m_blobUploadOptions.Metadata;
    options.AccessTier = m_blobUploadOptions.AccessTier;
    auto res
        = blockBlobClient.UploadFrom(blobContent.data(), static_cast<size_t>(blobSize), options);
    EXPECT_TRUE(res.Value.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(res.Value.LastModified));
    auto properties = blockBlobClient.GetProperties().Value;
    properties.HttpHeaders.ContentHash.Value.clear();
    EXPECT_EQ(properties.BlobSize, blobSize);
    EXPECT_EQ(properties.HttpHeaders, options.HttpHeaders);
    EXPECT_EQ(properties.Metadata, options.Metadata);
    EXPECT_EQ(properties.AccessTier.Value(), options.AccessTier.Value());
    EXPECT_EQ(properties.ETag, res.Value.ETag);
    EXPECT_EQ(properties.LastModified, res.Value.LastModified);
    std::vector<uint8_t> downloadContent(static_cast<size_t>(blobSize), '\x00');
    blockBlobClient.DownloadTo(downloadContent.data(), static_cast<size_t>(blobSize));
    EXPECT_EQ(
        downloadContent,
        std::vector<uint8_t>(
            blobContent.begin(), blobContent.begin() + static_cast<size_t>(blobSize)));
  }

  TEST_P(UploadBlockBlob, fromFile)
  {
    auto const testName(GetTestName());
    auto blockBlobClient = GetBlockBlobClient(testName);
    SetOptions();
    UploadBlockBlob::ParamType const& p(GetParam());
    auto const blobSize = p.Size;
    std::vector<uint8_t> blobContent(static_cast<size_t>(8_MB), 'x');

    Azure::Storage::Blobs::UploadBlockBlobFromOptions options;
    options.TransferOptions.ChunkSize = 1_MB;
    options.TransferOptions.Concurrency = p.Concurrency;
    options.HttpHeaders = m_blobUploadOptions.HttpHeaders;
    options.HttpHeaders.ContentHash.Value.clear();
    options.Metadata = m_blobUploadOptions.Metadata;
    options.AccessTier = m_blobUploadOptions.AccessTier;

    std::string tempFilename(testName);
    {
      Azure::Storage::_internal::FileWriter fileWriter(tempFilename);
      fileWriter.Write(blobContent.data(), static_cast<size_t>(blobSize), 0);
    }
    auto res = blockBlobClient.UploadFrom(tempFilename, options);
    EXPECT_TRUE(res.Value.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(res.Value.LastModified));
    auto properties = blockBlobClient.GetProperties().Value;
    properties.HttpHeaders.ContentHash.Value.clear();
    EXPECT_EQ(properties.BlobSize, blobSize);
    EXPECT_EQ(properties.HttpHeaders, options.HttpHeaders);
    EXPECT_EQ(properties.Metadata, options.Metadata);
    EXPECT_EQ(properties.AccessTier.Value(), options.AccessTier.Value());
    EXPECT_EQ(properties.ETag, res.Value.ETag);
    EXPECT_EQ(properties.LastModified, res.Value.LastModified);
    std::vector<uint8_t> downloadContent(static_cast<size_t>(blobSize), '\x00');
    blockBlobClient.DownloadTo(downloadContent.data(), static_cast<size_t>(blobSize));
    EXPECT_EQ(
        downloadContent,
        std::vector<uint8_t>(
            blobContent.begin(), blobContent.begin() + static_cast<size_t>(blobSize)));
    DeleteFile(tempFilename);
  }

  INSTANTIATE_TEST_SUITE_P(
      withParam,
      UploadBlockBlob,
      testing::ValuesIn(GetUploadParameters()),
      GetUploadSuffix);

  TEST_F(BlockBlobClientTest, DownloadError)
  {
    auto const testName(GetTestName());
    auto blockBlobClient = GetBlockBlobClient(testName);

    bool exceptionCaught = false;
    try
    {
      blockBlobClient.Download();
    }
    catch (const StorageException& e)
    {
      exceptionCaught = true;
      EXPECT_EQ(e.StatusCode, Azure::Core::Http::HttpStatusCode::NotFound);
      EXPECT_FALSE(e.ReasonPhrase.empty());
      EXPECT_FALSE(e.RequestId.empty());
      EXPECT_FALSE(e.ErrorCode.empty());
      EXPECT_FALSE(e.Message.empty());
      EXPECT_TRUE(e.RawResponse);
    }
    EXPECT_TRUE(exceptionCaught);
  }

  TEST_F(BlockBlobClientTest, DeleteIfExists)
  {
    auto const testName(GetTestName());
    auto blobClient = GetBlockBlobClient(testName);

    auto blobClientWithoutAuth = Azure::Storage::Blobs::BlockBlobClient(
        blobClient.GetUrl(), InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>());
    {
      auto response = blobClient.DeleteIfExists();
      EXPECT_FALSE(response.Value.Deleted);
    }
    std::vector<uint8_t> emptyContent;
    blobClient.UploadFrom(emptyContent.data(), emptyContent.size());
    EXPECT_THROW(blobClientWithoutAuth.DeleteIfExists(), StorageException);
    {
      auto response = blobClient.DeleteIfExists();
      EXPECT_TRUE(response.Value.Deleted);
    }

    blobClient.UploadFrom(emptyContent.data(), emptyContent.size());
    auto snapshot = blobClient.CreateSnapshot().Value.Snapshot;
    auto blobClientWithSnapshot = blobClient.WithSnapshot(snapshot);
    {
      auto response = blobClientWithSnapshot.DeleteIfExists();
      EXPECT_TRUE(response.Value.Deleted);
    }
    {
      auto response = blobClientWithSnapshot.DeleteIfExists();
      EXPECT_FALSE(response.Value.Deleted);
    }
  }

  TEST_F(BlockBlobClientTest, DeleteSnapshots)
  {
    auto const testName(GetTestName());
    auto blobClient = GetBlockBlobClient(testName);

    std::vector<uint8_t> emptyContent;
    blobClient.UploadFrom(emptyContent.data(), emptyContent.size());
    auto s1 = blobClient.CreateSnapshot().Value.Snapshot;
    Blobs::DeleteBlobOptions deleteOptions;
    EXPECT_THROW(blobClient.Delete(deleteOptions), StorageException);
    deleteOptions.DeleteSnapshots = Blobs::Models::DeleteSnapshotsOption::OnlySnapshots;
    EXPECT_NO_THROW(blobClient.Delete(deleteOptions));
    EXPECT_NO_THROW(blobClient.GetProperties());
    EXPECT_THROW(blobClient.WithSnapshot(s1).GetProperties(), StorageException);
    auto s2 = blobClient.CreateSnapshot().Value.Snapshot;
    deleteOptions.DeleteSnapshots = Blobs::Models::DeleteSnapshotsOption::IncludeSnapshots;
    EXPECT_NO_THROW(blobClient.Delete(deleteOptions));
    EXPECT_THROW(blobClient.GetProperties(), StorageException);
    EXPECT_THROW(blobClient.WithSnapshot(s2).GetProperties(), StorageException);
  }

  TEST_F(BlockBlobClientTest, SetTier)
  {
    auto const testName(GetTestName());
    auto blobClient = GetBlockBlobClient(testName);

    std::vector<uint8_t> emptyContent;
    std::string blobName(testName);

    blobClient.UploadFrom(emptyContent.data(), emptyContent.size());

    auto properties = blobClient.GetProperties().Value;
    ASSERT_TRUE(properties.AccessTier.HasValue());
    ASSERT_TRUE(properties.IsAccessTierInferred.HasValue());
    EXPECT_TRUE(properties.IsAccessTierInferred.Value());
    EXPECT_FALSE(properties.AccessTierChangedOn.HasValue());

    auto blobItem = GetBlobItem(blobName);
    ASSERT_TRUE(blobItem.Details.AccessTier.HasValue());
    ASSERT_TRUE(blobItem.Details.IsAccessTierInferred.HasValue());
    EXPECT_TRUE(blobItem.Details.IsAccessTierInferred.Value());
    EXPECT_FALSE(blobItem.Details.AccessTierChangedOn.HasValue());

    // choose a different tier
    auto targetTier = properties.AccessTier.Value() == Blobs::Models::AccessTier::Hot
        ? Blobs::Models::AccessTier::Cool
        : Blobs::Models::AccessTier::Hot;
    blobClient.SetAccessTier(targetTier);

    properties = blobClient.GetProperties().Value;
    ASSERT_TRUE(properties.AccessTier.HasValue());
    ASSERT_TRUE(properties.IsAccessTierInferred.HasValue());
    EXPECT_FALSE(properties.IsAccessTierInferred.Value());
    EXPECT_TRUE(properties.AccessTierChangedOn.HasValue());

    blobItem = GetBlobItem(blobName);
    ASSERT_TRUE(blobItem.Details.AccessTier.HasValue());
    ASSERT_TRUE(blobItem.Details.IsAccessTierInferred.HasValue());
    EXPECT_FALSE(blobItem.Details.IsAccessTierInferred.Value());
    EXPECT_TRUE(blobItem.Details.AccessTierChangedOn.HasValue());

    // set to archive, then rehydrate
    blobClient.SetAccessTier(Blobs::Models::AccessTier::Archive);
    blobClient.SetAccessTier(Blobs::Models::AccessTier::Hot);
    properties = blobClient.GetProperties().Value;
    ASSERT_TRUE(properties.ArchiveStatus.HasValue());
    EXPECT_EQ(
        properties.ArchiveStatus.Value(), Blobs::Models::ArchiveStatus::RehydratePendingToHot);
    ASSERT_TRUE(properties.RehydratePriority.HasValue());
    EXPECT_EQ(properties.RehydratePriority.Value(), Blobs::Models::RehydratePriority::Standard);

    blobItem = GetBlobItem(blobName);
    ASSERT_TRUE(blobItem.Details.ArchiveStatus.HasValue());
    EXPECT_EQ(
        blobItem.Details.ArchiveStatus.Value(),
        Blobs::Models::ArchiveStatus::RehydratePendingToHot);
    ASSERT_TRUE(blobItem.Details.RehydratePriority.HasValue());
    EXPECT_EQ(
        blobItem.Details.RehydratePriority.Value(), Blobs::Models::RehydratePriority::Standard);
  }

  TEST_F(BlockBlobClientTest, SetTierWithLeaseId)
  {
    auto const testName(GetTestName());
    auto blobClient = GetBlockBlobClient(testName);

    std::vector<uint8_t> emptyContent;
    blobClient.UploadFrom(emptyContent.data(), emptyContent.size());

    const std::string leaseId = Blobs::BlobLeaseClient::CreateUniqueLeaseId();
    Blobs::BlobLeaseClient leaseClient(blobClient, leaseId);
    leaseClient.Acquire(std::chrono::seconds(30));

    EXPECT_THROW(blobClient.SetAccessTier(Blobs::Models::AccessTier::Cool), StorageException);

    Blobs::SetBlobAccessTierOptions options;
    options.AccessConditions.LeaseId = leaseId;
    EXPECT_NO_THROW(blobClient.SetAccessTier(Blobs::Models::AccessTier::Cool, options));
  }

  TEST_F(BlockBlobClientTest, UncommittedBlob)
  {
    auto const testName(GetTestName());
    auto blobClient = GetBlockBlobClient(testName);

    const std::string blobName(testName);

    std::vector<uint8_t> buffer(100);
    Azure::Core::IO::MemoryBodyStream stream(buffer.data(), buffer.size());
    blobClient.StageBlock("YWJjZA==", stream);

    Blobs::GetBlockListOptions getBlockListOptions;
    getBlockListOptions.ListType = Blobs::Models::BlockListType::All;
    auto res = blobClient.GetBlockList(getBlockListOptions).Value;
    EXPECT_FALSE(res.ETag.HasValue());
    EXPECT_EQ(res.BlobSize, 0);
    EXPECT_TRUE(res.CommittedBlocks.empty());
    EXPECT_FALSE(res.UncommittedBlocks.empty());

    auto blobItem = GetBlobItem(blobName, Blobs::Models::ListBlobsIncludeFlags::UncomittedBlobs);
    EXPECT_EQ(blobItem.BlobSize, 0);
  }

  TEST_F(BlobContainerClientTest, SourceTagsConditions)
  {
    auto const testName(GetTestNameLowerCase());
    auto containerClient = GetBlobContainerTestClient();
    containerClient.CreateIfNotExists();

    auto sourceBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(),
        m_containerName,
        testName,
        InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>());
    std::vector<uint8_t> buffer;
    buffer.resize(1024);
  }

  TEST_F(BlobContainerClientTest, SourceBlobAccessConditions)
  {
    auto const testName(GetTestNameLowerCase());
    auto containerClient = GetBlobContainerTestClient();
    containerClient.CreateIfNotExists();

    auto sourceBlobClient = containerClient.GetBlockBlobClient(testName);

    std::vector<uint8_t> buffer;
    buffer.resize(1024);
    auto createResponse = sourceBlobClient.UploadFrom(buffer.data(), buffer.size());
    Azure::ETag eTag = createResponse.Value.ETag;
    auto lastModifiedTime = createResponse.Value.LastModified;
    auto timeBeforeStr = lastModifiedTime - std::chrono::seconds(2);
    auto timeAfterStr = lastModifiedTime + std::chrono::seconds(2);

    auto destBlobClient = containerClient.GetBlockBlobClient(testName + "2");

    {
      Blobs::StartBlobCopyFromUriOptions options;
      options.SourceAccessConditions.IfMatch = eTag;
      EXPECT_NO_THROW(destBlobClient.StartCopyFromUri(sourceBlobClient.GetUrl(), options));
      options.SourceAccessConditions.IfMatch = DummyETag;
      EXPECT_THROW(
          destBlobClient.StartCopyFromUri(sourceBlobClient.GetUrl(), options), StorageException);

      Blobs::CopyBlobFromUriOptions options2;
      options2.SourceAccessConditions.IfMatch = eTag;
      EXPECT_NO_THROW(destBlobClient.CopyFromUri(sourceBlobClient.GetUrl() + GetSas(), options2));
      options2.SourceAccessConditions.IfMatch = DummyETag;
      EXPECT_THROW(
          destBlobClient.CopyFromUri(sourceBlobClient.GetUrl() + GetSas(), options2),
          StorageException);
    }
    {
      Blobs::StartBlobCopyFromUriOptions options;
      options.SourceAccessConditions.IfNoneMatch = DummyETag;
      EXPECT_NO_THROW(destBlobClient.StartCopyFromUri(sourceBlobClient.GetUrl(), options));
      options.SourceAccessConditions.IfNoneMatch = eTag;
      EXPECT_THROW(
          destBlobClient.StartCopyFromUri(sourceBlobClient.GetUrl(), options), StorageException);

      Blobs::CopyBlobFromUriOptions options2;
      options2.SourceAccessConditions.IfNoneMatch = DummyETag;
      EXPECT_NO_THROW(destBlobClient.CopyFromUri(sourceBlobClient.GetUrl() + GetSas(), options2));
      options2.SourceAccessConditions.IfNoneMatch = eTag;
      EXPECT_THROW(
          destBlobClient.CopyFromUri(sourceBlobClient.GetUrl() + GetSas(), options2),
          StorageException);
    }
    {
      Blobs::StartBlobCopyFromUriOptions options;
      options.SourceAccessConditions.IfModifiedSince = timeBeforeStr;
      EXPECT_NO_THROW(destBlobClient.StartCopyFromUri(sourceBlobClient.GetUrl(), options));
      options.SourceAccessConditions.IfModifiedSince = timeAfterStr;
      EXPECT_THROW(
          destBlobClient.StartCopyFromUri(sourceBlobClient.GetUrl(), options), StorageException);

      sourceBlobClient.GetProperties();
      Blobs::CopyBlobFromUriOptions options2;
      options2.SourceAccessConditions.IfModifiedSince = timeBeforeStr;
      EXPECT_NO_THROW(destBlobClient.CopyFromUri(sourceBlobClient.GetUrl() + GetSas(), options2));
      options2.SourceAccessConditions.IfModifiedSince = timeAfterStr;
      EXPECT_THROW(
          destBlobClient.CopyFromUri(sourceBlobClient.GetUrl() + GetSas(), options2),
          StorageException);
    }
    {
      Blobs::StartBlobCopyFromUriOptions options;
      options.SourceAccessConditions.IfUnmodifiedSince = timeAfterStr;
      EXPECT_NO_THROW(destBlobClient.StartCopyFromUri(sourceBlobClient.GetUrl(), options));
      options.SourceAccessConditions.IfUnmodifiedSince = timeBeforeStr;
      EXPECT_THROW(
          destBlobClient.StartCopyFromUri(sourceBlobClient.GetUrl(), options), StorageException);

      Blobs::CopyBlobFromUriOptions options2;
      options2.SourceAccessConditions.IfUnmodifiedSince = timeAfterStr;
      EXPECT_NO_THROW(destBlobClient.CopyFromUri(sourceBlobClient.GetUrl() + GetSas(), options2));
      options2.SourceAccessConditions.IfUnmodifiedSince = timeBeforeStr;
      EXPECT_THROW(
          destBlobClient.CopyFromUri(sourceBlobClient.GetUrl() + GetSas(), options2),
          StorageException);
    }

    // lease
    {
      const std::string leaseId = Blobs::BlobLeaseClient::CreateUniqueLeaseId();
      const std::string dummyLeaseId = Blobs::BlobLeaseClient::CreateUniqueLeaseId();
      Blobs::BlobLeaseClient leaseClient(destBlobClient, leaseId);

      leaseClient.Acquire(std::chrono::seconds(60));

      Blobs::CopyBlobFromUriOptions options;
      options.AccessConditions.LeaseId = dummyLeaseId;
      EXPECT_THROW(
          destBlobClient.CopyFromUri(sourceBlobClient.GetUrl() + GetSas(), options),
          StorageException);
      options.AccessConditions.LeaseId = leaseId;
      EXPECT_NO_THROW(destBlobClient.CopyFromUri(sourceBlobClient.GetUrl() + GetSas(), options));
      leaseClient.Release();
    }
  }

  TEST_F(BlockBlobClientTest, DISABLED_Immutability)
  {
    auto const testName(GetTestName());

    auto blobClient = GetBlockBlobClient(testName);
    std::vector<uint8_t> emptyContent;
    blobClient.UploadFrom(emptyContent.data(), emptyContent.size());

    auto blobContainerClient = GetBlobContainerTestClient();
    ASSERT_TRUE(blobContainerClient.GetProperties().Value.HasImmutableStorageWithVersioning);

    Blobs::Models::BlobImmutabilityPolicy policy;
    policy.ExpiresOn = Azure::DateTime::Parse(
        Azure::DateTime(std::chrono::system_clock::now() + std::chrono::hours(24))
            .ToString(Azure::DateTime::DateFormat::Rfc1123),
        Azure::DateTime::DateFormat::Rfc1123);
    policy.PolicyMode = Blobs::Models::BlobImmutabilityPolicyMode::Unlocked;
    auto setPolicyResponse = blobClient.SetImmutabilityPolicy(policy);
    EXPECT_EQ(setPolicyResponse.Value.ImmutabilityPolicy, policy);
    auto blobProperties = blobClient.GetProperties().Value;
    ASSERT_TRUE(blobProperties.ImmutabilityPolicy.HasValue());
    EXPECT_EQ(blobProperties.ImmutabilityPolicy.Value(), policy);
    auto downloadResponse = blobClient.Download();
    ASSERT_TRUE(downloadResponse.Value.Details.ImmutabilityPolicy.HasValue());
    EXPECT_EQ(downloadResponse.Value.Details.ImmutabilityPolicy.Value(), policy);
    auto blobItem = GetBlobItem(testName, Blobs::Models::ListBlobsIncludeFlags::ImmutabilityPolicy);
    ASSERT_TRUE(blobItem.Details.ImmutabilityPolicy.HasValue());
    EXPECT_EQ(blobItem.Details.ImmutabilityPolicy.Value(), policy);

    EXPECT_NO_THROW(blobClient.DeleteImmutabilityPolicy());
    blobProperties = blobClient.GetProperties().Value;
    EXPECT_FALSE(blobProperties.ImmutabilityPolicy.HasValue());
    downloadResponse = blobClient.Download();
    ASSERT_FALSE(downloadResponse.Value.Details.ImmutabilityPolicy.HasValue());
    blobItem = GetBlobItem(testName, Blobs::Models::ListBlobsIncludeFlags::ImmutabilityPolicy);
    ASSERT_FALSE(blobItem.Details.ImmutabilityPolicy.HasValue());

    auto copySourceBlobClient = GetBlockBlobClient(testName + "src");
    copySourceBlobClient.UploadFrom(emptyContent.data(), emptyContent.size());
    {
      auto copyDestinationBlobClient = GetBlockBlobClient(testName + "dest1");
      Blobs::StartBlobCopyFromUriOptions options;
      options.ImmutabilityPolicy = policy;
      copyDestinationBlobClient.StartCopyFromUri(copySourceBlobClient.GetUrl() + GetSas(), options)
          .PollUntilDone(std::chrono::seconds(1));
      EXPECT_EQ(copyDestinationBlobClient.GetProperties().Value.ImmutabilityPolicy.Value(), policy);
    }
    {
      auto copyDestinationBlobClient = GetBlockBlobClient(testName + "dest2");
      Blobs::CopyBlobFromUriOptions options;
      options.ImmutabilityPolicy = policy;
      copyDestinationBlobClient.CopyFromUri(copySourceBlobClient.GetUrl() + GetSas(), options);
      EXPECT_EQ(copyDestinationBlobClient.GetProperties().Value.ImmutabilityPolicy.Value(), policy);
    }
  }

  TEST_F(BlockBlobClientTest, DISABLED_ImmutabilityAccessCondition)
  {
    auto const testName(GetTestName());
    auto blobClient = GetBlockBlobClient(testName);
    std::vector<uint8_t> emptyContent;
    auto uploadResponse = blobClient.UploadFrom(emptyContent.data(), emptyContent.size());
    auto lastModifiedTime = uploadResponse.Value.LastModified;
    auto timeBeforeStr = lastModifiedTime - std::chrono::minutes(1);
    auto timeAfterStr = lastModifiedTime + std::chrono::minutes(1);

    Blobs::Models::BlobImmutabilityPolicy policy;
    policy.ExpiresOn = Azure::DateTime::Parse(
        Azure::DateTime(std::chrono::system_clock::now() + std::chrono::hours(24))
            .ToString(Azure::DateTime::DateFormat::Rfc1123),
        Azure::DateTime::DateFormat::Rfc1123);
    policy.PolicyMode = Blobs::Models::BlobImmutabilityPolicyMode::Unlocked;

    Blobs::SetBlobImmutabilityPolicyOptions options;
    options.AccessConditions.IfUnmodifiedSince = timeBeforeStr;
    EXPECT_THROW(blobClient.SetImmutabilityPolicy(policy, options), StorageException);
    options.AccessConditions.IfUnmodifiedSince = timeAfterStr;
    EXPECT_NO_THROW(blobClient.SetImmutabilityPolicy(policy, options));
  }

  TEST_F(BlockBlobClientTest, DISABLED_LegalHold)
  {
    auto const testName(GetTestName());
    auto blobClient = GetBlockBlobClient(testName);
    std::vector<uint8_t> emptyContent;
    blobClient.UploadFrom(emptyContent.data(), emptyContent.size());

    auto setLegalHoldResponse = blobClient.SetLegalHold(true);
    EXPECT_TRUE(setLegalHoldResponse.Value.HasLegalHold);
    auto blobProperties = blobClient.GetProperties().Value;
    EXPECT_TRUE(blobProperties.HasLegalHold);
    auto downloadResponse = blobClient.Download();
    EXPECT_TRUE(downloadResponse.Value.Details.HasLegalHold);
    auto blobItem = GetBlobItem(testName, Blobs::Models::ListBlobsIncludeFlags::LegalHold);
    EXPECT_TRUE(blobItem.Details.HasLegalHold);

    setLegalHoldResponse = blobClient.SetLegalHold(false);
    EXPECT_FALSE(setLegalHoldResponse.Value.HasLegalHold);

    auto copySourceBlobClient = GetBlockBlobClient(testName + "src");
    copySourceBlobClient.UploadFrom(emptyContent.data(), emptyContent.size());
    {
      auto copyDestinationBlobClient = GetBlockBlobClient(testName + "dest1");
      Blobs::StartBlobCopyFromUriOptions options;
      options.HasLegalHold = true;
      copyDestinationBlobClient.StartCopyFromUri(copySourceBlobClient.GetUrl() + GetSas(), options)
          .PollUntilDone(std::chrono::seconds(1));
      EXPECT_TRUE(copyDestinationBlobClient.GetProperties().Value.HasLegalHold);
    }
    {
      auto copyDestinationBlobClient = GetBlockBlobClient(testName + "dest2");
      Blobs::CopyBlobFromUriOptions options;
      options.HasLegalHold = true;
      copyDestinationBlobClient.CopyFromUri(copySourceBlobClient.GetUrl() + GetSas(), options);
      EXPECT_TRUE(copyDestinationBlobClient.GetProperties().Value.HasLegalHold);
    }
  }

  TEST_F(BlockBlobClientTest, ContentHash)
  {
    auto const testName(GetTestName());
    auto srcBlobClient = GetBlockBlobClient(testName + "src");
    std::vector<uint8_t> blobContent = RandomBuffer(100);
    srcBlobClient.UploadFrom(blobContent.data(), blobContent.size());
    const std::vector<uint8_t> contentMd5
        = Azure::Core::Cryptography::Md5Hash().Final(blobContent.data(), blobContent.size());
    const std::vector<uint8_t> contentCrc64
        = Azure::Storage::Crc64Hash().Final(blobContent.data(), blobContent.size());

    Azure::Core::IO::MemoryBodyStream stream(blobContent.data(), blobContent.size());

    {
      auto destBlobClient = GetBlockBlobClient(testName + "dest0");
      Blobs::UploadBlockBlobOptions options;
      options.TransactionalContentHash = ContentHash();
      options.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Md5;
      options.TransactionalContentHash.Value().Value = Azure::Core::Convert::Base64Decode(DummyMd5);
      stream.Rewind();
      EXPECT_THROW(destBlobClient.Upload(stream, options), StorageException);
      options.TransactionalContentHash.Value().Value = contentMd5;
      stream.Rewind();
      EXPECT_NO_THROW(destBlobClient.Upload(stream, options));
      options.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Crc64;
      options.TransactionalContentHash.Value().Value
          = Azure::Core::Convert::Base64Decode(DummyCrc64);
      stream.Rewind();
      EXPECT_THROW(destBlobClient.Upload(stream, options), StorageException);
      options.TransactionalContentHash.Value().Value = contentCrc64;
      stream.Rewind();
      EXPECT_NO_THROW(destBlobClient.Upload(stream, options));
    }
    {
      auto destBlobClient = GetBlockBlobClient(testName + "dest1");
      Blobs::UploadBlockBlobFromUriOptions options;
      options.TransactionalContentHash = ContentHash();
      options.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Md5;
      options.TransactionalContentHash.Value().Value = Azure::Core::Convert::Base64Decode(DummyMd5);
      stream.Rewind();
      EXPECT_THROW(
          destBlobClient.UploadFromUri(srcBlobClient.GetUrl() + GetSas(), options),
          StorageException);
      options.TransactionalContentHash.Value().Value = contentMd5;
      stream.Rewind();
      EXPECT_NO_THROW(destBlobClient.UploadFromUri(srcBlobClient.GetUrl() + GetSas(), options));
      options.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Crc64;
      options.TransactionalContentHash.Value().Value
          = Azure::Core::Convert::Base64Decode(DummyCrc64);
      stream.Rewind();
      EXPECT_THROW(
          destBlobClient.UploadFromUri(srcBlobClient.GetUrl() + GetSas(), options),
          StorageException);
      options.TransactionalContentHash.Value().Value = contentCrc64;
      stream.Rewind();
      EXPECT_NO_THROW(destBlobClient.UploadFromUri(srcBlobClient.GetUrl() + GetSas(), options));
    }
    {
      auto destBlobClient = GetBlockBlobClient(testName + "dest2");
      Blobs::CopyBlobFromUriOptions options;
      options.TransactionalContentHash = ContentHash();
      options.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Md5;
      options.TransactionalContentHash.Value().Value = Azure::Core::Convert::Base64Decode(DummyMd5);
      stream.Rewind();
      EXPECT_THROW(
          destBlobClient.CopyFromUri(srcBlobClient.GetUrl() + GetSas(), options), StorageException);
      options.TransactionalContentHash.Value().Value = contentMd5;
      stream.Rewind();
      EXPECT_NO_THROW(destBlobClient.CopyFromUri(srcBlobClient.GetUrl() + GetSas(), options));
      options.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Crc64;
      options.TransactionalContentHash.Value().Value
          = Azure::Core::Convert::Base64Decode(DummyCrc64);
      stream.Rewind();
      EXPECT_THROW(
          destBlobClient.CopyFromUri(srcBlobClient.GetUrl() + GetSas(), options), StorageException);
      options.TransactionalContentHash.Value().Value = contentCrc64;
      stream.Rewind();
      EXPECT_NO_THROW(destBlobClient.CopyFromUri(srcBlobClient.GetUrl() + GetSas(), options));
    }
    {
      auto destBlobClient = GetBlockBlobClient(testName + "dest3");
      Blobs::StageBlockOptions options;
      options.TransactionalContentHash = ContentHash();
      options.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Md5;
      options.TransactionalContentHash.Value().Value = Azure::Core::Convert::Base64Decode(DummyMd5);
      stream.Rewind();
      EXPECT_THROW(destBlobClient.StageBlock("YWJjZA==", stream, options), StorageException);
      options.TransactionalContentHash.Value().Value = contentMd5;
      stream.Rewind();
      EXPECT_NO_THROW(destBlobClient.StageBlock("YWJjZA==", stream, options));
      options.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Crc64;
      options.TransactionalContentHash.Value().Value
          = Azure::Core::Convert::Base64Decode(DummyCrc64);
      stream.Rewind();
      EXPECT_THROW(destBlobClient.StageBlock("YWJjZA==", stream, options), StorageException);
      options.TransactionalContentHash.Value().Value = contentCrc64;
      stream.Rewind();
      EXPECT_NO_THROW(destBlobClient.StageBlock("YWJjZA==", stream, options));
    }
    {
      auto destBlobClient = GetBlockBlobClient(testName + "dest4");
      Blobs::StageBlockFromUriOptions options;
      options.TransactionalContentHash = ContentHash();
      options.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Md5;
      options.TransactionalContentHash.Value().Value = Azure::Core::Convert::Base64Decode(DummyMd5);
      EXPECT_THROW(
          destBlobClient.StageBlockFromUri("YWJjZA==", srcBlobClient.GetUrl() + GetSas(), options),
          StorageException);
      options.TransactionalContentHash.Value().Value = contentMd5;
      EXPECT_NO_THROW(
          destBlobClient.StageBlockFromUri("YWJjZA==", srcBlobClient.GetUrl() + GetSas(), options));
      options.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Crc64;
      options.TransactionalContentHash.Value().Value
          = Azure::Core::Convert::Base64Decode(DummyCrc64);
      EXPECT_THROW(
          destBlobClient.StageBlockFromUri("YWJjZA==", srcBlobClient.GetUrl() + GetSas(), options),
          StorageException);
      options.TransactionalContentHash.Value().Value = contentCrc64;
      EXPECT_NO_THROW(
          destBlobClient.StageBlockFromUri("YWJjZA==", srcBlobClient.GetUrl() + GetSas(), options));
    }
  }

  TEST_F(BlockBlobClientTest, UploadFromUri)
  {
    auto const testName(GetTestName());
    auto srcBlobClient = GetBlockBlobClient(testName + "src");
    std::vector<uint8_t> blobContent(100, 'a');
    srcBlobClient.UploadFrom(blobContent.data(), blobContent.size());

    const std::vector<uint8_t> blobMd5
        = Azure::Core::Cryptography::Md5Hash().Final(blobContent.data(), blobContent.size());
    const std::vector<uint8_t> blobCrc64
        = Azure::Storage::Crc64Hash().Final(blobContent.data(), blobContent.size());

    auto destBlobClient = GetBlockBlobClient(testName + "dest");
    auto uploadFromUriResult = destBlobClient.UploadFromUri(srcBlobClient.GetUrl() + GetSas());
    EXPECT_TRUE(uploadFromUriResult.Value.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(uploadFromUriResult.Value.LastModified));
    EXPECT_TRUE(uploadFromUriResult.Value.VersionId.HasValue());
    EXPECT_TRUE(uploadFromUriResult.Value.IsServerEncrypted);
    ASSERT_TRUE(uploadFromUriResult.Value.TransactionalContentHash.HasValue());
    if (uploadFromUriResult.Value.TransactionalContentHash.Value().Algorithm == HashAlgorithm::Md5)
    {
      EXPECT_EQ(uploadFromUriResult.Value.TransactionalContentHash.Value().Value, blobMd5);
    }
    else if (
        uploadFromUriResult.Value.TransactionalContentHash.Value().Algorithm
        == HashAlgorithm::Crc64)
    {
      EXPECT_EQ(uploadFromUriResult.Value.TransactionalContentHash.Value().Value, blobCrc64);
    }

    Blobs::UploadBlockBlobFromUriOptions options;
    options.CopySourceBlobProperties = false;
    options.HttpHeaders.ContentLanguage = "en-US";
    options.HttpHeaders.ContentType = "application/octet-stream";
    options.Metadata["k"] = "v";
    options.AccessTier = Blobs::Models::AccessTier::Cool;
    options.Tags["k1"] = "v1";
    uploadFromUriResult = destBlobClient.UploadFromUri(srcBlobClient.GetUrl() + GetSas(), options);
    auto destBlobProperties = destBlobClient.GetProperties().Value;
    destBlobProperties.HttpHeaders.ContentHash.Value.clear();
    EXPECT_EQ(destBlobProperties.HttpHeaders, options.HttpHeaders);
    EXPECT_EQ(destBlobProperties.Metadata, options.Metadata);
    EXPECT_EQ(destBlobProperties.AccessTier.Value(), options.AccessTier.Value());
    EXPECT_EQ(static_cast<size_t>(destBlobProperties.TagCount.Value()), options.Tags.size());
  }

  TEST_F(BlockBlobClientTest, SetGetTagsWithLeaseId)
  {
    auto const testName(GetTestName());
    auto blobClient = GetBlockBlobClient(testName);
    std::vector<uint8_t> emptyContent;
    blobClient.UploadFrom(emptyContent.data(), emptyContent.size());

    const std::map<std::string, std::string> tags{{"k", "v"}};

    Blobs::BlobLeaseClient leaseClient(blobClient, Blobs::BlobLeaseClient::CreateUniqueLeaseId());

    leaseClient.Acquire(std::chrono::seconds(60));

    Blobs::SetBlobTagsOptions setTagsOptions;
    setTagsOptions.AccessConditions.LeaseId = Blobs::BlobLeaseClient::CreateUniqueLeaseId();
    EXPECT_THROW(blobClient.SetTags(tags, setTagsOptions), StorageException);
    Blobs::GetBlobTagsOptions getTagsOptions;
    getTagsOptions.AccessConditions.LeaseId = Blobs::BlobLeaseClient::CreateUniqueLeaseId();
    EXPECT_THROW(blobClient.GetTags(getTagsOptions), StorageException);

    setTagsOptions.AccessConditions.LeaseId = leaseClient.GetLeaseId();
    EXPECT_NO_THROW(blobClient.SetTags(tags, setTagsOptions));
    getTagsOptions.AccessConditions.LeaseId = leaseClient.GetLeaseId();
    EXPECT_NO_THROW(blobClient.GetTags(getTagsOptions));

    leaseClient.Release();
  }

}}} // namespace Azure::Storage::Test
