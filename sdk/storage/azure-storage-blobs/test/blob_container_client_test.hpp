// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs.hpp"
#include "test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class BlobContainerClientTest : public ::testing::Test {
  protected:
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    static std::string GetSas();

    static std::shared_ptr<Azure::Storage::Blobs::BlobContainerClient> m_blobContainerClient;
    static std::string m_containerName;
  };

}}} // namespace Azure::Storage::Test
