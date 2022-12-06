// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "append_blob_client_test.hpp"

#include <azure/core/cryptography/hash.hpp>
#include <azure/storage/blobs/blob_lease_client.hpp>
#include <azure/storage/common/crypt.hpp>

namespace Azure { namespace Storage { namespace Test {

  void AppendBlobClientTest::SetUp() { BlobContainerClientTest::SetUp(); }

  void AppendBlobClientTest::TearDown()
  {
    if (m_appendBlobClient)
    {
      m_appendBlobClient->Delete();
    }
    // Delete container
    BlobContainerClientTest::TearDown();
  }

  // Requires blob versioning?
  TEST_F(AppendBlobClientTest, createappenddelete)
  {
    auto const testName(GetTestName());
    auto client = GetAppendBlobClient(testName);
    auto appendBlobClient = Blobs::AppendBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(),
        m_containerName,
        testName + "1",
        InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>());

    auto blobContentInfo = appendBlobClient.Create(m_blobUploadOptions);
    EXPECT_TRUE(blobContentInfo.Value.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(blobContentInfo.Value.LastModified));
    EXPECT_TRUE(blobContentInfo.Value.VersionId.HasValue());
    EXPECT_FALSE(blobContentInfo.Value.VersionId.Value().empty());
    EXPECT_FALSE(blobContentInfo.Value.EncryptionScope.HasValue());
    EXPECT_FALSE(blobContentInfo.Value.EncryptionKeySha256.HasValue());

    auto properties = appendBlobClient.GetProperties().Value;
    EXPECT_TRUE(properties.CommittedBlockCount.HasValue());
    EXPECT_EQ(properties.CommittedBlockCount.Value(), 0);
    EXPECT_EQ(properties.BlobSize, 0);

    auto blockContent
        = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    auto appendResponse = appendBlobClient.AppendBlock(blockContent);
    properties = appendBlobClient.GetProperties().Value;
    EXPECT_EQ(properties.CommittedBlockCount.Value(), 1);
    EXPECT_EQ(properties.BlobSize, static_cast<int64_t>(m_blobContent.size()));

    Azure::Storage::Blobs::AppendBlockOptions options;
    options.AccessConditions.IfAppendPositionEqual = 1_MB;
    blockContent = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    EXPECT_THROW(appendBlobClient.AppendBlock(blockContent, options), StorageException);
    options.AccessConditions.IfAppendPositionEqual = properties.BlobSize;
    blockContent = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    appendBlobClient.AppendBlock(blockContent, options);

    properties = appendBlobClient.GetProperties().Value;
    options = Azure::Storage::Blobs::AppendBlockOptions();
    options.AccessConditions.IfMaxSizeLessThanOrEqual
        = properties.BlobSize + m_blobContent.size() - 1;
    blockContent = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    EXPECT_THROW(appendBlobClient.AppendBlock(blockContent, options), StorageException);
    options.AccessConditions.IfMaxSizeLessThanOrEqual = properties.BlobSize + m_blobContent.size();
    blockContent = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    appendBlobClient.AppendBlock(blockContent, options);

    properties = appendBlobClient.GetProperties().Value;
    int64_t originalLength = properties.BlobSize;
    appendBlobClient.AppendBlockFromUri(client.GetUrl() + GetSas());
    properties = appendBlobClient.GetProperties().Value;
    EXPECT_EQ(properties.BlobSize, static_cast<int64_t>(originalLength + m_blobContent.size()));

    auto deleteResponse = appendBlobClient.Delete();
    EXPECT_TRUE(deleteResponse.Value.Deleted);
    EXPECT_THROW(appendBlobClient.Delete(), StorageException);
  }

  TEST_F(AppendBlobClientTest, createwithtags)
  {
    auto const testName(GetTestName());
    auto client = GetAppendBlobClient(testName);
    auto appendBlobClient = Blobs::AppendBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(),
        m_containerName,
        testName + "1",
        InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>());

    Blobs::CreateAppendBlobOptions options;
    options.Tags["key1"] = "value1";
    options.Tags["key2"] = "value2";
    options.Tags["key3 +-./:=_"] = "v1 +-./:=_";
    appendBlobClient.Create(options);

    EXPECT_EQ(appendBlobClient.GetTags().Value, options.Tags);
  }

  TEST_F(AppendBlobClientTest, accessconditionlastmodifiedtime)
  {
    auto const testName(GetTestName());
    auto appendBlobClient = GetAppendBlobClient(testName);

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

    auto lastModifiedTime = appendBlobClient.GetProperties().Value.LastModified;
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
          EXPECT_THROW(appendBlobClient.GetProperties(options), StorageException);
        }
        else
        {
          EXPECT_NO_THROW(appendBlobClient.GetProperties(options));
        }
      }
    }
  }

  TEST_F(AppendBlobClientTest, AccessConditionETag)
  {
    auto const testName(GetTestName());
    auto client = GetAppendBlobClient(testName);
    auto appendBlobClient = Blobs::AppendBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(),
        m_containerName,
        testName + "1",
        InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>());

    Blobs::CreateAppendBlobOptions createOptions;
    createOptions.AccessConditions.IfNoneMatch = Azure::ETag::Any();
    EXPECT_NO_THROW(appendBlobClient.Create(createOptions));
    EXPECT_THROW(appendBlobClient.Create(createOptions), StorageException);

    Azure::ETag eTag = appendBlobClient.GetProperties().Value.ETag;
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
          EXPECT_THROW(appendBlobClient.GetProperties(options), StorageException);
        }
        else
        {
          EXPECT_NO_THROW(appendBlobClient.GetProperties(options));
        }
      }
    }
  }

  TEST_F(AppendBlobClientTest, AccessConditionLeaseId)
  {
    auto const testName(GetTestName());
    auto client = GetAppendBlobClient(testName);
    auto appendBlobClient = Blobs::AppendBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(),
        m_containerName,
        testName + "leaseId",
        InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>());
    appendBlobClient.Create();

    std::string leaseId = Blobs::BlobLeaseClient::CreateUniqueLeaseId();
    Blobs::BlobLeaseClient leaseClient(appendBlobClient, leaseId);
    leaseClient.Acquire(std::chrono::seconds(30));
    EXPECT_THROW(appendBlobClient.Delete(), StorageException);
    Blobs::DeleteBlobOptions options;
    options.AccessConditions.LeaseId = leaseId;
    EXPECT_NO_THROW(appendBlobClient.Delete(options));
  }

  TEST_F(AppendBlobClientTest, Seal)
  {
    auto const testName(GetTestName());
    auto client = GetAppendBlobClient(testName);
    auto const extraBlobName = testName + "1";
    auto blobClient = Blobs::AppendBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(),
        m_containerName,
        extraBlobName,
        InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>());
    blobClient.Create();

    auto blockContent
        = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    blobClient.AppendBlock(blockContent);

    auto downloadResult = blobClient.Download();
    EXPECT_TRUE(downloadResult.Value.Details.IsSealed.HasValue());
    EXPECT_FALSE(downloadResult.Value.Details.IsSealed.Value());

    auto getPropertiesResult = blobClient.GetProperties();
    EXPECT_TRUE(getPropertiesResult.Value.IsSealed.HasValue());
    EXPECT_FALSE(getPropertiesResult.Value.IsSealed.Value());

    auto blobItem = GetBlobItem(extraBlobName);
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

    blobItem = GetBlobItem(extraBlobName);
    EXPECT_TRUE(blobItem.Details.IsSealed.HasValue());
    EXPECT_TRUE(blobItem.Details.IsSealed.Value());

    auto const extraBlobName2 = testName + "2";
    auto blobClient2 = Blobs::AppendBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(),
        m_containerName,
        extraBlobName2,
        InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>());

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
    auto const testName(GetTestName());
    auto client = GetAppendBlobClient(testName);
    auto blobClient = Blobs::AppendBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(),
        m_containerName,
        testName + "test",
        InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>());

    auto blobClientWithoutAuth = Azure::Storage::Blobs::AppendBlobClient(
        blobClient.GetUrl(), InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>());
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
    auto const testName(GetTestName());
    auto appendBlobClient = GetAppendBlobClient(testName);

    const std::vector<uint8_t> blobContent = RandomBuffer(10);
    const std::vector<uint8_t> contentMd5
        = Azure::Core::Cryptography::Md5Hash().Final(blobContent.data(), blobContent.size());
    const std::vector<uint8_t> contentCrc64
        = Azure::Storage::Crc64Hash().Final(blobContent.data(), blobContent.size());

    appendBlobClient.Create();
    auto contentStream = Azure::Core::IO::MemoryBodyStream(blobContent.data(), blobContent.size());
    appendBlobClient.AppendBlock(contentStream);

    auto appendBlobClient2 = GetAppendBlobClient(testName + "2");
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
