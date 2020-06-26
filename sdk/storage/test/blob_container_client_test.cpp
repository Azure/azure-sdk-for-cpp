// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blob_container_client_test.hpp"

namespace Azure { namespace Storage { namespace Test {

  std::shared_ptr<Azure::Storage::Blobs::BlobContainerClient>
      BlobContainerClientTest::m_blobContainerClient;
  std::string BlobContainerClientTest::m_containerName;

  void BlobContainerClientTest::SetUpTestSuite()
  {
    m_containerName = LowercaseRandomString();
    auto blobContainerClient
        = Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
            StandardStorageConnectionString(), m_containerName);
    m_blobContainerClient = std::make_shared<Azure::Storage::Blobs::BlobContainerClient>(
        std::move(blobContainerClient));
    m_blobContainerClient->Create();
  }

  void BlobContainerClientTest::TearDownTestSuite() { m_blobContainerClient->Delete(); }

  TEST_F(BlobContainerClientTest, CreateDelete)
  {
    auto container_client = Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
        StandardStorageConnectionString(), LowercaseRandomString());
    Azure::Storage::Blobs::CreateBlobContainerOptions options;
    std::map<std::string, std::string> metadata;
    metadata["key1"] = "one";
    metadata["KEY2"] = "TWO";
    options.Metadata = metadata;
    auto res = container_client.Create(options);
    EXPECT_FALSE(res.RequestId.empty());
    EXPECT_FALSE(res.Date.empty());
    EXPECT_FALSE(res.ETag.empty());
    EXPECT_FALSE(res.LastModified.empty());
    EXPECT_FALSE(res.Version.empty());
    EXPECT_THROW(container_client.Create(), std::runtime_error);

    auto res2 = container_client.Delete();
    EXPECT_FALSE(res2.RequestId.empty());
    EXPECT_FALSE(res2.Date.empty());
    EXPECT_FALSE(res2.Version.empty());
  }

  TEST_F(BlobContainerClientTest, Metadata)
  {
    std::map<std::string, std::string> metadata;
    metadata["key1"] = "one";
    metadata["KEY2"] = "TWO";
    auto res = m_blobContainerClient->SetMetadata(metadata);
    EXPECT_FALSE(res.RequestId.empty());
    EXPECT_FALSE(res.Date.empty());
    EXPECT_FALSE(res.ETag.empty());
    EXPECT_FALSE(res.LastModified.empty());
    EXPECT_FALSE(res.Version.empty());

    auto properties = m_blobContainerClient->GetProperties();
    EXPECT_FALSE(properties.RequestId.empty());
    EXPECT_FALSE(properties.Date.empty());
    EXPECT_FALSE(properties.ETag.empty());
    EXPECT_FALSE(properties.LastModified.empty());
    EXPECT_FALSE(properties.Version.empty());
    EXPECT_EQ(properties.Metadata, metadata);

    metadata.clear();
    m_blobContainerClient->SetMetadata(metadata);
    properties = m_blobContainerClient->GetProperties();
    EXPECT_TRUE(properties.Metadata.empty());
  }

  TEST_F(BlobContainerClientTest, ListBlobsFlat)
  {
    const std::string prefix1 = "prefix1-";
    const std::string prefix2 = "prefix2-";
    const std::string baseName = "blob";

    std::set<std::string> p1Blobs;
    std::set<std::string> p2Blobs;
    std::set<std::string> p1p2Blobs;

    for (int i = 0; i < 5; ++i)
    {
      std::string blobName = prefix1 + baseName + std::to_string(i);
      auto blobClient = m_blobContainerClient->GetBlockBlobClient(blobName);
      blobClient.Upload(new Azure::Storage::MemoryStream(nullptr, 0));
      p1Blobs.insert(blobName);
      p1p2Blobs.insert(blobName);
    }
    for (int i = 0; i < 5; ++i)
    {
      std::string blobName = prefix2 + baseName + std::to_string(i);
      auto blobClient = m_blobContainerClient->GetBlockBlobClient(blobName);
      blobClient.Upload(new Azure::Storage::MemoryStream(nullptr, 0));
      p2Blobs.insert(blobName);
      p1p2Blobs.insert(blobName);
    }

    Azure::Storage::Blobs::ListBlobsOptions options;
    options.MaxResults = 4;
    std::set<std::string> listBlobs;
    do
    {
      auto res = m_blobContainerClient->ListBlobs(options);
      EXPECT_FALSE(res.RequestId.empty());
      EXPECT_FALSE(res.Date.empty());
      ;
      EXPECT_FALSE(res.Version.empty());
      EXPECT_FALSE(res.ServiceEndpoint.empty());
      EXPECT_EQ(res.MaxResults, options.MaxResults);
      EXPECT_EQ(res.Container, m_containerName);

      options.Marker = res.NextMarker;
      for (const auto& blob : res.BlobItems)
      {
        EXPECT_FALSE(blob.Name.empty());
        EXPECT_FALSE(blob.CreationTime.empty());
        EXPECT_FALSE(blob.LastModified.empty());
        EXPECT_FALSE(blob.ETag.empty());
        EXPECT_NE(blob.BlobType, Azure::Storage::Blobs::BlobType::Unknown);
        EXPECT_NE(blob.Tier, Azure::Storage::Blobs::AccessTier::Unknown);
        listBlobs.insert(blob.Name);
      }
    } while (!options.Marker.empty());
    EXPECT_TRUE(
        std::includes(listBlobs.begin(), listBlobs.end(), p1p2Blobs.begin(), p1p2Blobs.end()));

    options.Prefix = prefix1;
    listBlobs.clear();
    do
    {
      auto res = m_blobContainerClient->ListBlobs(options);
      options.Marker = res.NextMarker;
      for (const auto& blob : res.BlobItems)
      {
        listBlobs.insert(blob.Name);
      }
    } while (!options.Marker.empty());
    EXPECT_TRUE(std::includes(listBlobs.begin(), listBlobs.end(), p1Blobs.begin(), p1Blobs.end()));
  }

  TEST_F(BlobContainerClientTest, ListBlobsHierarchy)
  {
    const std::string delimiter = "/";
    const std::string prefix = RandomString();
    std::set<std::string> blobs;
    std::string blobName = prefix;
    for (int i = 0; i < 5; ++i)
    {
      blobName = blobName + delimiter + RandomString();
      auto blobClient = m_blobContainerClient->GetBlockBlobClient(blobName);
      blobClient.Upload(new Azure::Storage::MemoryStream(nullptr, 0));
      blobs.insert(blobName);
    }

    Azure::Storage::Blobs::ListBlobsOptions options;
    options.Delimiter = delimiter;
    options.Prefix = prefix + delimiter;
    std::set<std::string> listBlobs;
    while (true)
    {
      auto res = m_blobContainerClient->ListBlobs(options);
      EXPECT_EQ(res.Delimiter, options.Delimiter);
      EXPECT_EQ(res.Prefix, options.Prefix);
      for (const auto& blob : res.BlobItems)
      {
        listBlobs.insert(blob.Name);
      }
      if (!res.NextMarker.empty())
      {
        options.Marker = res.NextMarker;
      }
      else if (!res.BlobItems.empty())
      {
        options.Prefix = res.BlobItems[0].Name + delimiter;
        options.Marker.clear();
      }
      else
      {
        break;
      }
    }
    EXPECT_EQ(listBlobs, blobs);
  }

}}} // namespace Azure::Storage::Test
