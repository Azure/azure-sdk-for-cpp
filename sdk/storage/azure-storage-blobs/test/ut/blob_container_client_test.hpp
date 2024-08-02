// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "blob_service_client_test.hpp"

#include <azure/storage/blobs.hpp>

namespace Azure { namespace Storage { namespace Test {

  class BlobContainerClientTest : public BlobServiceClientTest {
  protected:
    void SetUp() override;

    Blobs::BlobContainerClient GetBlobContainerClientForTest(
        const std::string& containerName,
        Blobs::BlobClientOptions clientOptions = Blobs::BlobClientOptions());

    std::string GetBlobContainerUrl(const std::string& containerName)
    {
      return GetBlobServiceUrl() + "/" + containerName;
    }

    std::string GetSas();

    Blobs::Models::BlobItem GetBlobItem(
        const std::string& blobName,
        Blobs::Models::ListBlobsIncludeFlags include = Blobs::Models::ListBlobsIncludeFlags::None)
    {
      return GetBlobItem(*m_blobContainerClient, blobName, include);
    }

    Blobs::Models::BlobItem GetBlobItem(
        const Blobs::BlobContainerClient& containerClient,
        const std::string& blobName,
        Blobs::Models::ListBlobsIncludeFlags include = Blobs::Models::ListBlobsIncludeFlags::None);

  protected:
    std::string m_containerName;
    std::shared_ptr<Blobs::BlobContainerClient> m_blobContainerClient;
  };

}}} // namespace Azure::Storage::Test
