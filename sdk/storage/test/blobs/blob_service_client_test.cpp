// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blobs/blob.hpp"
#include "test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class BlobServiceClientTest : public ::testing::Test {
  protected:
    BlobServiceClientTest()
        : m_blobServiceClient(Azure::Storage::Blobs::BlobServiceClient::CreateFromConnectionString(
            StandardStorageConnectionString()))
    {
    }

    Azure::Storage::Blobs::BlobServiceClient m_blobServiceClient;
  };

  TEST_F(BlobServiceClientTest, ListContainers)
  {
    const std::string prefix1 = "prefix1-" + LowercaseRandomString() + "-";
    const std::string prefix2 = "prefix2-" + LowercaseRandomString() + "-";

    std::set<std::string> p1Containers;
    std::set<std::string> p2Containers;
    std::set<std::string> p1p2Containers;

    for (int i = 0; i < 5; ++i)
    {
      std::string containerName = prefix1 + LowercaseRandomString();
      auto containerClient = m_blobServiceClient.GetBlobContainerClient(containerName);
      containerClient.Create();
      p1Containers.insert(containerName);
      p1p2Containers.insert(containerName);
    }
    for (int i = 0; i < 5; ++i)
    {
      std::string containerName = prefix2 + LowercaseRandomString();
      auto containerClient = m_blobServiceClient.GetBlobContainerClient(containerName);
      containerClient.Create();
      p2Containers.insert(containerName);
      p1p2Containers.insert(containerName);
    }

    Azure::Storage::Blobs::ListBlobContainersOptions options;
    options.MaxResults = 4;
    std::set<std::string> listContainers;
    do
    {
      auto res = m_blobServiceClient.ListBlobContainersSegment(options);
      EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderRequestId).empty());
      EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderDate).empty());
      EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderXMsVersion).empty());
      EXPECT_FALSE(res->ServiceEndpoint.empty());

      options.Marker = res->NextMarker;
      for (const auto& container : res->Items)
      {
        listContainers.insert(container.Name);
      }
    } while (!options.Marker.GetValue().empty());
    EXPECT_TRUE(std::includes(
        listContainers.begin(),
        listContainers.end(),
        p1p2Containers.begin(),
        p1p2Containers.end()));

    options.Prefix = prefix1;
    listContainers.clear();
    do
    {
      auto res = m_blobServiceClient.ListBlobContainersSegment(options);
      EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderRequestId).empty());
      EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderDate).empty());
      EXPECT_FALSE(res.GetRawResponse().GetHeaders().at(Details::c_HttpHeaderXMsVersion).empty());
      EXPECT_FALSE(res->ServiceEndpoint.empty());

      options.Marker = res->NextMarker;
      for (const auto& container : res->Items)
      {
        EXPECT_FALSE(container.Name.empty());
        EXPECT_FALSE(container.ETag.empty());
        EXPECT_FALSE(container.LastModified.empty());
        listContainers.insert(container.Name);
      }
    } while (!options.Marker.GetValue().empty());
    EXPECT_TRUE(std::includes(
        listContainers.begin(), listContainers.end(), p1Containers.begin(), p1Containers.end()));

    for (const auto& container : p1p2Containers)
    {
      auto container_client = m_blobServiceClient.GetBlobContainerClient(container);
      container_client.Delete();
    }
  }

}}} // namespace Azure::Storage::Test
