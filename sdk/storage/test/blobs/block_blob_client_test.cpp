// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "block_blob_client_test.hpp"

#include "common/crypt.hpp"
#include "common/file_io.hpp"

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
    /*
    This feature isn't GA yet.
    EXPECT_NO_THROW(snapshotClient.SetAccessTier(Azure::Storage::Blobs::AccessTier::Cool));
    */
    EXPECT_THROW(
        snapshotClient.SetHttpHeaders(Azure::Storage::Blobs::BlobHttpHeaders()), StorageError);
    EXPECT_THROW(versionClient.Upload(&emptyContent), StorageError);
    EXPECT_THROW(versionClient.SetMetadata({}), StorageError);
    /*
    This feature isn't GA yet
    EXPECT_NO_THROW(versionClient.SetAccessTier(Azure::Storage::Blobs::AccessTier::Cool));
    */
    EXPECT_THROW(
        versionClient.SetHttpHeaders(Azure::Storage::Blobs::BlobHttpHeaders()), StorageError);

    Azure::Storage::Blobs::CreateSnapshotOptions options;
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
    std::string tempFilename = RandomString();
    std::vector<uint8_t> downloadBuffer = m_blobContent;
    for (int c : {1, 2, 4})
    {
      Azure::Storage::Blobs::DownloadBlobToBufferOptions options;
      options.Concurrency = c;

      // download whole blob
      downloadBuffer.assign(downloadBuffer.size(), '\x00');
      auto res = m_blockBlobClient->DownloadToBuffer(downloadBuffer.data(), downloadBuffer.size());
      EXPECT_EQ(downloadBuffer, m_blobContent);
      EXPECT_EQ(static_cast<std::size_t>(res->ContentLength), downloadBuffer.size());
      res = m_blockBlobClient->DownloadToFile(tempFilename);
      auto downloadFile = ReadFile(tempFilename);
      EXPECT_EQ(downloadFile, m_blobContent);
      EXPECT_EQ(static_cast<std::size_t>(res->ContentLength), downloadFile.size());
      DeleteFile(tempFilename);

      // download whole blob
      downloadBuffer.assign(downloadBuffer.size(), '\x00');
      options.Offset = 0;
      res = m_blockBlobClient->DownloadToBuffer(downloadBuffer.data(), downloadBuffer.size());
      EXPECT_EQ(downloadBuffer, m_blobContent);
      EXPECT_EQ(static_cast<std::size_t>(res->ContentLength), downloadBuffer.size());
      res = m_blockBlobClient->DownloadToFile(tempFilename);
      downloadFile = ReadFile(tempFilename);
      EXPECT_EQ(downloadFile, m_blobContent);
      EXPECT_EQ(static_cast<std::size_t>(res->ContentLength), downloadFile.size());
      DeleteFile(tempFilename);

      // download whole blob
      downloadBuffer.assign(downloadBuffer.size(), '\x00');
      options.Offset = 0;
      options.Length = downloadBuffer.size();
      res = m_blockBlobClient->DownloadToBuffer(downloadBuffer.data(), downloadBuffer.size());
      EXPECT_EQ(downloadBuffer, m_blobContent);
      EXPECT_EQ(static_cast<std::size_t>(res->ContentLength), downloadBuffer.size());
      res = m_blockBlobClient->DownloadToFile(tempFilename);
      downloadFile = ReadFile(tempFilename);
      EXPECT_EQ(downloadFile, m_blobContent);
      EXPECT_EQ(static_cast<std::size_t>(res->ContentLength), downloadFile.size());
      DeleteFile(tempFilename);

      // download whole blob
      downloadBuffer.assign(downloadBuffer.size(), '\x00');
      options.Offset = 0;
      options.Length = downloadBuffer.size() * 2;
      res = m_blockBlobClient->DownloadToBuffer(downloadBuffer.data(), downloadBuffer.size() * 2);
      EXPECT_EQ(downloadBuffer, m_blobContent);
      EXPECT_EQ(static_cast<std::size_t>(res->ContentLength), downloadBuffer.size());
      res = m_blockBlobClient->DownloadToFile(tempFilename);
      downloadFile = ReadFile(tempFilename);
      EXPECT_EQ(downloadFile, m_blobContent);
      EXPECT_EQ(static_cast<std::size_t>(res->ContentLength), downloadFile.size());
      DeleteFile(tempFilename);

      options.InitialChunkSize = 4_KB;
      options.ChunkSize = 4_KB;

      auto downloadRange = [&](int64_t offset, int64_t length) {
        int64_t actualLength
            = std::min(length, static_cast<int64_t>(m_blobContent.size()) - offset);

        auto optionsCopy = options;
        optionsCopy.Offset = offset;
        optionsCopy.Length = length;
        if (actualLength > 0)
        {
          std::vector<uint8_t> downloadContent(static_cast<std::size_t>(actualLength), '\x00');
          auto res = m_blockBlobClient->DownloadToBuffer(
              downloadContent.data(), static_cast<std::size_t>(actualLength), optionsCopy);
          EXPECT_EQ(
              downloadContent,
              std::vector<uint8_t>(
                  m_blobContent.begin() + static_cast<std::size_t>(offset),
                  m_blobContent.begin() + static_cast<std::size_t>(offset)
                      + static_cast<std::size_t>(actualLength)));
          EXPECT_EQ(res->ContentLength, actualLength);

          std::string tempFilename2 = RandomString();
          res = m_blockBlobClient->DownloadToFile(tempFilename2, optionsCopy);
          auto downloadFile = ReadFile(tempFilename2);
          EXPECT_EQ(
              downloadFile,
              std::vector<uint8_t>(
                  m_blobContent.begin() + static_cast<std::size_t>(offset),
                  m_blobContent.begin() + static_cast<std::size_t>(offset)
                      + static_cast<std::size_t>(actualLength)));
          EXPECT_EQ(res->ContentLength, actualLength);
          DeleteFile(tempFilename2);
        }
        else
        {
          EXPECT_THROW(
              m_blockBlobClient->DownloadToBuffer(nullptr, 8 * 1024 * 1024, optionsCopy),
              StorageError);
          EXPECT_THROW(m_blockBlobClient->DownloadToFile(tempFilename, optionsCopy), StorageError);
          DeleteFile(tempFilename);
        }
      };

      // random range
      std::vector<std::future<void>> downloadRangeTasks;
      std::mt19937_64 random_generator(std::random_device{}());
      for (int i = 0; i < 16; ++i)
      {
        std::uniform_int_distribution<int64_t> offsetDistribution(0, m_blobContent.size() - 1);
        int64_t offset = offsetDistribution(random_generator);
        std::uniform_int_distribution<int64_t> lengthDistribution(1, 64_KB);
        int64_t length = lengthDistribution(random_generator);
        downloadRangeTasks.emplace_back(
            std::async(std::launch::async, downloadRange, offset, length));
      }
      downloadRangeTasks.emplace_back(std::async(std::launch::async, downloadRange, 0, 1));
      downloadRangeTasks.emplace_back(std::async(std::launch::async, downloadRange, 1, 1));
      downloadRangeTasks.emplace_back(
          std::async(std::launch::async, downloadRange, m_blobContent.size() - 1, 1));
      downloadRangeTasks.emplace_back(
          std::async(std::launch::async, downloadRange, m_blobContent.size() - 1, 2));
      downloadRangeTasks.emplace_back(
          std::async(std::launch::async, downloadRange, m_blobContent.size(), 1));
      downloadRangeTasks.emplace_back(
          std::async(std::launch::async, downloadRange, m_blobContent.size() + 1, 2));

      for (auto& task : downloadRangeTasks)
      {
        task.get();
      }

      // buffer not big enough
      options.Offset = 1;
      for (int64_t length : {1ULL, 2ULL, 4_KB, 5_KB, 8_KB, 11_KB, 20_KB})
      {
        options.Length = length;
        EXPECT_THROW(
            m_blockBlobClient->DownloadToBuffer(
                downloadBuffer.data(), static_cast<std::size_t>(length - 1), options),
            std::runtime_error);
      }
    }
  }

  TEST_F(BlockBlobClientTest, ConcurrentUploadFromNonExistingFile)
  {
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    std::string emptyFilename = RandomString();
    EXPECT_THROW(blockBlobClient.UploadFromFile(emptyFilename), std::runtime_error);
    EXPECT_THROW(blockBlobClient.Delete(), StorageError);
  }

  TEST_F(BlockBlobClientTest, ConcurrentDownloadNonExistingBlob)
  {
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    std::vector<uint8_t> blobContent(100);
    std::string tempFilename = RandomString();

    EXPECT_THROW(
        blockBlobClient.DownloadToBuffer(blobContent.data(), blobContent.size()), StorageError);
    EXPECT_THROW(blockBlobClient.DownloadToFile(tempFilename), StorageError);
    DeleteFile(tempFilename);
  }

  TEST_F(BlockBlobClientTest, ConcurrentUploadEmptyBlob)
  {
    std::vector<uint8_t> emptyContent;
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());

    blockBlobClient.UploadFromBuffer(emptyContent.data(), emptyContent.size());
    EXPECT_NO_THROW(blockBlobClient.Delete());

    std::string emptyFilename = RandomString();
    {
      Details::FileWriter writer(emptyFilename);
    }
    blockBlobClient.UploadFromFile(emptyFilename);
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

    auto res = blockBlobClient.DownloadToBuffer(emptyContent.data(), 0);
    EXPECT_EQ(res->ContentLength, 0);
    EXPECT_FALSE(res->ETag.empty());
    EXPECT_FALSE(res->LastModified.empty());
    EXPECT_EQ(res->HttpHeaders, m_blobUploadOptions.HttpHeaders);
    EXPECT_EQ(res->Metadata, m_blobUploadOptions.Metadata);
    EXPECT_EQ(res->BlobType, Azure::Storage::Blobs::BlobType::BlockBlob);
    res = blockBlobClient.DownloadToFile(tempFilename);
    EXPECT_EQ(res->ContentLength, 0);
    EXPECT_FALSE(res->ETag.empty());
    EXPECT_FALSE(res->LastModified.empty());
    EXPECT_EQ(res->HttpHeaders, m_blobUploadOptions.HttpHeaders);
    EXPECT_EQ(res->Metadata, m_blobUploadOptions.Metadata);
    EXPECT_EQ(res->BlobType, Azure::Storage::Blobs::BlobType::BlockBlob);
    EXPECT_TRUE(ReadFile(tempFilename).empty());
    DeleteFile(tempFilename);

    res = blockBlobClient.DownloadToBuffer(emptyContent.data(), static_cast<std::size_t>(8_MB));
    EXPECT_EQ(res->ContentLength, 0);
    EXPECT_FALSE(res->ETag.empty());
    EXPECT_FALSE(res->LastModified.empty());
    EXPECT_EQ(res->HttpHeaders, m_blobUploadOptions.HttpHeaders);
    EXPECT_EQ(res->Metadata, m_blobUploadOptions.Metadata);
    EXPECT_EQ(res->BlobType, Azure::Storage::Blobs::BlobType::BlockBlob);
    res = blockBlobClient.DownloadToFile(tempFilename);
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
      Azure::Storage::Blobs::DownloadBlobToBufferOptions options;
      options.InitialChunkSize = 10;
      options.ChunkSize = 10;
      options.Concurrency = c;

      res = blockBlobClient.DownloadToBuffer(
          emptyContent.data(), static_cast<std::size_t>(8_MB), options);
      EXPECT_EQ(res->ContentLength, 0);
      EXPECT_FALSE(res->ETag.empty());
      EXPECT_FALSE(res->LastModified.empty());
      EXPECT_EQ(res->HttpHeaders, m_blobUploadOptions.HttpHeaders);
      EXPECT_EQ(res->Metadata, m_blobUploadOptions.Metadata);
      EXPECT_EQ(res->BlobType, Azure::Storage::Blobs::BlobType::BlockBlob);
      res = blockBlobClient.DownloadToFile(tempFilename, options);
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
          blockBlobClient.DownloadToBuffer(
              emptyContent.data(), static_cast<std::size_t>(8_MB), options),
          StorageError);
      EXPECT_THROW(blockBlobClient.DownloadToFile(tempFilename, options), StorageError);

      options.Offset = 1;
      EXPECT_THROW(
          blockBlobClient.DownloadToBuffer(
              emptyContent.data(), static_cast<std::size_t>(8_MB), options),
          StorageError);
      EXPECT_THROW(blockBlobClient.DownloadToFile(tempFilename, options), StorageError);

      options.Offset = 0;
      options.Length = 1;
      EXPECT_THROW(
          blockBlobClient.DownloadToBuffer(
              emptyContent.data(), static_cast<std::size_t>(8_MB), options),
          StorageError);
      EXPECT_THROW(blockBlobClient.DownloadToFile(tempFilename, options), StorageError);

      options.Offset = 100;
      options.Length = 100;
      EXPECT_THROW(
          blockBlobClient.DownloadToBuffer(
              emptyContent.data(), static_cast<std::size_t>(8_MB), options),
          StorageError);
      EXPECT_THROW(blockBlobClient.DownloadToFile(tempFilename, options), StorageError);
      DeleteFile(tempFilename);
    }
  }

  TEST_F(BlockBlobClientTest, ConcurrentUpload)
  {
    std::string tempFilename = RandomString();

    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    for (int c : {1, 2, 5})
    {
      for (int64_t length :
           {0ULL, 1ULL, 2ULL, 2_KB, 4_KB, 999_KB, 1_MB, 2_MB - 1, 3_MB, 5_MB, 8_MB - 1234, 8_MB})
      {
        Azure::Storage::Blobs::UploadBlobOptions options;
        options.ChunkSize = 1_MB;
        options.Concurrency = c;
        options.HttpHeaders = m_blobUploadOptions.HttpHeaders;
        options.HttpHeaders.ContentMd5.clear();
        options.Metadata = m_blobUploadOptions.Metadata;
        options.Tier = m_blobUploadOptions.Tier;
        {
          auto res = blockBlobClient.UploadFromBuffer(
              m_blobContent.data(), static_cast<std::size_t>(length), options);
          EXPECT_FALSE(res->ETag.empty());
          EXPECT_FALSE(res->LastModified.empty());
          EXPECT_FALSE(res->SequenceNumber.HasValue());
          EXPECT_FALSE(res->ContentCrc64.HasValue());
          EXPECT_FALSE(res->ContentMd5.HasValue());
          auto properties = *blockBlobClient.GetProperties();
          EXPECT_EQ(properties.ContentLength, length);
          EXPECT_EQ(properties.HttpHeaders, options.HttpHeaders);
          EXPECT_EQ(properties.Metadata, options.Metadata);
          EXPECT_EQ(properties.Tier.GetValue(), options.Tier.GetValue());
          EXPECT_EQ(properties.ETag, res->ETag);
          EXPECT_EQ(properties.LastModified, res->LastModified);
          std::vector<uint8_t> downloadContent(static_cast<std::size_t>(length), '\x00');
          blockBlobClient.DownloadToBuffer(
              downloadContent.data(), static_cast<std::size_t>(length));
          EXPECT_EQ(
              downloadContent,
              std::vector<uint8_t>(
                  m_blobContent.begin(), m_blobContent.begin() + static_cast<std::size_t>(length)));
        }
        {
          {
            Azure::Storage::Details::FileWriter fileWriter(tempFilename);
            fileWriter.Write(m_blobContent.data(), length, 0);
          }
          auto res = blockBlobClient.UploadFromFile(tempFilename, options);
          EXPECT_FALSE(res->ETag.empty());
          EXPECT_FALSE(res->LastModified.empty());
          EXPECT_FALSE(res->SequenceNumber.HasValue());
          EXPECT_FALSE(res->ContentCrc64.HasValue());
          EXPECT_FALSE(res->ContentMd5.HasValue());
          auto properties = *blockBlobClient.GetProperties();
          EXPECT_EQ(properties.ContentLength, length);
          EXPECT_EQ(properties.HttpHeaders, options.HttpHeaders);
          EXPECT_EQ(properties.Metadata, options.Metadata);
          EXPECT_EQ(properties.Tier.GetValue(), options.Tier.GetValue());
          EXPECT_EQ(properties.ETag, res->ETag);
          EXPECT_EQ(properties.LastModified, res->LastModified);
          std::vector<uint8_t> downloadContent(static_cast<std::size_t>(length), '\x00');
          blockBlobClient.DownloadToBuffer(
              downloadContent.data(), static_cast<std::size_t>(length));
          EXPECT_EQ(
              downloadContent,
              std::vector<uint8_t>(
                  m_blobContent.begin(), m_blobContent.begin() + static_cast<std::size_t>(length)));
        }
      }
    }
    DeleteFile(tempFilename);
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
