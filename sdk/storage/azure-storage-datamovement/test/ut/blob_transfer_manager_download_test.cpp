// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blob_transfer_manager_test.hpp"

#include <vector>

#include <azure/storage/blobs.hpp>
#include <azure/storage/datamovement/blob_transfer_manager.hpp>
#include <azure/storage/datamovement/directory_iterator.hpp>

namespace Azure { namespace Storage { namespace Test {

  TEST_F(BlobTransferManagerTest, SingleDownload_LIVEONLY_)
  {
    const auto testName = GetTestNameLowerCase();
    auto blobServiceClient = GetClientForTest(testName);
    auto containerClient = blobServiceClient.GetBlobContainerClient(GetContainerValidName());
    containerClient.CreateIfNotExists();
    auto blobClient = containerClient.GetBlobClient(testName);

    const std::string tempFilename = "localfile" + testName;
    for (auto fileSize : std::vector<uint64_t>{0, 123, 456_KB, 2_MB, 4_MB, 8_MB, 25_MB + 1234})
    {
      auto content = RandomBuffer(static_cast<size_t>(fileSize));
      blobClient.AsBlockBlobClient().UploadFrom(content.data(), content.size());

      Blobs::BlobTransferManager m;
      auto job = m.ScheduleDownload(blobClient, tempFilename);
      EXPECT_FALSE(job.Id.empty());
      EXPECT_EQ(job.SourceUrl, blobClient.GetUrl());
      EXPECT_FALSE(job.DestinationUrl.empty());
      EXPECT_EQ(job.Type, TransferType::SingleDownload);

      auto jobStatus = job.WaitHandle.get();
      EXPECT_EQ(jobStatus, JobStatus::Succeeded);

      EXPECT_EQ(ReadFile(tempFilename), content);
      DeleteFile(tempFilename);
    }
    containerClient.DeleteIfExists();
  }

}}} // namespace Azure::Storage::Test
