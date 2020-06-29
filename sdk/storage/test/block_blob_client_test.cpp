// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "block_blob_client_test.hpp"

#include "common/crypt.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  bool operator==(
      const Azure::Storage::Blobs::BlobHttpHeaders& lhs,
      const Azure::Storage::Blobs::BlobHttpHeaders& rhs)
  {
    return lhs.ContentType == rhs.ContentType && lhs.ContentEncoding == rhs.ContentEncoding
        && lhs.ContentLanguage == rhs.ContentLanguage && lhs.ContentMD5 == rhs.ContentMD5
        && lhs.CacheControl == rhs.CacheControl && lhs.ContentDisposition == rhs.ContentDisposition;
  }

}}} // namespace Azure::Storage::Blobs

namespace Azure { namespace Storage { namespace Test {

  std::shared_ptr<Azure::Storage::Blobs::BlockBlobClient> BlockBlobClientTest::m_blockBlobClient;
  std::string BlockBlobClientTest::m_blobName;
  Azure::Storage::Blobs::UploadBlobOptions BlockBlobClientTest::m_blobUploadOptions;
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
    m_blobUploadOptions.Metadata = {{"key1", "V1"}, {"KEY2", "Value2"}};
    m_blobUploadOptions.Properties.ContentType = "application/x-binary";
    m_blobUploadOptions.Properties.ContentLanguage = "en-US";
    m_blobUploadOptions.Properties.ContentDisposition = "attachment";
    m_blobUploadOptions.Properties.CacheControl = "no-cache";
    m_blobUploadOptions.Properties.ContentEncoding = "identity";
    m_blobUploadOptions.Properties.ContentMD5 = "";
    m_blobUploadOptions.Tier = Azure::Storage::Blobs::AccessTier::Hot;
    m_blockBlobClient->Upload(
        Azure::Storage::CreateMemoryStream(m_blobContent.data(), m_blobContent.size()),
        m_blobUploadOptions);
    m_blobUploadOptions.Properties.ContentMD5 = m_blockBlobClient->GetProperties().ContentMD5;
  }

  void BlockBlobClientTest::TearDownTestSuite() { BlobContainerClientTest::TearDownTestSuite(); }

  TEST_F(BlockBlobClientTest, CreateDelete)
  {
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    blockBlobClient.Upload(
        Azure::Storage::CreateMemoryStream(m_blobContent.data(), m_blobContent.size()),
        m_blobUploadOptions);

    blockBlobClient.Delete();
    EXPECT_THROW(blockBlobClient.Delete(), std::runtime_error);
  }

  TEST_F(BlockBlobClientTest, UploadDownload)
  {
    auto res = m_blockBlobClient->Download();
    EXPECT_EQ(ReadBodyStream(res.BodyStream), m_blobContent);
    EXPECT_FALSE(res.RequestId.empty());
    EXPECT_FALSE(res.Date.empty());
    EXPECT_FALSE(res.Version.empty());
    EXPECT_FALSE(res.ETag.empty());
    EXPECT_FALSE(res.LastModified.empty());
    EXPECT_EQ(res.Properties, m_blobUploadOptions.Properties);
    EXPECT_EQ(res.Metadata, m_blobUploadOptions.Metadata);
    EXPECT_EQ(res.BlobType, Azure::Storage::Blobs::BlobType::BlockBlob);
    Azure::Storage::Blobs::DownloadBlobOptions options;
    options.Offset = 1_MB;
    options.Length = 2_MB;
    res = m_blockBlobClient->Download(options);
    EXPECT_EQ(
        ReadBodyStream(res.BodyStream),
        std::vector<uint8_t>(
            m_blobContent.begin() + static_cast<std::size_t>(options.Offset.GetValue()),
            m_blobContent.begin()
                + static_cast<std::size_t>(options.Offset.GetValue() + options.Length.GetValue())));
    EXPECT_FALSE(res.ContentRange.GetValue().empty());
  }

  TEST_F(BlockBlobClientTest, CopyFromUri)
  {
    auto blobClient = m_blobContainerClient->GetBlobClient(RandomString());
    auto res = blobClient.StartCopyFromUri(m_blockBlobClient->GetUri());
    ;
    EXPECT_FALSE(res.RequestId.empty());
    EXPECT_FALSE(res.Date.empty());
    EXPECT_FALSE(res.Version.empty());
    EXPECT_FALSE(res.ETag.empty());
    EXPECT_FALSE(res.LastModified.empty());
    EXPECT_FALSE(res.CopyId.empty());
    EXPECT_TRUE(
        res.CopyStatus == Azure::Storage::Blobs::CopyStatus::Pending
        || res.CopyStatus == Azure::Storage::Blobs::CopyStatus::Success);
  }

  TEST_F(BlockBlobClientTest, SnapShot)
  {
    auto res = m_blockBlobClient->CreateSnapshot();
    EXPECT_FALSE(res.RequestId.empty());
    EXPECT_FALSE(res.Date.empty());
    EXPECT_FALSE(res.Version.empty());
    EXPECT_FALSE(res.ETag.empty());
    EXPECT_FALSE(res.LastModified.empty());
    EXPECT_FALSE(res.Snapshot.empty());
    auto snapshotClient = m_blockBlobClient->WithSnapshot(res.Snapshot);
    EXPECT_EQ(ReadBodyStream(snapshotClient.Download().BodyStream), m_blobContent);
    EXPECT_EQ(snapshotClient.GetProperties().Metadata, m_blobUploadOptions.Metadata);
    EXPECT_THROW(
        snapshotClient.Upload(Azure::Storage::CreateMemoryStream(nullptr, 0)), std::runtime_error);
    EXPECT_THROW(snapshotClient.SetMetadata({}), std::runtime_error);
    EXPECT_THROW(
        snapshotClient.SetAccessTier(Azure::Storage::Blobs::AccessTier::Cool), std::runtime_error);
    EXPECT_THROW(snapshotClient.SetHttpHeaders(), std::runtime_error);

    Azure::Storage::Blobs::CreateSnapshotOptions options;
    options.Metadata = {{"snapshotkey1", "snapshotvalue1"}, {"snapshotKEY2", "SNAPSHOTVALUE2"}};
    res = m_blockBlobClient->CreateSnapshot(options);
    EXPECT_FALSE(res.Snapshot.empty());
    snapshotClient = m_blockBlobClient->WithSnapshot(res.Snapshot);
    EXPECT_EQ(snapshotClient.GetProperties().Metadata, options.Metadata);
  }

  TEST_F(BlockBlobClientTest, Properties)
  {
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    blockBlobClient.Upload(
        Azure::Storage::CreateMemoryStream(m_blobContent.data(), m_blobContent.size()));
    blockBlobClient.SetMetadata(m_blobUploadOptions.Metadata);
    blockBlobClient.SetAccessTier(Azure::Storage::Blobs::AccessTier::Cool);
    Azure::Storage::Blobs::SetBlobHttpHeadersOptions options;
    options.ContentType = m_blobUploadOptions.Properties.ContentType;
    options.ContentEncoding = m_blobUploadOptions.Properties.ContentEncoding;
    options.ContentLanguage = m_blobUploadOptions.Properties.ContentLanguage;
    options.ContentMD5 = m_blobUploadOptions.Properties.ContentMD5;
    options.CacheControl = m_blobUploadOptions.Properties.CacheControl;
    options.ContentDisposition = m_blobUploadOptions.Properties.ContentDisposition;
    blockBlobClient.SetHttpHeaders(options);

    auto res = blockBlobClient.GetProperties();
    EXPECT_FALSE(res.RequestId.empty());
    EXPECT_FALSE(res.Date.empty());
    EXPECT_FALSE(res.Version.empty());
    EXPECT_FALSE(res.ETag.empty());
    EXPECT_FALSE(res.LastModified.empty());
    EXPECT_FALSE(res.CreationTime.empty());
    EXPECT_EQ(res.Metadata, m_blobUploadOptions.Metadata);
    EXPECT_EQ(res.ContentLength, static_cast<int64_t>(m_blobContent.size()));
    EXPECT_EQ(res.ContentType, options.ContentType);
    EXPECT_EQ(res.ContentEncoding, options.ContentEncoding);
    EXPECT_EQ(res.ContentLanguage, options.ContentLanguage);
    EXPECT_EQ(res.ContentMD5, options.ContentMD5);
    EXPECT_EQ(res.CacheControl, options.CacheControl);
    EXPECT_EQ(res.ContentDisposition, options.ContentDisposition);
    EXPECT_EQ(res.Tier.GetValue(), Azure::Storage::Blobs::AccessTier::Cool);
    EXPECT_FALSE(res.AccessTierChangeTime.GetValue().empty());
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
    blockBlobClient.StageBlock(
        blockId1, Azure::Storage::CreateMemoryStream(block1Content.data(), block1Content.size()));
    Azure::Storage::Blobs::CommitBlockListOptions options;
    options.Properties = m_blobUploadOptions.Properties;
    options.Metadata = m_blobUploadOptions.Metadata;
    blockBlobClient.CommitBlockList(
        {{Azure::Storage::Blobs::BlockType::Uncommitted, blockId1}}, options);
    auto res = blockBlobClient.GetBlockList();
    EXPECT_FALSE(res.RequestId.empty());
    EXPECT_FALSE(res.Date.empty());
    EXPECT_FALSE(res.Version.empty());
    EXPECT_FALSE(res.ETag.empty());
    EXPECT_FALSE(res.LastModified.empty());
    EXPECT_EQ(res.ContentLength, static_cast<int64_t>(block1Content.size()));
    ASSERT_FALSE(res.CommittedBlocks.empty());
    EXPECT_EQ(res.CommittedBlocks[0].Name, blockId1);
    EXPECT_EQ(res.CommittedBlocks[0].Size, static_cast<int64_t>(block1Content.size()));
    EXPECT_TRUE(res.UncommittedBlocks.empty());

    // TODO: StageBlockFromUri must be authorized with SAS, but we don't have SAS for now.
    /*
    blockBlobClient.StageBlockFromUri(blockId2, m_blockBlobClient->GetUri());
    res = blockBlobClient.GetBlockList();
    EXPECT_EQ(res.ContentLength, block1Content.size());
    ASSERT_FALSE(res.UncommittedBlocks.empty());
    EXPECT_EQ(res.UncommittedBlocks[0].Name, blockId2);
    EXPECT_EQ(res.UncommittedBlocks[0].Size, m_blobContent.size());

    blockBlobClient.CommitBlockList(
        {{Azure::Storage::Blobs::BlockType::Committed, blockId1},
         {Azure::Storage::Blobs::BlockType::Uncommitted, blockId2}});
    res = blockBlobClient.GetBlockList();
    EXPECT_EQ(res.ContentLength, block1Content.size() + m_blobContent.size());
    EXPECT_TRUE(res.UncommittedBlocks.empty());
    */
  }

}}} // namespace Azure::Storage::Test
