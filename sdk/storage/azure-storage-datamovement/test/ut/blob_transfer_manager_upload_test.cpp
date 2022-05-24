// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blob_transfer_manager_test.hpp"

#include <algorithm>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include <azure/storage/datamovement/blob_transfer_manager.hpp>
#include <azure/storage/datamovement/directory_iterator.hpp>

namespace Azure { namespace Storage { namespace Test {

  TEST_F(BlobTransferManagerTest, SingleUpload_LIVEONLY_)
  {
    const auto testName = GetTestNameLowerCase();
    auto blobServiceClient = GetClientForTest(testName);
    auto containerClient = blobServiceClient.GetBlobContainerClient(GetContainerValidName());
    containerClient.CreateIfNotExists();
    auto blobClient = containerClient.GetBlobClient(testName);

    const std::string tempFilename = "localfile" + testName;
    for (auto fileSize : std::vector<uint64_t>{0, 123, 456_KB, 2_MB, 4_MB, 8_MB, 25_MB + 1234})
    {
      WriteFile(tempFilename, RandomBuffer(static_cast<size_t>(fileSize)));

      Blobs::BlobTransferManager m;
      auto job = m.ScheduleUpload(tempFilename, blobClient);
      EXPECT_FALSE(job.Id.empty());
      EXPECT_FALSE(job.SourceUrl.empty());
      EXPECT_EQ(job.DestinationUrl, blobClient.GetUrl());
      EXPECT_EQ(job.Type, TransferType::SingleUpload);

      auto jobStatus = job.WaitHandle.get();
      EXPECT_EQ(jobStatus, JobStatus::Succeeded);

      EXPECT_EQ(ReadFile(tempFilename), ReadBodyStream(blobClient.Download().Value.BodyStream));
      DeleteFile(tempFilename);
    }
    containerClient.DeleteIfExists();
  }

  TEST_F(BlobTransferManagerTest, SingleUploadPauseResume_LIVEONLY_)
  {
    const auto testName = GetTestNameLowerCase();
    auto blobServiceClient = GetClientForTest(testName);
    auto containerClient = blobServiceClient.GetBlobContainerClient(GetContainerValidName());
    containerClient.CreateIfNotExists();
    auto blobClient = containerClient.GetBlobClient(testName);

    const std::string tempFilename = "localfile" + testName;
    constexpr size_t fileSize = static_cast<size_t>(256_MB);
    WriteFile(tempFilename, RandomBuffer(fileSize));

    StorageTransferManagerOptions options;
    options.NumThreads = 2;
    Blobs::BlobTransferManager m(options);
    auto job = m.ScheduleUpload(tempFilename, blobClient);

    for (int i = 0; i < 6; ++i)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      try
      {
        m.PauseJob(job.Id);
      }
      catch (std::exception&)
      {
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      m.ResumeJob(job.Id);
    }

    auto jobStatus = job.WaitHandle.get();
    EXPECT_EQ(jobStatus, JobStatus::Succeeded);

    const std::string tempDownloadFile = "localfiledownloadtemp" + testName;
    {
      Blobs::DownloadBlobToOptions downloadOptions;
      downloadOptions.TransferOptions.InitialChunkSize = 0;
      blobClient.DownloadTo(tempDownloadFile, downloadOptions);
    }

    EXPECT_EQ(ReadFile(tempFilename), ReadFile(tempDownloadFile));
    DeleteFile(tempFilename);
    DeleteFile(tempDownloadFile);
    containerClient.DeleteIfExists();
  }

  TEST_F(BlobTransferManagerTest, DirectoryUpload_LIVEONLY_)
  {
    const auto testName = GetTestNameLowerCase();
    auto blobServiceClient = GetClientForTest(testName);
    const auto containerName = GetContainerValidName();
    auto containerClient = blobServiceClient.GetBlobContainerClient(containerName);
    containerClient.CreateIfNotExists();
    const std::string localDir = "dir_l1";
    const std::string serviceDir = "folder1";
    auto blobFolder = Blobs::BlobFolder::CreateFromConnectionString(
        StandardStorageConnectionString(), containerName, serviceDir);

    std::vector<std::string> files;
    CreateDir("dir_l1");
    WriteFile("dir_l1/file1", RandomBuffer(static_cast<size_t>(5_MB)));
    WriteFile("dir_l1/file2", RandomBuffer(static_cast<size_t>(213_KB)));
    WriteFile("dir_l1/file3", RandomBuffer(0));
    CreateDir("dir_l1/dir_l2");
    WriteFile("dir_l1/dir_l2/file4", RandomBuffer(123));
    CreateDir("dir_l1/dir_l2/dir_l3");
    CreateDir("dir_l1/dir_l2/dir_l3/dir_l4");
    CreateDir("dir_l1/dir_l2/dir_l3/dir_l4/dir_l5");
    CreateDir("dir_l1/dir_l2_2");
    CreateDir("dir_l1/dir_l2_2/dir_l3_2");
    WriteFile("dir_l1/dir_l2_2/dir_l3_2/file4", RandomBuffer(static_cast<size_t>(10_MB)));
    files.push_back("file1");
    files.push_back("file2");
    files.push_back("file3");
    files.push_back("dir_l2/file4");
    files.push_back("dir_l2_2/dir_l3_2/file4");

    Blobs::BlobTransferManager m;
    auto job = m.ScheduleUploadDirectory(localDir, blobFolder);
    EXPECT_FALSE(job.Id.empty());
    EXPECT_FALSE(job.SourceUrl.empty());
    EXPECT_EQ(job.DestinationUrl, blobFolder.GetUrl());
    EXPECT_EQ(job.Type, TransferType::DirectoryUpload);

    auto jobStatus = job.WaitHandle.get();
    EXPECT_EQ(jobStatus, JobStatus::Succeeded);

    for (const auto& f : files)
    {
      EXPECT_EQ(
          ReadFile(localDir + "/" + f),
          ReadBodyStream(blobFolder.GetBlobClient(f).Download().Value.BodyStream));
    }

    std::vector<std::string> serviceFiles;
    {
      Blobs::ListBlobsOptions options;
      options.Prefix = serviceDir + "/";
      for (auto page = containerClient.ListBlobs(options); page.HasPage(); page.MoveToNextPage())
      {
        for (const auto& b : page.Blobs)
        {
          serviceFiles.push_back(b.Name.substr(serviceDir.length() + 1));
        }
      }
    }
    std::sort(files.begin(), files.end());
    std::sort(serviceFiles.begin(), serviceFiles.end());
    EXPECT_EQ(files, serviceFiles);
    DeleteDir(localDir);
    containerClient.DeleteIfExists();
  }

}}} // namespace Azure::Storage::Test
