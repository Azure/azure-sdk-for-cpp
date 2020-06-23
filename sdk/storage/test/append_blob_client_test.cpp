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
    m_blobUploadOptions.Metadata = {{"key1", "V1"}, {"KEY2", "Value2"}};
    m_blobUploadOptions.Properties.ContentType = "application/x-binary";
    m_blobUploadOptions.Properties.ContentLanguage = "en-US";
    m_blobUploadOptions.Properties.ContentDisposition = "attachment";
    m_blobUploadOptions.Properties.CacheControl = "no-cache";
    m_blobUploadOptions.Properties.ContentEncoding = "identify";
    m_blobUploadOptions.Properties.ContentMD5 = "";
    m_appendBlobClient->Create(m_blobUploadOptions);
    m_appendBlobClient->AppendBlock(new Azure::Storage::MemoryStream(m_blobContent));
    m_blobUploadOptions.Properties.ContentMD5 = m_appendBlobClient->GetProperties().ContentMD5;
  }

  void AppendBlobClientTest::TearDownTestSuite() { BlobContainerClientTest::TearDownTestSuite(); }

  TEST_F(AppendBlobClientTest, CreateAppendDelete)
  {
    auto appendBlobClient = Azure::Storage::Blobs::AppendBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());
    appendBlobClient.Create(m_blobUploadOptions);

    auto properties = appendBlobClient.GetProperties();
    EXPECT_EQ(properties.CommittedBlockCount, 0);
    EXPECT_EQ(properties.ContentLength, 0);

    appendBlobClient.AppendBlock(new Azure::Storage::MemoryStream(m_blobContent));
    properties = appendBlobClient.GetProperties();
    EXPECT_EQ(properties.CommittedBlockCount, 1);
    EXPECT_EQ(properties.ContentLength, m_blobContent.size());

    Azure::Storage::Blobs::AppendBlockOptions options;
    options.AppendPosition = 1_MB;
    EXPECT_THROW(appendBlobClient.AppendBlock(new Azure::Storage::MemoryStream(m_blobContent), options), std::runtime_error);
    options.AppendPosition = properties.ContentLength;
    appendBlobClient.AppendBlock(new Azure::Storage::MemoryStream(m_blobContent), options);

    properties = appendBlobClient.GetProperties();
    options = Azure::Storage::Blobs::AppendBlockOptions();
    options.MaxSize = properties.ContentLength + m_blobContent.size() - 1;
    EXPECT_THROW(appendBlobClient.AppendBlock(new Azure::Storage::MemoryStream(m_blobContent), options), std::runtime_error);
    options.MaxSize = properties.ContentLength + m_blobContent.size();
    appendBlobClient.AppendBlock(new Azure::Storage::MemoryStream(m_blobContent), options);

    // TODO: AppendBlockFromUri must be authorized with SAS, but we don't have SAS for now.

    appendBlobClient.Delete();
    EXPECT_THROW(appendBlobClient.Delete(), std::runtime_error);
  }

}}} // namespace Azure::Storage::Test
