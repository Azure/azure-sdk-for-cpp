// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "blob_container_client_test.hpp"

#include <azure/storage/blobs.hpp>

namespace Azure { namespace Storage { namespace Test {

  class BlockBlobClientTest : public BlobContainerClientTest {
  protected:
    void SetUp() override;

    Blobs::BlockBlobClient GetBlockBlobClientForTest(
        const std::string& blobName,
        Blobs::BlobClientOptions clientOptions = Blobs::BlobClientOptions())
    {
      auto containerClient = GetBlobContainerClientForTest(m_containerName, clientOptions);
      return containerClient.GetBlockBlobClient(blobName);
    }

  protected:
    std::shared_ptr<Azure::Storage::Blobs::BlockBlobClient> m_blockBlobClient;
    std::string m_blobName;
    std::vector<uint8_t> m_blobContent;
    Azure::Storage::Blobs::UploadBlockBlobOptions m_blobUploadOptions;
  };

}}} // namespace Azure::Storage::Test
