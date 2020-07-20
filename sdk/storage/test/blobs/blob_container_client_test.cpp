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
    metadata["key2"] = "TWO";
    options.Metadata = metadata;
    auto res = container_client.Create(options);
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderRequestId).empty());
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderDate).empty());
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderXMsVersion).empty());
    EXPECT_FALSE(res->ETag.empty());
    EXPECT_FALSE(res->LastModified.empty());
    EXPECT_THROW(container_client.Create(), std::runtime_error);

    auto res2 = container_client.Delete();
    EXPECT_FALSE(res2.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderRequestId).empty());
    EXPECT_FALSE(res2.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderDate).empty());
    EXPECT_FALSE(res2.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderXMsVersion).empty());
  }

  TEST_F(BlobContainerClientTest, Metadata)
  {
    std::map<std::string, std::string> metadata;
    metadata["key1"] = "one";
    metadata["key2"] = "TWO";
    auto res = m_blobContainerClient->SetMetadata(metadata);
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderRequestId).empty());
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderDate).empty());
    EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderXMsVersion).empty());
    EXPECT_FALSE(res->ETag.empty());
    EXPECT_FALSE(res->LastModified.empty());

    auto res2 = m_blobContainerClient->GetProperties();
    EXPECT_FALSE(res2.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderRequestId).empty());
    EXPECT_FALSE(res2.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderDate).empty());
    EXPECT_FALSE(res2.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderXMsVersion).empty());
    auto properties = *res2;
    EXPECT_FALSE(properties.ETag.empty());
    EXPECT_FALSE(properties.LastModified.empty());
    EXPECT_EQ(properties.Metadata, metadata);

    metadata.clear();
    m_blobContainerClient->SetMetadata(metadata);
    properties = *m_blobContainerClient->GetProperties();
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
      auto emptyContent = Azure::Core::Http::MemoryBodyStream(nullptr, 0);
      blobClient.Upload(&emptyContent);
      p1Blobs.insert(blobName);
      p1p2Blobs.insert(blobName);
    }
    for (int i = 0; i < 5; ++i)
    {
      std::string blobName = prefix2 + baseName + std::to_string(i);
      auto blobClient = m_blobContainerClient->GetBlockBlobClient(blobName);
      auto emptyContent = Azure::Core::Http::MemoryBodyStream(nullptr, 0);
      blobClient.Upload(&emptyContent);
      p2Blobs.insert(blobName);
      p1p2Blobs.insert(blobName);
    }

    Azure::Storage::Blobs::ListBlobsOptions options;
    options.MaxResults = 4;
    std::set<std::string> listBlobs;
    do
    {
      auto res = m_blobContainerClient->ListBlobsFlat(options);
      EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderRequestId).empty());
      EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderDate).empty());
      EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderXMsVersion).empty());
      EXPECT_FALSE(res->ServiceEndpoint.empty());
      EXPECT_EQ(res->Container, m_containerName);

      options.Marker = res->NextMarker;
      for (const auto& blob : res->Items)
      {
        EXPECT_FALSE(blob.Name.empty());
        EXPECT_FALSE(blob.CreationTime.empty());
        EXPECT_FALSE(blob.LastModified.empty());
        EXPECT_FALSE(blob.ETag.empty());
        EXPECT_NE(blob.BlobType, Azure::Storage::Blobs::BlobType::Unknown);
        EXPECT_NE(blob.Tier, Azure::Storage::Blobs::AccessTier::Unknown);
        listBlobs.insert(blob.Name);
      }
    } while (!options.Marker.GetValue().empty());
    EXPECT_TRUE(
        std::includes(listBlobs.begin(), listBlobs.end(), p1p2Blobs.begin(), p1p2Blobs.end()));

    options.Prefix = prefix1;
    listBlobs.clear();
    do
    {
      auto res = m_blobContainerClient->ListBlobsFlat(options);
      options.Marker = res->NextMarker;
      for (const auto& blob : res->Items)
      {
        listBlobs.insert(blob.Name);
      }
    } while (!options.Marker.GetValue().empty());
    EXPECT_TRUE(std::includes(listBlobs.begin(), listBlobs.end(), p1Blobs.begin(), p1Blobs.end()));
  }

  TEST_F(BlobContainerClientTest, ListBlobsHierarchy)
  {
    const std::string delimiter = "/";
    const std::string prefix = RandomString();
    const std::string prefix1 = prefix + "-" + RandomString();
    const std::string prefix2 = prefix + "-" + RandomString();
    std::set<std::string> blobs;
    for (const auto& blobNamePrefix : {prefix1, prefix2})
    {
      for (int i = 0; i < 3; ++i)
      {
        std::string blobName = blobNamePrefix + delimiter + RandomString();
        auto blobClient = m_blobContainerClient->GetBlockBlobClient(blobName);
        auto emptyContent = Azure::Core::Http::MemoryBodyStream(nullptr, 0);
        blobClient.Upload(&emptyContent);
        blobs.insert(blobName);
      }
    }

    Azure::Storage::Blobs::ListBlobsOptions options;
    options.Prefix = prefix;
    std::set<std::string> items;
    while (true)
    {
      auto res = m_blobContainerClient->ListBlobsByHierarchy(delimiter, options);
      EXPECT_EQ(res->Delimiter, delimiter);
      EXPECT_EQ(res->Prefix, options.Prefix.GetValue());
      EXPECT_TRUE(res->Items.empty());
      for (const auto& i : res->BlobPrefixes)
      {
        items.emplace(i.Name);
      }
      if (!res->NextMarker.empty())
      {
        options.Marker = res->NextMarker;
      }
      else
      {
        break;
      }
    }
    EXPECT_EQ(items, (std::set<std::string>{prefix1 + delimiter, prefix2 + delimiter}));

    items.clear();
    for (const auto& p : {prefix1, prefix2})
    {
      options.Prefix = p + delimiter;
      while (true)
      {
        auto res = m_blobContainerClient->ListBlobsByHierarchy(delimiter, options);
        EXPECT_EQ(res->Delimiter, delimiter);
        EXPECT_EQ(res->Prefix, options.Prefix.GetValue());
        EXPECT_TRUE(res->BlobPrefixes.empty());
        for (const auto& i : res->Items)
        {
          items.emplace(i.Name);
        }
        if (!res->NextMarker.empty())
        {
          options.Marker = res->NextMarker;
        }
        else
        {
          break;
        }
      }
    }
    EXPECT_EQ(items, blobs);
  }

}}} // namespace Azure::Storage::Test
