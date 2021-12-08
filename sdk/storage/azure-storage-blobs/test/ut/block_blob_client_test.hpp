// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/blobs.hpp>

#include "blob_container_client_test.hpp"
#include "test/ut/test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class BlockBlobClientTest : public BlobContainerClientTest {
  private:
    std::shared_ptr<Azure::Storage::Blobs::BlockBlobClient> m_blockBlobClient;
    std::string m_blobName;

  protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
    std::vector<uint8_t> m_blobContent;
    Azure::Storage::Blobs::UploadBlockBlobOptions m_blobUploadOptions;

    Azure::Storage::Blobs::BlockBlobClient const& GetBlockBlobClient(std::string const& blobName)
    {
      // Create container
      auto const testName(GetTestNameLowerCase());
      auto containerClient = GetBlobContainerClient(testName);
      containerClient.Create();

      m_blockBlobClient = std::make_shared<Azure::Storage::Blobs::BlockBlobClient>(
          containerClient.GetBlockBlobClient(blobName));

      return *m_blockBlobClient;
    }

    void UploadBlockBlob(size_t blobSize = 1_KB)
    {
      m_blobContent = std::vector<uint8_t>(blobSize, 'x');
      m_blobUploadOptions.Metadata = {{"key1", "V1"}, {"key2", "Value2"}};
      m_blobUploadOptions.HttpHeaders.ContentType = "application/x-binary";
      m_blobUploadOptions.HttpHeaders.ContentLanguage = "en-US";
      m_blobUploadOptions.HttpHeaders.ContentDisposition = "attachment";
      m_blobUploadOptions.HttpHeaders.CacheControl = "no-cache";
      m_blobUploadOptions.HttpHeaders.ContentEncoding = "identity";
      m_blobUploadOptions.HttpHeaders.ContentHash.Value.clear();
      m_blobUploadOptions.AccessTier = Azure::Storage::Blobs::Models::AccessTier::Hot;
      auto blobContent
          = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
      m_blockBlobClient->Upload(blobContent, m_blobUploadOptions);
      m_blobUploadOptions.HttpHeaders.ContentHash
          = m_blockBlobClient->GetProperties().Value.HttpHeaders.ContentHash;
    }
  };

}}} // namespace Azure::Storage::Test
