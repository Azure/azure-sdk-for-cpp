// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "test/ut/test_base.hpp"

#include <azure/storage/blobs.hpp>

namespace Azure { namespace Storage { namespace Test {

  class BlobServiceClientTest : public StorageTest {
  protected:
    std::shared_ptr<Blobs::BlobServiceClient> m_blobServiceClient;
    std::string m_accountName;

    std::string GetBlobServiceUrl()
    {
      return "https://" + StandardStorageAccountName() + ".blob.core.windows.net";
    }

    Blobs::BlobServiceClient GetBlobServiceClientOAuth()
    {
      if (m_useTokenCredentialByDefault)
      {
        return *m_blobServiceClient;
      }
      else
      {
        auto options = InitStorageClientOptions<Blobs::BlobClientOptions>();
        return Blobs::BlobServiceClient(
            m_blobServiceClient->GetUrl(), GetTestCredential(), options);
      }
    }

    void SetUp() override
    {
      StorageTest::SetUp();

      m_accountName = StandardStorageAccountName();
      auto options = InitStorageClientOptions<Blobs::BlobClientOptions>();
      if (m_useTokenCredentialByDefault)
      {
        m_blobServiceClient = std::make_shared<Blobs::BlobServiceClient>(
            GetBlobServiceUrl(), GetTestCredential(), options);
      }
      else
      {
        m_blobServiceClient = std::make_shared<Blobs::BlobServiceClient>(
            Blobs::BlobServiceClient::CreateFromConnectionString(
                StandardStorageConnectionString(), options));
      }
    }
  };

}}} // namespace Azure::Storage::Test
