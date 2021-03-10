// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "append_blob_client_test.hpp"

#include <azure/storage/blobs/blob_lease_client.hpp>

namespace Azure { namespace Storage { namespace Test {

  std::shared_ptr<Azure::Storage::Blobs::AppendBlobClient> AppendBlobClientTest::m_appendBlobClient;
  std::string AppendBlobClientTest::m_blobName;
  Azure::Storage::Blobs::CreateAppendBlobOptions AppendBlobClientTest::m_blobUploadOptions;
  std::vector<uint8_t> AppendBlobClientTest::m_blobContent;

  void AppendBlobClientTest::SetUpTestSuite()
  {
    BlobContainerClientTest::SetUpTestSuite();

    m_blobName = RandomString();
    auto appendBlobClient = Azure::Storage::Blobs::AppendBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, m_blobName);
    m_appendBlobClient
        = std::make_shared<Azure::Storage::Blobs::AppendBlobClient>(std::move(appendBlobClient));
    m_blobContent.resize(100);
    RandomBuffer(reinterpret_cast<char*>(&m_blobContent[0]), m_blobContent.size());
    m_blobUploadOptions.Metadata = {{"key1", "V1"}, {"key2", "Value2"}};
    m_blobUploadOptions.HttpHeaders.ContentType = "application/x-binary";
    m_blobUploadOptions.HttpHeaders.ContentLanguage = "en-US";
    m_blobUploadOptions.HttpHeaders.ContentDisposition = "attachment";
    m_blobUploadOptions.HttpHeaders.CacheControl = "no-cache";
    m_blobUploadOptions.HttpHeaders.ContentEncoding = "identify";
    m_blobUploadOptions.HttpHeaders.ContentHash.Value.clear();
    m_appendBlobClient->Create(m_blobUploadOptions);
    auto blockContent
        = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    m_appendBlobClient->AppendBlock(&blockContent);
    m_blobUploadOptions.HttpHeaders.ContentHash
        = m_appendBlobClient->GetProperties()->HttpHeaders.ContentHash;
  }

  void AppendBlobClientTest::TearDownTestSuite() { BlobContainerClientTest::TearDownTestSuite(); }

  TEST_F(AppendBlobClientTest, CreateAppendDelete)
  {
    auto appendBlobClient = Azure::Storage::Blobs::AppendBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    auto blobContentInfo = appendBlobClient.Create(m_blobUploadOptions);
    EXPECT_FALSE(blobContentInfo->RequestId.empty());
    EXPECT_TRUE(blobContentInfo->ETag.HasValue());
    EXPECT_TRUE(IsValidTime(blobContentInfo->LastModified));
    EXPECT_TRUE(blobContentInfo->VersionId.HasValue());
    EXPECT_FALSE(blobContentInfo->VersionId.GetValue().empty());
    EXPECT_FALSE(blobContentInfo->EncryptionScope.HasValue());
    EXPECT_FALSE(blobContentInfo->EncryptionKeySha256.HasValue());

    auto properties = *appendBlobClient.GetProperties();
    EXPECT_FALSE(properties.RequestId.empty());
    EXPECT_TRUE(properties.CommittedBlockCount.HasValue());
    EXPECT_EQ(properties.CommittedBlockCount.GetValue(), 0);
    EXPECT_EQ(properties.BlobSize, 0);

    auto blockContent
        = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    auto appendResponse = appendBlobClient.AppendBlock(&blockContent);
    EXPECT_FALSE(appendResponse->RequestId.empty());
    properties = *appendBlobClient.GetProperties();
    EXPECT_EQ(properties.CommittedBlockCount.GetValue(), 1);
    EXPECT_EQ(properties.BlobSize, static_cast<int64_t>(m_blobContent.size()));

    Azure::Storage::Blobs::AppendBlockOptions options;
    options.AccessConditions.IfAppendPositionEqual = 1_MB;
    blockContent = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    EXPECT_THROW(appendBlobClient.AppendBlock(&blockContent, options), StorageException);
    options.AccessConditions.IfAppendPositionEqual = properties.BlobSize;
    blockContent = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    appendBlobClient.AppendBlock(&blockContent, options);

    properties = *appendBlobClient.GetProperties();
    options = Azure::Storage::Blobs::AppendBlockOptions();
    options.AccessConditions.IfMaxSizeLessThanOrEqual
        = properties.BlobSize + m_blobContent.size() - 1;
    blockContent = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    EXPECT_THROW(appendBlobClient.AppendBlock(&blockContent, options), StorageException);
    options.AccessConditions.IfMaxSizeLessThanOrEqual = properties.BlobSize + m_blobContent.size();
    blockContent = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    appendBlobClient.AppendBlock(&blockContent, options);

    properties = *appendBlobClient.GetProperties();
    int64_t originalLength = properties.BlobSize;
    appendBlobClient.AppendBlockFromUri(m_appendBlobClient->GetUrl() + GetSas());
    properties = *appendBlobClient.GetProperties();
    EXPECT_EQ(properties.BlobSize, static_cast<int64_t>(originalLength + m_blobContent.size()));

    auto deleteResponse = appendBlobClient.Delete();
    EXPECT_TRUE(deleteResponse->Deleted);
    EXPECT_FALSE(deleteResponse->RequestId.empty());
    EXPECT_THROW(appendBlobClient.Delete(), StorageException);
  }

  TEST_F(AppendBlobClientTest, AccessConditionLastModifiedTime)
  {
    auto appendBlobClient = Azure::Storage::Blobs::AppendBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    appendBlobClient.Create();

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

    auto lastModifiedTime = appendBlobClient.GetProperties()->LastModified;
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
    auto appendBlobClient = Azure::Storage::Blobs::AppendBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());

    Blobs::CreateAppendBlobOptions createOptions;
    createOptions.AccessConditions.IfNoneMatch = Azure::ETag::Any();
    EXPECT_NO_THROW(appendBlobClient.Create(createOptions));
    EXPECT_THROW(appendBlobClient.Create(createOptions), StorageException);

    Azure::ETag eTag = appendBlobClient.GetProperties()->ETag;
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
    auto appendBlobClient = Azure::Storage::Blobs::AppendBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    appendBlobClient.Create();

    std::string leaseId = Blobs::BlobLeaseClient::CreateUniqueLeaseId();
    Blobs::BlobLeaseClient leaseClient(appendBlobClient, leaseId);
    leaseClient.Acquire(std::chrono::seconds(30));
    EXPECT_THROW(appendBlobClient.Delete(), StorageException);
    Blobs::DeleteBlobOptions options;
    options.AccessConditions.LeaseId = leaseId;
    EXPECT_NO_THROW(appendBlobClient.Delete(options));
  }

  TEST_F(AppendBlobClientTest, SourceBlobAccessConditions)
  {
    auto sourceBlobClient = Azure::Storage::Blobs::AppendBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    sourceBlobClient.Create();
    Blobs::BlobLeaseClient sourceLeaseClient(
        sourceBlobClient, Blobs::BlobLeaseClient::CreateUniqueLeaseId());
    auto leaseResponse = sourceLeaseClient.Acquire(Blobs::BlobLeaseClient::InfiniteLeaseDuration);
    std::string leaseId = leaseResponse->LeaseId;
    Azure::ETag eTag = leaseResponse->ETag;
    auto lastModifiedTime = leaseResponse->LastModified;
    auto timeBeforeStr = lastModifiedTime - std::chrono::seconds(1);
    auto timeAfterStr = lastModifiedTime + std::chrono::seconds(1);

    auto destBlobClient = Azure::Storage::Blobs::AppendBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());

    {
      Blobs::StartCopyBlobFromUriOptions options;
      options.SourceAccessConditions.LeaseId = Blobs::BlobLeaseClient::CreateUniqueLeaseId();
      /*
      don't know why, the copy operation also succeeds even if the lease id doesn't match.
      EXPECT_THROW(
          destBlobClient.StartCopyFromUri(sourceBlobClient.GetUrl(), options), StorageException);
      */
      options.SourceAccessConditions.LeaseId = leaseId;
      EXPECT_NO_THROW(destBlobClient.StartCopyFromUri(sourceBlobClient.GetUrl(), options));
    }
    sourceLeaseClient.Break();
    {
      Blobs::StartCopyBlobFromUriOptions options;
      options.SourceAccessConditions.IfMatch = eTag;
      EXPECT_NO_THROW(destBlobClient.StartCopyFromUri(sourceBlobClient.GetUrl(), options));
      options.SourceAccessConditions.IfMatch = DummyETag;
      EXPECT_THROW(
          destBlobClient.StartCopyFromUri(sourceBlobClient.GetUrl(), options), StorageException);
    }
    {
      Blobs::StartCopyBlobFromUriOptions options;
      options.SourceAccessConditions.IfNoneMatch = DummyETag;
      EXPECT_NO_THROW(destBlobClient.StartCopyFromUri(sourceBlobClient.GetUrl(), options));
      options.SourceAccessConditions.IfNoneMatch = eTag;
      EXPECT_THROW(
          destBlobClient.StartCopyFromUri(sourceBlobClient.GetUrl(), options), StorageException);
    }
    {
      Blobs::StartCopyBlobFromUriOptions options;
      options.SourceAccessConditions.IfModifiedSince = timeBeforeStr;
      EXPECT_NO_THROW(destBlobClient.StartCopyFromUri(sourceBlobClient.GetUrl(), options));
      options.SourceAccessConditions.IfModifiedSince = timeAfterStr;
      EXPECT_THROW(
          destBlobClient.StartCopyFromUri(sourceBlobClient.GetUrl(), options), StorageException);
    }
    {
      Blobs::StartCopyBlobFromUriOptions options;
      options.SourceAccessConditions.IfUnmodifiedSince = timeAfterStr;
      EXPECT_NO_THROW(destBlobClient.StartCopyFromUri(sourceBlobClient.GetUrl(), options));
      options.SourceAccessConditions.IfUnmodifiedSince = timeBeforeStr;
      EXPECT_THROW(
          destBlobClient.StartCopyFromUri(sourceBlobClient.GetUrl(), options), StorageException);
    }
  }

  TEST_F(AppendBlobClientTest, Seal)
  {
    std::string blobName = RandomString();
    auto blobClient = m_blobContainerClient->GetAppendBlobClient(blobName);
    blobClient.Create();
    auto blockContent
        = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    blobClient.AppendBlock(&blockContent);

    auto downloadResult = blobClient.Download();
    EXPECT_TRUE(downloadResult->Details.IsSealed.HasValue());
    EXPECT_FALSE(downloadResult->Details.IsSealed.GetValue());

    auto getPropertiesResult = blobClient.GetProperties();
    EXPECT_TRUE(getPropertiesResult->IsSealed.HasValue());
    EXPECT_FALSE(getPropertiesResult->IsSealed.GetValue());

    Azure::Storage::Blobs::ListBlobsSinglePageOptions options;
    options.Prefix = blobName;
    do
    {
      auto res = m_blobContainerClient->ListBlobsSinglePage(options);
      options.ContinuationToken = res->ContinuationToken;
      for (const auto& blob : res->Items)
      {
        if (blob.Name == blobName)
        {
          EXPECT_TRUE(blob.Details.IsSealed.HasValue());
          EXPECT_FALSE(blob.Details.IsSealed.GetValue());
        }
      }
    } while (options.ContinuationToken.HasValue());

    Blobs::SealAppendBlobOptions sealOptions;
    sealOptions.AccessConditions.IfAppendPositionEqual = m_blobContent.size() + 1;
    EXPECT_THROW(blobClient.Seal(sealOptions), StorageException);

    sealOptions.AccessConditions.IfAppendPositionEqual = m_blobContent.size();
    auto sealResult = blobClient.Seal(sealOptions);
    EXPECT_FALSE(sealResult->RequestId.empty());
    EXPECT_TRUE(sealResult->ETag.HasValue());
    EXPECT_TRUE(IsValidTime(sealResult->LastModified));
    EXPECT_TRUE(sealResult->IsSealed);

    downloadResult = blobClient.Download();
    EXPECT_TRUE(downloadResult->Details.IsSealed.HasValue());
    EXPECT_TRUE(downloadResult->Details.IsSealed.GetValue());

    getPropertiesResult = blobClient.GetProperties();
    EXPECT_TRUE(getPropertiesResult->IsSealed.HasValue());
    EXPECT_TRUE(getPropertiesResult->IsSealed.GetValue());

    do
    {
      auto res = m_blobContainerClient->ListBlobsSinglePage(options);
      options.ContinuationToken = res->ContinuationToken;
      for (const auto& blob : res->Items)
      {
        if (blob.Name == blobName)
        {
          EXPECT_TRUE(blob.Details.IsSealed.HasValue());
          EXPECT_TRUE(blob.Details.IsSealed.GetValue());
        }
      }
    } while (options.ContinuationToken.HasValue());

    auto blobClient2 = m_blobContainerClient->GetAppendBlobClient(RandomString());

    Blobs::StartCopyBlobFromUriOptions copyOptions;
    copyOptions.ShouldSealDestination = false;
    auto copyResult = blobClient2.StartCopyFromUri(blobClient.GetUrl() + GetSas(), copyOptions);
    getPropertiesResult = copyResult.PollUntilDone(std::chrono::seconds(1));
    ASSERT_TRUE(getPropertiesResult->CopyStatus.HasValue());
    EXPECT_EQ(getPropertiesResult->CopyStatus.GetValue(), Blobs::Models::CopyStatus::Success);
    EXPECT_FALSE(getPropertiesResult->IsSealed.GetValue());

    copyOptions.ShouldSealDestination = true;
    copyResult = blobClient2.StartCopyFromUri(blobClient.GetUrl() + GetSas(), copyOptions);
    getPropertiesResult = copyResult.PollUntilDone(std::chrono::seconds(1));
    EXPECT_TRUE(getPropertiesResult->IsSealed.HasValue());
    EXPECT_TRUE(getPropertiesResult->IsSealed.GetValue());
    ASSERT_TRUE(getPropertiesResult->CopyStatus.HasValue());
    EXPECT_EQ(getPropertiesResult->CopyStatus.GetValue(), Blobs::Models::CopyStatus::Success);
  }

  TEST_F(AppendBlobClientTest, CreateIfNotExists)
  {
    auto blobClient = Azure::Storage::Blobs::AppendBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    auto blobClientWithoutAuth = Azure::Storage::Blobs::AppendBlobClient(blobClient.GetUrl());
    EXPECT_THROW(blobClientWithoutAuth.CreateIfNotExists(), StorageException);
    {
      auto response = blobClient.CreateIfNotExists();
      EXPECT_FALSE(response->RequestId.empty());
      EXPECT_TRUE(response->Created);
    }
    auto blobContent
        = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    blobClient.AppendBlock(&blobContent);
    {
      auto response = blobClient.CreateIfNotExists();
      EXPECT_FALSE(response->RequestId.empty());
      EXPECT_FALSE(response->Created);
    }
    auto downloadStream = std::move(blobClient.Download()->BodyStream);
    EXPECT_EQ(
        Azure::Core::IO::BodyStream::ReadToEnd(*downloadStream, Azure::Core::Context()),
        m_blobContent);
  }

}}} // namespace Azure::Storage::Test
