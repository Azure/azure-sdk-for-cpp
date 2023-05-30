// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "test_base.hpp"

#include <azure/storage/blobs/block_blob_client.hpp>

#include <unordered_map>

namespace Azure { namespace Storage { namespace Test {

  class ConnectionReuseTest : public StorageTest {
  };

  void updateHitCountPerServerMap(
      const Azure::Core::CaseInsensitiveMap& headers,
      std::unordered_map<std::string, int32_t>& hitCountPerServerMap)
  {
    // The third part of a storage request id means the server node id.
    // For Example, RequestId:3bcf963b-601e-0054-1f40-910c39000000, '0054' is the server node which
    // served this request.
    std::string serverId = headers.find("x-ms-request-id")->second.substr(14, 4);
    if (hitCountPerServerMap.count(serverId) == 0)
    {
      hitCountPerServerMap[serverId] = 1;
    }
    else
    {
      ++hitCountPerServerMap[serverId];
    }
  }

  // If connection is reused, the requests with the same connection should hit the same sever. So
  // this test verifies whether a series of requests hit the same server.
  TEST_F(ConnectionReuseTest, IsConnectionReused_LIVEONLY_)
  {
    const std::string containerName = LowercaseRandomString();
    const std::string blobName = LowercaseRandomString();

    auto clientOptions = InitStorageClientOptions<Azure::Storage::Blobs::BlobClientOptions>();
    auto containerClient1 = Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
        StandardStorageConnectionString(), containerName + "1", clientOptions);
    auto containerClient2 = Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
        StandardStorageConnectionString(), containerName + "2", clientOptions);
    containerClient1.Create();
    containerClient2.Create();

    auto buffer = RandomBuffer(100);
    Core::IO::MemoryBodyStream bodyStream(buffer.data(), buffer.size());

    std::vector<Azure::Storage::Blobs::BlockBlobClient> blobClients;
    for (int i = 0; i < 5; ++i)
    {
      blobClients.push_back(containerClient1.GetBlockBlobClient(blobName + std::to_string(i)));
      blobClients.push_back(containerClient2.GetBlockBlobClient(blobName + std::to_string(i)));
    }
    std::unordered_map<std::string, int32_t> hitCountPerServerMap;
    int32_t totalHitCount = 0;
    for (auto& blobClient : blobClients)
    {
      auto uploadResult = blobClient.Upload(bodyStream);
      updateHitCountPerServerMap(uploadResult.RawResponse->GetHeaders(), hitCountPerServerMap);
      ++totalHitCount;
      auto downloadResult = blobClient.Download();
      updateHitCountPerServerMap(downloadResult.RawResponse->GetHeaders(), hitCountPerServerMap);
      ++totalHitCount;
      auto deleteResult = blobClient.Delete();
      updateHitCountPerServerMap(deleteResult.RawResponse->GetHeaders(), hitCountPerServerMap);
      ++totalHitCount;
      deleteResult = blobClient.DeleteIfExists();
      updateHitCountPerServerMap(deleteResult.RawResponse->GetHeaders(), hitCountPerServerMap);
      ++totalHitCount;
    }

    int32_t maxSingleServerHitCount = 0;
    for (auto& hitCountEntry : hitCountPerServerMap)
    {
      maxSingleServerHitCount = std::max(maxSingleServerHitCount, hitCountEntry.second);
    }
    // At least 80% requests should use the same connection.
    EXPECT_TRUE(maxSingleServerHitCount > totalHitCount * 0.8);

    containerClient1.Delete();
    containerClient2.Delete();
  }

}}} // namespace Azure::Storage::Test
