//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/blobs.hpp>

#include "blob_container_client_test.hpp"
#include "test/ut/test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class PageBlobClientTest : public BlobContainerClientTest {
  private:
    std::shared_ptr<Azure::Storage::Blobs::PageBlobClient> m_pageBlobClient;

  protected:
    std::string m_blobName;
    Azure::Storage::Blobs::CreatePageBlobOptions m_blobUploadOptions;
    std::vector<uint8_t> m_blobContent;

    virtual void SetUp() override;
    virtual void TearDown() override;

    Azure::Storage::Blobs::PageBlobClient const& GetPageBlobClient(std::string const& blobName)
    {
      // Create container
      auto containerClient = GetBlobContainerTestClient();
      containerClient.CreateIfNotExists();

      m_blobContent = std::vector<uint8_t>(static_cast<size_t>(1_KB), 'x');
      m_pageBlobClient = std::make_unique<Azure::Storage::Blobs::PageBlobClient>(
          containerClient.GetPageBlobClient(blobName));

      return *m_pageBlobClient;
    }

    void SetOptions()
    {
      m_blobUploadOptions.Metadata = {{"key1", "V1"}, {"key2", "Value2"}};
      m_blobUploadOptions.HttpHeaders.ContentType = "application/x-binary";
      m_blobUploadOptions.HttpHeaders.ContentLanguage = "en-US";
      m_blobUploadOptions.HttpHeaders.ContentDisposition = "attachment";
      m_blobUploadOptions.HttpHeaders.CacheControl = "no-cache";
      m_blobUploadOptions.HttpHeaders.ContentEncoding = "identity";
      m_blobUploadOptions.HttpHeaders.ContentHash.Value.clear();
    }

    void UploadPage(unsigned long long blobSize = 1_KB)
    {
      SetOptions();

      m_blobContent = std::vector<uint8_t>(static_cast<size_t>(blobSize), 'x');
      m_pageBlobClient->Create(m_blobContent.size(), m_blobUploadOptions);
      auto pageContent
          = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
      m_pageBlobClient->UploadPages(0, pageContent);
      m_blobUploadOptions.HttpHeaders.ContentHash
          = m_pageBlobClient->GetProperties().Value.HttpHeaders.ContentHash;
    }
  };

}}} // namespace Azure::Storage::Test
