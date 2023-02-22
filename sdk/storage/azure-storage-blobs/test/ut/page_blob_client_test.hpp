// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/blobs.hpp>

#include "blob_container_client_test.hpp"

namespace Azure { namespace Storage { namespace Test {

  class PageBlobClientTest : public BlobContainerClientTest {
  protected:
    void SetUp() override;

    Blobs::PageBlobClient GetPageBlobClientTestForTest(
        const std::string& blobName,
        Blobs::BlobClientOptions clientOptions = Blobs::BlobClientOptions())
    {
      auto containerClient = GetBlobContainerClientForTest(m_containerName, clientOptions);
      return containerClient.GetPageBlobClient(blobName);
    }

  protected:
    std::shared_ptr<Azure::Storage::Blobs::PageBlobClient> m_pageBlobClient;
    std::string m_blobName;
    std::vector<uint8_t> m_blobContent;
  };

}}} // namespace Azure::Storage::Test
