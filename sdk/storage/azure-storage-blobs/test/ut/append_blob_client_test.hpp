// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/blobs.hpp>

#include "blob_container_client_test.hpp"
#include "test/ut/test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class AppendBlobClientTest : public BlobContainerClientTest {
  private:
    std::shared_ptr<Azure::Storage::Blobs::AppendBlobClient> m_appendBlobClient;

  protected:
    Azure::Storage::Blobs::CreateAppendBlobOptions m_blobUploadOptions;
    std::vector<uint8_t> m_blobContent;

    virtual void SetUp() override;
    virtual void TearDown() override;

    Azure::Storage::Blobs::AppendBlobClient const& GetAppendBlobClient(std::string const& blobName)
    {
      // Create container
      auto containerClient = GetBlobContainerTestClient();
      containerClient.CreateIfNotExists();

      m_appendBlobClient = std::make_unique<Azure::Storage::Blobs::AppendBlobClient>(
          containerClient.GetAppendBlobClient(blobName));

      m_blobContent = std::vector<uint8_t>(100, 'x');
      m_blobUploadOptions.Metadata = {{"key1", "V1"}, {"key2", "Value2"}};
      m_blobUploadOptions.HttpHeaders.ContentType = "application/x-binary";
      m_blobUploadOptions.HttpHeaders.ContentLanguage = "en-US";
      m_blobUploadOptions.HttpHeaders.ContentDisposition = "attachment";
      m_blobUploadOptions.HttpHeaders.CacheControl = "no-cache";
      m_blobUploadOptions.HttpHeaders.ContentEncoding = "identify";
      m_blobUploadOptions.HttpHeaders.ContentHash.Value.clear();
      m_appendBlobClient->Create(m_blobUploadOptions);
      auto blockContent
          = Azure::Core::IO::MemoryBodyStream(m_blobContent.data(), m_blobContent.size());
      m_appendBlobClient->AppendBlock(blockContent);
      m_blobUploadOptions.HttpHeaders.ContentHash
          = m_appendBlobClient->GetProperties().Value.HttpHeaders.ContentHash;

      return *m_appendBlobClient;
    }
  };

}}} // namespace Azure::Storage::Test