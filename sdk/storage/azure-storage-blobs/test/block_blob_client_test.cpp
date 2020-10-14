// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "block_blob_client_test.hpp"

#include "azure/storage/common/crypt.hpp"
#include "azure/storage/common/file_io.hpp"

#include <future>
#include <random>
#include <vector>

namespace Azure { namespace Storage { namespace Blobs {

  bool operator==(
      const Azure::Storage::Blobs::BlobHttpHeaders& lhs,
      const Azure::Storage::Blobs::BlobHttpHeaders& rhs)
  {
    return lhs.ContentType == rhs.ContentType && lhs.ContentEncoding == rhs.ContentEncoding
        && lhs.ContentLanguage == rhs.ContentLanguage && lhs.ContentMd5 == rhs.ContentMd5
        && lhs.CacheControl == rhs.CacheControl && lhs.ContentDisposition == rhs.ContentDisposition;
  }

}}} // namespace Azure::Storage::Blobs

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
    m_blobContent.resize(static_cast<std::size_t>(8_MB));
    RandomBuffer(reinterpret_cast<char*>(&m_blobContent[0]), m_blobContent.size());
    m_blobUploadOptions.Metadata = {{"key1", "V1"}, {"key2", "Value2"}};
    m_blobUploadOptions.HttpHeaders.ContentType = "application/x-binary";
    m_blobUploadOptions.HttpHeaders.ContentLanguage = "en-US";
    m_blobUploadOptions.HttpHeaders.ContentDisposition = "attachment";
    m_blobUploadOptions.HttpHeaders.CacheControl = "no-cache";
    m_blobUploadOptions.HttpHeaders.ContentEncoding = "identity";
    m_blobUploadOptions.HttpHeaders.ContentMd5 = "";
    m_blobUploadOptions.Tier = Azure::Storage::Blobs::AccessTier::Hot;
    auto blobContent
        = Azure::Core::Http::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    m_blockBlobClient->Upload(&blobContent, m_blobUploadOptions);
    m_blobUploadOptions.HttpHeaders.ContentMd5
        = m_blockBlobClient->GetProperties()->HttpHeaders.ContentMd5;
  }

  void BlockBlobClientTest::TearDownTestSuite() { BlobContainerClientTest::TearDownTestSuite(); }

  TEST_F(BlockBlobClientTest, CreateDelete)
  {
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    auto blobContent
        = Azure::Core::Http::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    auto blobContentInfo = blockBlobClient.Upload(&blobContent, m_blobUploadOptions);
    EXPECT_FALSE(blobContentInfo->ETag.empty());
    EXPECT_FALSE(blobContentInfo->LastModified.empty());
    EXPECT_TRUE(blobContentInfo->VersionId.HasValue());
    EXPECT_FALSE(blobContentInfo->VersionId.GetValue().empty());
    EXPECT_FALSE(blobContentInfo->EncryptionScope.HasValue());
    EXPECT_FALSE(blobContentInfo->EncryptionKeySha256.HasValue());

    blockBlobClient.Delete();
    EXPECT_THROW(blockBlobClient.Delete(), StorageError);
  }

  TEST_F(BlockBlobClientTest, UploadDownload)
  {
    auto res = m_blockBlobClient->Download();
    EXPECT_EQ(ReadBodyStream(res->BodyStream), m_blobContent);
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderRequestId).empty());
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderDate).empty());
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderXMsVersion).empty());
    EXPECT_FALSE(res->ETag.empty());
    EXPECT_FALSE(res->LastModified.empty());
    EXPECT_FALSE(res->CreationTime.empty());
    EXPECT_EQ(res->HttpHeaders, m_blobUploadOptions.HttpHeaders);
    EXPECT_EQ(res->Metadata, m_blobUploadOptions.Metadata);
    EXPECT_EQ(res->BlobType, Azure::Storage::Blobs::BlobType::BlockBlob);
    Azure::Storage::Blobs::DownloadBlobOptions options;
    options.Offset = 1_MB;
    options.Length = 2_MB;
    res = m_blockBlobClient->Download(options);
    EXPECT_EQ(
        ReadBodyStream(res->BodyStream),
        std::vector<uint8_t>(
            m_blobContent.begin() + static_cast<std::size_t>(options.Offset.GetValue()),
            m_blobContent.begin()
                + static_cast<std::size_t>(options.Offset.GetValue() + options.Length.GetValue())));
    EXPECT_FALSE(res->ContentRange.GetValue().empty());
  }

  TEST_F(BlockBlobClientTest, LastAccessTime)
  {
    {
      auto res = m_blockBlobClient->Download();
      ASSERT_TRUE(res->LastAccessTime.HasValue());
      EXPECT_FALSE(res->LastAccessTime.GetValue().empty());
    }
    {
      auto res = m_blockBlobClient->GetProperties();
      ASSERT_TRUE(res->LastAccessTime.HasValue());
      EXPECT_FALSE(res->LastAccessTime.GetValue().empty());
    }
    {
      std::string lastAccessTime;

      Azure::Storage::Blobs::ListBlobsSegmentOptions options;
      options.Prefix = m_blobName;
      do
      {
        auto res = m_blobContainerClient->ListBlobsFlatSegment(options);
        options.ContinuationToken = res->ContinuationToken;
        for (const auto& blob : res->Items)
        {
          if (blob.Name == m_blobName)
          {
            lastAccessTime = blob.LastAccessTime.GetValue();
            break;
          }
        }
      } while (!options.ContinuationToken.GetValue().empty());

      EXPECT_FALSE(lastAccessTime.empty());
    }
  }

  TEST_F(BlockBlobClientTest, DownloadEmpty)
  {
    std::vector<uint8_t> emptyContent;
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    auto blobContent
        = Azure::Core::Http::MemoryBodyStream(emptyContent.data(), emptyContent.size());
    blockBlobClient.Upload(&blobContent);
    blockBlobClient.SetHttpHeaders(m_blobUploadOptions.HttpHeaders);
    blockBlobClient.SetMetadata(m_blobUploadOptions.Metadata);

    auto res = blockBlobClient.Download();
    EXPECT_EQ(res->BodyStream->Length(), 0);
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderRequestId).empty());
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderDate).empty());
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderXMsVersion).empty());
    EXPECT_FALSE(res->ETag.empty());
    EXPECT_FALSE(res->LastModified.empty());
    EXPECT_EQ(res->HttpHeaders, m_blobUploadOptions.HttpHeaders);
    EXPECT_EQ(res->Metadata, m_blobUploadOptions.Metadata);
    EXPECT_EQ(res->BlobType, Azure::Storage::Blobs::BlobType::BlockBlob);

    Azure::Storage::Blobs::DownloadBlobOptions options;
    options.Offset = 0;
    EXPECT_THROW(blockBlobClient.Download(options), StorageError);
    options.Length = 1;
    EXPECT_THROW(blockBlobClient.Download(options), StorageError);
  }

  TEST_F(BlockBlobClientTest, CopyFromUri)
  {
    auto blobClient = m_blobContainerClient->GetBlobClient(RandomString());
    auto res = blobClient.StartCopyFromUri(m_blockBlobClient->GetUri());

    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderRequestId).empty());
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderDate).empty());
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderXMsVersion).empty());
    EXPECT_FALSE(res->ETag.empty());
    EXPECT_FALSE(res->LastModified.empty());
    EXPECT_FALSE(res->CopyId.empty());
    EXPECT_TRUE(res->VersionId.HasValue());
    EXPECT_FALSE(res->VersionId.GetValue().empty());
    EXPECT_TRUE(
        res->CopyStatus == Azure::Storage::Blobs::CopyStatus::Pending
        || res->CopyStatus == Azure::Storage::Blobs::CopyStatus::Success);
    auto properties = *blobClient.GetProperties();
    EXPECT_EQ(properties.CopyId.GetValue(), res->CopyId);
    EXPECT_FALSE(properties.CopySource.GetValue().empty());
    EXPECT_TRUE(
        properties.CopyStatus.GetValue() == Azure::Storage::Blobs::CopyStatus::Pending
        || properties.CopyStatus.GetValue() == Azure::Storage::Blobs::CopyStatus::Success);
    EXPECT_FALSE(properties.CopyProgress.GetValue().empty());
    if (properties.CopyStatus.GetValue() == Azure::Storage::Blobs::CopyStatus::Success)
    {
      EXPECT_FALSE(properties.CopyCompletionTime.GetValue().empty());
    }
  }

  TEST_F(BlockBlobClientTest, SnapShotVersions)
  {
    auto res = m_blockBlobClient->CreateSnapshot();
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderRequestId).empty());
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderDate).empty());
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderXMsVersion).empty());
    EXPECT_FALSE(res->ETag.empty());
    EXPECT_FALSE(res->LastModified.empty());
    EXPECT_FALSE(res->Snapshot.empty());
    EXPECT_TRUE(res->VersionId.HasValue());
    EXPECT_FALSE(res->VersionId.GetValue().empty());
    auto snapshotClient = m_blockBlobClient->WithSnapshot(res->Snapshot);
    EXPECT_EQ(ReadBodyStream(snapshotClient.Download()->BodyStream), m_blobContent);
    EXPECT_EQ(snapshotClient.GetProperties()->Metadata, m_blobUploadOptions.Metadata);
    auto versionClient = m_blockBlobClient->WithVersionId(res->VersionId.GetValue());
    EXPECT_EQ(ReadBodyStream(versionClient.Download()->BodyStream), m_blobContent);
    EXPECT_EQ(versionClient.GetProperties()->Metadata, m_blobUploadOptions.Metadata);
    auto emptyContent = Azure::Core::Http::MemoryBodyStream(nullptr, 0);
    EXPECT_THROW(snapshotClient.Upload(&emptyContent), StorageError);
    EXPECT_THROW(snapshotClient.SetMetadata({}), StorageError);
    EXPECT_NO_THROW(snapshotClient.SetAccessTier(Azure::Storage::Blobs::AccessTier::Cool));
    EXPECT_THROW(
        snapshotClient.SetHttpHeaders(Azure::Storage::Blobs::BlobHttpHeaders()), StorageError);
    EXPECT_THROW(versionClient.Upload(&emptyContent), StorageError);
    EXPECT_THROW(versionClient.SetMetadata({}), StorageError);
    EXPECT_NO_THROW(versionClient.SetAccessTier(Azure::Storage::Blobs::AccessTier::Cool));
    EXPECT_THROW(
        versionClient.SetHttpHeaders(Azure::Storage::Blobs::BlobHttpHeaders()), StorageError);

    Azure::Storage::Blobs::CreateBlobSnapshotOptions options;
    options.Metadata = {{"snapshotkey1", "snapshotvalue1"}, {"snapshotkey2", "SNAPSHOTVALUE2"}};
    res = m_blockBlobClient->CreateSnapshot(options);
    EXPECT_FALSE(res->Snapshot.empty());
    snapshotClient = m_blockBlobClient->WithSnapshot(res->Snapshot);
    EXPECT_EQ(snapshotClient.GetProperties()->Metadata, options.Metadata);

    EXPECT_NO_THROW(snapshotClient.Delete());
    EXPECT_NO_THROW(versionClient.Delete());
    EXPECT_NO_THROW(m_blockBlobClient->GetProperties());
  }

  TEST_F(BlockBlobClientTest, Properties)
  {
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    auto blobContent
        = Azure::Core::Http::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    blockBlobClient.Upload(&blobContent);
    blockBlobClient.SetMetadata(m_blobUploadOptions.Metadata);
    blockBlobClient.SetAccessTier(Azure::Storage::Blobs::AccessTier::Cool);
    blockBlobClient.SetHttpHeaders(m_blobUploadOptions.HttpHeaders);

    auto res = blockBlobClient.GetProperties();
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderRequestId).empty());
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderDate).empty());
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderXMsVersion).empty());
    EXPECT_FALSE(res->ETag.empty());
    EXPECT_FALSE(res->LastModified.empty());
    EXPECT_FALSE(res->CreationTime.empty());
    EXPECT_EQ(res->Metadata, m_blobUploadOptions.Metadata);
    EXPECT_EQ(res->ContentLength, static_cast<int64_t>(m_blobContent.size()));
    EXPECT_EQ(res->HttpHeaders, m_blobUploadOptions.HttpHeaders);
    EXPECT_EQ(res->Tier.GetValue(), Azure::Storage::Blobs::AccessTier::Cool);
    EXPECT_FALSE(res->AccessTierChangeTime.GetValue().empty());
  }

  TEST_F(BlockBlobClientTest, StageBlock)
  {
    const std::string blockId1 = Azure::Storage::Base64Encode("0");
    const std::string blockId2 = Azure::Storage::Base64Encode("1");
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    std::vector<uint8_t> block1Content;
    block1Content.resize(100);
    RandomBuffer(reinterpret_cast<char*>(&block1Content[0]), block1Content.size());
    auto blockContent
        = Azure::Core::Http::MemoryBodyStream(block1Content.data(), block1Content.size());
    blockBlobClient.StageBlock(blockId1, &blockContent);
    Azure::Storage::Blobs::CommitBlockListOptions options;
    options.HttpHeaders = m_blobUploadOptions.HttpHeaders;
    options.Metadata = m_blobUploadOptions.Metadata;
    auto blobContentInfo = blockBlobClient.CommitBlockList(
        {{Azure::Storage::Blobs::BlockType::Uncommitted, blockId1}}, options);
    EXPECT_FALSE(blobContentInfo->ETag.empty());
    EXPECT_FALSE(blobContentInfo->LastModified.empty());
    EXPECT_TRUE(blobContentInfo->VersionId.HasValue());
    EXPECT_FALSE(blobContentInfo->VersionId.GetValue().empty());
    auto res = blockBlobClient.GetBlockList();
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderRequestId).empty());
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderDate).empty());
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderXMsVersion).empty());
    EXPECT_FALSE(res->ETag.empty());
    EXPECT_FALSE(res->LastModified.empty());
    EXPECT_EQ(res->ContentLength, static_cast<int64_t>(block1Content.size()));
    ASSERT_FALSE(res->CommittedBlocks.empty());
    EXPECT_EQ(res->CommittedBlocks[0].Name, blockId1);
    EXPECT_EQ(res->CommittedBlocks[0].Size, static_cast<int64_t>(block1Content.size()));
    EXPECT_TRUE(res->UncommittedBlocks.empty());

    blockBlobClient.StageBlockFromUri(blockId2, m_blockBlobClient->GetUri() + GetSas());
    Blobs::GetBlockListOptions options2;
    options2.ListType = Blobs::BlockListTypeOption::All;
    res = blockBlobClient.GetBlockList(options2);
    EXPECT_EQ(res->ContentLength, static_cast<int64_t>(block1Content.size()));
    ASSERT_FALSE(res->UncommittedBlocks.empty());
    EXPECT_EQ(res->UncommittedBlocks[0].Name, blockId2);
    EXPECT_EQ(res->UncommittedBlocks[0].Size, static_cast<int64_t>(m_blobContent.size()));

    blockBlobClient.CommitBlockList(
        {{Azure::Storage::Blobs::BlockType::Committed, blockId1},
         {Azure::Storage::Blobs::BlockType::Uncommitted, blockId2}});
    res = blockBlobClient.GetBlockList(options2);
    EXPECT_EQ(
        res->ContentLength, static_cast<int64_t>(block1Content.size() + m_blobContent.size()));
    EXPECT_TRUE(res->UncommittedBlocks.empty());
  }

  TEST_F(BlockBlobClientTest, ConcurrentDownload)
  {
    auto testDownloadToBuffer = [](int concurrency,
                                   int64_t downloadSize,
                                   Azure::Core::Nullable<int64_t> offset = {},
                                   Azure::Core::Nullable<int64_t> length = {},
                                   Azure::Core::Nullable<int64_t> initialChunkSize = {},
                                   Azure::Core::Nullable<int64_t> chunkSize = {}) {
      std::vector<uint8_t> downloadBuffer;
      std::vector<uint8_t> expectedData = m_blobContent;
      int64_t blobSize = m_blobContent.size();
      int64_t actualDownloadSize = std::min(downloadSize, blobSize);
      if (offset.HasValue() && length.HasValue())
      {
        actualDownloadSize = std::min(length.GetValue(), blobSize - offset.GetValue());
        if (actualDownloadSize >= 0)
        {
          expectedData.assign(
              m_blobContent.begin() + static_cast<std::ptrdiff_t>(offset.GetValue()),
              m_blobContent.begin()
                  + static_cast<std::ptrdiff_t>(offset.GetValue() + actualDownloadSize));
        }
        else
        {
          expectedData.clear();
        }
      }
      else if (offset.HasValue())
      {
        actualDownloadSize = blobSize - offset.GetValue();
        if (actualDownloadSize >= 0)
        {
          expectedData.assign(
              m_blobContent.begin() + static_cast<std::ptrdiff_t>(offset.GetValue()),
              m_blobContent.end());
        }
        else
        {
          expectedData.clear();
        }
      }
      downloadBuffer.resize(static_cast<std::size_t>(downloadSize), '\x00');
      Blobs::DownloadBlobToOptions options;
      options.Concurrency = concurrency;
      options.Offset = offset;
      options.Length = length;
      options.InitialChunkSize = initialChunkSize;
      options.ChunkSize = chunkSize;
      if (actualDownloadSize > 0)
      {
        auto res
            = m_blockBlobClient->DownloadTo(downloadBuffer.data(), downloadBuffer.size(), options);
        EXPECT_EQ(res->ContentLength, actualDownloadSize);
        downloadBuffer.resize(static_cast<std::size_t>(res->ContentLength));
        EXPECT_EQ(downloadBuffer, expectedData);
      }
      else
      {
        EXPECT_THROW(
            m_blockBlobClient->DownloadTo(downloadBuffer.data(), downloadBuffer.size(), options),
            StorageError);
      }
    };
    auto testDownloadToFile = [](int concurrency,
                                 int64_t downloadSize,
                                 Azure::Core::Nullable<int64_t> offset = {},
                                 Azure::Core::Nullable<int64_t> length = {},
                                 Azure::Core::Nullable<int64_t> initialChunkSize = {},
                                 Azure::Core::Nullable<int64_t> chunkSize = {}) {
      std::string tempFilename = RandomString();
      std::vector<uint8_t> expectedData = m_blobContent;
      int64_t blobSize = m_blobContent.size();
      int64_t actualDownloadSize = std::min(downloadSize, blobSize);
      if (offset.HasValue() && length.HasValue())
      {
        actualDownloadSize = std::min(length.GetValue(), blobSize - offset.GetValue());
        if (actualDownloadSize >= 0)
        {
          expectedData.assign(
              m_blobContent.begin() + static_cast<std::ptrdiff_t>(offset.GetValue()),
              m_blobContent.begin()
                  + static_cast<std::ptrdiff_t>(offset.GetValue() + actualDownloadSize));
        }
        else
        {
          expectedData.clear();
        }
      }
      else if (offset.HasValue())
      {
        actualDownloadSize = blobSize - offset.GetValue();
        if (actualDownloadSize >= 0)
        {
          expectedData.assign(
              m_blobContent.begin() + static_cast<std::ptrdiff_t>(offset.GetValue()),
              m_blobContent.end());
        }
        else
        {
          expectedData.clear();
        }
      }
      Blobs::DownloadBlobToOptions options;
      options.Concurrency = concurrency;
      options.Offset = offset;
      options.Length = length;
      options.InitialChunkSize = initialChunkSize;
      options.ChunkSize = chunkSize;
      if (actualDownloadSize > 0)
      {
        auto res = m_blockBlobClient->DownloadTo(tempFilename, options);
        EXPECT_EQ(res->ContentLength, actualDownloadSize);
        EXPECT_EQ(ReadFile(tempFilename), expectedData);
      }
      else
      {
        EXPECT_THROW(m_blockBlobClient->DownloadTo(tempFilename, options), StorageError);
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
      options.Concurrency = c;
      options.Offset = 1;
      for (int64_t length : {1ULL, 2ULL, 4_KB, 5_KB, 8_KB, 11_KB, 20_KB})
      {
        std::vector<uint8_t> downloadBuffer;
        downloadBuffer.resize(static_cast<std::size_t>(length - 1));
        options.Length = length;
        EXPECT_THROW(
            m_blockBlobClient->DownloadTo(
                downloadBuffer.data(), static_cast<std::size_t>(length - 1), options),
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
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    std::string emptyFilename = RandomString();
    EXPECT_THROW(blockBlobClient.UploadFrom(emptyFilename), std::runtime_error);
    EXPECT_THROW(blockBlobClient.Delete(), StorageError);
  }

  TEST_F(BlockBlobClientTest, ConcurrentDownloadNonExistingBlob)
  {
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    std::vector<uint8_t> blobContent(100);
    std::string tempFilename = RandomString();

    EXPECT_THROW(blockBlobClient.DownloadTo(blobContent.data(), blobContent.size()), StorageError);
    EXPECT_THROW(blockBlobClient.DownloadTo(tempFilename), StorageError);
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
      Details::FileWriter writer(emptyFilename);
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
    auto blobContent
        = Azure::Core::Http::MemoryBodyStream(emptyContent.data(), emptyContent.size());
    blockBlobClient.Upload(&blobContent);
    blockBlobClient.SetHttpHeaders(m_blobUploadOptions.HttpHeaders);
    blockBlobClient.SetMetadata(m_blobUploadOptions.Metadata);

    auto res = blockBlobClient.DownloadTo(emptyContent.data(), 0);
    EXPECT_EQ(res->ContentLength, 0);
    EXPECT_FALSE(res->ETag.empty());
    EXPECT_FALSE(res->LastModified.empty());
    EXPECT_EQ(res->HttpHeaders, m_blobUploadOptions.HttpHeaders);
    EXPECT_EQ(res->Metadata, m_blobUploadOptions.Metadata);
    EXPECT_EQ(res->BlobType, Azure::Storage::Blobs::BlobType::BlockBlob);
    res = blockBlobClient.DownloadTo(tempFilename);
    EXPECT_EQ(res->ContentLength, 0);
    EXPECT_FALSE(res->ETag.empty());
    EXPECT_FALSE(res->LastModified.empty());
    EXPECT_EQ(res->HttpHeaders, m_blobUploadOptions.HttpHeaders);
    EXPECT_EQ(res->Metadata, m_blobUploadOptions.Metadata);
    EXPECT_EQ(res->BlobType, Azure::Storage::Blobs::BlobType::BlockBlob);
    EXPECT_TRUE(ReadFile(tempFilename).empty());
    DeleteFile(tempFilename);

    res = blockBlobClient.DownloadTo(emptyContent.data(), static_cast<std::size_t>(8_MB));
    EXPECT_EQ(res->ContentLength, 0);
    EXPECT_FALSE(res->ETag.empty());
    EXPECT_FALSE(res->LastModified.empty());
    EXPECT_EQ(res->HttpHeaders, m_blobUploadOptions.HttpHeaders);
    EXPECT_EQ(res->Metadata, m_blobUploadOptions.Metadata);
    EXPECT_EQ(res->BlobType, Azure::Storage::Blobs::BlobType::BlockBlob);
    res = blockBlobClient.DownloadTo(tempFilename);
    EXPECT_EQ(res->ContentLength, 0);
    EXPECT_FALSE(res->ETag.empty());
    EXPECT_FALSE(res->LastModified.empty());
    EXPECT_EQ(res->HttpHeaders, m_blobUploadOptions.HttpHeaders);
    EXPECT_EQ(res->Metadata, m_blobUploadOptions.Metadata);
    EXPECT_EQ(res->BlobType, Azure::Storage::Blobs::BlobType::BlockBlob);
    EXPECT_TRUE(ReadFile(tempFilename).empty());
    DeleteFile(tempFilename);

    for (int c : {1, 2})
    {
      Azure::Storage::Blobs::DownloadBlobToOptions options;
      options.InitialChunkSize = 10;
      options.ChunkSize = 10;
      options.Concurrency = c;

      res = blockBlobClient.DownloadTo(
          emptyContent.data(), static_cast<std::size_t>(8_MB), options);
      EXPECT_EQ(res->ContentLength, 0);
      EXPECT_FALSE(res->ETag.empty());
      EXPECT_FALSE(res->LastModified.empty());
      EXPECT_EQ(res->HttpHeaders, m_blobUploadOptions.HttpHeaders);
      EXPECT_EQ(res->Metadata, m_blobUploadOptions.Metadata);
      EXPECT_EQ(res->BlobType, Azure::Storage::Blobs::BlobType::BlockBlob);
      res = blockBlobClient.DownloadTo(tempFilename, options);
      EXPECT_EQ(res->ContentLength, 0);
      EXPECT_FALSE(res->ETag.empty());
      EXPECT_FALSE(res->LastModified.empty());
      EXPECT_EQ(res->HttpHeaders, m_blobUploadOptions.HttpHeaders);
      EXPECT_EQ(res->Metadata, m_blobUploadOptions.Metadata);
      EXPECT_EQ(res->BlobType, Azure::Storage::Blobs::BlobType::BlockBlob);
      EXPECT_TRUE(ReadFile(tempFilename).empty());
      DeleteFile(tempFilename);

      options.Offset = 0;
      EXPECT_THROW(
          blockBlobClient.DownloadTo(emptyContent.data(), static_cast<std::size_t>(8_MB), options),
          StorageError);
      EXPECT_THROW(blockBlobClient.DownloadTo(tempFilename, options), StorageError);

      options.Offset = 1;
      EXPECT_THROW(
          blockBlobClient.DownloadTo(emptyContent.data(), static_cast<std::size_t>(8_MB), options),
          StorageError);
      EXPECT_THROW(blockBlobClient.DownloadTo(tempFilename, options), StorageError);

      options.Offset = 0;
      options.Length = 1;
      EXPECT_THROW(
          blockBlobClient.DownloadTo(emptyContent.data(), static_cast<std::size_t>(8_MB), options),
          StorageError);
      EXPECT_THROW(blockBlobClient.DownloadTo(tempFilename, options), StorageError);

      options.Offset = 100;
      options.Length = 100;
      EXPECT_THROW(
          blockBlobClient.DownloadTo(emptyContent.data(), static_cast<std::size_t>(8_MB), options),
          StorageError);
      EXPECT_THROW(blockBlobClient.DownloadTo(tempFilename, options), StorageError);
      DeleteFile(tempFilename);
    }
  }

  TEST_F(BlockBlobClientTest, ConcurrentUpload)
  {
    std::vector<uint8_t> blobContent = RandomBuffer(static_cast<std::size_t>(8_MB));

    auto testUploadFromBuffer = [&](int concurrency, int64_t blobSize) {
      auto blockBlobClient = m_blobContainerClient->GetBlockBlobClient(RandomString());

      Azure::Storage::Blobs::UploadBlockBlobFromOptions options;
      options.ChunkSize = 1_MB;
      options.Concurrency = concurrency;
      options.HttpHeaders = m_blobUploadOptions.HttpHeaders;
      options.HttpHeaders.ContentMd5.clear();
      options.Metadata = m_blobUploadOptions.Metadata;
      options.Tier = m_blobUploadOptions.Tier;
      auto res = blockBlobClient.UploadFrom(
          blobContent.data(), static_cast<std::size_t>(blobSize), options);
      EXPECT_FALSE(res->ETag.empty());
      EXPECT_FALSE(res->LastModified.empty());
      auto properties = *blockBlobClient.GetProperties();
      properties.HttpHeaders.ContentMd5.clear();
      EXPECT_EQ(properties.ContentLength, blobSize);
      EXPECT_EQ(properties.HttpHeaders, options.HttpHeaders);
      EXPECT_EQ(properties.Metadata, options.Metadata);
      EXPECT_EQ(properties.Tier.GetValue(), options.Tier.GetValue());
      EXPECT_EQ(properties.ETag, res->ETag);
      EXPECT_EQ(properties.LastModified, res->LastModified);
      std::vector<uint8_t> downloadContent(static_cast<std::size_t>(blobSize), '\x00');
      blockBlobClient.DownloadTo(downloadContent.data(), static_cast<std::size_t>(blobSize));
      EXPECT_EQ(
          downloadContent,
          std::vector<uint8_t>(
              blobContent.begin(), blobContent.begin() + static_cast<std::size_t>(blobSize)));
    };

    auto testUploadFromFile = [&](int concurrency, int64_t blobSize) {
      auto blockBlobClient = m_blobContainerClient->GetBlockBlobClient(RandomString());

      Azure::Storage::Blobs::UploadBlockBlobFromOptions options;
      options.ChunkSize = 1_MB;
      options.Concurrency = concurrency;
      options.HttpHeaders = m_blobUploadOptions.HttpHeaders;
      options.HttpHeaders.ContentMd5.clear();
      options.Metadata = m_blobUploadOptions.Metadata;
      options.Tier = m_blobUploadOptions.Tier;

      std::string tempFilename = RandomString();
      {
        Azure::Storage::Details::FileWriter fileWriter(tempFilename);
        fileWriter.Write(blobContent.data(), blobSize, 0);
      }
      auto res = blockBlobClient.UploadFrom(tempFilename, options);
      EXPECT_FALSE(res->ETag.empty());
      EXPECT_FALSE(res->LastModified.empty());
      auto properties = *blockBlobClient.GetProperties();
      properties.HttpHeaders.ContentMd5.clear();
      EXPECT_EQ(properties.ContentLength, blobSize);
      EXPECT_EQ(properties.HttpHeaders, options.HttpHeaders);
      EXPECT_EQ(properties.Metadata, options.Metadata);
      EXPECT_EQ(properties.Tier.GetValue(), options.Tier.GetValue());
      EXPECT_EQ(properties.ETag, res->ETag);
      EXPECT_EQ(properties.LastModified, res->LastModified);
      std::vector<uint8_t> downloadContent(static_cast<std::size_t>(blobSize), '\x00');
      blockBlobClient.DownloadTo(downloadContent.data(), static_cast<std::size_t>(blobSize));
      EXPECT_EQ(
          downloadContent,
          std::vector<uint8_t>(
              blobContent.begin(), blobContent.begin() + static_cast<std::size_t>(blobSize)));
      DeleteFile(tempFilename);
    };

    std::vector<std::future<void>> futures;
    for (int c : {1, 2, 5})
    {
      for (int64_t l :
           {0ULL, 1ULL, 2ULL, 2_KB, 4_KB, 999_KB, 1_MB, 2_MB - 1, 3_MB, 5_MB, 8_MB - 1234, 8_MB})
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

  TEST_F(BlockBlobClientTest, DownloadError)
  {
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    bool exceptionCaught = false;
    try
    {
      blockBlobClient.Download();
    }
    catch (StorageError& e)
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

}}} // namespace Azure::Storage::Test
