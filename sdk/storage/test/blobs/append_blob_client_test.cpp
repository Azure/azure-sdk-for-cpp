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

}}} // namespace Azure::Storage::Test
