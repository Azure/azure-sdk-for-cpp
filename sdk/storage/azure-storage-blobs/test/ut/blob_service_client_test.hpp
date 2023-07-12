// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "test/ut/test_base.hpp"

#include <azure/storage/blobs.hpp>

namespace Azure { namespace Storage { namespace Test {

  class BlobServiceClientTest : public StorageTest {
  protected:
    std::shared_ptr<Blobs::BlobServiceClient> m_blobServiceClient;

    void SetUp() override
    {
      StorageTest::SetUp();

      auto options = InitStorageClientOptions<Blobs::BlobClientOptions>();
      m_blobServiceClient = std::make_shared<Blobs::BlobServiceClient>(
          Blobs::BlobServiceClient::CreateFromConnectionString(
              StandardStorageConnectionString(), options));
    }
  };

}}} // namespace Azure::Storage::Test
