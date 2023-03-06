// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/blobs.hpp>

#include "blob_container_client_test.hpp"
#include "test/ut/test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class AppendBlobClientTest : public BlobContainerClientTest {
  protected:
    void SetUp() override;

    Blobs::AppendBlobClient GetAppendBlobClientForTest(
        const std::string& blobName,
        Blobs::BlobClientOptions clientOptions = Blobs::BlobClientOptions())
    {
      auto containerClient = GetBlobContainerClientForTest(m_containerName, clientOptions);
      return containerClient.GetAppendBlobClient(blobName);
    }

  protected:
    std::shared_ptr<Azure::Storage::Blobs::AppendBlobClient> m_appendBlobClient;
    std::string m_blobName;
    std::vector<uint8_t> m_blobContent;
  };

}}} // namespace Azure::Storage::Test
