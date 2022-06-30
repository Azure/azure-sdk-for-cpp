// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blob_transfer_manager_test.hpp"

#include <vector>

#include <azure/storage/blobs.hpp>
#include <azure/storage/datamovement/blob_folder.hpp>
#include <azure/storage/datamovement/blob_transfer_manager.hpp>
#include <azure/storage/datamovement/filesystem.hpp>

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

  TEST_F(BlobTransferManagerTest, DirectoryUploadDownload_LIVEONLY_)
  {
    const auto testName = GetTestNameLowerCase();
    auto blobServiceClient = GetClientForTest(testName);
    const auto containerName = GetContainerValidName();
    auto containerClient = blobServiceClient.GetBlobContainerClient(containerName);
    containerClient.CreateIfNotExists();
    const std::string localDir = "dir_l1";
    const std::string serviceDir = "folder1";
    auto blobFolder = Blobs::BlobFolder(containerClient, serviceDir);

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

    const std::string destDir = "dir_dest";

    auto downloadJob = m.ScheduleDownloadDirectory(blobFolder, destDir);
    EXPECT_FALSE(downloadJob.Id.empty());
    EXPECT_EQ(downloadJob.SourceUrl, blobFolder.GetUrl());
    EXPECT_FALSE(downloadJob.DestinationUrl.empty());
    EXPECT_EQ(downloadJob.Type, TransferType::DirectoryDownload);

    auto downloadJobStatus = downloadJob.WaitHandle.get();
    EXPECT_EQ(downloadJobStatus, JobStatus::Succeeded);

    for (const auto& f : files)
    {
      EXPECT_EQ(ReadFile(localDir + "/" + f), ReadFile(destDir + "/" + f));
    }

    std::vector<std::string> destFiles;
    {
      std::queue<std::string> dirQueue;
      dirQueue.push(destDir);

      while (!dirQueue.empty())
      {
        const std::string currentDir = dirQueue.front();
        _internal::DirectoryIterator currentIter(currentDir);
        dirQueue.pop();

        auto entry = currentIter.Next();
        while (!entry.Name.empty())
        {
          if (!entry.IsDirectory)
          {
            std::string fileName = currentDir + "/" + entry.Name;
            destFiles.emplace_back(fileName.substr(destDir.length() + 1));
          }
          else
          {
            dirQueue.push(currentDir + "/" + entry.Name);
          }
          entry = currentIter.Next();
        }
      }
    }
    std::sort(files.begin(), files.end());
    std::sort(destFiles.begin(), destFiles.end());
    EXPECT_EQ(files, destFiles);
    DeleteDir(localDir);
    DeleteDir(destDir);
    containerClient.DeleteIfExists();
  }

}}} // namespace Azure::Storage::Test
