// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blob_transfer_manager_test.hpp"

#include <azure/storage/datamovement/blob_transfer_manager.hpp>

namespace Azure { namespace Storage { namespace Test {

  TEST_F(BlobTransferManagerTest, SingleCopy_LIVEONLY_)
  {
    const auto testName = GetTestNameLowerCase();
    auto blobServiceClient = GetClientForTest(testName);
    auto containerClient = blobServiceClient.GetBlobContainerClient(GetContainerValidName());
    containerClient.CreateIfNotExists();

    for (auto fileSize : std::vector<uint64_t>{0, 123, 456_KB, 2_MB, 4_MB, 8_MB, 25_MB + 1234})
    {
      auto srcBlobClient
          = containerClient.GetBlockBlobClient(testName + std::to_string(fileSize) + "src");
      auto destBlobClient
          = containerClient.GetBlobClient(testName + std::to_string(fileSize) + "dest");
      auto buffer = RandomBuffer(static_cast<size_t>(fileSize));
      srcBlobClient.UploadFrom(buffer.data(), buffer.size());

      Blobs::BlobTransferManager m;
      auto job = m.ScheduleCopy(srcBlobClient, destBlobClient);
      EXPECT_FALSE(job.Id.empty());
      EXPECT_EQ(job.SourceUrl, srcBlobClient.GetUrl());
      EXPECT_EQ(job.DestinationUrl, destBlobClient.GetUrl());
      EXPECT_EQ(job.Type, TransferType::SingleCopy);

      auto jobStatus = job.WaitHandle.get();
      EXPECT_EQ(jobStatus, JobStatus::Succeeded);

      EXPECT_EQ(buffer, ReadBodyStream(destBlobClient.Download().Value.BodyStream));
    }
    containerClient.DeleteIfExists();
  }

  TEST_F(BlobTransferManagerTest, SingleCopySnapshot_LIVEONLY_)
  {
    const auto testName = GetTestNameLowerCase();
    auto blobServiceClient = GetClientForTest(testName);
    auto containerClient = blobServiceClient.GetBlobContainerClient(GetContainerValidName());
    containerClient.CreateIfNotExists();

    auto blobClient = containerClient.GetBlockBlobClient("SingleCopySnapshotSrc");
    blobClient.UploadFrom(reinterpret_cast<const uint8_t*>("a"), 1);
    auto snapshot = blobClient.CreateSnapshot().Value.Snapshot;
    blobClient.UploadFrom(reinterpret_cast<const uint8_t*>("b"), 1);

    auto snapshotClient = blobClient.WithSnapshot(snapshot);

    auto destBlobClient = containerClient.GetBlockBlobClient("SingleCopySnapshotDest");
    Blobs::BlobTransferManager m;
    auto job = m.ScheduleCopy(snapshotClient, destBlobClient);

    EXPECT_FALSE(job.Id.empty());
    EXPECT_EQ(job.SourceUrl, snapshotClient.GetUrl());
    EXPECT_EQ(job.DestinationUrl, destBlobClient.GetUrl());
    EXPECT_EQ(job.Type, TransferType::SingleCopy);

    auto jobStatus = job.WaitHandle.get();
    EXPECT_EQ(jobStatus, JobStatus::Succeeded);

    EXPECT_EQ(
        std::vector<uint8_t>(1, uint8_t('a')),
        ReadBodyStream(destBlobClient.Download().Value.BodyStream));
    containerClient.DeleteIfExists();
  }

  TEST_F(BlobTransferManagerTest, BigBlobSingleCopy_LIVEONLY_)
  {
    const auto testName = GetTestNameLowerCase();
    auto blobServiceClient
        = Blobs::BlobServiceClient::CreateFromConnectionString(StandardStorageConnectionString());
    const auto containerName = GetContainerValidName();
    auto srcContainerClient = blobServiceClient.GetBlobContainerClient(containerName);
    srcContainerClient.CreateIfNotExists();

    constexpr size_t blobSize = 2_GB;

    const auto srcBlobName = "BigBlobSingleCopySrc";
    auto srcBlobClient = srcContainerClient.GetBlockBlobClient(srcBlobName);

    std::string srcUrl;
    {
      Sas::BlobSasBuilder b;
      b.ExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);
      b.BlobContainerName = containerName;
      b.BlobName = srcBlobName;
      b.Resource = Sas::BlobSasResource::Blob;
      b.SetPermissions(Sas::BlobSasPermissions::Read);
      auto keyCredential
          = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;
      auto sasToken = b.GenerateSasToken(*keyCredential);
      srcUrl = _internal::ApplySasToken(srcBlobClient.GetUrl(), sasToken);
    }
    bool needUpload = true;
    try
    {
      int64_t currSize = srcBlobClient.GetProperties().Value.BlobSize;
      needUpload = currSize < blobSize;
    }
    catch (std::exception&)
    {
    }

    if (needUpload)
    {
      auto buffer = RandomBuffer(blobSize);
      Blobs::UploadBlockBlobFromOptions options;
      options.TransferOptions.Concurrency = 32;
      srcBlobClient.UploadFrom(buffer.data(), buffer.size(), options);
    }

    auto destContainerClient = Blobs::BlobContainerClient::CreateFromConnectionString(
        AdlsGen2ConnectionString(), containerName);
    destContainerClient.CreateIfNotExists();
    auto destBlobClient = destContainerClient.GetBlockBlobClient("BigBlobSingleCopyDest");

    Blobs::BlobTransferManager m;
    auto job = m.ScheduleCopy(Blobs::BlobClient(srcUrl), destBlobClient);

    auto jobStatus = job.WaitHandle.get();
    EXPECT_EQ(jobStatus, JobStatus::Succeeded);

    destContainerClient.DeleteIfExists();
    srcContainerClient.DeleteIfExists();
  }

}}} // namespace Azure::Storage::Test
