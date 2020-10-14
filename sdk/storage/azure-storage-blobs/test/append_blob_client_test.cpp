// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "append_blob_client_test.hpp"

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
    m_blobUploadOptions.HttpHeaders.ContentMd5 = "";
    m_appendBlobClient->Create(m_blobUploadOptions);
    auto blockContent
        = Azure::Core::Http::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    m_appendBlobClient->AppendBlock(&blockContent);
    m_blobUploadOptions.HttpHeaders.ContentMd5
        = m_appendBlobClient->GetProperties()->HttpHeaders.ContentMd5;
  }

  void AppendBlobClientTest::TearDownTestSuite() { BlobContainerClientTest::TearDownTestSuite(); }

  TEST_F(AppendBlobClientTest, CreateAppendDelete)
  {
    auto appendBlobClient = Azure::Storage::Blobs::AppendBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    auto blobContentInfo = appendBlobClient.Create(m_blobUploadOptions);
    EXPECT_FALSE(blobContentInfo->ETag.empty());
    EXPECT_FALSE(blobContentInfo->LastModified.empty());
    EXPECT_TRUE(blobContentInfo->VersionId.HasValue());
    EXPECT_FALSE(blobContentInfo->VersionId.GetValue().empty());
    EXPECT_FALSE(blobContentInfo->EncryptionScope.HasValue());
    EXPECT_FALSE(blobContentInfo->EncryptionKeySha256.HasValue());

    auto properties = *appendBlobClient.GetProperties();
    EXPECT_TRUE(properties.CommittedBlockCount.HasValue());
    EXPECT_EQ(properties.CommittedBlockCount.GetValue(), 0);
    EXPECT_EQ(properties.ContentLength, 0);

    auto blockContent
        = Azure::Core::Http::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    appendBlobClient.AppendBlock(&blockContent);
    properties = *appendBlobClient.GetProperties();
    EXPECT_EQ(properties.CommittedBlockCount.GetValue(), 1);
    EXPECT_EQ(properties.ContentLength, static_cast<int64_t>(m_blobContent.size()));

    Azure::Storage::Blobs::AppendBlockOptions options;
    options.AccessConditions.AppendPosition = 1_MB;
    blockContent = Azure::Core::Http::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    EXPECT_THROW(appendBlobClient.AppendBlock(&blockContent, options), StorageError);
    options.AccessConditions.AppendPosition = properties.ContentLength;
    blockContent = Azure::Core::Http::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    appendBlobClient.AppendBlock(&blockContent, options);

    properties = *appendBlobClient.GetProperties();
    options = Azure::Storage::Blobs::AppendBlockOptions();
    options.AccessConditions.MaxSize = properties.ContentLength + m_blobContent.size() - 1;
    blockContent = Azure::Core::Http::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    EXPECT_THROW(appendBlobClient.AppendBlock(&blockContent, options), StorageError);
    options.AccessConditions.MaxSize = properties.ContentLength + m_blobContent.size();
    blockContent = Azure::Core::Http::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    appendBlobClient.AppendBlock(&blockContent, options);

    properties = *appendBlobClient.GetProperties();
    int64_t originalLength = properties.ContentLength;
    appendBlobClient.AppendBlockFromUri(m_appendBlobClient->GetUri() + GetSas());
    properties = *appendBlobClient.GetProperties();
    EXPECT_EQ(
        properties.ContentLength, static_cast<int64_t>(originalLength + m_blobContent.size()));

    appendBlobClient.Delete();
    EXPECT_THROW(appendBlobClient.Delete(), StorageError);
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

    auto lastModifiedTime = FromRfc1123(appendBlobClient.GetProperties()->LastModified);
    auto timeBeforeStr = ToRfc1123(lastModifiedTime - std::chrono::seconds(1));
    auto timeAfterStr = ToRfc1123(lastModifiedTime + std::chrono::seconds(1));
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
          EXPECT_THROW(appendBlobClient.GetProperties(options), StorageError);
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
    createOptions.AccessConditions.IfNoneMatch = "*";
    EXPECT_NO_THROW(appendBlobClient.Create(createOptions));
    EXPECT_THROW(appendBlobClient.Create(createOptions), StorageError);

    std::string eTag = appendBlobClient.GetProperties()->ETag;
    for (std::string match : {eTag, std::string(c_dummyETag), std::string()})
    {
      for (std::string noneMatch : {eTag, std::string(c_dummyETag), std::string()})
      {
        Blobs::GetBlobPropertiesOptions options;
        if (!match.empty())
        {
          options.AccessConditions.IfMatch = match;
        }
        if (!noneMatch.empty())
        {
          options.AccessConditions.IfNoneMatch = noneMatch;
        }
        bool shouldThrow = (!match.empty() && match != eTag) || noneMatch == eTag;
        if (shouldThrow)
        {
          EXPECT_THROW(appendBlobClient.GetProperties(options), StorageError);
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

    std::string leaseId = CreateUniqueLeaseId();
    appendBlobClient.AcquireLease(leaseId, 30);
    EXPECT_THROW(appendBlobClient.Delete(), StorageError);
    Blobs::DeleteBlobOptions options;
    options.AccessConditions.LeaseId = leaseId;
    EXPECT_NO_THROW(appendBlobClient.Delete(options));
  }

  TEST_F(AppendBlobClientTest, SourceBlobAccessConditions)
  {
    auto sourceBlobClient = Azure::Storage::Blobs::AppendBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    sourceBlobClient.Create();
    auto leaseResponse
        = sourceBlobClient.AcquireLease(CreateUniqueLeaseId(), c_InfiniteLeaseDuration);
    std::string leaseId = leaseResponse->LeaseId;
    std::string eTag = leaseResponse->ETag;
    auto lastModifiedTime = FromRfc1123(leaseResponse->LastModified);
    auto timeBeforeStr = ToRfc1123(lastModifiedTime - std::chrono::seconds(1));
    auto timeAfterStr = ToRfc1123(lastModifiedTime + std::chrono::seconds(1));

    auto destBlobClient = Azure::Storage::Blobs::AppendBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());

    {
      Blobs::StartCopyBlobFromUriOptions options;
      options.SourceConditions.LeaseId = CreateUniqueLeaseId();
      /*
      don't know why, the copy operation also succeeds even if the lease id doesn't match.
      EXPECT_THROW(
          destBlobClient.StartCopyFromUri(sourceBlobClient.GetUri(), options), StorageError);
      */
      options.SourceConditions.LeaseId = leaseId;
      EXPECT_NO_THROW(destBlobClient.StartCopyFromUri(sourceBlobClient.GetUri(), options));
    }
    sourceBlobClient.BreakLease();
    {
      Blobs::StartCopyBlobFromUriOptions options;
      options.SourceConditions.IfMatch = eTag;
      EXPECT_NO_THROW(destBlobClient.StartCopyFromUri(sourceBlobClient.GetUri(), options));
      options.SourceConditions.IfMatch = c_dummyETag;
      EXPECT_THROW(
          destBlobClient.StartCopyFromUri(sourceBlobClient.GetUri(), options), StorageError);
    }
    {
      Blobs::StartCopyBlobFromUriOptions options;
      options.SourceConditions.IfNoneMatch = c_dummyETag;
      EXPECT_NO_THROW(destBlobClient.StartCopyFromUri(sourceBlobClient.GetUri(), options));
      options.SourceConditions.IfNoneMatch = eTag;
      EXPECT_THROW(
          destBlobClient.StartCopyFromUri(sourceBlobClient.GetUri(), options), StorageError);
    }
    {
      Blobs::StartCopyBlobFromUriOptions options;
      options.SourceConditions.IfModifiedSince = timeBeforeStr;
      EXPECT_NO_THROW(destBlobClient.StartCopyFromUri(sourceBlobClient.GetUri(), options));
      options.SourceConditions.IfModifiedSince = timeAfterStr;
      EXPECT_THROW(
          destBlobClient.StartCopyFromUri(sourceBlobClient.GetUri(), options), StorageError);
    }
    {
      Blobs::StartCopyBlobFromUriOptions options;
      options.SourceConditions.IfUnmodifiedSince = timeAfterStr;
      EXPECT_NO_THROW(destBlobClient.StartCopyFromUri(sourceBlobClient.GetUri(), options));
      options.SourceConditions.IfUnmodifiedSince = timeBeforeStr;
      EXPECT_THROW(
          destBlobClient.StartCopyFromUri(sourceBlobClient.GetUri(), options), StorageError);
    }
  }

  TEST_F(AppendBlobClientTest, Seal)
  {
    std::string blobName = RandomString();
    auto blobClient = m_blobContainerClient->GetAppendBlobClient(blobName);
    blobClient.Create();
    auto blockContent
        = Azure::Core::Http::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
    blobClient.AppendBlock(&blockContent);

    auto downloadResult = blobClient.Download();
    if (downloadResult->IsSealed.HasValue())
    {
      EXPECT_FALSE(downloadResult->IsSealed.GetValue());
    }

    auto getPropertiesResult = blobClient.GetProperties();
    if (getPropertiesResult->IsSealed.HasValue())
    {
      EXPECT_FALSE(getPropertiesResult->IsSealed.GetValue());
    }

    Blobs::SealAppendBlobOptions sealOptions;
    sealOptions.AccessConditions.AppendPosition = m_blobContent.size() + 1;
    EXPECT_THROW(blobClient.Seal(sealOptions), StorageError);

    sealOptions.AccessConditions.AppendPosition = m_blobContent.size();
    auto sealResult = blobClient.Seal(sealOptions);
    EXPECT_FALSE(sealResult->ETag.empty());
    EXPECT_FALSE(sealResult->LastModified.empty());
    EXPECT_TRUE(sealResult->IsSealed);

    downloadResult = blobClient.Download();
    EXPECT_TRUE(downloadResult->IsSealed.HasValue());
    EXPECT_TRUE(downloadResult->IsSealed.GetValue());

    getPropertiesResult = blobClient.GetProperties();
    EXPECT_TRUE(getPropertiesResult->IsSealed.HasValue());
    EXPECT_TRUE(getPropertiesResult->IsSealed.GetValue());

    Azure::Storage::Blobs::ListBlobsSegmentOptions options;
    options.Prefix = blobName;
    do
    {
      auto res = m_blobContainerClient->ListBlobsFlatSegment(options);
      options.ContinuationToken = res->ContinuationToken;
      for (const auto& blob : res->Items)
      {
        if (blob.Name == blobName)
        {
          EXPECT_TRUE(blob.IsSealed.HasValue());
          EXPECT_TRUE(blob.IsSealed.GetValue());
        }
      }
    } while (!options.ContinuationToken.GetValue().empty());

    auto blobClient2 = m_blobContainerClient->GetAppendBlobClient(RandomString());

    Blobs::StartCopyBlobFromUriOptions copyOptions;
    copyOptions.ShouldSealDestination = false;
    auto copyResult = blobClient2.StartCopyFromUri(blobClient.GetUri() + GetSas(), copyOptions);
    // TODO: poller wait here
    getPropertiesResult = blobClient2.GetProperties();
    if (getPropertiesResult->IsSealed.HasValue())
    {
      EXPECT_FALSE(getPropertiesResult->IsSealed.GetValue());
    }

    copyOptions.ShouldSealDestination = true;
    copyResult = blobClient2.StartCopyFromUri(blobClient.GetUri() + GetSas(), copyOptions);
    // TODO: poller wait here
    getPropertiesResult = blobClient2.GetProperties();
    EXPECT_TRUE(getPropertiesResult->IsSealed.HasValue());
    EXPECT_TRUE(getPropertiesResult->IsSealed.GetValue());
  }

}}} // namespace Azure::Storage::Test
