// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "block_blob_client_test.hpp"

#include <future>
#include <random>
#include <vector>

#include <azure/core/cryptography/hash.hpp>
#include <azure/storage/common/crypt.hpp>

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

  void BlockBlobClientTest::SetUp()
  {
    BlobContainerClientTest::SetUp();
    if (shouldSkipTest())
    {
      return;
    }
    m_blobName = RandomString();
    m_blockBlobClient = std::make_shared<Blobs::BlockBlobClient>(
        m_blobContainerClient->GetBlockBlobClient(m_blobName));
    m_blobUploadOptions.Metadata = {{"key1", "V1"}, {"key2", "Value2"}};
    m_blobUploadOptions.HttpHeaders.ContentType = "application/x-binary";
    m_blobUploadOptions.HttpHeaders.ContentLanguage = "en-US";
    m_blobUploadOptions.HttpHeaders.ContentDisposition = "attachment";
    m_blobUploadOptions.HttpHeaders.CacheControl = "no-cache";
    m_blobUploadOptions.HttpHeaders.ContentEncoding = "identity";
    m_blobUploadOptions.HttpHeaders.ContentHash.Value.clear();
    m_blobUploadOptions.AccessTier = Azure::Storage::Blobs::Models::AccessTier::Hot;
    m_blobContent = RandomBuffer(static_cast<size_t>(1_KB));
    auto blobContent
        = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    m_blockBlobClient->Upload(blobContent, m_blobUploadOptions);
    m_blobUploadOptions.HttpHeaders.ContentHash
        = m_blockBlobClient->GetProperties().Value.HttpHeaders.ContentHash;
  }

  TEST_F(BlockBlobClientTest, CreateDelete)
  {
    auto blobClient = *m_blockBlobClient;

    auto blobContent
        = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    auto blobContentInfo = blobClient.Upload(blobContent, m_blobUploadOptions);
    EXPECT_TRUE(blobContentInfo.Value.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(blobContentInfo.Value.LastModified));
    EXPECT_TRUE(blobContentInfo.Value.VersionId.HasValue());
    EXPECT_FALSE(blobContentInfo.Value.VersionId.Value().empty());
    EXPECT_FALSE(blobContentInfo.Value.EncryptionScope.HasValue());
    EXPECT_FALSE(blobContentInfo.Value.EncryptionKeySha256.HasValue());

    blobClient.Delete();
    EXPECT_THROW(blobClient.Delete(), StorageException);
  }

  TEST_F(BlockBlobClientTest, SoftDelete)
  {
    const std::string blobName = m_blobName;
    auto blobClient = *m_blockBlobClient;

    std::vector<uint8_t> emptyContent;
    auto blobContent = Azure::Core::IO::MemoryBodyStream(emptyContent.data(), emptyContent.size());
    blobClient.Upload(blobContent);

    auto blobItem = GetBlobItem(blobName);
    EXPECT_FALSE(blobItem.IsDeleted);
    EXPECT_FALSE(blobItem.Details.DeletedOn.HasValue());
    EXPECT_FALSE(blobItem.Details.RemainingRetentionDays.HasValue());

    blobClient.Delete();

    /*
    // Soft delete doesn't work in storage account with versioning enabled.
    blobItem = GetBlobItem(blobName, Blobs::Models::ListBlobsIncludeFlags::Deleted);
    EXPECT_TRUE(blobItem.IsDeleted);
    ASSERT_TRUE(blobItem.Details.DeletedOn.HasValue());
    EXPECT_TRUE(IsValidTime(blobItem.Details.DeletedOn.Value()));
    EXPECT_TRUE(blobItem.Details.RemainingRetentionDays.HasValue());
    */
  }

  TEST_F(BlockBlobClientTest, SmallUploadDownload)
  {
    // small default 1Kb upload/download
    auto blobClient = *m_blockBlobClient;

    auto res = blobClient.Download();
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
    res = blobClient.Download(options);
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

  TEST_F(BlockBlobClientTest, UploadDownload)
  {
    auto blobClient = *m_blockBlobClient;
    m_blobContent = RandomBuffer(100);
    {
      Azure::Core::Cryptography::Md5Hash md5hash;
      md5hash.Append(m_blobContent.data(), m_blobContent.size());
      m_blobUploadOptions.HttpHeaders.ContentHash.Value = md5hash.Final();
      Blobs::UploadBlockBlobOptions options;
      options.HttpHeaders = m_blobUploadOptions.HttpHeaders;
      options.Metadata = m_blobUploadOptions.Metadata;
      Core::IO::MemoryBodyStream bodyStream(m_blobContent.data(), m_blobContent.size());
      blobClient.Upload(bodyStream, options);
    }

    auto res = blobClient.Download();
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
    options.Range = {10, 20};
    res = blobClient.Download(options);
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
    auto blobClient = *m_blockBlobClient;

    std::map<std::string, std::string> tags;
    tags["key1"] = "value1";
    tags["key2"] = "value2";
    tags["key3 +-./:=_"] = "v1 +-./:=_";

    std::vector<uint8_t> blobContent = RandomBuffer(10);
    {
      Blobs::UploadBlockBlobOptions options;
      options.Tags = tags;
      auto stream = Azure::Core::IO::MemoryBodyStream(blobContent.data(), blobContent.size());
      blobClient.Upload(stream, options);
      EXPECT_EQ(blobClient.GetTags().Value, tags);
      blobClient.Delete();
    }

    {
      Blobs::UploadBlockBlobFromOptions options;
      options.TransferOptions.SingleUploadThreshold = 0;
      options.TransferOptions.ChunkSize = blobContent.size() / 2;
      options.Tags = tags;

      {
        blobClient.UploadFrom(blobContent.data(), blobContent.size(), options);
        EXPECT_EQ(blobClient.GetTags().Value, tags);
        blobClient.Delete();
      }
      {
        const std::string tempFilename = "file" + RandomString();
        WriteFile(tempFilename, blobContent);
        blobClient.UploadFrom(tempFilename, options);
        EXPECT_EQ(blobClient.GetTags().Value, tags);
        blobClient.Delete();
      }
    }
  }

  TEST_F(BlockBlobClientTest, DownloadTransactionalHash)
  {
    auto blobClient = *m_blockBlobClient;

    const std::vector<uint8_t> dataPart1 = RandomBuffer(10);
    const std::vector<uint8_t> dataPart2 = RandomBuffer(20);

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
    auto blobClient = *m_blockBlobClient;

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
      EXPECT_TRUE(IsValidTime(GetBlobItem(m_blobName).Details.LastAccessedOn.Value()));
    }
  }

  TEST_F(BlockBlobClientTest, DownloadEmpty)
  {
    auto blobClient = *m_blockBlobClient;

    std::vector<uint8_t> emptyContent;
    auto blobContent = Azure::Core::IO::MemoryBodyStream(emptyContent.data(), emptyContent.size());
    blobClient.Upload(blobContent);
    blobClient.SetHttpHeaders(m_blobUploadOptions.HttpHeaders);
    blobClient.SetMetadata(m_blobUploadOptions.Metadata);

    auto res = blobClient.Download();
    EXPECT_EQ(res.Value.BodyStream->Length(), 0);
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderRequestId).empty());
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderDate).empty());
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderXMsVersion).empty());
    EXPECT_TRUE(res.Value.Details.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(res.Value.Details.LastModified));
    EXPECT_EQ(res.Value.Details.HttpHeaders, m_blobUploadOptions.HttpHeaders);
    EXPECT_EQ(res.Value.Details.Metadata, m_blobUploadOptions.Metadata);
    EXPECT_EQ(res.Value.BlobType, Azure::Storage::Blobs::Models::BlobType::BlockBlob);

    std::string tempFilename = RandomString();
    EXPECT_NO_THROW(blobClient.DownloadTo(tempFilename));
    EXPECT_TRUE(ReadFile(tempFilename).empty());
    DeleteFile(tempFilename);

    std::vector<uint8_t> buff;
    EXPECT_NO_THROW(blobClient.DownloadTo(buff.data(), 0));

    Azure::Storage::Blobs::DownloadBlobOptions options;
    options.Range = Core::Http::HttpRange();
    options.Range.Value().Offset = 0;
    EXPECT_THROW(blobClient.Download(options), StorageException);
    options.Range.Value().Length = 1;
    EXPECT_THROW(blobClient.Download(options), StorageException);
  }

  TEST_F(BlockBlobClientTest, SyncCopyFromUri)
  {
    auto sourceBlobClient = m_blobContainerClient->GetBlockBlobClient("source" + RandomString());
    sourceBlobClient.UploadFrom(m_blobContent.data(), m_blobContent.size());

    const std::string blobName = "dest" + RandomString();
    auto destBlobClient = m_blobContainerClient->GetBlockBlobClient(blobName);

    auto res = destBlobClient.CopyFromUri(sourceBlobClient.GetUrl() + GetSas());
    EXPECT_EQ(res.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::Accepted);
    EXPECT_TRUE(res.Value.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(res.Value.LastModified));
    EXPECT_FALSE(res.Value.CopyId.empty());
    EXPECT_EQ(res.Value.CopyStatus, Azure::Storage::Blobs::Models::CopyStatus::Success);

    auto downloadResult = destBlobClient.Download();
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

  TEST_F(BlockBlobClientTest, SyncCopyFromUriEncryptionScope)
  {
    Blobs::BlobClientOptions clientOptions;
    const auto encryptionScope = GetTestEncryptionScope();
    clientOptions.EncryptionScope = encryptionScope;
    const auto containerName = LowercaseRandomString();
    const auto blobName = "b";
    auto containerClient = GetBlobContainerClientForTest(containerName, clientOptions);
    containerClient.CreateIfNotExists();
    auto srcBlobClient = containerClient.GetBlockBlobClient(blobName);
    uint8_t data;
    srcBlobClient.UploadFrom(&data, 1);

    auto properties = srcBlobClient.GetProperties().Value;
    ASSERT_TRUE(properties.EncryptionScope.HasValue());
    EXPECT_EQ(properties.EncryptionScope.Value(), encryptionScope);

    {
      Sas::BlobSasBuilder builder;
      builder.ExpiresOn = std::chrono::system_clock::now() + std::chrono::hours(1);
      builder.BlobContainerName = containerName;
      builder.BlobName = blobName;
      builder.Resource = Sas::BlobSasResource::Blob;
      builder.EncryptionScope = encryptionScope;
      builder.SetPermissions("r");
      auto keyCredential
          = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;
      auto sasToken = builder.GenerateSasToken(*keyCredential);

      auto destBlobClient = *m_blockBlobClient;
      auto response = destBlobClient.CopyFromUri(srcBlobClient.GetUrl() + sasToken);
      EXPECT_FALSE(response.Value.EncryptionScope.HasValue());
      properties = destBlobClient.GetProperties().Value;
      EXPECT_FALSE(properties.EncryptionScope.HasValue());

      destBlobClient = containerClient.GetBlockBlobClient(RandomString());
      response = destBlobClient.CopyFromUri(srcBlobClient.GetUrl() + sasToken);
      ASSERT_TRUE(response.Value.EncryptionScope.HasValue());
      EXPECT_EQ(response.Value.EncryptionScope.Value(), encryptionScope);
      properties = destBlobClient.GetProperties().Value;
      ASSERT_TRUE(properties.EncryptionScope.HasValue());
      EXPECT_EQ(properties.EncryptionScope.Value(), encryptionScope);
    }
  }

  TEST_F(BlockBlobClientTest, AsyncCopyFromUri)
  {
    auto sourceBlobClient = *m_blockBlobClient;

    const std::string blobName = RandomString();
    auto destBlobClient = GetBlockBlobClientForTest(blobName);

    auto res = destBlobClient.StartCopyFromUri(sourceBlobClient.GetUrl());
    EXPECT_EQ(res.GetRawResponse().GetStatusCode(), Azure::Core::Http::HttpStatusCode::Accepted);
    res.PollUntilDone(PollInterval());
    auto properties = destBlobClient.GetProperties().Value;
    EXPECT_FALSE(properties.CopyId.Value().empty());
    EXPECT_FALSE(properties.CopySource.Value().empty());
    EXPECT_TRUE(
        properties.CopyStatus.Value() == Azure::Storage::Blobs::Models::CopyStatus::Success);
    EXPECT_FALSE(properties.CopyProgress.Value().empty());
    EXPECT_TRUE(IsValidTime(properties.CopyCompletedOn.Value()));
    ASSERT_TRUE(properties.IsIncrementalCopy.HasValue());
    EXPECT_FALSE(properties.IsIncrementalCopy.Value());
    EXPECT_FALSE(properties.IncrementalCopyDestinationSnapshot.HasValue());

    auto downloadResult = destBlobClient.Download();
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
    auto sourceBlobClient = *m_blockBlobClient;

    const std::string blobName = "dest" + RandomString();
    auto destBlobClient = GetBlockBlobClientForTest(blobName);

    Blobs::StartBlobCopyFromUriOptions options;
    options.Tags["key1"] = "value1";
    options.Tags["key2"] = "value2";
    options.Tags["key3 +-./:=_"] = "v1 +-./:=_";
    options.Metadata["key1"] = "value1";
    options.Metadata["key2"] = "value2";
    options.AccessTier = Blobs::Models::AccessTier::Cool;
    auto operation = destBlobClient.StartCopyFromUri(sourceBlobClient.GetUrl(), options);
    operation.PollUntilDone(std::chrono::seconds(1));
    EXPECT_EQ(destBlobClient.GetTags().Value, options.Tags);
    auto properties = destBlobClient.GetProperties().Value;
    EXPECT_EQ(properties.Metadata, options.Metadata);
    EXPECT_EQ(properties.AccessTier.Value(), options.AccessTier.Value());

    Blobs::CopyBlobFromUriOptions options2;
    options2.Tags = options.Tags;
    options2.Metadata = options.Metadata;
    options2.AccessTier = options.AccessTier;
    destBlobClient.CopyFromUri(sourceBlobClient.GetUrl() + GetSas(), options2);
    EXPECT_EQ(destBlobClient.GetTags().Value, options2.Tags);
    properties = destBlobClient.GetProperties().Value;
    EXPECT_EQ(properties.Metadata, options2.Metadata);
    EXPECT_EQ(properties.AccessTier.Value(), options2.AccessTier.Value());

    options2.CopySourceTagsMode = Blobs::Models::BlobCopySourceTagsMode::Copy;
    options2.Tags.clear();
    destBlobClient.CopyFromUri(sourceBlobClient.GetUrl() + GetSas(), options2);
    EXPECT_TRUE(destBlobClient.GetTags().Value.empty());
  }

  TEST_F(BlockBlobClientTest, SnapShotVersions)
  {
    auto blobClient = *m_blockBlobClient;

    auto res = blobClient.CreateSnapshot();
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderRequestId).empty());
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderDate).empty());
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderXMsVersion).empty());
    EXPECT_TRUE(res.Value.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(res.Value.LastModified));
    EXPECT_FALSE(res.Value.Snapshot.empty());
    EXPECT_TRUE(res.Value.VersionId.HasValue());
    EXPECT_FALSE(res.Value.VersionId.Value().empty());
    auto snapshotClient = blobClient.WithSnapshot(res.Value.Snapshot);
    EXPECT_EQ(ReadBodyStream(snapshotClient.Download().Value.BodyStream), m_blobContent);
    EXPECT_EQ(snapshotClient.GetProperties().Value.Metadata, m_blobUploadOptions.Metadata);
    EXPECT_TRUE(snapshotClient.GetProperties().Value.IsServerEncrypted);
    auto versionClient = blobClient.WithVersionId(res.Value.VersionId.Value());
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
    res = blobClient.CreateSnapshot(options);
    EXPECT_FALSE(res.Value.Snapshot.empty());
    auto snapshotClient2 = blobClient.WithSnapshot(res.Value.Snapshot);
    EXPECT_EQ(snapshotClient2.GetProperties().Value.Metadata, options.Metadata);

    EXPECT_NO_THROW(snapshotClient.Delete());
    EXPECT_NO_THROW(snapshotClient2.Delete());
    EXPECT_NO_THROW(versionClient.Delete());
    EXPECT_NO_THROW(blobClient.GetProperties());
  }

  TEST_F(BlockBlobClientTest, IsCurrentVersion)
  {
    auto blobClient = *m_blockBlobClient;

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

    auto blobItem = GetBlobItem(m_blobName, Blobs::Models::ListBlobsIncludeFlags::Versions);
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
    auto blobClient = *m_blockBlobClient;

    blobClient.SetAccessTier(Azure::Storage::Blobs::Models::AccessTier::Cool);

    auto res = blobClient.GetProperties();
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
    auto srcBlobClient = *m_blockBlobClient;

    auto blobClient = GetBlockBlobClientForTest(RandomString());

    const std::string blockId1 = Base64EncodeText("0");
    const std::string blockId2 = Base64EncodeText("1");
    std::vector<uint8_t> block1Content = RandomBuffer(100);
    auto blockContent
        = Azure::Core::IO::MemoryBodyStream(block1Content.data(), block1Content.size());
    blobClient.StageBlock(blockId1, blockContent);
    Azure::Storage::Blobs::CommitBlockListOptions options;
    options.HttpHeaders = m_blobUploadOptions.HttpHeaders;
    options.Metadata = m_blobUploadOptions.Metadata;
    auto blobContentInfo = blobClient.CommitBlockList({blockId1}, options);
    EXPECT_TRUE(blobContentInfo.Value.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(blobContentInfo.Value.LastModified));
    EXPECT_TRUE(blobContentInfo.Value.VersionId.HasValue());
    EXPECT_FALSE(blobContentInfo.Value.VersionId.Value().empty());
    auto res = blobClient.GetBlockList();
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

    blobClient.StageBlockFromUri(blockId2, srcBlobClient.GetUrl() + GetSas());
    Blobs::GetBlockListOptions options2;
    options2.ListType = Blobs::Models::BlockListType::All;
    res = blobClient.GetBlockList(options2);
    EXPECT_EQ(res.Value.BlobSize, static_cast<int64_t>(block1Content.size()));
    ASSERT_FALSE(res.Value.UncommittedBlocks.empty());
    EXPECT_EQ(res.Value.UncommittedBlocks[0].Name, blockId2);
    EXPECT_EQ(res.Value.UncommittedBlocks[0].Size, static_cast<int64_t>(m_blobContent.size()));

    blobClient.CommitBlockList({blockId1, blockId2});
    res = blobClient.GetBlockList(options2);
    EXPECT_EQ(
        res.Value.BlobSize, static_cast<int64_t>(block1Content.size() + m_blobContent.size()));
    EXPECT_TRUE(res.Value.UncommittedBlocks.empty());
  }

  TEST_F(BlockBlobClientTest, DeleteIfExists)
  {
    auto blobClient = GetBlockBlobClientForTest(RandomString());

    auto blobClientWithoutAuth = Azure::Storage::Blobs::BlockBlobClient(
        blobClient.GetUrl(), InitStorageClientOptions<Azure::Storage::Blobs::BlobClientOptions>());
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
    auto blobClient = *m_blockBlobClient;

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
    const std::string blobName = RandomString();
    auto blobClient = GetBlockBlobClientForTest(blobName);

    std::vector<uint8_t> emptyContent;
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

  TEST_F(BlockBlobClientTest, DISABLED_SetTierCold)
  {
    m_blockBlobClient->SetAccessTier(Blobs::Models::AccessTier::Cold);
    auto properties = m_blockBlobClient->GetProperties().Value;
    EXPECT_EQ(properties.AccessTier.Value(), Blobs::Models::AccessTier::Cold);
  }

  TEST_F(BlockBlobClientTest, SetTierWithLeaseId)
  {
    auto blobClient = *m_blockBlobClient;

    std::vector<uint8_t> emptyContent;
    blobClient.UploadFrom(emptyContent.data(), emptyContent.size());

    const std::string leaseId = RandomUUID();
    Blobs::BlobLeaseClient leaseClient(blobClient, leaseId);
    leaseClient.Acquire(std::chrono::seconds(30));

    EXPECT_THROW(blobClient.SetAccessTier(Blobs::Models::AccessTier::Cool), StorageException);

    Blobs::SetBlobAccessTierOptions options;
    options.AccessConditions.LeaseId = leaseId;
    EXPECT_NO_THROW(blobClient.SetAccessTier(Blobs::Models::AccessTier::Cool, options));
  }

  TEST_F(BlockBlobClientTest, UncommittedBlob)
  {
    const std::string blobName = RandomString();
    auto blobClient = GetBlockBlobClientForTest(blobName);

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

  TEST_F(BlockBlobClientTest, SourceTagsConditions)
  {
    auto containerClient = *m_blobContainerClient;

    auto sourceBlobClient = *m_blockBlobClient;
    std::map<std::string, std::string> tags;
    tags["key1"] = "value1";
    sourceBlobClient.SetTags(tags);

    const std::string successfulTagConditions = "key1 = 'value1'";
    const std::string failedTagConditions = "key1 != 'value1'";

    auto destBlobClient = containerClient.GetBlockBlobClient("dest" + RandomString());
    {
      Blobs::StartBlobCopyFromUriOptions options;
      options.SourceAccessConditions.TagConditions = successfulTagConditions;
      EXPECT_NO_THROW(destBlobClient.StartCopyFromUri(sourceBlobClient.GetUrl(), options));
      options.SourceAccessConditions.TagConditions = failedTagConditions;
      EXPECT_THROW(
          destBlobClient.StartCopyFromUri(sourceBlobClient.GetUrl(), options), StorageException);

      Blobs::UploadBlockBlobFromUriOptions options2;
      options2.SourceAccessConditions.TagConditions = successfulTagConditions;
      EXPECT_NO_THROW(destBlobClient.UploadFromUri(sourceBlobClient.GetUrl() + GetSas(), options2));
      options2.SourceAccessConditions.TagConditions = failedTagConditions;
      EXPECT_NO_THROW(destBlobClient.UploadFromUri(sourceBlobClient.GetUrl() + GetSas(), options2));
    }
  }

  TEST_F(BlockBlobClientTest, SourceBlobAccessConditions)
  {
    auto containerClient = *m_blobContainerClient;

    auto sourceBlobClient = containerClient.GetBlockBlobClient("source" + RandomString());

    std::vector<uint8_t> buffer;
    buffer.resize(1024);
    auto createResponse = sourceBlobClient.UploadFrom(buffer.data(), buffer.size());
    Azure::ETag eTag = createResponse.Value.ETag;
    auto lastModifiedTime = createResponse.Value.LastModified;
    auto timeBeforeStr = lastModifiedTime - std::chrono::seconds(2);
    auto timeAfterStr = lastModifiedTime + std::chrono::seconds(2);

    auto destBlobClient = containerClient.GetBlockBlobClient("dest" + RandomString());

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
      const std::string leaseId = RandomUUID();
      const std::string dummyLeaseId = RandomUUID();
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
    const auto ImmutabilityMaxLength = std::chrono::seconds(5);
    const std::string blobName = m_blobName;
    auto blobClient = *m_blockBlobClient;

    std::vector<uint8_t> emptyContent;
    blobClient.UploadFrom(emptyContent.data(), emptyContent.size());

    auto blobContainerClient = *m_blobContainerClient;
    ASSERT_TRUE(blobContainerClient.GetProperties().Value.HasImmutableStorageWithVersioning);

    Blobs::Models::BlobImmutabilityPolicy policy;
    policy.ExpiresOn = Azure::DateTime::Parse(
        Azure::DateTime(std::chrono::system_clock::now() + ImmutabilityMaxLength)
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
    auto blobItem = GetBlobItem(blobName, Blobs::Models::ListBlobsIncludeFlags::ImmutabilityPolicy);
    ASSERT_TRUE(blobItem.Details.ImmutabilityPolicy.HasValue());
    EXPECT_EQ(blobItem.Details.ImmutabilityPolicy.Value(), policy);

    EXPECT_NO_THROW(blobClient.DeleteImmutabilityPolicy());
    blobProperties = blobClient.GetProperties().Value;
    EXPECT_FALSE(blobProperties.ImmutabilityPolicy.HasValue());
    downloadResponse = blobClient.Download();
    ASSERT_FALSE(downloadResponse.Value.Details.ImmutabilityPolicy.HasValue());
    blobItem = GetBlobItem(blobName, Blobs::Models::ListBlobsIncludeFlags::ImmutabilityPolicy);
    ASSERT_FALSE(blobItem.Details.ImmutabilityPolicy.HasValue());

    auto copySourceBlobClient = GetBlockBlobClientForTest(blobName + "src");
    copySourceBlobClient.UploadFrom(emptyContent.data(), emptyContent.size());
    {
      auto copyDestinationBlobClient = GetBlockBlobClientForTest(blobName + "dest1");
      Blobs::StartBlobCopyFromUriOptions options;
      options.ImmutabilityPolicy = policy;
      copyDestinationBlobClient.StartCopyFromUri(copySourceBlobClient.GetUrl() + GetSas(), options)
          .PollUntilDone(std::chrono::seconds(1));
      EXPECT_EQ(copyDestinationBlobClient.GetProperties().Value.ImmutabilityPolicy.Value(), policy);
    }
    {
      auto copyDestinationBlobClient = GetBlockBlobClientForTest(blobName + "dest2");
      Blobs::CopyBlobFromUriOptions options;
      options.ImmutabilityPolicy = policy;
      copyDestinationBlobClient.CopyFromUri(copySourceBlobClient.GetUrl() + GetSas(), options);
      EXPECT_EQ(copyDestinationBlobClient.GetProperties().Value.ImmutabilityPolicy.Value(), policy);
    }

    std::this_thread::sleep_for(ImmutabilityMaxLength);
  }

  TEST_F(BlockBlobClientTest, DISABLED_ImmutabilityAccessCondition)
  {
    const auto ImmutabilityMaxLength = std::chrono::seconds(5);

    auto blobClient = *m_blockBlobClient;
    std::vector<uint8_t> emptyContent;
    auto uploadResponse = blobClient.UploadFrom(emptyContent.data(), emptyContent.size());
    auto lastModifiedTime = uploadResponse.Value.LastModified;
    auto timeBeforeStr = lastModifiedTime - std::chrono::minutes(1);
    auto timeAfterStr = lastModifiedTime + std::chrono::minutes(1);

    Blobs::Models::BlobImmutabilityPolicy policy;
    policy.ExpiresOn = Azure::DateTime::Parse(
        Azure::DateTime(std::chrono::system_clock::now() + ImmutabilityMaxLength)
            .ToString(Azure::DateTime::DateFormat::Rfc1123),
        Azure::DateTime::DateFormat::Rfc1123);
    policy.PolicyMode = Blobs::Models::BlobImmutabilityPolicyMode::Unlocked;

    Blobs::SetBlobImmutabilityPolicyOptions options;
    options.AccessConditions.IfUnmodifiedSince = timeBeforeStr;
    EXPECT_THROW(blobClient.SetImmutabilityPolicy(policy, options), StorageException);
    options.AccessConditions.IfUnmodifiedSince = timeAfterStr;
    EXPECT_NO_THROW(blobClient.SetImmutabilityPolicy(policy, options));

    std::this_thread::sleep_for(ImmutabilityMaxLength);
  }

  TEST_F(BlockBlobClientTest, DISABLED_LegalHold)
  {
    const auto testName = m_blobName;
    auto blobClient = *m_blockBlobClient;
    std::vector<uint8_t> emptyContent;

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

    auto copySourceBlobClient = GetBlockBlobClientForTest(RandomString() + "src");
    copySourceBlobClient.UploadFrom(emptyContent.data(), emptyContent.size());
    {
      auto copyDestinationBlobClient = GetBlockBlobClientForTest(RandomString() + "dest1");
      Blobs::StartBlobCopyFromUriOptions options;
      options.HasLegalHold = true;
      copyDestinationBlobClient.StartCopyFromUri(copySourceBlobClient.GetUrl() + GetSas(), options)
          .PollUntilDone(std::chrono::seconds(1));
      EXPECT_TRUE(copyDestinationBlobClient.GetProperties().Value.HasLegalHold);
    }
    {
      auto copyDestinationBlobClient = GetBlockBlobClientForTest(RandomString() + "dest2");
      Blobs::CopyBlobFromUriOptions options;
      options.HasLegalHold = true;
      copyDestinationBlobClient.CopyFromUri(copySourceBlobClient.GetUrl() + GetSas(), options);
      EXPECT_TRUE(copyDestinationBlobClient.GetProperties().Value.HasLegalHold);
    }
  }

  TEST_F(BlockBlobClientTest, ContentHash)
  {
    auto srcBlobClient = *m_blockBlobClient;
    std::vector<uint8_t> blobContent = RandomBuffer(100);
    srcBlobClient.UploadFrom(blobContent.data(), blobContent.size());
    const std::vector<uint8_t> contentMd5
        = Azure::Core::Cryptography::Md5Hash().Final(blobContent.data(), blobContent.size());
    const std::vector<uint8_t> contentCrc64
        = Azure::Storage::Crc64Hash().Final(blobContent.data(), blobContent.size());

    Azure::Core::IO::MemoryBodyStream stream(blobContent.data(), blobContent.size());

    {
      auto destBlobClient = GetBlockBlobClientForTest(RandomString() + "dest0");
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
      auto destBlobClient = GetBlockBlobClientForTest(RandomString() + "dest1");
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
      auto destBlobClient = GetBlockBlobClientForTest(RandomString() + "dest2");
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
      auto destBlobClient = GetBlockBlobClientForTest(RandomString() + "dest3");
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
      auto destBlobClient = GetBlockBlobClientForTest(RandomString() + "dest4");
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
    auto srcBlobClient = *m_blockBlobClient;
    std::vector<uint8_t> blobContent = RandomBuffer(100);
    srcBlobClient.UploadFrom(blobContent.data(), blobContent.size());
    std::map<std::string, std::string> srcTags;
    srcTags["srctags"] = "a1212";
    srcBlobClient.SetTags(srcTags);

    const std::vector<uint8_t> blobMd5
        = Azure::Core::Cryptography::Md5Hash().Final(blobContent.data(), blobContent.size());
    const std::vector<uint8_t> blobCrc64
        = Azure::Storage::Crc64Hash().Final(blobContent.data(), blobContent.size());

    auto destBlobClient = GetBlockBlobClientForTest(RandomString() + "dest");
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

    options.CopySourceTagsMode = Blobs::Models::BlobCopySourceTagsMode::Copy;
    options.Tags.clear();
    uploadFromUriResult = destBlobClient.UploadFromUri(srcBlobClient.GetUrl() + GetSas(), options);
    EXPECT_EQ(destBlobClient.GetTags().Value, srcTags);
  }

  TEST_F(BlockBlobClientTest, SetGetTagsWithLeaseId)
  {
    auto blobClient = *m_blockBlobClient;
    std::vector<uint8_t> emptyContent;
    blobClient.UploadFrom(emptyContent.data(), emptyContent.size());

    const std::map<std::string, std::string> tags{{"k", "v"}};

    Blobs::BlobLeaseClient leaseClient(blobClient, RandomUUID());

    leaseClient.Acquire(std::chrono::seconds(60));

    Blobs::SetBlobTagsOptions setTagsOptions;
    setTagsOptions.AccessConditions.LeaseId = RandomUUID();
    EXPECT_THROW(blobClient.SetTags(tags, setTagsOptions), StorageException);
    Blobs::GetBlobTagsOptions getTagsOptions;
    getTagsOptions.AccessConditions.LeaseId = RandomUUID();
    EXPECT_THROW(blobClient.GetTags(getTagsOptions), StorageException);

    setTagsOptions.AccessConditions.LeaseId = leaseClient.GetLeaseId();
    EXPECT_NO_THROW(blobClient.SetTags(tags, setTagsOptions));
    getTagsOptions.AccessConditions.LeaseId = leaseClient.GetLeaseId();
    EXPECT_NO_THROW(blobClient.GetTags(getTagsOptions));

    leaseClient.Release();
  }

  TEST_F(BlockBlobClientTest, MaximumBlocks)
  {
    auto blobClient = *m_blockBlobClient;

    const std::vector<uint8_t> content = RandomBuffer(1);
    const std::string blockId = Base64EncodeText(std::string(64, '0'));
    auto blockContent = Azure::Core::IO::MemoryBodyStream(content.data(), content.size());
    blobClient.StageBlock(blockId, blockContent);

    std::vector<std::string> blockIds(50000, blockId);
    EXPECT_NO_THROW(blobClient.CommitBlockList(blockIds));

    EXPECT_EQ(
        blobClient.GetProperties().Value.BlobSize,
        static_cast<int64_t>(blockIds.size() * content.size()));
  }

  TEST_F(BlockBlobClientTest, DownloadError)
  {
    auto blockBlobClient = GetBlockBlobClientForTest(RandomString());

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

  TEST_F(BlockBlobClientTest, DownloadNonExistingToFile)
  {
    auto blockBlobClient = GetBlockBlobClientForTest(RandomString());

    const std::string filename = RandomString();
    EXPECT_THROW(blockBlobClient.DownloadTo(filename), StorageException);
    EXPECT_THROW(ReadFile(filename), std::runtime_error);
  }

  TEST_F(BlockBlobClientTest, ConcurrentUploadFromNonExistingFile)
  {
    auto blockBlobClient = GetBlockBlobClientForTest(RandomString());

    std::string emptyFilename = RandomString();
    EXPECT_THROW(blockBlobClient.UploadFrom(emptyFilename), std::runtime_error);
    EXPECT_THROW(blockBlobClient.Delete(), StorageException);
  }

  TEST_F(BlockBlobClientTest, ConcurrentDownloadNonExistingBlob)
  {
    auto blockBlobClient = GetBlockBlobClientForTest(RandomString());

    std::vector<uint8_t> blobContent(100);
    std::string tempFilename = RandomString();

    EXPECT_THROW(
        blockBlobClient.DownloadTo(blobContent.data(), blobContent.size()), StorageException);
    EXPECT_THROW(blockBlobClient.DownloadTo(tempFilename), StorageException);
  }

  TEST_F(BlockBlobClientTest, ConcurrentUploadEmptyBlob)
  {
    auto blockBlobClient = GetBlockBlobClientForTest(RandomString());

    std::vector<uint8_t> emptyContent;

    blockBlobClient.UploadFrom(emptyContent.data(), emptyContent.size());
    EXPECT_NO_THROW(blockBlobClient.Delete());

    std::string emptyFilename = RandomString();
    WriteFile(emptyFilename, std::vector<uint8_t>{});
    blockBlobClient.UploadFrom(emptyFilename);
    EXPECT_NO_THROW(blockBlobClient.Delete());

    DeleteFile(emptyFilename);
  }

  TEST_F(BlockBlobClientTest, ConcurrentDownloadEmptyBlob)
  {
    auto blockBlobClient = GetBlockBlobClientForTest(RandomString());
    std::string tempFilename = RandomString();

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

  TEST_F(BlockBlobClientTest, ConcurrentDownload_LIVEONLY_)
  {
    auto blobClient = *m_blockBlobClient;
    const auto blobContent = RandomBuffer(static_cast<size_t>(8_MB));
    blobClient.UploadFrom(blobContent.data(), blobContent.size());

    auto testDownloadToBuffer = [&](int concurrency,
                                    int64_t downloadSize,
                                    Azure::Nullable<int64_t> offset = {},
                                    Azure::Nullable<int64_t> length = {},
                                    Azure::Nullable<int64_t> initialChunkSize = {},
                                    Azure::Nullable<int64_t> chunkSize = {}) {
      std::vector<uint8_t> downloadBuffer;
      std::vector<uint8_t> expectedData = blobContent;
      int64_t blobSize = blobContent.size();
      int64_t actualDownloadSize = std::min(downloadSize, blobSize);
      if (offset.HasValue() && length.HasValue())
      {
        actualDownloadSize = std::min(length.Value(), blobSize - offset.Value());
        if (actualDownloadSize >= 0)
        {
          expectedData.assign(
              blobContent.begin() + static_cast<ptrdiff_t>(offset.Value()),
              blobContent.begin() + static_cast<ptrdiff_t>(offset.Value() + actualDownloadSize));
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
              blobContent.begin() + static_cast<ptrdiff_t>(offset.Value()), blobContent.end());
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
        auto res = blobClient.DownloadTo(downloadBuffer.data(), downloadBuffer.size(), options);
        EXPECT_EQ(res.Value.BlobSize, blobSize);
        EXPECT_EQ(res.Value.ContentRange.Length.Value(), actualDownloadSize);
        EXPECT_EQ(res.Value.ContentRange.Offset, offset.HasValue() ? offset.Value() : 0);
        downloadBuffer.resize(static_cast<size_t>(res.Value.ContentRange.Length.Value()));
        EXPECT_EQ(downloadBuffer, expectedData);
      }
      else
      {
        EXPECT_THROW(
            blobClient.DownloadTo(downloadBuffer.data(), downloadBuffer.size(), options),
            StorageException);
      }
    };

    auto testDownloadToFile = [&](int concurrency,
                                  int64_t downloadSize,
                                  Azure::Nullable<int64_t> offset = {},
                                  Azure::Nullable<int64_t> length = {},
                                  Azure::Nullable<int64_t> initialChunkSize = {},
                                  Azure::Nullable<int64_t> chunkSize = {}) {
      std::string tempFilename = RandomString() + "file" + std::to_string(concurrency);
      if (offset)
      {
        tempFilename.append(std::to_string(offset.Value()));
      }
      std::vector<uint8_t> expectedData = blobContent;
      int64_t blobSize = blobContent.size();
      int64_t actualDownloadSize = std::min(downloadSize, blobSize);
      if (offset.HasValue() && length.HasValue())
      {
        actualDownloadSize = std::min(length.Value(), blobSize - offset.Value());
        if (actualDownloadSize >= 0)
        {
          expectedData.assign(
              blobContent.begin() + static_cast<ptrdiff_t>(offset.Value()),
              blobContent.begin() + static_cast<ptrdiff_t>(offset.Value() + actualDownloadSize));
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
              blobContent.begin() + static_cast<ptrdiff_t>(offset.Value()), blobContent.end());
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
        auto res = blobClient.DownloadTo(tempFilename, options);
        EXPECT_EQ(res.Value.BlobSize, blobSize);
        EXPECT_EQ(res.Value.ContentRange.Length.Value(), actualDownloadSize);
        EXPECT_EQ(res.Value.ContentRange.Offset, offset.HasValue() ? offset.Value() : 0);
        EXPECT_EQ(ReadFile(tempFilename), expectedData);
      }
      else
      {
        EXPECT_THROW(blobClient.DownloadTo(tempFilename, options), StorageException);
      }
      DeleteFile(tempFilename);
    };

    const int64_t blobSize = blobContent.size();
    std::vector<std::future<void>> futures;
    for (int c : {1, 2, 4})
    {
      // random range
      for (int i = 0; i < 16; ++i)
      {
        int64_t offset = RandomInt(0, blobContent.size() - 1);
        int64_t length = RandomInt(1, 64_KB);
        futures.emplace_back(std::async(
            std::launch::async, testDownloadToBuffer, c, blobSize, offset, length, 8_KB, 4_KB));
        futures.emplace_back(std::async(
            std::launch::async, testDownloadToFile, c, blobSize, offset, length, 4_KB, 7_KB));
      }

      // buffer not big enough
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
            blobClient.DownloadTo(downloadBuffer.data(), static_cast<size_t>(length - 1), options),
            std::runtime_error);
      }
    }
    for (auto& f : futures)
    {
      f.get();
    }
  }

  TEST_F(BlockBlobClientTest, ConcurrentUpload_LIVEONLY_)
  {

    const auto blobContent = RandomBuffer(static_cast<size_t>(8_MB));

    auto testUploadFromBuffer = [&](int concurrency,
                                    int64_t bufferSize,
                                    Azure::Nullable<int64_t> singleUploadThreshold = {},
                                    Azure::Nullable<int64_t> chunkSize = {}) {
      Blobs::UploadBlockBlobFromOptions options;
      options.TransferOptions.Concurrency = concurrency;
      if (singleUploadThreshold.HasValue())
      {
        options.TransferOptions.SingleUploadThreshold = singleUploadThreshold.Value();
      }
      if (chunkSize.HasValue())
      {
        options.TransferOptions.ChunkSize = chunkSize.Value();
      }

      auto blobClient = m_blobContainerClient->GetBlockBlobClient(RandomString());
      EXPECT_NO_THROW(
          blobClient.UploadFrom(blobContent.data(), static_cast<size_t>(bufferSize), options));
      std::vector<uint8_t> downloadBuffer(static_cast<size_t>(bufferSize), '\x00');
      Blobs::DownloadBlobToOptions downloadOptions;
      downloadOptions.TransferOptions.Concurrency = 1;
      ASSERT_NO_THROW(
          blobClient.DownloadTo(downloadBuffer.data(), downloadBuffer.size(), downloadOptions));
      std::vector<uint8_t> expectedData(
          blobContent.begin(), blobContent.begin() + static_cast<size_t>(bufferSize));
      EXPECT_EQ(downloadBuffer, expectedData);
    };

    auto testUploadFromFile = [&](int concurrency,
                                  int64_t fileSize,
                                  Azure::Nullable<int64_t> singleUploadThreshold = {},
                                  Azure::Nullable<int64_t> chunkSize = {}) {
      Blobs::UploadBlockBlobFromOptions options;
      options.TransferOptions.Concurrency = concurrency;
      if (singleUploadThreshold.HasValue())
      {
        options.TransferOptions.SingleUploadThreshold = singleUploadThreshold.Value();
      }
      if (chunkSize.HasValue())
      {
        options.TransferOptions.ChunkSize = chunkSize.Value();
      }

      const std::string tempFileName = RandomString();
      WriteFile(
          tempFileName,
          std::vector<uint8_t>(
              blobContent.begin(), blobContent.begin() + static_cast<size_t>(fileSize)));
      auto blobClient = m_blobContainerClient->GetBlockBlobClient(RandomString());
      EXPECT_NO_THROW(blobClient.UploadFrom(tempFileName, options));
      DeleteFile(tempFileName);
      std::vector<uint8_t> downloadBuffer(static_cast<size_t>(fileSize), '\x00');
      Blobs::DownloadBlobToOptions downloadOptions;
      downloadOptions.TransferOptions.Concurrency = 1;
      ASSERT_NO_THROW(
          blobClient.DownloadTo(downloadBuffer.data(), downloadBuffer.size(), downloadOptions));
      std::vector<uint8_t> expectedData(
          blobContent.begin(), blobContent.begin() + static_cast<size_t>(fileSize));
      EXPECT_EQ(downloadBuffer, expectedData);
    };

    for (int c : {1, 2, 4})
    {
      std::vector<std::future<void>> futures;
      // random range
      for (int i = 0; i < 16; ++i)
      {
        int64_t fileSize = RandomInt(1, 1_MB);
        futures.emplace_back(
            std::async(std::launch::async, testUploadFromBuffer, c, fileSize, 4_KB, 47_KB));
        futures.emplace_back(
            std::async(std::launch::async, testUploadFromFile, c, fileSize, 2_KB, 185_KB));
        futures.emplace_back(
            std::async(std::launch::async, testUploadFromBuffer, c, fileSize, 0, 117_KB));
        futures.emplace_back(
            std::async(std::launch::async, testUploadFromFile, c, fileSize, 0, 259_KB));
      }
      for (auto& f : futures)
      {
        f.get();
      }
    }
  }

}}} // namespace Azure::Storage::Test
