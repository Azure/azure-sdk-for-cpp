// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs.hpp"
#include "blob_container_client_test.hpp"
#include "test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class PageBlobClientTest : public BlobContainerClientTest {
  protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    static std::shared_ptr<Azure::Storage::Blobs::PageBlobClient> m_pageBlobClient;
    static std::string m_blobName;
    static Azure::Storage::Blobs::CreatePageBlobOptions m_blobUploadOptions;
    static std::vector<uint8_t> m_blobContent;
  };

}}} // namespace Azure::Storage::Test
