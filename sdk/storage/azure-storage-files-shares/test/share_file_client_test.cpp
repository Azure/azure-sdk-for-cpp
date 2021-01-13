// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "share_file_client_test.hpp"

#include <algorithm>
#include <chrono>
#include <future>

#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/file_io.hpp>
#include <azure/storage/common/storage_common.hpp>

namespace Azure { namespace Storage { namespace Test {

  std::shared_ptr<Files::Shares::ShareFileClient> FileShareFileClientTest::m_fileClient;
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
    m_fileShareDirectoryClient = std::make_shared<Files::Shares::ShareDirectoryClient>(
        m_shareClient->GetDirectoryClient(m_directoryName));
    m_fileShareDirectoryClient->Create();
    m_fileClient = std::make_shared<Files::Shares::ShareFileClient>(
        m_fileShareDirectoryClient->GetFileClient(m_fileName));
    m_fileClient->Create(1024);
  }

  void FileShareFileClientTest::TearDownTestSuite()
  {
    Files::Shares::DeleteShareOptions options;
    options.IncludeSnapshots = true;
    m_shareClient->Delete(options);
  }

  TEST_F(FileShareFileClientTest, CreateDeleteFiles)
  {
    {
      // Normal create/delete.
      std::vector<Files::Shares::ShareFileClient> fileClients;
      for (int32_t i = 0; i < 5; ++i)
      {
        auto fileName = RandomString(10);
        Files::Shares::ShareFileClient client = m_fileShareDirectoryClient->GetFileClient(fileName);
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
        Files::Shares::ShareFileClient client = m_fileShareDirectoryClient->GetFileClient(fileName);
        EXPECT_NO_THROW(client.Create(1024));
        EXPECT_NO_THROW(client.Create(1024));
      }
    }
    {
      // DeleteIfExists.
      {
        auto client
            = m_shareClient->GetRootDirectoryClient().GetFileClient(LowercaseRandomString());
        EXPECT_NO_THROW(client.Create(1024));
        EXPECT_NO_THROW(client.Delete());
        EXPECT_NO_THROW(client.DeleteIfExists());
      }
      {
        auto client
            = m_shareClient->GetRootDirectoryClient().GetFileClient(LowercaseRandomString());
        auto deleteResult = client.DeleteIfExists();
        EXPECT_FALSE(deleteResult->Deleted);
      }
      {
        auto shareClient = Files::Shares::ShareClient::CreateFromConnectionString(
            StandardStorageConnectionString(), LowercaseRandomString());
        auto client
            = m_shareClient->GetRootDirectoryClient().GetFileClient(LowercaseRandomString());
        auto deleteResult = client.DeleteIfExists();
        EXPECT_FALSE(deleteResult->Deleted);
      }
      {
        auto client = m_shareClient->GetRootDirectoryClient()
                          .GetSubdirectoryClient(LowercaseRandomString())
                          .GetFileClient(LowercaseRandomString());
        auto deleteResult = client.DeleteIfExists();
        EXPECT_FALSE(deleteResult->Deleted);
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
      Files::Shares::CreateShareFileOptions options1;
      Files::Shares::CreateShareFileOptions options2;
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
      Files::Shares::CreateShareFileOptions options1;
      Files::Shares::CreateShareFileOptions options2;
      options1.Permission = permission;
      options2.Permission = permission;

      EXPECT_NO_THROW(client1.Create(1024, options1));
      EXPECT_NO_THROW(client2.Create(1024, options2));
      auto result1 = client1.GetProperties()->FilePermissionKey;
      auto result2 = client2.GetProperties()->FilePermissionKey;
      EXPECT_EQ(result1, result2);

      auto client3 = m_fileShareDirectoryClient->GetFileClient(LowercaseRandomString());
      Files::Shares::CreateShareFileOptions options3;
      options3.SmbProperties.PermissionKey = result1;
      EXPECT_NO_THROW(client3.Create(1024, options3));
      auto result3 = client3.GetProperties()->FilePermissionKey;
      EXPECT_EQ(result1, result3);
    }

    {
      // Set permission with SetProperties works
      Files::Shares::Models::FileShareSmbProperties properties;
      properties.Attributes = Files::Shares::Models::FileAttributes::System
          | Files::Shares::Models::FileAttributes::NotContentIndexed;
      properties.CreatedOn = std::chrono::system_clock::now();
      properties.LastWrittenOn = std::chrono::system_clock::now();
      properties.PermissionKey = "";
      auto client1 = m_fileShareDirectoryClient->GetFileClient(LowercaseRandomString());
      auto client2 = m_fileShareDirectoryClient->GetFileClient(LowercaseRandomString());

      EXPECT_NO_THROW(client1.Create(1024));
      EXPECT_NO_THROW(client2.Create(1024));
      Files::Shares::SetShareFilePropertiesOptions options1;
      Files::Shares::SetShareFilePropertiesOptions options2;
      options1.Permission = permission;
      options2.Permission = permission;
      EXPECT_NO_THROW(client1.SetProperties(GetInterestingHttpHeaders(), properties, options1));
      EXPECT_NO_THROW(client2.SetProperties(GetInterestingHttpHeaders(), properties, options2));
      auto result1 = client1.GetProperties()->FilePermissionKey;
      auto result2 = client1.GetProperties()->FilePermissionKey;
      EXPECT_EQ(result1, result2);

      auto client3 = m_fileShareDirectoryClient->GetFileClient(LowercaseRandomString());
      Files::Shares::CreateShareFileOptions options3;
      options3.SmbProperties.PermissionKey = result1;
      std::string permissionKey;
      EXPECT_NO_THROW(permissionKey = client3.Create(1024, options3)->FilePermissionKey);
      auto result3 = client3.GetProperties()->FilePermissionKey;
      EXPECT_EQ(permissionKey, result3);
    }
  }

  TEST_F(FileShareFileClientTest, FileSmbProperties)
  {
    Files::Shares::Models::FileShareSmbProperties properties;
    properties.Attributes = Files::Shares::Models::FileAttributes::System
        | Files::Shares::Models::FileAttributes::NotContentIndexed;
    properties.CreatedOn = std::chrono::system_clock::now();
    properties.LastWrittenOn = std::chrono::system_clock::now();
    properties.PermissionKey = m_fileClient->GetProperties()->FilePermissionKey;
    {
      // Create directory with SmbProperties works
      auto client1 = m_fileShareDirectoryClient->GetFileClient(LowercaseRandomString());
      auto client2 = m_fileShareDirectoryClient->GetFileClient(LowercaseRandomString());
      Files::Shares::CreateShareFileOptions options1;
      Files::Shares::CreateShareFileOptions options2;
      options1.SmbProperties = properties;
      options2.SmbProperties = properties;

      EXPECT_NO_THROW(client1.Create(1024, options1));
      EXPECT_NO_THROW(client2.Create(1024, options2));
      auto directoryProperties1 = client1.GetProperties();
      auto directoryProperties2 = client2.GetProperties();
      EXPECT_EQ(directoryProperties2->FileCreatedOn, directoryProperties1->FileCreatedOn);
      EXPECT_EQ(directoryProperties2->FileLastWrittenOn, directoryProperties1->FileLastWrittenOn);
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
      EXPECT_EQ(directoryProperties2->FileCreatedOn, directoryProperties1->FileCreatedOn);
      EXPECT_EQ(directoryProperties2->FileLastWrittenOn, directoryProperties1->FileLastWrittenOn);
      EXPECT_EQ(directoryProperties2->FileAttributes, directoryProperties1->FileAttributes);
    }
  }

  TEST_F(FileShareFileClientTest, HandlesFunctionalityWorks)
  {
    auto result = m_fileClient->ListHandlesSinglePage();
    EXPECT_TRUE(result->Handles.empty());
    EXPECT_TRUE(result->ContinuationToken.empty());
    EXPECT_NO_THROW(m_fileClient->ForceCloseAllHandles());
  }

  TEST_F(FileShareFileClientTest, LeaseRelated)
  {
    std::string leaseId1 = CreateUniqueLeaseId();
    auto lastModified = m_fileClient->GetProperties()->LastModified;
    auto aLease = *m_fileClient->AcquireLease(leaseId1);
    EXPECT_FALSE(aLease.ETag.empty());
    EXPECT_TRUE(aLease.LastModified >= lastModified);
    EXPECT_EQ(aLease.LeaseId, leaseId1);
    lastModified = m_fileClient->GetProperties()->LastModified;
    aLease = *m_fileClient->AcquireLease(leaseId1);
    EXPECT_FALSE(aLease.ETag.empty());
    EXPECT_TRUE(aLease.LastModified >= lastModified);
    EXPECT_EQ(aLease.LeaseId, leaseId1);

    auto properties = *m_fileClient->GetProperties();
    EXPECT_EQ(properties.LeaseState.GetValue(), Files::Shares::Models::LeaseStateType::Leased);
    EXPECT_EQ(properties.LeaseStatus.GetValue(), Files::Shares::Models::LeaseStatusType::Locked);

    std::string leaseId2 = CreateUniqueLeaseId();
    EXPECT_NE(leaseId1, leaseId2);
    lastModified = m_fileClient->GetProperties()->LastModified;
    auto cLease = *m_fileClient->ChangeLease(leaseId1, leaseId2);
    EXPECT_FALSE(cLease.ETag.empty());
    EXPECT_TRUE(cLease.LastModified >= lastModified);
    EXPECT_EQ(cLease.LeaseId, leaseId2);

    lastModified = m_fileClient->GetProperties()->LastModified;
    auto fileInfo = *m_fileClient->ReleaseLease(leaseId2);
    EXPECT_FALSE(fileInfo.ETag.empty());
    EXPECT_TRUE(fileInfo.LastModified >= lastModified);

    aLease = *m_fileClient->AcquireLease(CreateUniqueLeaseId());
    lastModified = m_fileClient->GetProperties()->LastModified;
    auto brokenLease = *m_fileClient->BreakLease();
    EXPECT_FALSE(brokenLease.ETag.empty());
    EXPECT_TRUE(brokenLease.LastModified >= lastModified);

    aLease = *m_fileClient->AcquireLease(CreateUniqueLeaseId());
    lastModified = m_fileClient->GetProperties()->LastModified;
    brokenLease = *m_fileClient->BreakLease();
    EXPECT_FALSE(brokenLease.ETag.empty());
    EXPECT_TRUE(brokenLease.LastModified >= lastModified);
    m_fileClient->BreakLease();
  }

  TEST_F(FileShareFileClientTest, ConcurrentUpload)
  {
    std::vector<uint8_t> fileContent = RandomBuffer(static_cast<std::size_t>(8_MB));

    auto testUploadFromBuffer = [&](int concurrency, int64_t fileSize) {
      auto fileClient = m_fileShareDirectoryClient->GetFileClient(RandomString());

      Files::Shares::UploadShareFileFromOptions options;
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

      Files::Shares::UploadShareFileFromOptions options;
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
      Files::Shares::DownloadShareFileToOptions options;
      options.Concurrency = concurrency;
      if (offset.HasValue())
      {
        options.Range = Core::Http::Range();
        options.Range.GetValue().Offset = offset.GetValue();
        options.Range.GetValue().Length = length;
      }

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
            StorageException);
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
      Files::Shares::DownloadShareFileToOptions options;
      options.Concurrency = concurrency;
      if (offset.HasValue())
      {
        options.Range = Core::Http::Range();
        options.Range.GetValue().Offset = offset.GetValue();
        options.Range.GetValue().Length = length;
      }
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
        EXPECT_THROW(m_fileClient->DownloadTo(tempFilename, options), StorageException);
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
      Files::Shares::DownloadShareFileToOptions options;
      options.Concurrency = c;
      options.Range = Core::Http::Range();
      options.Range.GetValue().Offset = 1;
      for (int64_t length : {1ULL, 2ULL, 4_KB, 5_KB, 8_KB, 11_KB, 20_KB})
      {
        std::vector<uint8_t> downloadBuffer;
        downloadBuffer.resize(static_cast<std::size_t>(length - 1));
        options.Range.GetValue().Length = length;
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

  TEST_F(FileShareFileClientTest, RangeUploadDownload)
  {
    auto rangeSize = 1 * 1024 * 1024;
    auto numOfChunks = 3;
    auto rangeContent = RandomBuffer(rangeSize);
    auto memBodyStream = Core::Http::MemoryBodyStream(rangeContent);
    {
      // Simple upload/download.
      auto fileClient = m_shareClient->GetFileClient(LowercaseRandomString(10));
      fileClient.Create(static_cast<int64_t>(numOfChunks) * rangeSize);
      for (int32_t i = 0; i < numOfChunks; ++i)
      {
        memBodyStream.Rewind();
        EXPECT_NO_THROW(
            fileClient.UploadRange(static_cast<int64_t>(rangeSize) * i, &memBodyStream));
      }

      for (int32_t i = 0; i < numOfChunks; ++i)
      {
        std::vector<uint8_t> resultBuffer;
        Files::Shares::DownloadShareFileOptions downloadOptions;
        downloadOptions.Range = Core::Http::Range();
        downloadOptions.Range.GetValue().Offset = static_cast<int64_t>(rangeSize) * i;
        downloadOptions.Range.GetValue().Length = rangeSize;
        EXPECT_NO_THROW(
            resultBuffer = Core::Http::BodyStream::ReadToEnd(
                Core::Context(), *fileClient.Download(downloadOptions)->BodyStream));
        EXPECT_EQ(rangeContent, resultBuffer);
      }
    }

    {
      // MD5 works.
      memBodyStream.Rewind();
      auto md5 = Md5::Hash(rangeContent.data(), rangeContent.size());
      auto invalidMd5 = Md5::Hash(std::string("This is garbage."));
      auto fileClient = m_shareClient->GetFileClient(LowercaseRandomString(10));
      Files::Shares::UploadShareFileRangeOptions uploadOptions;
      fileClient.Create(static_cast<int64_t>(numOfChunks) * rangeSize);
      ContentHash hash;
      hash.Value = md5;
      hash.Algorithm = HashAlgorithm::Md5;
      uploadOptions.TransactionalContentHash = hash;
      EXPECT_NO_THROW(fileClient.UploadRange(0, &memBodyStream, uploadOptions));
      hash.Value = invalidMd5;
      uploadOptions.TransactionalContentHash = hash;
      memBodyStream.Rewind();
      EXPECT_THROW(fileClient.UploadRange(0, &memBodyStream, uploadOptions), StorageException);
    }
  }

  TEST_F(FileShareFileClientTest, CopyRelated)
  {
    size_t fileSize = 1 * 1024 * 1024;
    auto fileContent = RandomBuffer(fileSize);
    auto memBodyStream = Core::Http::MemoryBodyStream(fileContent);
    {
      // Simple copy works.
      auto fileClient = m_shareClient->GetFileClient(LowercaseRandomString(10));
      fileClient.Create(fileSize);

      auto destFileClient = m_shareClient->GetFileClient(LowercaseRandomString(10));
      Files::Shares::Models::StartCopyShareFileResult result;
      EXPECT_NO_THROW(result = destFileClient.StartCopy(fileClient.GetUri()).ExtractValue());
      EXPECT_EQ(Files::Shares::Models::CopyStatusType::Success, result.CopyStatus);
      EXPECT_FALSE(result.CopyId.empty());
    }

    {
      // Copy mode with override and empty permission throws error..
      auto fileClient = m_shareClient->GetFileClient(LowercaseRandomString(10));
      fileClient.Create(fileSize);

      auto destFileClient = m_shareClient->GetFileClient(LowercaseRandomString(10));
      Files::Shares::StartCopyShareFileOptions copyOptions;
      copyOptions.PermissionCopyMode = Files::Shares::Models::PermissionCopyModeType::Override;
      EXPECT_THROW(destFileClient.StartCopy(fileClient.GetUri(), copyOptions), std::runtime_error);
    }
  }

  TEST_F(FileShareFileClientTest, RangeRelated)
  {
    size_t fileSize = 1 * 1024 * 1024;
    auto fileContent = RandomBuffer(fileSize);
    auto memBodyStream = Core::Http::MemoryBodyStream(fileContent);
    auto halfContent
        = std::vector<uint8_t>(fileContent.begin(), fileContent.begin() + fileSize / 2);
    halfContent.resize(fileSize);
    auto fileClient = m_shareClient->GetFileClient(LowercaseRandomString(10));
    fileClient.Create(fileSize);
    EXPECT_NO_THROW(fileClient.UploadRange(0, &memBodyStream));
    EXPECT_NO_THROW(fileClient.ClearRange(fileSize / 2, fileSize / 2));
    std::vector<uint8_t> downloadContent(static_cast<std::size_t>(fileSize), '\x00');
    EXPECT_NO_THROW(
        fileClient.DownloadTo(downloadContent.data(), static_cast<std::size_t>(fileSize)));
    EXPECT_EQ(halfContent, downloadContent);

    EXPECT_NO_THROW(fileClient.ClearRange(512, 512));
    Files::Shares::Models::GetShareFileRangeListResult result;
    EXPECT_NO_THROW(result = fileClient.GetRangeList().ExtractValue());
    EXPECT_EQ(2U, result.Ranges.size());
    EXPECT_EQ(0, result.Ranges[0].Offset);
    EXPECT_TRUE(result.Ranges[0].Length.HasValue());
    EXPECT_EQ(512, result.Ranges[0].Length.GetValue());
    EXPECT_EQ(1024, result.Ranges[1].Offset);
    EXPECT_TRUE(result.Ranges[1].Length.HasValue());
    EXPECT_EQ(static_cast<int32_t>(fileSize / 2) - 1024, result.Ranges[1].Length.GetValue());
  }

  TEST_F(FileShareFileClientTest, PreviousRangeWithSnapshot)
  {
    size_t fileSize = 1 * 1024 * 1024;
    auto fileContent = RandomBuffer(fileSize);
    auto memBodyStream = Core::Http::MemoryBodyStream(fileContent);
    auto halfContent
        = std::vector<uint8_t>(fileContent.begin(), fileContent.begin() + fileSize / 2);
    halfContent.resize(fileSize);
    auto fileClient = m_shareClient->GetFileClient(LowercaseRandomString(10));
    fileClient.Create(fileSize);
    EXPECT_NO_THROW(fileClient.UploadRange(0, &memBodyStream));
    EXPECT_NO_THROW(fileClient.ClearRange(fileSize / 2, fileSize / 2));
    std::vector<uint8_t> downloadContent(static_cast<std::size_t>(fileSize), '\x00');
    EXPECT_NO_THROW(
        fileClient.DownloadTo(downloadContent.data(), static_cast<std::size_t>(fileSize)));
    EXPECT_EQ(halfContent, downloadContent);

    auto snapshot1 = m_shareClient->CreateSnapshot()->Snapshot;
    EXPECT_NO_THROW(fileClient.ClearRange(500, 2048));
    auto snapshot2 = m_shareClient->CreateSnapshot()->Snapshot;
    Files::Shares::Models::GetShareFileRangeListResult result;
    Files::Shares::GetShareFileRangeListOptions options;
    options.PrevShareSnapshot = snapshot1;
    EXPECT_NO_THROW(result = fileClient.GetRangeList(options).ExtractValue());
    EXPECT_EQ(2U, result.Ranges.size());
    EXPECT_EQ(0, result.Ranges[0].Offset);
    EXPECT_TRUE(result.Ranges[0].Length.HasValue());
    EXPECT_EQ(512, result.Ranges[0].Length.GetValue());
    EXPECT_EQ(2048, result.Ranges[1].Offset);
    EXPECT_TRUE(result.Ranges[1].Length.HasValue());
    EXPECT_EQ(512, result.Ranges[1].Length.GetValue());
    EXPECT_NO_THROW(fileClient.ClearRange(3096, 2048));
    auto snapshot3 = m_shareClient->CreateSnapshot()->Snapshot;
    options.PrevShareSnapshot = snapshot1;
    EXPECT_NO_THROW(result = fileClient.GetRangeList(options).ExtractValue());
    EXPECT_EQ(4U, result.Ranges.size());
    EXPECT_EQ(0, result.Ranges[0].Offset);
    EXPECT_TRUE(result.Ranges[0].Length.HasValue());
    EXPECT_EQ(512, result.Ranges[0].Length.GetValue());
    EXPECT_EQ(2048, result.Ranges[1].Offset);
    EXPECT_TRUE(result.Ranges[1].Length.HasValue());
    EXPECT_EQ(512, result.Ranges[1].Length.GetValue());
    EXPECT_EQ(3072, result.Ranges[2].Offset);
    EXPECT_TRUE(result.Ranges[2].Length.HasValue());
    EXPECT_EQ(512, result.Ranges[2].Length.GetValue());
    EXPECT_EQ(5120, result.Ranges[3].Offset);
    EXPECT_TRUE(result.Ranges[3].Length.HasValue());
    EXPECT_EQ(512, result.Ranges[3].Length.GetValue());

    EXPECT_EQ(2U, result.ClearRanges.size());
    EXPECT_EQ(512, result.ClearRanges[0].Offset);
    EXPECT_TRUE(result.ClearRanges[0].Length.HasValue());
    EXPECT_EQ(1536, result.ClearRanges[0].Length.GetValue());
    EXPECT_EQ(3584, result.ClearRanges[1].Offset);
    EXPECT_TRUE(result.ClearRanges[1].Length.HasValue());
    EXPECT_EQ(1536, result.ClearRanges[1].Length.GetValue());
  }

}}} // namespace Azure::Storage::Test
