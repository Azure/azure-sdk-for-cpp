// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/blobs.hpp>

#include "blob_service_client_test.hpp"

namespace Azure { namespace Storage { namespace Test {

  class BlobContainerClientTest : public BlobServiceClientTest {
  protected:
    void SetUp() override;

    Blobs::BlobContainerClient GetBlobContainerClientForTest(
        const std::string& containerName,
        Blobs::BlobClientOptions clientOptions = Blobs::BlobClientOptions());

    std::string GetSas();
    Blobs::Models::BlobItem GetBlobItem(
        const std::string& blobName,
        Blobs::Models::ListBlobsIncludeFlags include = Blobs::Models::ListBlobsIncludeFlags::None);

  protected:
    std::string m_containerName;
    std::shared_ptr<Blobs::BlobContainerClient> m_blobContainerClient;
  };

}}} // namespace Azure::Storage::Test
