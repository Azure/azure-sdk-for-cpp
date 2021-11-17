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

}}}} // namespace Azure::Storage::Blobs::Models

namespace Azure { namespace Storage { namespace Test {

  std::shared_ptr<Azure::Storage::Blobs::BlockBlobClient> BlockBlobClientTest::m_blockBlobClient;
  std::string BlockBlobClientTest::m_blobName;
  Azure::Storage::Blobs::UploadBlockBlobOptions BlockBlobClientTest::m_blobUploadOptions;
  std::vector<uint8_t> BlockBlobClientTest::m_blobContent;

  void BlockBlobClientTest::SetUpTestSuite()
  {
    BlobContainerClientTest::SetUpTestSuite();

    m_blobName = RandomString();
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, m_blobName);
    m_blockBlobClient
        = std::make_shared<Azure::Storage::Blobs::BlockBlobClient>(std::move(blockBlobClient));
    m_blobContent.resize(static_cast<size_t>(8_MB));
    RandomBuffer(reinterpret_cast<char*>(&m_blobContent[0]), m_blobContent.size());
    m_blobUploadOptions.Metadata = {{"key1", "V1"}, {"key2", "Value2"}};
    m_blobUploadOptions.HttpHeaders.ContentType = "application/x-binary";
    m_blobUploadOptions.HttpHeaders.ContentLanguage = "en-US";
    m_blobUploadOptions.HttpHeaders.ContentDisposition = "attachment";
    m_blobUploadOptions.HttpHeaders.CacheControl = "no-cache";
    m_blobUploadOptions.HttpHeaders.ContentEncoding = "identity";
    m_blobUploadOptions.HttpHeaders.ContentHash.Value.clear();
    m_blobUploadOptions.AccessTier = Azure::Storage::Blobs::Models::AccessTier::Hot;
    auto blobContent
        = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    m_blockBlobClient->Upload(blobContent, m_blobUploadOptions);
    m_blobUploadOptions.HttpHeaders.ContentHash
        = m_blockBlobClient->GetProperties().Value.HttpHeaders.ContentHash;
  }

  void BlockBlobClientTest::TearDownTestSuite() { BlobContainerClientTest::TearDownTestSuite(); }

  TEST_F(BlockBlobClientTest, CreateDelete)
  {
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    auto blobContent
        = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    auto blobContentInfo = blockBlobClient.Upload(blobContent, m_blobUploadOptions);
    EXPECT_TRUE(blobContentInfo.Value.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(blobContentInfo.Value.LastModified));
    EXPECT_TRUE(blobContentInfo.Value.VersionId.HasValue());
    EXPECT_FALSE(blobContentInfo.Value.VersionId.Value().empty());
    EXPECT_FALSE(blobContentInfo.Value.EncryptionScope.HasValue());
    EXPECT_FALSE(blobContentInfo.Value.EncryptionKeySha256.HasValue());

    blockBlobClient.Delete();
    EXPECT_THROW(blockBlobClient.Delete(), StorageException);
  }

  TEST_F(BlockBlobClientTest, SoftDelete)
  {
    const std::string blobName = RandomString();
    std::vector<uint8_t> emptyContent;
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, blobName);
    auto blobContent = Azure::Core::IO::MemoryBodyStream(emptyContent.data(), emptyContent.size());
    blockBlobClient.Upload(blobContent);

    auto blobItem = GetBlobItem(blobName);
    EXPECT_FALSE(blobItem.IsDeleted);
    EXPECT_FALSE(blobItem.Details.DeletedOn.HasValue());
    EXPECT_FALSE(blobItem.Details.RemainingRetentionDays.HasValue());

    blockBlobClient.Delete();

    /*
    // Soft delete doesn't work in storage account with versioning enabled.
    blobItem = GetBlobItem(blobName, Blobs::Models::ListBlobsIncludeFlags::Deleted);
    EXPECT_TRUE(blobItem.IsDeleted);
    ASSERT_TRUE(blobItem.Details.DeletedOn.HasValue());
    EXPECT_TRUE(IsValidTime(blobItem.Details.DeletedOn.Value()));
    EXPECT_TRUE(blobItem.Details.RemainingRetentionDays.HasValue());
    */
  }

  TEST_F(BlockBlobClientTest, UploadDownload)
  {
    auto res = m_blockBlobClient->Download();
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
    res = m_blockBlobClient->Download(options);
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
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    std::map<std::string, std::string> tags;
    tags["key1"] = "value1";
    tags["key2"] = "value2";
    tags["key3 +-./:=_"] = "v1 +-./:=_";

    std::vector<uint8_t> blobContent(100, 'a');
    {
      Blobs::UploadBlockBlobOptions options;
      options.Tags = tags;
      auto stream = Azure::Core::IO::MemoryBodyStream(blobContent.data(), blobContent.size());
      blockBlobClient.Upload(stream, options);
      EXPECT_EQ(blockBlobClient.GetTags().Value, tags);
      blockBlobClient.Delete();
    }

    {
      Blobs::UploadBlockBlobFromOptions options;
      options.TransferOptions.SingleUploadThreshold = 0;
      options.TransferOptions.ChunkSize = blobContent.size() / 2;
      options.Tags = tags;

      {
        blockBlobClient.UploadFrom(blobContent.data(), blobContent.size(), options);
        EXPECT_EQ(blockBlobClient.GetTags().Value, tags);
        blockBlobClient.Delete();
      }
      {
        const std::string tempFilename = RandomString();
        {
          Azure::Storage::_internal::FileWriter fileWriter(tempFilename);
          fileWriter.Write(blobContent.data(), blobContent.size(), 0);
        }
        blockBlobClient.UploadFrom(tempFilename, options);
        EXPECT_EQ(blockBlobClient.GetTags().Value, tags);
        blockBlobClient.Delete();
      }
    }
  }

  TEST_F(BlockBlobClientTest, DownloadTransactionalHash)
  {
    const std::vector<uint8_t> dataPart1(static_cast<size_t>(4_MB + 1), 'a');
    const std::vector<uint8_t> dataPart2(static_cast<size_t>(4_MB + 1), 'b');

    const std::string blockId1 = Base64EncodeText("0");
    const std::string blockId2 = Base64EncodeText("1");

    auto blobClient = m_blobContainerClient->GetBlockBlobClient(RandomString());
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
    {
      auto res = m_blockBlobClient->Download();
      ASSERT_TRUE(res.Value.Details.LastAccessedOn.HasValue());
      EXPECT_TRUE(IsValidTime(res.Value.Details.LastAccessedOn.Value()));
    }
    {
      auto res = m_blockBlobClient->GetProperties();
      ASSERT_TRUE(res.Value.LastAccessedOn.HasValue());
      EXPECT_TRUE(IsValidTime(res.Value.LastAccessedOn.Value()));
    }
    {
      EXPECT_TRUE(IsValidTime(GetBlobItem(m_blobName).Details.LastAccessedOn.Value()));
    }
  }

  TEST_F(BlockBlobClientTest, DownloadEmpty)
  {
    std::vector<uint8_t> emptyContent;
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
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

    std::string tempFilename = RandomString();
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
    const std::string blobName = RandomString();
    auto blobClient = m_blobContainerClient->GetBlobClient(blobName);
    auto res = blobClient.CopyFromUri(m_blockBlobClient->GetUrl() + GetSas());
    EXPECT_EQ(res.RawResponse->GetStatusCode(), Azure::Core::Http::HttpStatusCode::Accepted);
    EXPECT_TRUE(res.Value.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(res.Value.LastModified));
    EXPECT_FALSE(res.Value.CopyId.empty());
    EXPECT_EQ(res.Value.CopyStatus, Azure::Storage::Blobs::Models::CopyStatus::Success);

    auto downloadResult = blobClient.Download();
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
    const std::string blobName = RandomString();
    auto blobClient = m_blobContainerClient->GetBlobClient(blobName);
    auto res = blobClient.StartCopyFromUri(m_blockBlobClient->GetUrl());
    EXPECT_EQ(res.GetRawResponse().GetStatusCode(), Azure::Core::Http::HttpStatusCode::Accepted);
    res.PollUntilDone(std::chrono::seconds(1));
    auto properties = blobClient.GetProperties().Value;
    EXPECT_FALSE(properties.CopyId.Value().empty());
    EXPECT_FALSE(properties.CopySource.Value().empty());
    EXPECT_TRUE(
        properties.CopyStatus.Value() == Azure::Storage::Blobs::Models::CopyStatus::Success);
    EXPECT_FALSE(properties.CopyProgress.Value().empty());
    EXPECT_TRUE(IsValidTime(properties.CopyCompletedOn.Value()));
    ASSERT_TRUE(properties.IsIncrementalCopy.HasValue());
    EXPECT_FALSE(properties.IsIncrementalCopy.Value());
    EXPECT_FALSE(properties.IncrementalCopyDestinationSnapshot.HasValue());

    auto downloadResult = blobClient.Download();
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
    auto blobClient = m_blobContainerClient->GetBlockBlobClient(RandomString());
    Blobs::StartBlobCopyFromUriOptions options;
    options.Tags["key1"] = "value1";
    options.Tags["key2"] = "value2";
    options.Tags["key3 +-./:=_"] = "v1 +-./:=_";
    options.Metadata["key1"] = "value1";
    options.Metadata["key2"] = "value2";
    options.AccessTier = Blobs::Models::AccessTier::Cool;
    auto operation = blobClient.StartCopyFromUri(m_blockBlobClient->GetUrl(), options);
    operation.PollUntilDone(std::chrono::seconds(1));
    EXPECT_EQ(blobClient.GetTags().Value, options.Tags);
    auto properties = blobClient.GetProperties().Value;
    EXPECT_EQ(properties.Metadata, options.Metadata);
    EXPECT_EQ(properties.AccessTier.Value(), options.AccessTier.Value());

    Blobs::CopyBlobFromUriOptions options2;
    options2.Tags = options.Tags;
    options2.Metadata = options.Metadata;
    options2.AccessTier = options.AccessTier;
    blobClient.CopyFromUri(m_blockBlobClient->GetUrl() + GetSas(), options2);
    EXPECT_EQ(blobClient.GetTags().Value, options2.Tags);
    properties = blobClient.GetProperties().Value;
    EXPECT_EQ(properties.Metadata, options2.Metadata);
    EXPECT_EQ(properties.AccessTier.Value(), options2.AccessTier.Value());
  }

  TEST_F(BlockBlobClientTest, SnapShotVersions)
  {
    auto res = m_blockBlobClient->CreateSnapshot();
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderRequestId).empty());
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderDate).empty());
    EXPECT_FALSE(res.RawResponse->GetHeaders().at(_internal::HttpHeaderXMsVersion).empty());
    EXPECT_TRUE(res.Value.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(res.Value.LastModified));
    EXPECT_FALSE(res.Value.Snapshot.empty());
    EXPECT_TRUE(res.Value.VersionId.HasValue());
    EXPECT_FALSE(res.Value.VersionId.Value().empty());
    auto snapshotClient = m_blockBlobClient->WithSnapshot(res.Value.Snapshot);
    EXPECT_EQ(ReadBodyStream(snapshotClient.Download().Value.BodyStream), m_blobContent);
    EXPECT_EQ(snapshotClient.GetProperties().Value.Metadata, m_blobUploadOptions.Metadata);
    EXPECT_TRUE(snapshotClient.GetProperties().Value.IsServerEncrypted);
    auto versionClient = m_blockBlobClient->WithVersionId(res.Value.VersionId.Value());
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
    res = m_blockBlobClient->CreateSnapshot(options);
    EXPECT_FALSE(res.Value.Snapshot.empty());
    snapshotClient = m_blockBlobClient->WithSnapshot(res.Value.Snapshot);
    EXPECT_EQ(snapshotClient.GetProperties().Value.Metadata, options.Metadata);

    EXPECT_NO_THROW(snapshotClient.Delete());
    EXPECT_NO_THROW(versionClient.Delete());
    EXPECT_NO_THROW(m_blockBlobClient->GetProperties());
  }

  TEST_F(BlockBlobClientTest, IsCurrentVersion)
  {
    std::vector<uint8_t> emptyContent;
    std::string blobName = RandomString();
    auto blobClient = m_blobContainerClient->GetBlockBlobClient(blobName);
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

    auto blobItem = GetBlobItem(blobName, Blobs::Models::ListBlobsIncludeFlags::Versions);
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
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
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
    const std::string blockId1 = Base64EncodeText("0");
    const std::string blockId2 = Base64EncodeText("1");
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
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

    blockBlobClient.StageBlockFromUri(blockId2, m_blockBlobClient->GetUrl() + GetSas());
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

  TEST_F(BlockBlobClientTest, ConcurrentDownload)
  {
    auto testDownloadToBuffer = [](int concurrency,
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
        auto res
            = m_blockBlobClient->DownloadTo(downloadBuffer.data(), downloadBuffer.size(), options);
        EXPECT_EQ(res.Value.BlobSize, blobSize);
        EXPECT_EQ(res.Value.ContentRange.Length.Value(), actualDownloadSize);
        EXPECT_EQ(res.Value.ContentRange.Offset, offset.HasValue() ? offset.Value() : 0);
        downloadBuffer.resize(static_cast<size_t>(res.Value.ContentRange.Length.Value()));
        EXPECT_EQ(downloadBuffer, expectedData);
      }
      else
      {
        EXPECT_THROW(
            m_blockBlobClient->DownloadTo(downloadBuffer.data(), downloadBuffer.size(), options),
            StorageException);
      }
    };
    auto testDownloadToFile = [](int concurrency,
                                 int64_t downloadSize,
                                 Azure::Nullable<int64_t> offset = {},
                                 Azure::Nullable<int64_t> length = {},
                                 Azure::Nullable<int64_t> initialChunkSize = {},
                                 Azure::Nullable<int64_t> chunkSize = {}) {
      std::string tempFilename = RandomString();
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
        auto res = m_blockBlobClient->DownloadTo(tempFilename, options);
        EXPECT_EQ(res.Value.BlobSize, blobSize);
        EXPECT_EQ(res.Value.ContentRange.Length.Value(), actualDownloadSize);
        EXPECT_EQ(res.Value.ContentRange.Offset, offset.HasValue() ? offset.Value() : 0);
        EXPECT_EQ(ReadFile(tempFilename), expectedData);
      }
      else
      {
        EXPECT_THROW(m_blockBlobClient->DownloadTo(tempFilename, options), StorageException);
      }
      DeleteFile(tempFilename);
    };

    const int64_t blobSize = m_blobContent.size();
    std::vector<std::future<void>> futures;
    for (int c : {1, 2, 4})
    {
      // download whole blob
      futures.emplace_back(std::async(std::launch::async, testDownloadToBuffer, c, blobSize));
      futures.emplace_back(std::async(std::launch::async, testDownloadToFile, c, blobSize));
      futures.emplace_back(std::async(std::launch::async, testDownloadToBuffer, c, blobSize, 0));
      futures.emplace_back(std::async(std::launch::async, testDownloadToFile, c, blobSize, 0));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToBuffer, c, blobSize, 0, blobSize));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToFile, c, blobSize, 0, blobSize));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToBuffer, c, blobSize, 0, blobSize * 2));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToFile, c, blobSize, 0, blobSize * 2));
      futures.emplace_back(std::async(std::launch::async, testDownloadToBuffer, c, blobSize * 2));
      futures.emplace_back(std::async(std::launch::async, testDownloadToFile, c, blobSize * 2));

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

      futures.emplace_back(std::async(std::launch::async, testDownloadToBuffer, c, blobSize, 0, 1));
      futures.emplace_back(std::async(std::launch::async, testDownloadToFile, c, blobSize, 0, 1));
      futures.emplace_back(std::async(std::launch::async, testDownloadToBuffer, c, blobSize, 1, 1));
      futures.emplace_back(std::async(std::launch::async, testDownloadToFile, c, blobSize, 1, 1));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToBuffer, c, blobSize, blobSize - 1, 1));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToFile, c, blobSize, blobSize - 1, 1));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToBuffer, c, blobSize, blobSize - 1, 2));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToFile, c, blobSize, blobSize - 1, 2));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToBuffer, c, blobSize, blobSize, 1));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToFile, c, blobSize, blobSize, 1));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToBuffer, c, blobSize, blobSize + 1, 2));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToFile, c, blobSize, blobSize + 1, 2));

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
            m_blockBlobClient->DownloadTo(
                downloadBuffer.data(), static_cast<size_t>(length - 1), options),
            std::runtime_error);
      }

      // initial chunk size
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToBuffer, c, blobSize, 0, 1024, 512, 1024));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToFile, c, blobSize, 0, 1024, 512, 1024));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToBuffer, c, blobSize, 0, 1024, 1024, 1024));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToFile, c, blobSize, 0, 1024, 1024, 1024));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToBuffer, c, blobSize, 0, 1024, 2048, 1024));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToFile, c, blobSize, 0, 1024, 2048, 1024));
    }
    for (auto& f : futures)
    {
      f.get();
    }
  }

  TEST_F(BlockBlobClientTest, ConcurrentUploadFromNonExistingFile)
  {
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    std::string emptyFilename = RandomString();
    EXPECT_THROW(blockBlobClient.UploadFrom(emptyFilename), std::runtime_error);
    EXPECT_THROW(blockBlobClient.Delete(), StorageException);
  }

  TEST_F(BlockBlobClientTest, ConcurrentDownloadNonExistingBlob)
  {
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    std::vector<uint8_t> blobContent(100);
    std::string tempFilename = RandomString();

    EXPECT_THROW(
        blockBlobClient.DownloadTo(blobContent.data(), blobContent.size()), StorageException);
    EXPECT_THROW(blockBlobClient.DownloadTo(tempFilename), StorageException);
    DeleteFile(tempFilename);
  }

  TEST_F(BlockBlobClientTest, ConcurrentUploadEmptyBlob)
  {
    std::vector<uint8_t> emptyContent;
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());

    blockBlobClient.UploadFrom(emptyContent.data(), emptyContent.size());
    EXPECT_NO_THROW(blockBlobClient.Delete());

    std::string emptyFilename = RandomString();
    {
      _internal::FileWriter writer(emptyFilename);
    }
    blockBlobClient.UploadFrom(emptyFilename);
    EXPECT_NO_THROW(blockBlobClient.Delete());

    DeleteFile(emptyFilename);
  }

  TEST_F(BlockBlobClientTest, ConcurrentDownloadEmptyBlob)
  {
    std::string tempFilename = RandomString();

    std::vector<uint8_t> emptyContent;
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
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

  TEST_F(BlockBlobClientTest, ConcurrentUpload)
  {
    std::vector<uint8_t> blobContent = RandomBuffer(static_cast<size_t>(8_MB));

    auto testUploadFromBuffer = [&](int concurrency, int64_t blobSize) {
      auto blockBlobClient = m_blobContainerClient->GetBlockBlobClient(RandomString());

      Azure::Storage::Blobs::UploadBlockBlobFromOptions options;
      options.TransferOptions.ChunkSize = 1_MB;
      options.TransferOptions.Concurrency = concurrency;
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
    };

    auto testUploadFromFile = [&](int concurrency, int64_t blobSize) {
      auto blockBlobClient = m_blobContainerClient->GetBlockBlobClient(RandomString());

      Azure::Storage::Blobs::UploadBlockBlobFromOptions options;
      options.TransferOptions.ChunkSize = 1_MB;
      options.TransferOptions.Concurrency = concurrency;
      options.HttpHeaders = m_blobUploadOptions.HttpHeaders;
      options.HttpHeaders.ContentHash.Value.clear();
      options.Metadata = m_blobUploadOptions.Metadata;
      options.AccessTier = m_blobUploadOptions.AccessTier;

      std::string tempFilename = RandomString();
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
    };

    std::vector<std::future<void>> futures;
    for (int c : {1, 2, 5})
    {
      for (int64_t l :
           {0ULL, 1ULL, 2ULL, 2_KB, 4_KB, 999_KB, 1_MB, 2_MB - 1, 3_MB, 5_MB, 8_MB - 1234, 8_MB})
      {
        ASSERT_GE(blobContent.size(), static_cast<size_t>(l));
        futures.emplace_back(std::async(std::launch::async, testUploadFromBuffer, c, l));
        futures.emplace_back(std::async(std::launch::async, testUploadFromFile, c, l));
      }
    }
    for (auto& f : futures)
    {
      f.get();
    }
  }

  TEST_F(BlockBlobClientTest, DownloadError)
  {
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
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
    auto blobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    auto blobClientWithoutAuth = Azure::Storage::Blobs::BlockBlobClient(blobClient.GetUrl());
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
    std::vector<uint8_t> emptyContent;
    auto blobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
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
    std::vector<uint8_t> emptyContent;
    std::string blobName = RandomString();
    auto blobClient = m_blobContainerClient->GetBlockBlobClient(blobName);
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
    std::vector<uint8_t> emptyContent;
    auto blobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
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
    const std::string blobName = RandomString();
    auto blobClient = m_blobContainerClient->GetBlockBlobClient(blobName);

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
    auto sourceBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    std::vector<uint8_t> buffer;
    buffer.resize(1024);
  }

  TEST_F(BlobContainerClientTest, SourceBlobAccessConditions)
  {
    auto sourceBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    std::vector<uint8_t> buffer;
    buffer.resize(1024);
    auto createResponse = sourceBlobClient.UploadFrom(buffer.data(), buffer.size());
    Azure::ETag eTag = createResponse.Value.ETag;
    auto lastModifiedTime = createResponse.Value.LastModified;
    auto timeBeforeStr = lastModifiedTime - std::chrono::seconds(2);
    auto timeAfterStr = lastModifiedTime + std::chrono::seconds(2);

    auto destBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());

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

    // content md5
    {
      const auto hash = sourceBlobClient.GetProperties().Value.HttpHeaders.ContentHash;
      ASSERT_FALSE(hash.Value.empty());

      Blobs::CopyBlobFromUriOptions options;
      options.TransactionalContentHash = hash;
      options.TransactionalContentHash.Value().Value = Azure::Core::Convert::Base64Decode(DummyMd5);
      EXPECT_THROW(
          destBlobClient.CopyFromUri(sourceBlobClient.GetUrl() + GetSas(), options),
          StorageException);
      options.TransactionalContentHash = hash;
      EXPECT_NO_THROW(destBlobClient.CopyFromUri(sourceBlobClient.GetUrl() + GetSas(), options));
    }
  }

}}} // namespace Azure::Storage::Test
