// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blob_transfer_manager_test.hpp"

#include <vector>

#include <azure/storage/blobs.hpp>
#include <azure/storage/common/internal/file_io.hpp>
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

  TEST_F(BlobTransferManagerTest, SingleDownloadPauseResume_LIVEONLY_)
  {
    const auto testName = GetTestNameLowerCase();
    auto blobServiceClient = GetClientForTest(testName);
    const auto containerName = GetContainerValidName();
    auto containerClient = blobServiceClient.GetBlobContainerClient(containerName);
    containerClient.CreateIfNotExists();
    auto blobClient = containerClient.GetBlobClient(testName);

    ResumeJobOptions resumeOptions;
    {
      auto parsedConnectionString
          = _internal::ParseConnectionString(StandardStorageConnectionString());
      resumeOptions.SourceCredential.SharedKeyCredential = parsedConnectionString.KeyCredential;
    }

    const std::string tempFilename = "localfile" + testName;
    const std::string backupFilename = tempFilename + ".bk";
    constexpr size_t fileSize = static_cast<size_t>(256_MB);

    if (_internal::PathExists(tempFilename))
    {
      _internal::Remove(tempFilename);
    }
    {
      int64_t serviceFileSize = -1;
      try
      {
        serviceFileSize = blobClient.GetProperties().Value.BlobSize;
      }
      catch (StorageException&)
      {
      }
      if (!(serviceFileSize == fileSize && _internal::IsRegularFile(backupFilename)
            && _internal::GetFileSize(backupFilename) == fileSize))
      {
        WriteFile(backupFilename, RandomBuffer(fileSize));
        Blobs::UploadBlockBlobFromOptions options;
        options.TransferOptions.Concurrency = 32;
        options.TransferOptions.SingleUploadThreshold = 0;
        blobClient.AsBlockBlobClient().UploadFrom(backupFilename, options);
      }
    }

    StorageTransferManagerOptions options;
    options.NumThreads = 2;
    std::unique_ptr<Blobs::BlobTransferManager> m
        = std::make_unique<Blobs::BlobTransferManager>(options);
    auto job = m->ScheduleDownload(blobClient, tempFilename);

    bool atLeasePausedOnce = false;
    bool atLeaseDestructedOnce = false;
    for (int i = 0; i < 10; ++i)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(10) * (2 << i));
      if (i % 2 == 0)
      {
        m.reset();
        m = std::make_unique<Blobs::BlobTransferManager>(options);
        atLeaseDestructedOnce = true;
      }
      else
      {
        try
        {
          m->PauseJob(job.Id);
          atLeasePausedOnce = true;
        }
        catch (std::exception&)
        {
          break;
        }
      }
      auto status = job.WaitHandle.get();
      EXPECT_TRUE(status == JobStatus::Succeeded || status == JobStatus::Paused);
      if (status == JobStatus::Succeeded)
      {
        break;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      job = m->ResumeJob(job.Id, resumeOptions);
    }
    EXPECT_TRUE(atLeasePausedOnce);
    EXPECT_TRUE(atLeaseDestructedOnce);

    auto jobStatus = job.WaitHandle.get();
    EXPECT_EQ(jobStatus, JobStatus::Succeeded);
    EXPECT_THROW(m->PauseJob(job.Id), std::exception);

    EXPECT_EQ(ReadFile(tempFilename), ReadFile(backupFilename));
    DeleteFile(tempFilename);
    DeleteFile(backupFilename);
    containerClient.DeleteIfExists();
  }

  TEST_F(BlobTransferManagerTest, DirectoryDownload_LIVEONLY_)
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
    _internal::CreateDirectory("dir_l1");
    WriteFile("dir_l1/file1", RandomBuffer(static_cast<size_t>(5_MB)));
    WriteFile("dir_l1/file2", RandomBuffer(static_cast<size_t>(213_KB)));
    WriteFile("dir_l1/file3", RandomBuffer(0));
    _internal::CreateDirectory("dir_l1/dir_l2");
    WriteFile("dir_l1/dir_l2/file4", RandomBuffer(123));
    _internal::CreateDirectory("dir_l1/dir_l2/dir_l3");
    _internal::CreateDirectory("dir_l1/dir_l2/dir_l3/dir_l4");
    _internal::CreateDirectory("dir_l1/dir_l2/dir_l3/dir_l4/dir_l5");
    _internal::CreateDirectory("dir_l1/dir_l2_2");
    _internal::CreateDirectory("dir_l1/dir_l2_2/dir_l3_2");
    WriteFile("dir_l1/dir_l2_2/dir_l3_2/file4", RandomBuffer(static_cast<size_t>(10_MB)));
    files.push_back("file1");
    files.push_back("file2");
    files.push_back("file3");
    files.push_back("dir_l2/file4");
    files.push_back("dir_l2_2/dir_l3_2/file4");

    Blobs::BlobTransferManager m;
    auto job = m.ScheduleUploadDirectory(localDir, blobFolder);
    auto jobStatus = job.WaitHandle.get();
    ASSERT_EQ(jobStatus, JobStatus::Succeeded);

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
    _internal::Remove(localDir);
    _internal::Remove(destDir);
    containerClient.DeleteIfExists();
  }

  TEST_F(BlobTransferManagerTest, SinglePageBlobDownload_LIVEONLY_)
  {
    const auto testName = GetTestNameLowerCase();
    auto blobServiceClient = GetClientForTest(testName);
    auto containerClient = blobServiceClient.GetBlobContainerClient(GetContainerValidName());
    containerClient.CreateIfNotExists();
    auto blobClient = containerClient.GetPageBlobClient(testName);

    const std::string tempFilename = "localfile" + testName;
    const std::string tempFilename2 = "localfile" + testName + "2";

    struct TestOptions
    {
      int64_t blobSize;
      std::vector<int64_t> ranges;
    };

    std::vector<TestOptions> testOptions;
    testOptions.push_back(TestOptions{0, {}});
    testOptions.push_back(TestOptions{512, {}});
    testOptions.push_back(TestOptions{24_MB + 1024, {}});
    testOptions.push_back(TestOptions{512, {0, 512}});
    testOptions.push_back(TestOptions{1536, {512, 512}});
    testOptions.push_back(TestOptions{4096, {0, 512, 512, 512, 1536, 512, 3072, 512}});
    testOptions.push_back(TestOptions{24_MB, {512, 3_MB, 15_MB, 4_MB, 19_MB, 4_MB, 23_MB, 1_MB}});
    testOptions.push_back(
        TestOptions{24_MB, {1_MB, 4_MB, 5_MB, 4_MB, 9_MB, 4_MB, 13_MB, 4_MB, 17_MB, 4_MB}});

    for (const auto& testOption : testOptions)
    {
      blobClient.Create(testOption.blobSize);
      auto writer = std::make_unique<Azure::Storage::_internal::FileWriter>(tempFilename2);
      if (testOption.blobSize > 0)
      {
        writer->Write(reinterpret_cast<const uint8_t*>("\0"), 1, testOption.blobSize - 1);
      }
      for (size_t i = 0; i < testOption.ranges.size(); i += 2)
      {
        int64_t offset = testOption.ranges[i];
        int64_t length = testOption.ranges[i + 1];
        auto content = RandomBuffer(static_cast<size_t>(length));
        writer->Write(content.data(), content.size(), offset);
        auto stream = Azure::Core::IO::MemoryBodyStream(content.data(), content.size());
        blobClient.UploadPages(offset, stream);
      }
      writer.reset();

      Blobs::BlobTransferManager m;
      auto job = m.ScheduleDownload(blobClient, tempFilename);
      EXPECT_FALSE(job.Id.empty());
      EXPECT_EQ(job.SourceUrl, blobClient.GetUrl());
      EXPECT_FALSE(job.DestinationUrl.empty());
      EXPECT_EQ(job.Type, TransferType::SingleDownload);

      auto jobStatus = job.WaitHandle.get();
      EXPECT_EQ(jobStatus, JobStatus::Succeeded);

      auto b1 = ReadFile(tempFilename);
      auto b2 = ReadFile(tempFilename2);
      ASSERT_EQ(b1.size(), b2.size());
      for (size_t i = 0; i < b1.size(); ++i)
      {
        if (b1[i] != b2[i])
        {
          std::cout << i << std::endl;
        }
        ASSERT_EQ(b1[i], b2[i]);
      }
      EXPECT_EQ(ReadFile(tempFilename), ReadFile(tempFilename2));
      DeleteFile(tempFilename);
      DeleteFile(tempFilename2);
    }

    containerClient.DeleteIfExists();
  }

}}} // namespace Azure::Storage::Test
