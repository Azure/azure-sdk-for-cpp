// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "append_blob_client_test.hpp"

#include <azure/core/cryptography/hash.hpp>
#include <azure/storage/blobs/blob_lease_client.hpp>
#include <azure/storage/common/crypt.hpp>

namespace Azure { namespace Storage { namespace Blobs { namespace Models {

  bool operator==(const BlobHttpHeaders& lhs, const BlobHttpHeaders& rhs);

}}}} // namespace Azure::Storage::Blobs::Models

namespace Azure { namespace Storage { namespace Test {

  void AppendBlobClientTest::SetUp()
  {
    BlobContainerClientTest::SetUp();
    if (shouldSkipTest())
    {
      return;
    }
    m_blobName = RandomString();
    m_appendBlobClient = std::make_shared<Blobs::AppendBlobClient>(
        m_blobContainerClient->GetAppendBlobClient(m_blobName));
    m_appendBlobClient->Create();
    std::vector<uint8_t> blobContent1 = RandomBuffer(static_cast<size_t>(1_KB));
    std::vector<uint8_t> blobContent2 = RandomBuffer(512);
    auto blobContent = Azure::Core::IO::MemoryBodyStream(blobContent1.data(), blobContent1.size());
    m_appendBlobClient->AppendBlock(blobContent);
    blobContent = Azure::Core::IO::MemoryBodyStream(blobContent2.data(), blobContent2.size());
    m_appendBlobClient->AppendBlock(blobContent);
    m_blobContent = blobContent1;
    m_blobContent.insert(m_blobContent.end(), blobContent2.begin(), blobContent2.end());
  }

  // Requires blob versioning?
  TEST_F(AppendBlobClientTest, CreateAppendDelete)
  {
    auto blobClient = *m_appendBlobClient;

    Blobs::CreateAppendBlobOptions createOptions;
    createOptions.HttpHeaders.ContentType = "application/x-binary";
    createOptions.HttpHeaders.ContentLanguage = "en-US";
    createOptions.HttpHeaders.ContentDisposition = "attachment";
    createOptions.HttpHeaders.CacheControl = "no-cache";
    createOptions.HttpHeaders.ContentEncoding = "identify";
    createOptions.Metadata = RandomMetadata();
    createOptions.Tags["key1"] = "value1";
    createOptions.Tags["key2"] = "value2";
    createOptions.Tags["key3 +-./:=_"] = "v1 +-./:=_";
    auto blobContentInfo = blobClient.Create(createOptions);
    EXPECT_TRUE(blobContentInfo.Value.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(blobContentInfo.Value.LastModified));
    EXPECT_TRUE(blobContentInfo.Value.VersionId.HasValue());
    EXPECT_FALSE(blobContentInfo.Value.VersionId.Value().empty());
    EXPECT_FALSE(blobContentInfo.Value.EncryptionScope.HasValue());
    EXPECT_FALSE(blobContentInfo.Value.EncryptionKeySha256.HasValue());

    auto properties = blobClient.GetProperties().Value;
    EXPECT_TRUE(properties.CommittedBlockCount.HasValue());
    EXPECT_EQ(properties.CommittedBlockCount.Value(), 0);
    EXPECT_EQ(properties.BlobSize, 0);
    EXPECT_EQ(properties.Metadata, createOptions.Metadata);
    EXPECT_EQ(properties.HttpHeaders, createOptions.HttpHeaders);
    EXPECT_EQ(blobClient.GetTags().Value, createOptions.Tags);

    auto blockContent
        = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    auto appendResponse = blobClient.AppendBlock(blockContent);
    properties = blobClient.GetProperties().Value;
    EXPECT_EQ(properties.CommittedBlockCount.Value(), 1);
    EXPECT_EQ(properties.BlobSize, static_cast<int64_t>(m_blobContent.size()));

    Azure::Storage::Blobs::AppendBlockOptions options;
    options.AccessConditions.IfAppendPositionEqual = 1_MB;
    blockContent = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    EXPECT_THROW(blobClient.AppendBlock(blockContent, options), StorageException);
    options.AccessConditions.IfAppendPositionEqual = properties.BlobSize;
    blockContent = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    blobClient.AppendBlock(blockContent, options);

    properties = blobClient.GetProperties().Value;
    options = Azure::Storage::Blobs::AppendBlockOptions();
    options.AccessConditions.IfMaxSizeLessThanOrEqual
        = properties.BlobSize + m_blobContent.size() - 1;
    blockContent = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    EXPECT_THROW(blobClient.AppendBlock(blockContent, options), StorageException);
    options.AccessConditions.IfMaxSizeLessThanOrEqual = properties.BlobSize + m_blobContent.size();
    blockContent = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    blobClient.AppendBlock(blockContent, options);

    properties = blobClient.GetProperties().Value;
    int64_t originalLength = properties.BlobSize;
    blobClient.AppendBlockFromUri(blobClient.GetUrl() + GetSas());
    properties = blobClient.GetProperties().Value;
    EXPECT_EQ(properties.BlobSize, 2 * originalLength);

    auto deleteResponse = blobClient.Delete();
    EXPECT_TRUE(deleteResponse.Value.Deleted);
    EXPECT_THROW(blobClient.Delete(), StorageException);
  }

  TEST_F(AppendBlobClientTest, AccessConditionLastModifiedTime)
  {
    auto blobClient = *m_appendBlobClient;

    enum class TimePoint
    {
      TimeBefore,
      TimeAfter,
      None,
    };

    enum class Condition
    {
      ModifiedSince,
      UnmodifiedSince,
    };

    auto lastModifiedTime = blobClient.GetProperties().Value.LastModified;
    auto timeBeforeStr = lastModifiedTime - std::chrono::seconds(1);
    auto timeAfterStr = lastModifiedTime + std::chrono::seconds(1);
    for (auto condition : {Condition::ModifiedSince, Condition::UnmodifiedSince})
    {
      for (auto sinceTime : {TimePoint::TimeBefore, TimePoint::TimeAfter})
      {
        Blobs::GetBlobPropertiesOptions options;
        if (condition == Condition::ModifiedSince)
        {
          options.AccessConditions.IfModifiedSince
              = sinceTime == TimePoint::TimeBefore ? timeBeforeStr : timeAfterStr;
        }
        else if (condition == Condition::UnmodifiedSince)
        {
          options.AccessConditions.IfUnmodifiedSince
              = sinceTime == TimePoint::TimeBefore ? timeBeforeStr : timeAfterStr;
        }
        bool shouldThrow
            = (condition == Condition::ModifiedSince && sinceTime == TimePoint::TimeAfter)
            || (condition == Condition::UnmodifiedSince && sinceTime == TimePoint::TimeBefore);
        if (shouldThrow)
        {
          EXPECT_THROW(blobClient.GetProperties(options), StorageException);
        }
        else
        {
          EXPECT_NO_THROW(blobClient.GetProperties(options));
        }
      }
    }
  }

  TEST_F(AppendBlobClientTest, AccessConditionETag)
  {
    auto blobClient = GetAppendBlobClientForTest(RandomString());

    Blobs::CreateAppendBlobOptions createOptions;
    createOptions.AccessConditions.IfNoneMatch = Azure::ETag::Any();
    EXPECT_NO_THROW(blobClient.Create(createOptions));
    EXPECT_THROW(blobClient.Create(createOptions), StorageException);

    Azure::ETag eTag = blobClient.GetProperties().Value.ETag;
    for (Azure::ETag match : {eTag, DummyETag, Azure::ETag()})
    {
      for (Azure::ETag noneMatch : {eTag, DummyETag, Azure::ETag()})
      {
        Blobs::GetBlobPropertiesOptions options;
        if (match.HasValue())
        {
          options.AccessConditions.IfMatch = match;
        }
        if (noneMatch.HasValue())
        {
          options.AccessConditions.IfNoneMatch = noneMatch;
        }
        bool shouldThrow = (match.HasValue() && match != eTag) || noneMatch == eTag;
        if (shouldThrow)
        {
          EXPECT_THROW(blobClient.GetProperties(options), StorageException);
        }
        else
        {
          EXPECT_NO_THROW(blobClient.GetProperties(options));
        }
      }
    }
  }

  TEST_F(AppendBlobClientTest, AccessConditionLeaseId)
  {
    auto blobClient = GetAppendBlobClientForTest(RandomString());
    blobClient.Create();

    std::string leaseId = RandomUUID();
    Blobs::BlobLeaseClient leaseClient(blobClient, leaseId);
    leaseClient.Acquire(std::chrono::seconds(30));
    EXPECT_THROW(blobClient.Delete(), StorageException);
    Blobs::DeleteBlobOptions options;
    options.AccessConditions.LeaseId = leaseId;
    EXPECT_NO_THROW(blobClient.Delete(options));
  }

  TEST_F(AppendBlobClientTest, Seal)
  {
    auto blobClient = *m_appendBlobClient;

    auto downloadResult = blobClient.Download();
    EXPECT_TRUE(downloadResult.Value.Details.IsSealed.HasValue());
    EXPECT_FALSE(downloadResult.Value.Details.IsSealed.Value());

    auto getPropertiesResult = blobClient.GetProperties();
    EXPECT_TRUE(getPropertiesResult.Value.IsSealed.HasValue());
    EXPECT_FALSE(getPropertiesResult.Value.IsSealed.Value());

    auto blobItem = GetBlobItem(m_blobName);
    EXPECT_TRUE(blobItem.Details.IsSealed.HasValue());
    EXPECT_FALSE(blobItem.Details.IsSealed.Value());

    Blobs::SealAppendBlobOptions sealOptions;
    sealOptions.AccessConditions.IfAppendPositionEqual = m_blobContent.size() + 1;
    EXPECT_THROW(blobClient.Seal(sealOptions), StorageException);

    sealOptions.AccessConditions.IfAppendPositionEqual = m_blobContent.size();
    auto sealResult = blobClient.Seal(sealOptions);
    EXPECT_TRUE(sealResult.Value.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(sealResult.Value.LastModified));
    EXPECT_TRUE(sealResult.Value.IsSealed);

    downloadResult = blobClient.Download();
    EXPECT_TRUE(downloadResult.Value.Details.IsSealed.HasValue());
    EXPECT_TRUE(downloadResult.Value.Details.IsSealed.Value());

    getPropertiesResult = blobClient.GetProperties();
    EXPECT_TRUE(getPropertiesResult.Value.IsSealed.HasValue());
    EXPECT_TRUE(getPropertiesResult.Value.IsSealed.Value());

    blobItem = GetBlobItem(m_blobName);
    EXPECT_TRUE(blobItem.Details.IsSealed.HasValue());
    EXPECT_TRUE(blobItem.Details.IsSealed.Value());

    auto blobClient2 = GetAppendBlobClientForTest(RandomString());

    Blobs::StartBlobCopyFromUriOptions copyOptions;
    copyOptions.ShouldSealDestination = false;
    auto copyResult = blobClient2.StartCopyFromUri(blobClient.GetUrl() + GetSas(), copyOptions);
    getPropertiesResult = copyResult.PollUntilDone(std::chrono::seconds(1));
    ASSERT_TRUE(getPropertiesResult.Value.CopyStatus.HasValue());
    EXPECT_EQ(getPropertiesResult.Value.CopyStatus.Value(), Blobs::Models::CopyStatus::Success);
    EXPECT_FALSE(getPropertiesResult.Value.IsSealed.Value());

    copyOptions.ShouldSealDestination = true;
    copyResult = blobClient2.StartCopyFromUri(blobClient.GetUrl() + GetSas(), copyOptions);
    getPropertiesResult = copyResult.PollUntilDone(std::chrono::seconds(1));
    EXPECT_TRUE(getPropertiesResult.Value.IsSealed.HasValue());
    EXPECT_TRUE(getPropertiesResult.Value.IsSealed.Value());
    ASSERT_TRUE(getPropertiesResult.Value.CopyStatus.HasValue());
    EXPECT_EQ(getPropertiesResult.Value.CopyStatus.Value(), Blobs::Models::CopyStatus::Success);
  }

  TEST_F(AppendBlobClientTest, CreateIfNotExists)
  {
    auto blobClient = GetAppendBlobClientForTest(RandomString());

    auto blobClientWithoutAuth = Azure::Storage::Blobs::AppendBlobClient(
        blobClient.GetUrl(), InitStorageClientOptions<Azure::Storage::Blobs::BlobClientOptions>());
    EXPECT_THROW(blobClientWithoutAuth.CreateIfNotExists(), StorageException);
    {
      auto response = blobClient.CreateIfNotExists();
      EXPECT_TRUE(response.Value.Created);
    }
    auto blobContent
        = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    blobClient.AppendBlock(blobContent);
    {
      auto response = blobClient.CreateIfNotExists();
      EXPECT_FALSE(response.Value.Created);
    }
    auto downloadStream = std::move(blobClient.Download().Value.BodyStream);
    EXPECT_EQ(downloadStream->ReadToEnd(Azure::Core::Context()), m_blobContent);
  }

  TEST_F(AppendBlobClientTest, ContentHash)
  {
    auto appendBlobClient = GetAppendBlobClientForTest(RandomString());

    const std::vector<uint8_t> blobContent = RandomBuffer(10);
    const std::vector<uint8_t> contentMd5
        = Azure::Core::Cryptography::Md5Hash().Final(blobContent.data(), blobContent.size());
    const std::vector<uint8_t> contentCrc64
        = Azure::Storage::Crc64Hash().Final(blobContent.data(), blobContent.size());

    appendBlobClient.Create();
    auto contentStream = Azure::Core::IO::MemoryBodyStream(blobContent.data(), blobContent.size());
    appendBlobClient.AppendBlock(contentStream);

    auto appendBlobClient2 = GetAppendBlobClientForTest(RandomString());
    appendBlobClient2.Create();

    Blobs::AppendBlockOptions options1;
    options1.TransactionalContentHash = ContentHash();
    options1.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Md5;
    options1.TransactionalContentHash.Value().Value = Azure::Core::Convert::Base64Decode(DummyMd5);
    contentStream.Rewind();
    EXPECT_THROW(appendBlobClient2.AppendBlock(contentStream, options1), StorageException);
    options1.TransactionalContentHash.Value().Value = contentMd5;
    contentStream.Rewind();
    EXPECT_NO_THROW(appendBlobClient2.AppendBlock(contentStream, options1));
    options1.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Crc64;
    options1.TransactionalContentHash.Value().Value
        = Azure::Core::Convert::Base64Decode(DummyCrc64);
    contentStream.Rewind();
    EXPECT_THROW(appendBlobClient2.AppendBlock(contentStream, options1), StorageException);
    options1.TransactionalContentHash.Value().Value = contentCrc64;
    contentStream.Rewind();
    EXPECT_NO_THROW(appendBlobClient2.AppendBlock(contentStream, options1));

    Blobs::AppendBlockFromUriOptions options2;
    options2.TransactionalContentHash = ContentHash();
    options2.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Md5;
    options2.TransactionalContentHash.Value().Value = Azure::Core::Convert::Base64Decode(DummyMd5);
    EXPECT_THROW(
        appendBlobClient2.AppendBlockFromUri(appendBlobClient.GetUrl() + GetSas(), options2),
        StorageException);
    options2.TransactionalContentHash.Value().Value = contentMd5;
    EXPECT_NO_THROW(
        appendBlobClient2.AppendBlockFromUri(appendBlobClient.GetUrl() + GetSas(), options2));
    options2.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Crc64;
    options2.TransactionalContentHash.Value().Value
        = Azure::Core::Convert::Base64Decode(DummyCrc64);
    // EXPECT_THROW(
    //    appendBlobClient2.AppendBlockFromUri(appendBlobClient.GetUrl() + GetSas(), options2),
    //    StorageException);
    options2.TransactionalContentHash.Value().Value = contentCrc64;
    EXPECT_NO_THROW(
        appendBlobClient2.AppendBlockFromUri(appendBlobClient.GetUrl() + GetSas(), options2));
  }

}}} // namespace Azure::Storage::Test
