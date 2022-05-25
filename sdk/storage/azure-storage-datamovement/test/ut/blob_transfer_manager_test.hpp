// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "test/ut/test_base.hpp"

#include <memory>

#include <azure/storage/blobs.hpp>

namespace Azure { namespace Storage { namespace Test {

  class BlobTransferManagerTest : public Azure::Storage::Test::StorageTest {
  private:
    std::unique_ptr<Azure::Storage::Blobs::BlobServiceClient> m_client;

  protected:
    const Azure::Storage::Blobs::BlobServiceClient& GetClientForTest(const std::string& testName)
    {
      m_testContext.RenameTest(testName);
      return *m_client;
    }

    void SetUp() override
    {
      StorageTest::SetUp();

      auto options = InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>();
      m_client = std::make_unique<Azure::Storage::Blobs::BlobServiceClient>(
          Azure::Storage::Blobs::BlobServiceClient::CreateFromConnectionString(
              StandardStorageConnectionString(), options));
    }

    static void CreateDir(const std::string& dir);

    static void DeleteDir(const std::string& dir);
  };

}}} // namespace Azure::Storage::Test
