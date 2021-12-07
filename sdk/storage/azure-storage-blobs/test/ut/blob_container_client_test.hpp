// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/blobs.hpp>

#include "test/ut/test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class BlobContainerClientTest : public StorageTest {
  private:
    std::shared_ptr<Azure::Storage::Blobs::BlobContainerClient> m_blobContainerClient;
    std::string m_containerName;
    std::string m_testName;

  protected:
    virtual void SetUp() override
    {
      StorageTest::SetUp();

      auto options = InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>();
      m_containerName = GetTestNameLowerCase();
    }

    virtual void TearDown() override
    {
      if (m_blobContainerClient)
      {
        m_blobContainerClient->Delete();
      }
      StorageTest::TearDown();
    }

    Azure::Storage::Blobs::BlobContainerClient const& GetBlobContainerClient(
        std::string const& containerName)
    {
      // set the interceptor for the current test
      if (m_testName.empty())
      {
        // Internal state for the test name allows a test to create new container client with any
        // name but just the first name is used as the test name for recordings.
        m_testName = GetTestName(true);
        m_testContext.RenameTest(containerName);
      }
      auto options = InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>();
      m_blobContainerClient = std::make_unique<Azure::Storage::Blobs::BlobContainerClient>(
          Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
              StandardStorageConnectionString(), containerName, options));

      return *m_blobContainerClient;
    }

    std::string GetSas();
    Blobs::Models::BlobItem GetBlobItem(
        const std::string& blobName,
        Blobs::Models::ListBlobsIncludeFlags include = Blobs::Models::ListBlobsIncludeFlags::None);
  };

}}} // namespace Azure::Storage::Test
