// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "share_file_client_test.hpp"

#include "common/file_io.hpp"
#include "common/storage_common.hpp"

#include <algorithm>
#include <future>

namespace Azure { namespace Storage { namespace Test {

  std::shared_ptr<Files::Shares::FileClient> FileShareFileClientTest::m_fileClient;
  std::string FileShareFileClientTest::m_fileName;
  std::vector<uint8_t> FileShareFileClientTest::m_fileContent;

  void FileShareFileClientTest::SetUpTestSuite()
  {
    m_directoryName = LowercaseRandomString();
    m_shareName = LowercaseRandomString();
    m_fileName = LowercaseRandomString();
    m_shareClient = std::make_shared<Files::Shares::ShareClient>(
        Files::Shares::ShareClient::CreateFromConnectionString(
            StandardStorageConnectionString(), m_shareName));
    m_shareClient->Create();
    m_fileShareDirectoryClient = std::make_shared<Files::Shares::DirectoryClient>(
        m_shareClient->GetDirectoryClient(m_directoryName));
    m_fileShareDirectoryClient->Create();
    m_fileClient = std::make_shared<Files::Shares::FileClient>(
        m_fileShareDirectoryClient->GetFileClient(m_fileName));
    m_fileClient->Create(1024);
  }

  void FileShareFileClientTest::TearDownTestSuite() { m_shareClient->Delete(); }

  TEST_F(FileShareFileClientTest, CreateDeleteFiles)
  {
    {
      // Normal create/delete.
      std::vector<Files::Shares::FileClient> fileClients;
      for (int32_t i = 0; i < 5; ++i)
      {
        auto fileName = RandomString(10);
        Files::Shares::FileClient client = m_fileShareDirectoryClient->GetFileClient(fileName);
        EXPECT_NO_THROW(client.Create(1024));
        fileClients.emplace_back(std::move(client));
      }
      for (const auto& client : fileClients)
      {
        EXPECT_NO_THROW(client.Delete());
      }
    }
    {
      // Create file that already exist overwrites.
      for (int32_t i = 0; i < 5; ++i)
      {
        auto fileName = RandomString(10);
        Files::Shares::FileClient client = m_fileShareDirectoryClient->GetFileClient(fileName);
        EXPECT_NO_THROW(client.Create(1024));
        EXPECT_NO_THROW(client.Create(1024));
      }
    }
  }

  TEST_F(FileShareFileClientTest, FileMetadata)
  {
    auto metadata1 = RandomMetadata();
    auto metadata2 = RandomMetadata();
    {
      // Set/Get Metadata works
      EXPECT_NO_THROW(m_fileClient->SetMetadata(metadata1));
      auto result = m_fileClient->GetProperties()->Metadata;
      EXPECT_EQ(metadata1, result);
      EXPECT_NO_THROW(m_fileClient->SetMetadata(metadata2));
      result = m_fileClient->GetProperties()->Metadata;
      EXPECT_EQ(metadata2, result);
    }

    {
      // Create directory with metadata works
      auto client1 = m_fileShareDirectoryClient->GetFileClient(LowercaseRandomString());
      auto client2 = m_fileShareDirectoryClient->GetFileClient(LowercaseRandomString());
      Files::Shares::CreateFileOptions options1;
      Files::Shares::CreateFileOptions options2;
      options1.Metadata = metadata1;
      options2.Metadata = metadata2;

      EXPECT_NO_THROW(client1.Create(1024, options1));
      EXPECT_NO_THROW(client2.Create(1024, options2));
      auto result = client1.GetProperties()->Metadata;
      EXPECT_EQ(metadata1, result);
      result = client2.GetProperties()->Metadata;
      EXPECT_EQ(metadata2, result);
    }
  }

  TEST_F(FileShareFileClientTest, FilePermission)
  {
    std::string permission = "O:S-1-5-21-2127521184-1604012920-1887927527-21560751G:S-1-5-21-"
                             "2127521184-1604012920-1887927527-513D:AI(A;;FA;;;SY)(A;;FA;;;BA)(A;;"
                             "0x1200a9;;;S-1-5-21-397955417-626881126-188441444-3053964)";

    {
      // Create directory with permission/permission key works
      auto client1 = m_fileShareDirectoryClient->GetFileClient(LowercaseRandomString());
      auto client2 = m_fileShareDirectoryClient->GetFileClient(LowercaseRandomString());
      Files::Shares::CreateFileOptions options1;
      Files::Shares::CreateFileOptions options2;
      options1.FilePermission = permission;
      options2.FilePermission = permission;

      EXPECT_NO_THROW(client1.Create(1024, options1));
      EXPECT_NO_THROW(client2.Create(1024, options2));
      auto result1 = client1.GetProperties()->FilePermissionKey;
      auto result2 = client2.GetProperties()->FilePermissionKey;
      EXPECT_EQ(result1, result2);

      auto client3 = m_fileShareDirectoryClient->GetFileClient(LowercaseRandomString());
      Files::Shares::CreateFileOptions options3;
      options3.SmbProperties.FilePermissionKey = result1;
      EXPECT_NO_THROW(client3.Create(1024, options3));
      auto result3 = client3.GetProperties()->FilePermissionKey;
      EXPECT_EQ(result1, result3);
    }

    {
      // Set permission with SetProperties works
      Files::Shares::FileShareSmbProperties properties;
      properties.Attributes = Files::Shares::FileAttributes::System
          | Files::Shares::FileAttributes::NotContentIndexed;
      properties.FileCreationTime = ToIso8601(std::chrono::system_clock::now(), 7);
      properties.FileLastWriteTime = ToIso8601(std::chrono::system_clock::now(), 7);
      properties.FilePermissionKey = "";
      auto client1 = m_fileShareDirectoryClient->GetFileClient(LowercaseRandomString());
      auto client2 = m_fileShareDirectoryClient->GetFileClient(LowercaseRandomString());

      EXPECT_NO_THROW(client1.Create(1024));
      EXPECT_NO_THROW(client2.Create(1024));
      Files::Shares::SetFilePropertiesOptions options1;
      Files::Shares::SetFilePropertiesOptions options2;
      options1.FilePermission = permission;
      options2.FilePermission = permission;
      EXPECT_NO_THROW(client1.SetProperties(GetInterestingHttpHeaders(), properties, options1));
      EXPECT_NO_THROW(client2.SetProperties(GetInterestingHttpHeaders(), properties, options2));
      auto result1 = client1.GetProperties()->FilePermissionKey;
      auto result2 = client1.GetProperties()->FilePermissionKey;
      EXPECT_EQ(result1, result2);

      auto client3 = m_fileShareDirectoryClient->GetFileClient(LowercaseRandomString());
      Files::Shares::CreateFileOptions options3;
      options3.SmbProperties.FilePermissionKey = result1;
      std::string permissionKey;
      EXPECT_NO_THROW(permissionKey = client3.Create(1024, options3)->FilePermissionKey);
      auto result3 = client3.GetProperties()->FilePermissionKey;
      EXPECT_EQ(permissionKey, result3);
    }
  }

  TEST_F(FileShareFileClientTest, DirectorySmbProperties)
  {
    Files::Shares::FileShareSmbProperties properties;
    properties.Attributes
        = Files::Shares::FileAttributes::System | Files::Shares::FileAttributes::NotContentIndexed;
    properties.FileCreationTime = ToIso8601(std::chrono::system_clock::now(), 7);
    properties.FileLastWriteTime = ToIso8601(std::chrono::system_clock::now(), 7);
    properties.FilePermissionKey = m_fileClient->GetProperties()->FilePermissionKey;
    {
      // Create directory with SmbProperties works
      auto client1 = m_fileShareDirectoryClient->GetFileClient(LowercaseRandomString());
      auto client2 = m_fileShareDirectoryClient->GetFileClient(LowercaseRandomString());
      Files::Shares::CreateFileOptions options1;
      Files::Shares::CreateFileOptions options2;
      options1.SmbProperties = properties;
      options2.SmbProperties = properties;

      EXPECT_NO_THROW(client1.Create(1024, options1));
      EXPECT_NO_THROW(client2.Create(1024, options2));
      auto directoryProperties1 = client1.GetProperties();
      auto directoryProperties2 = client2.GetProperties();
      EXPECT_EQ(directoryProperties2->FileCreationTime, directoryProperties1->FileCreationTime);
      EXPECT_EQ(directoryProperties2->FileLastWriteTime, directoryProperties1->FileLastWriteTime);
      EXPECT_EQ(directoryProperties2->FileAttributes, directoryProperties1->FileAttributes);
    }

    {
      // SetProperties works
      auto client1 = m_fileShareDirectoryClient->GetFileClient(LowercaseRandomString());
      auto client2 = m_fileShareDirectoryClient->GetFileClient(LowercaseRandomString());

      EXPECT_NO_THROW(client1.Create(1024));
      EXPECT_NO_THROW(client2.Create(1024));
      EXPECT_NO_THROW(client1.SetProperties(GetInterestingHttpHeaders(), properties));
      EXPECT_NO_THROW(client2.SetProperties(GetInterestingHttpHeaders(), properties));
      auto directoryProperties1 = client1.GetProperties();
      auto directoryProperties2 = client2.GetProperties();
      EXPECT_EQ(directoryProperties2->FileCreationTime, directoryProperties1->FileCreationTime);
      EXPECT_EQ(directoryProperties2->FileLastWriteTime, directoryProperties1->FileLastWriteTime);
      EXPECT_EQ(directoryProperties2->FileAttributes, directoryProperties1->FileAttributes);
    }
  }

  TEST_F(FileShareFileClientTest, HandlesFunctionalityWorks)
  {
    auto result = m_fileShareDirectoryClient->ListHandlesSegmented();
    EXPECT_TRUE(result->HandleList.empty());
    EXPECT_TRUE(result->NextMarker.empty());
  }

  TEST_F(FileShareFileClientTest, LeaseRelated)
  {
    std::string leaseId1 = CreateUniqueLeaseId();
    auto aLease = *m_fileClient->AcquireLease(leaseId1);
    EXPECT_FALSE(aLease.ETag.empty());
    EXPECT_FALSE(aLease.LastModified.empty());
    EXPECT_EQ(aLease.LeaseId, leaseId1);
    aLease = *m_fileClient->AcquireLease(leaseId1);
    EXPECT_FALSE(aLease.ETag.empty());
    EXPECT_FALSE(aLease.LastModified.empty());
    EXPECT_EQ(aLease.LeaseId, leaseId1);

    auto properties = *m_fileClient->GetProperties();
    EXPECT_EQ(properties.LeaseState.GetValue(), Files::Shares::LeaseStateType::Leased);
    EXPECT_EQ(properties.LeaseStatus.GetValue(), Files::Shares::LeaseStatusType::Locked);

    std::string leaseId2 = CreateUniqueLeaseId();
    EXPECT_NE(leaseId1, leaseId2);
    auto cLease = *m_fileClient->ChangeLease(leaseId1, leaseId2);
    EXPECT_FALSE(cLease.ETag.empty());
    EXPECT_FALSE(cLease.LastModified.empty());
    EXPECT_EQ(cLease.LeaseId, leaseId2);

    auto fileInfo = *m_fileClient->ReleaseLease(leaseId2);
    EXPECT_FALSE(fileInfo.ETag.empty());
    EXPECT_FALSE(fileInfo.LastModified.empty());

    aLease = *m_fileClient->AcquireLease(CreateUniqueLeaseId());
    properties = *m_fileClient->GetProperties();
    auto brokenLease = *m_fileClient->BreakLease();
    EXPECT_FALSE(brokenLease.ETag.empty());
    EXPECT_FALSE(brokenLease.LastModified.empty());

    aLease = *m_fileClient->AcquireLease(CreateUniqueLeaseId());
    brokenLease = *m_fileClient->BreakLease();
    EXPECT_FALSE(brokenLease.ETag.empty());
    EXPECT_FALSE(brokenLease.LastModified.empty());
    m_fileClient->BreakLease();
  }

  TEST_F(FileShareFileClientTest, ConcurrentUpload)
  {
    std::vector<uint8_t> fileContent = RandomBuffer(static_cast<std::size_t>(8_MB));

    auto testUploadFromBuffer = [&](int concurrency, int64_t fileSize) {
      auto fileClient = m_fileShareDirectoryClient->GetFileClient(RandomString());

      Files::Shares::UploadFileFromOptions options;
      options.ChunkSize = 512_KB;
      options.Concurrency = concurrency;
      options.HttpHeaders = GetInterestingHttpHeaders();
      options.Metadata = RandomMetadata();

      auto res
          = fileClient.UploadFrom(fileContent.data(), static_cast<std::size_t>(fileSize), options);

      auto properties = *fileClient.GetProperties();
      EXPECT_EQ(properties.ContentLength, fileSize);
      EXPECT_EQ(properties.Metadata, options.Metadata);
      std::vector<uint8_t> downloadContent(static_cast<std::size_t>(fileSize), '\x00');
      fileClient.DownloadTo(downloadContent.data(), static_cast<std::size_t>(fileSize));
      EXPECT_EQ(
          downloadContent,
          std::vector<uint8_t>(
              fileContent.begin(), fileContent.begin() + static_cast<std::size_t>(fileSize)));
    };

    auto testUploadFromFile = [&](int concurrency, int64_t fileSize) {
      auto fileClient = m_fileShareDirectoryClient->GetFileClient(RandomString());

      Files::Shares::UploadFileFromOptions options;
      options.ChunkSize = 512_KB;
      options.Concurrency = concurrency;
      options.HttpHeaders = GetInterestingHttpHeaders();
      options.Metadata = RandomMetadata();

      std::string tempFilename = RandomString();
      {
        Azure::Storage::Details::FileWriter fileWriter(tempFilename);
        fileWriter.Write(fileContent.data(), fileSize, 0);
      }

      auto res = fileClient.UploadFrom(tempFilename, options);

      auto properties = *fileClient.GetProperties();
      EXPECT_EQ(properties.ContentLength, fileSize);
      EXPECT_EQ(properties.Metadata, options.Metadata);
      std::vector<uint8_t> downloadContent(static_cast<std::size_t>(fileSize), '\x00');
      fileClient.DownloadTo(downloadContent.data(), static_cast<std::size_t>(fileSize));
      EXPECT_EQ(
          downloadContent,
          std::vector<uint8_t>(
              fileContent.begin(), fileContent.begin() + static_cast<std::size_t>(fileSize)));

      DeleteFile(tempFilename);
    };

    std::vector<std::future<void>> futures;
    for (int c : {1, 2, 5})
    {
      for (int64_t l : {0ULL, 512ULL, 1_KB, 4_KB, 1_MB, 4_MB + 512})
      {
        ASSERT_GE(fileContent.size(), static_cast<std::size_t>(l));
        futures.emplace_back(std::async(std::launch::async, testUploadFromBuffer, c, l));
        futures.emplace_back(std::async(std::launch::async, testUploadFromFile, c, l));
      }
    }
    for (auto& f : futures)
    {
      f.get();
    }
  }

  TEST_F(FileShareFileClientTest, ConcurrentDownload)
  {
    m_fileContent = RandomBuffer(8 * 1024 * 1024);
    m_fileClient->UploadFrom(m_fileContent.data(), 8 * 1024 * 1024);
    auto testDownloadToBuffer = [](int concurrency,
                                   int64_t downloadSize,
                                   Azure::Core::Nullable<int64_t> offset = {},
                                   Azure::Core::Nullable<int64_t> length = {},
                                   Azure::Core::Nullable<int64_t> initialChunkSize = {},
                                   Azure::Core::Nullable<int64_t> chunkSize = {}) {
      std::vector<uint8_t> downloadBuffer;
      std::vector<uint8_t> expectedData = m_fileContent;
      int64_t fileSize = m_fileContent.size();
      int64_t actualDownloadSize = std::min(downloadSize, fileSize);
      if (offset.HasValue() && length.HasValue())
      {
        actualDownloadSize = std::min(length.GetValue(), fileSize - offset.GetValue());
        if (actualDownloadSize >= 0)
        {
          expectedData.assign(
              m_fileContent.begin() + static_cast<std::ptrdiff_t>(offset.GetValue()),
              m_fileContent.begin()
                  + static_cast<std::ptrdiff_t>(offset.GetValue() + actualDownloadSize));
        }
        else
        {
          expectedData.clear();
        }
      }
      else if (offset.HasValue())
      {
        actualDownloadSize = fileSize - offset.GetValue();
        if (actualDownloadSize >= 0)
        {
          expectedData.assign(
              m_fileContent.begin() + static_cast<std::ptrdiff_t>(offset.GetValue()),
              m_fileContent.end());
        }
        else
        {
          expectedData.clear();
        }
      }
      downloadBuffer.resize(static_cast<std::size_t>(downloadSize), '\x00');
      Files::Shares::DownloadFileToOptions options;
      options.Concurrency = concurrency;
      options.Offset = offset;
      options.Length = length;
      options.InitialChunkSize = initialChunkSize;
      options.ChunkSize = chunkSize;
      if (actualDownloadSize > 0)
      {
        auto res = m_fileClient->DownloadTo(downloadBuffer.data(), downloadBuffer.size(), options);
        EXPECT_EQ(res->ContentLength, actualDownloadSize);
        downloadBuffer.resize(static_cast<std::size_t>(res->ContentLength));
        EXPECT_EQ(downloadBuffer, expectedData);
      }
      else
      {
        EXPECT_THROW(
            m_fileClient->DownloadTo(downloadBuffer.data(), downloadBuffer.size(), options),
            StorageError);
      }
    };
    auto testDownloadToFile = [](int concurrency,
                                 int64_t downloadSize,
                                 Azure::Core::Nullable<int64_t> offset = {},
                                 Azure::Core::Nullable<int64_t> length = {},
                                 Azure::Core::Nullable<int64_t> initialChunkSize = {},
                                 Azure::Core::Nullable<int64_t> chunkSize = {}) {
      std::string tempFilename = RandomString();
      std::vector<uint8_t> expectedData = m_fileContent;
      int64_t fileSize = m_fileContent.size();
      int64_t actualDownloadSize = std::min(downloadSize, fileSize);
      if (offset.HasValue() && length.HasValue())
      {
        actualDownloadSize = std::min(length.GetValue(), fileSize - offset.GetValue());
        if (actualDownloadSize >= 0)
        {
          expectedData.assign(
              m_fileContent.begin() + static_cast<std::ptrdiff_t>(offset.GetValue()),
              m_fileContent.begin()
                  + static_cast<std::ptrdiff_t>(offset.GetValue() + actualDownloadSize));
        }
        else
        {
          expectedData.clear();
        }
      }
      else if (offset.HasValue())
      {
        actualDownloadSize = fileSize - offset.GetValue();
        if (actualDownloadSize >= 0)
        {
          expectedData.assign(
              m_fileContent.begin() + static_cast<std::ptrdiff_t>(offset.GetValue()),
              m_fileContent.end());
        }
        else
        {
          expectedData.clear();
        }
      }
      Files::Shares::DownloadFileToOptions options;
      options.Concurrency = concurrency;
      options.Offset = offset;
      options.Length = length;
      options.InitialChunkSize = initialChunkSize;
      options.ChunkSize = chunkSize;
      if (actualDownloadSize > 0)
      {
        auto res = m_fileClient->DownloadTo(tempFilename, options);
        EXPECT_EQ(res->ContentLength, actualDownloadSize);
        EXPECT_EQ(ReadFile(tempFilename), expectedData);
      }
      else
      {
        EXPECT_THROW(m_fileClient->DownloadTo(tempFilename, options), StorageError);
      }
      DeleteFile(tempFilename);
    };

    const int64_t fileSize = m_fileContent.size();
    std::vector<std::future<void>> futures;
    for (int c : {1, 2, 4})
    {
      // download whole file
      futures.emplace_back(std::async(std::launch::async, testDownloadToBuffer, c, fileSize));
      futures.emplace_back(std::async(std::launch::async, testDownloadToFile, c, fileSize));
      futures.emplace_back(std::async(std::launch::async, testDownloadToBuffer, c, fileSize, 0));
      futures.emplace_back(std::async(std::launch::async, testDownloadToFile, c, fileSize, 0));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToBuffer, c, fileSize, 0, fileSize));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToFile, c, fileSize, 0, fileSize));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToBuffer, c, fileSize, 0, fileSize * 2));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToFile, c, fileSize, 0, fileSize * 2));
      futures.emplace_back(std::async(std::launch::async, testDownloadToBuffer, c, fileSize * 2));
      futures.emplace_back(std::async(std::launch::async, testDownloadToFile, c, fileSize * 2));

      // random range
      std::mt19937_64 random_generator(std::random_device{}());
      for (int i = 0; i < 16; ++i)
      {
        std::uniform_int_distribution<int64_t> offsetDistribution(0, m_fileContent.size() - 1);
        int64_t offset = offsetDistribution(random_generator);
        std::uniform_int_distribution<int64_t> lengthDistribution(1, 64_KB);
        int64_t length = lengthDistribution(random_generator);
        futures.emplace_back(std::async(
            std::launch::async, testDownloadToBuffer, c, fileSize, offset, length, 4_KB, 4_KB));
        futures.emplace_back(std::async(
            std::launch::async, testDownloadToFile, c, fileSize, offset, length, 4_KB, 4_KB));
      }

      futures.emplace_back(std::async(std::launch::async, testDownloadToBuffer, c, fileSize, 0, 1));
      futures.emplace_back(std::async(std::launch::async, testDownloadToFile, c, fileSize, 0, 1));
      futures.emplace_back(std::async(std::launch::async, testDownloadToBuffer, c, fileSize, 1, 1));
      futures.emplace_back(std::async(std::launch::async, testDownloadToFile, c, fileSize, 1, 1));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToBuffer, c, fileSize, fileSize - 1, 1));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToFile, c, fileSize, fileSize - 1, 1));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToBuffer, c, fileSize, fileSize - 1, 2));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToFile, c, fileSize, fileSize - 1, 2));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToBuffer, c, fileSize, fileSize, 1));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToFile, c, fileSize, fileSize, 1));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToBuffer, c, fileSize, fileSize + 1, 2));
      futures.emplace_back(
          std::async(std::launch::async, testDownloadToFile, c, fileSize, fileSize + 1, 2));

      // buffer not big enough
      Files::Shares::DownloadFileToOptions options;
      options.Concurrency = c;
      options.Offset = 1;
      for (int64_t length : {1ULL, 2ULL, 4_KB, 5_KB, 8_KB, 11_KB, 20_KB})
      {
        std::vector<uint8_t> downloadBuffer;
        downloadBuffer.resize(static_cast<std::size_t>(length - 1));
        options.Length = length;
        EXPECT_THROW(
            m_fileClient->DownloadTo(
                downloadBuffer.data(), static_cast<std::size_t>(length - 1), options),
            std::runtime_error);
      }
    }
    for (auto& f : futures)
    {
      f.get();
    }
  }

}}} // namespace Azure::Storage::Test
