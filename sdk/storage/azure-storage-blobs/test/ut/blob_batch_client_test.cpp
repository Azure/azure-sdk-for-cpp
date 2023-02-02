// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/blobs.hpp>

#include "test/ut/test_base.hpp"

namespace Azure { namespace Storage { namespace Test {

  class BlobBatchClientTest : public StorageTest {
  private:
    std::unique_ptr<Azure::Storage::Blobs::BlobServiceClient> m_client;

  protected:
    // Required to rename the test propertly once the test is started.
    // We can only know the test instance name until the test instance is run.
    Azure::Storage::Blobs::BlobServiceClient const& GetClientForTest(std::string const& testName)
    {
      // set the interceptor for the current test
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
  };

  TEST_F(BlobBatchClientTest, DISABLED_SubmitDeleteBatch)
  {
    const std::string testName = GetTestNameLowerCase();

    const std::string containerName1 = testName + "1";
    const std::string blob1Name = "b1";
    const std::string blob2Name = "b2";
    const std::string containerName2 = testName + "2";
    const std::string blob3Name = "b3";

    auto serviceClient = GetClientForTest(testName);
    auto container1Client = serviceClient.GetBlobContainerClient(containerName1);
    container1Client.CreateIfNotExists();
    auto container2Client = serviceClient.GetBlobContainerClient(containerName2);
    container2Client.CreateIfNotExists();
    auto blob1Client = container1Client.GetAppendBlobClient(blob1Name);
    blob1Client.Create();
    auto blob2Client = container1Client.GetAppendBlobClient(blob2Name);
    blob2Client.Create();
    auto blob3Client = container2Client.GetAppendBlobClient(blob3Name);
    blob3Client.Create();
    blob3Client.CreateSnapshot();

    auto batch = serviceClient.CreateBatch();
    auto delete1Response = batch.DeleteBlobUrl(blob1Client.GetUrl());
    auto delete2Response = batch.DeleteBlob(containerName1, blob2Name);
    Blobs::DeleteBlobOptions deleteOptions;
    deleteOptions.DeleteSnapshots = Blobs::Models::DeleteSnapshotsOption::OnlySnapshots;
    auto delete3Response = batch.DeleteBlobUrl(blob3Client.GetUrl(), deleteOptions);
    auto submitBatchResponse = serviceClient.SubmitBatch(batch);

    EXPECT_TRUE(delete1Response.GetResponse().Value.Deleted);
    EXPECT_TRUE(delete2Response.GetResponse().Value.Deleted);
    EXPECT_TRUE(delete3Response.GetResponse().Value.Deleted);
    EXPECT_THROW(blob1Client.GetProperties(), StorageException);
    EXPECT_THROW(blob2Client.GetProperties(), StorageException);
    EXPECT_NO_THROW(blob3Client.GetProperties());

    container1Client.Delete();
    container2Client.Delete();
  }

  TEST_F(BlobBatchClientTest, DISABLED_SnapshotVersion)
  {
    const std::string testName = GetTestNameLowerCase();

    const std::string containerName1 = testName + "1";
    const std::string blob1Name = "blockblob1";
    auto serviceClient = GetClientForTest(testName);
    auto container1Client = serviceClient.GetBlobContainerClient(containerName1);
    container1Client.CreateIfNotExists();
    auto blob1Client = container1Client.GetBlockBlobClient(blob1Name);
    auto versionId = blob1Client.UploadFrom(nullptr, 0).Value.VersionId.Value();
    auto snapshotId = blob1Client.CreateSnapshot().Value.Snapshot;

    EXPECT_NO_THROW(blob1Client.WithVersionId(versionId).GetProperties());
    EXPECT_NO_THROW(blob1Client.WithSnapshot(snapshotId).GetProperties());

    auto batch = serviceClient.CreateBatch();
    auto r1 = batch.SetBlobAccessTierUrl(
        blob1Client.WithVersionId(versionId).GetUrl(), Blobs::Models::AccessTier::Cool);
    auto r2 = batch.SetBlobAccessTierUrl(
        blob1Client.WithSnapshot(snapshotId).GetUrl(), Blobs::Models::AccessTier::Cool);
    EXPECT_NO_THROW(serviceClient.SubmitBatch(batch));
    EXPECT_NO_THROW(r1.GetResponse());
    EXPECT_NO_THROW(r2.GetResponse());
    EXPECT_EQ(
        blob1Client.WithVersionId(versionId).GetProperties().Value.AccessTier.Value(),
        Blobs::Models::AccessTier::Cool);
    EXPECT_EQ(
        blob1Client.WithSnapshot(snapshotId).GetProperties().Value.AccessTier.Value(),
        Blobs::Models::AccessTier::Cool);

    batch = serviceClient.CreateBatch();
    auto r3 = batch.DeleteBlobUrl(blob1Client.WithVersionId(versionId).GetUrl());
    auto r4 = batch.DeleteBlobUrl(blob1Client.WithSnapshot(snapshotId).GetUrl());
    EXPECT_NO_THROW(serviceClient.SubmitBatch(batch));
    EXPECT_NO_THROW(r3.GetResponse());
    EXPECT_NO_THROW(r4.GetResponse());
    EXPECT_THROW(blob1Client.WithVersionId(versionId).GetProperties(), StorageException);
    EXPECT_THROW(blob1Client.WithSnapshot(snapshotId).GetProperties(), StorageException);

    container1Client.DeleteIfExists();
  }

  TEST_F(BlobBatchClientTest, SubmitSetTierBatch_LIVEONLY_)
  {
    const std::string testName = GetTestNameLowerCase();

    const std::string containerName = testName;
    const std::string blob1Name = "b1";
    const std::string blob2Name = "b2";

    auto containerSasToken = [&]() {
      Sas::BlobSasBuilder sasBuilder;
      sasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
      sasBuilder.ExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(5);
      sasBuilder.BlobContainerName = containerName;
      sasBuilder.Resource = Sas::BlobSasResource::BlobContainer;
      sasBuilder.SetPermissions(Sas::BlobContainerSasPermissions::All);
      return sasBuilder.GenerateSasToken(
          *_internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential);
    }();

    auto serviceClient = GetClientForTest(testName);
    serviceClient.GetBlobContainerClient(containerName).CreateIfNotExists();
    auto containerClient = Blobs::BlobContainerClient(
        serviceClient.GetBlobContainerClient(containerName).GetUrl() + containerSasToken,
        InitClientOptions<Blobs::BlobClientOptions>());
    auto blob1Client = containerClient.GetBlockBlobClient(blob1Name);
    blob1Client.UploadFrom(nullptr, 0);
    auto blob2Client = containerClient.GetBlockBlobClient(blob2Name);
    blob2Client.UploadFrom(nullptr, 0);

    auto batch = containerClient.CreateBatch();
    auto setTier1Response = batch.SetBlobAccessTier(blob1Name, Blobs::Models::AccessTier::Cool);
    auto setTier2Response = batch.SetBlobAccessTier(blob2Name, Blobs::Models::AccessTier::Archive);
    auto submitBatchResponse = containerClient.SubmitBatch(batch);

    EXPECT_NO_THROW(setTier1Response.GetResponse());
    EXPECT_NO_THROW(setTier2Response.GetResponse());
    EXPECT_EQ(
        blob1Client.GetProperties().Value.AccessTier.Value(), Blobs::Models::AccessTier::Cool);
    EXPECT_EQ(
        blob2Client.GetProperties().Value.AccessTier.Value(), Blobs::Models::AccessTier::Archive);

    serviceClient.DeleteBlobContainer(containerName);
  }

  TEST_F(BlobBatchClientTest, DISABLED_TokenAuthorization)
  {
    const std::string testName = GetTestNameLowerCase();

    std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential
        = std::make_shared<Azure::Identity::ClientSecretCredential>(
            AadTenantId(), AadClientId(), AadClientSecret());
    Blobs::BlobClientOptions options;

    auto serviceClient = InitTestClient<Blobs::BlobServiceClient, Blobs::BlobClientOptions>(
        GetClientForTest(testName).GetUrl(), credential, options);

    const std::string containerName = testName;
    const std::string blobName = "b1";

    auto containerClient = serviceClient->GetBlobContainerClient(containerName);
    containerClient.CreateIfNotExists();
    auto blobClient = containerClient.GetAppendBlobClient(blobName);
    blobClient.Create();

    auto batch = containerClient.CreateBatch();
    auto delete1Response = batch.DeleteBlobUrl(blobClient.GetUrl());
    auto submitBatchResponse = containerClient.SubmitBatch(batch);

    EXPECT_TRUE(delete1Response.GetResponse().Value.Deleted);

    containerClient.Delete();
  }

  TEST_F(BlobBatchClientTest, Exceptions_LIVEONLY_)
  {
    const std::string testName = GetTestNameLowerCase();

    const std::string containerName = testName;
    const std::string blobName = "b1";

    auto serviceClient = GetClientForTest(testName);
    auto containerClient = serviceClient.GetBlobContainerClient(containerName);
    containerClient.CreateIfNotExists();
    auto blobClient = containerClient.GetBlockBlobClient(blobName);
    blobClient.UploadFrom(nullptr, 0);

    // Empty batch
    auto batch = containerClient.CreateBatch();

    try
    {
      containerClient.SubmitBatch(batch);
      FAIL();
    }
    catch (StorageException& e)
    {
      EXPECT_EQ(e.StatusCode, Azure::Core::Http::HttpStatusCode::BadRequest);
      EXPECT_FALSE(e.ReasonPhrase.empty());
      EXPECT_FALSE(e.RequestId.empty());
      EXPECT_FALSE(e.ClientRequestId.empty());
      EXPECT_EQ(e.ErrorCode, "InvalidInput");
    }
    catch (...)
    {
      FAIL();
    }

    // Partial failure
    {
      auto r1 = batch.SetBlobAccessTierUrl(blobClient.GetUrl(), Blobs::Models::AccessTier::Hot);
      auto r2 = batch.SetBlobAccessTier("BlobNameNotExists", Blobs::Models::AccessTier::Hot);
      EXPECT_NO_THROW(containerClient.SubmitBatch(batch));
      EXPECT_NO_THROW(r1.GetResponse());
      EXPECT_THROW(r2.GetResponse(), StorageException);
    }

    // Mixed operations
    auto batch2 = containerClient.CreateBatch();
    batch2.SetBlobAccessTierUrl(blobClient.GetUrl(), Blobs::Models::AccessTier::Cool);
    batch2.DeleteBlobUrl(blobClient.GetUrl());

    try
    {
      containerClient.SubmitBatch(batch2);
      FAIL();
    }
    catch (StorageException& e)
    {
      EXPECT_EQ(e.StatusCode, Azure::Core::Http::HttpStatusCode::BadRequest);
      EXPECT_FALSE(e.ReasonPhrase.empty());
      EXPECT_FALSE(e.RequestId.empty());
      EXPECT_FALSE(e.ClientRequestId.empty());
      EXPECT_EQ(e.ErrorCode, "AllBatchSubRequestsShouldBeSameApi");
    }
    catch (...)
    {
      FAIL();
    }

    auto containerExpiredSasToken = [&]() {
      Sas::BlobSasBuilder sasBuilder;
      sasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
      sasBuilder.ExpiresOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
      sasBuilder.BlobContainerName = containerName;
      sasBuilder.Resource = Sas::BlobSasResource::BlobContainer;
      sasBuilder.SetPermissions(Sas::BlobContainerSasPermissions::All);
      return sasBuilder.GenerateSasToken(
          *_internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential);
    }();
    auto containerSasToken = [&]() {
      Sas::BlobSasBuilder sasBuilder;
      sasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
      sasBuilder.ExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(5);
      sasBuilder.BlobContainerName = containerName;
      sasBuilder.Resource = Sas::BlobSasResource::BlobContainer;
      sasBuilder.SetPermissions(Sas::BlobContainerSasPermissions::All);
      return sasBuilder.GenerateSasToken(
          *_internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential);
    }();
    auto containerSasClient = Blobs::BlobContainerClient(
        serviceClient.GetBlobContainerClient(containerName).GetUrl() + containerExpiredSasToken);
    auto batch3 = containerSasClient.CreateBatch();
    batch3.DeleteBlobUrl(blobClient.GetUrl() + containerSasToken);
    try
    {
      containerSasClient.SubmitBatch(batch3);
      FAIL();
    }
    catch (StorageException& e)
    {
      EXPECT_EQ(e.StatusCode, Azure::Core::Http::HttpStatusCode::Forbidden);
      EXPECT_FALSE(e.ReasonPhrase.empty());
      EXPECT_FALSE(e.RequestId.empty());
      EXPECT_FALSE(e.ClientRequestId.empty());
      EXPECT_EQ(e.ErrorCode, "AuthenticationFailed");
    }
    catch (...)
    {
      FAIL();
    }

    containerClient.Delete();
  }

}}} // namespace Azure::Storage::Test
