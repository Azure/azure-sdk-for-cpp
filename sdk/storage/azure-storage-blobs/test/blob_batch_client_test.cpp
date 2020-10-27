// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs.hpp"
#include "azure/storage/blobs/blob_sas_builder.hpp"
#include "test_base.hpp"

#include <thread>

namespace Azure { namespace Storage { namespace Test {

  class BlobBatchClientTest : public ::testing::Test {
  protected:
    BlobBatchClientTest()
        : m_blobBatchClient(Azure::Storage::Blobs::BlobBatchClient::CreateFromConnectionString(
            StandardStorageConnectionString()))
    {
    }

    Azure::Storage::Blobs::BlobBatchClient m_blobBatchClient;
  };

  TEST_F(BlobBatchClientTest, BatchSasAuth)
  {
    AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = SasProtocol::HttpsAndHtttp;
    accountSasBuilder.StartsOn
        = ToIso8601(std::chrono::system_clock::now() - std::chrono::minutes(5));
    accountSasBuilder.ExpiresOn
        = ToIso8601(std::chrono::system_clock::now() + std::chrono::minutes(60));
    accountSasBuilder.Services = AccountSasServices::Blobs;
    accountSasBuilder.ResourceTypes = AccountSasResource::Object | AccountSasResource::Container;
    accountSasBuilder.SetPermissions(AccountSasPermissions::All);
    auto keyCredential
        = Details::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    auto serviceClient
        = Blobs::BlobServiceClient::CreateFromConnectionString(StandardStorageConnectionString());
    std::string containerName = LowercaseRandomString();
    auto containerClient = serviceClient.GetBlobContainerClient(containerName);
    containerClient.Create();
    std::string blobName = RandomString();
    auto blobClient = containerClient.GetBlockBlobClient(blobName);
    blobClient.UploadFrom(nullptr, 0);

    auto batch = Azure::Storage::Blobs::BlobBatchClient::CreateBatch();
    batch.DeleteBlob(containerName, blobName);

    auto batchClient = Blobs::BlobBatchClient(serviceClient.GetUri());

    EXPECT_THROW(batchClient.SubmitBatch(batch), StorageError);

    batchClient = Blobs::BlobBatchClient(
        serviceClient.GetUri() + accountSasBuilder.ToSasQueryParameters(*keyCredential));

    EXPECT_NO_THROW(batchClient.SubmitBatch(batch));

    containerClient.Delete();
  }

  TEST_F(BlobBatchClientTest, Batch)
  {
    auto serviceClient
        = Blobs::BlobServiceClient::CreateFromConnectionString(StandardStorageConnectionString());
    std::string containerName1 = LowercaseRandomString();
    std::string containerName2 = LowercaseRandomString();
    auto containerClient1 = serviceClient.GetBlobContainerClient(containerName1);
    containerClient1.Create();
    auto containerClient2 = serviceClient.GetBlobContainerClient(containerName2);
    containerClient2.Create();

    std::string blobName11 = RandomString();
    auto blobClient11 = containerClient1.GetBlockBlobClient(blobName11);
    blobClient11.UploadFrom(nullptr, 0);

    std::string blobName12 = RandomString();
    auto blobClient12 = containerClient1.GetBlockBlobClient(blobName12);
    blobClient12.UploadFrom(nullptr, 0);

    std::string blobName21 = RandomString();
    auto blobClient21 = containerClient2.GetBlockBlobClient(blobName21);
    blobClient21.UploadFrom(nullptr, 0);

    std::string blobName22 = RandomString();

    auto batch = Azure::Storage::Blobs::BlobBatchClient::CreateBatch();
    int32_t id1 = batch.SetBlobAccessTier(containerName1, blobName11, Blobs::AccessTier::Cool);
    int32_t id2 = batch.SetBlobAccessTier(containerName1, blobName12, Blobs::AccessTier::Hot);
    int32_t id3 = batch.SetBlobAccessTier(containerName2, blobName21, Blobs::AccessTier::Hot);
    int32_t id4 = batch.SetBlobAccessTier(containerName2, blobName22, Blobs::AccessTier::Cool);
    unused(id1, id2, id3, id4);
    
    std::size_t failedId = static_cast<std::size_t>(id4);
    std::size_t batchSize = static_cast<std::size_t>(id4) + 1;

    auto batchResult = m_blobBatchClient.SubmitBatch(batch);
    EXPECT_EQ(batchResult->SetBlobAccessTierResults.size(), batchSize);
    EXPECT_TRUE(batchResult->DeleteBlobResults.empty());
    for (std::size_t i = 0; i < batchSize; ++i)
    {
      if (i != failedId)
      {
        EXPECT_EQ(
            batchResult->SetBlobAccessTierResults[i].GetRawResponse().GetStatusCode(),
            Azure::Core::Http::HttpStatusCode::Ok);
      }
      else
      {
        EXPECT_NE(
            batchResult->SetBlobAccessTierResults[i].GetRawResponse().GetStatusCode(),
            Azure::Core::Http::HttpStatusCode::Ok);
      }
    }

    batch = Azure::Storage::Blobs::BlobBatchClient::CreateBatch();
    id1 = batch.DeleteBlob(containerName1, blobName11);
    id2 = batch.DeleteBlob(containerName1, blobName12);
    id3 = batch.DeleteBlob(containerName2, blobName21);
    id4 = batch.DeleteBlob(containerName2, blobName22);

    failedId = static_cast<std::size_t>(id4);
    batchSize = static_cast<std::size_t>(id4) + 1;

    batchResult = m_blobBatchClient.SubmitBatch(batch);

    EXPECT_EQ(batchResult->DeleteBlobResults.size(), batchSize);
    EXPECT_TRUE(batchResult->SetBlobAccessTierResults.empty());
    for (std::size_t i = 0; i < batchSize; ++i)
    {
      if (i != failedId)
      {
        EXPECT_EQ(
            batchResult->DeleteBlobResults[i].GetRawResponse().GetStatusCode(),
            Azure::Core::Http::HttpStatusCode::Accepted);
      }
      else
      {
        EXPECT_NE(
            batchResult->DeleteBlobResults[i].GetRawResponse().GetStatusCode(),
            Azure::Core::Http::HttpStatusCode::Accepted);
      }
    }

    containerClient1.Delete();
    containerClient2.Delete();
  }

}}} // namespace Azure::Storage::Test
