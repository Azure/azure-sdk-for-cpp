// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "block_blob_client_test.hpp"

namespace Azure { namespace Storage { namespace Test {

  void updateDistinctServers(
      const Azure::Core::CaseInsensitiveMap& headers,
      std::unordered_set<std::string>& distinctServers)
  {
    // The third part of a storage request id means the server node id.
    // For Example, RequestId:3bcf963b-601e-0054-1f40-910c39000000, '0054' is the server node which
    // served this request.
    std::string serverId = headers.find("x-ms-request-id")->second.substr(14, 4);
    distinctServers.insert(serverId);
  }

  // If connection is reused, the requests with the same connection should hit the same sever. So
  // this test verifies whether a series of requests hit the same server.
  TEST_F(BlockBlobClientTest, IsConnectionReused_LIVEONLY_)
  {
    const std::string containerName = LowercaseRandomString();
    const std::string blobName = LowercaseRandomString();

    auto containerClient1 = GetBlobContainerClientForTest(containerName + "1");
    auto containerClient2 = GetBlobContainerClientForTest(containerName + "2");
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
    std::unordered_set<std::string> distinctServers;
    size_t totalHitCount = 0;
    for (auto& blobClient : blobClients)
    {
      auto uploadResult = blobClient.Upload(bodyStream);
      updateDistinctServers(uploadResult.RawResponse->GetHeaders(), distinctServers);
      ++totalHitCount;
      auto downloadResult = blobClient.Download();
      ReadBodyStream(downloadResult.Value.BodyStream);
      updateDistinctServers(downloadResult.RawResponse->GetHeaders(), distinctServers);
      ++totalHitCount;
      auto deleteResult = blobClient.Delete();
      updateDistinctServers(deleteResult.RawResponse->GetHeaders(), distinctServers);
      ++totalHitCount;
    }

    size_t distinctServersLessThan = totalHitCount / 5;
    EXPECT_TRUE(distinctServers.size() < distinctServersLessThan);
  }
}}} // namespace Azure::Storage::Test
