// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "share_file_client_test.hpp"

#include <algorithm>
#include <chrono>
#include <future>
#include <random>

#include <azure/core/cryptography/hash.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/file_io.hpp>
#include <azure/storage/common/storage_common.hpp>

namespace Azure { namespace Storage { namespace Test {

  void FileShareFileClientTest::SetUp()
  {
    FileShareDirectoryClientTest::SetUp();
    if (shouldSkipTest())
    {
      return;
    }
    m_fileName = RandomString();
    m_fileClient = std::make_shared<Files::Shares::ShareFileClient>(
        m_fileShareDirectoryClient->GetFileClient(m_fileName));
    m_fileClient->Create(1024);
  }

  TEST_F(FileShareFileClientTest, CreateDeleteFiles)
  {
    {
      // Normal create/delete.
      auto fileName = RandomString();
      Files::Shares::ShareFileClient client = m_fileShareDirectoryClient->GetFileClient(fileName);
      EXPECT_NO_THROW(client.Create(1024));
      EXPECT_NO_THROW(client.Delete());
    }
    {
      // Create file that already exist overwrites.
      auto fileName = RandomString();
      Files::Shares::ShareFileClient client = m_fileShareDirectoryClient->GetFileClient(fileName);
      EXPECT_NO_THROW(client.Create(1024));
      EXPECT_NO_THROW(client.Create(1024));
    }
    {
      // DeleteIfExists.
      {
        auto client = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString() + "1");
        EXPECT_NO_THROW(client.Create(1024));
        EXPECT_NO_THROW(client.Delete());
        EXPECT_NO_THROW(client.DeleteIfExists());
      }
      {
        auto client = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString() + "2");
        auto deleteResult = client.DeleteIfExists();
        EXPECT_FALSE(deleteResult.Value.Deleted);
      }
      {
        auto shareClient = Files::Shares::ShareClient::CreateFromConnectionString(
            StandardStorageConnectionString(), LowercaseRandomString());
        auto client = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString() + "3");
        auto deleteResult = client.DeleteIfExists();
        EXPECT_FALSE(deleteResult.Value.Deleted);
      }
      {
        auto subdirClient
            = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(RandomString() + "4");
        auto client = subdirClient.GetFileClient(RandomString() + "5");
        auto deleteResult = client.DeleteIfExists();
        EXPECT_FALSE(deleteResult.Value.Deleted);
      }
    }
  }

  TEST_F(FileShareFileClientTest, DownloadEmptyFile)
  {
    auto fileClient = m_fileShareDirectoryClient->GetFileClient(RandomString());
    fileClient.Create(0);

    auto res = fileClient.Download();
    EXPECT_EQ(res.Value.BodyStream->Length(), 0);

    std::string tempFilename = RandomString() + "1";
    EXPECT_NO_THROW(fileClient.DownloadTo(tempFilename));
    EXPECT_TRUE(ReadFile(tempFilename).empty());
    DeleteFile(tempFilename);

    std::vector<uint8_t> buff;
    EXPECT_NO_THROW(fileClient.DownloadTo(buff.data(), 0));
  }

  TEST_F(FileShareFileClientTest, DownloadNonExistingToFile)
  {
    const auto tempFilename = RandomString();
    auto fileClient = m_fileShareDirectoryClient->GetFileClient(RandomString());

    EXPECT_THROW(fileClient.DownloadTo(tempFilename), StorageException);
    EXPECT_THROW(ReadFile(tempFilename), std::runtime_error);
    DeleteFile(tempFilename);
  }

  TEST_F(FileShareFileClientTest, FileMetadata)
  {
    auto metadata1 = RandomMetadata();
    auto metadata2 = RandomMetadata();
    {
      // Set/Get Metadata works
      EXPECT_NO_THROW(m_fileClient->SetMetadata(metadata1));
      auto result = m_fileClient->GetProperties().Value.Metadata;
      EXPECT_EQ(metadata1, result);
      EXPECT_NO_THROW(m_fileClient->SetMetadata(metadata2));
      result = m_fileClient->GetProperties().Value.Metadata;
      EXPECT_EQ(metadata2, result);
    }

    {
      // Create directory with metadata works
      auto client1 = m_fileShareDirectoryClient->GetFileClient(RandomString() + "1");
      auto client2 = m_fileShareDirectoryClient->GetFileClient(RandomString() + "2");
      Files::Shares::CreateFileOptions options1;
      Files::Shares::CreateFileOptions options2;
      options1.Metadata = metadata1;
      options2.Metadata = metadata2;

      EXPECT_NO_THROW(client1.Create(1024, options1));
      EXPECT_NO_THROW(client2.Create(1024, options2));
      auto result = client1.GetProperties().Value.Metadata;
      EXPECT_EQ(metadata1, result);
      result = client2.GetProperties().Value.Metadata;
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
      auto client1 = m_fileShareDirectoryClient->GetFileClient(RandomString() + "d1");
      auto client2 = m_fileShareDirectoryClient->GetFileClient(RandomString() + "d2");
      Files::Shares::CreateFileOptions options1;
      Files::Shares::CreateFileOptions options2;
      options1.Permission = permission;
      options2.Permission = permission;

      EXPECT_NO_THROW(client1.Create(1024, options1));
      EXPECT_NO_THROW(client2.Create(1024, options2));
      auto result1 = client1.GetProperties().Value.SmbProperties.PermissionKey;
      auto result2 = client2.GetProperties().Value.SmbProperties.PermissionKey;
      EXPECT_TRUE(result1.HasValue());
      EXPECT_TRUE(result2.HasValue());
      EXPECT_EQ(result1.Value(), result2.Value());

      auto client3 = m_fileShareDirectoryClient->GetFileClient(RandomString() + "d3");
      Files::Shares::CreateFileOptions options3;
      options3.SmbProperties.PermissionKey = result1;
      EXPECT_NO_THROW(client3.Create(1024, options3));
      auto result3 = client3.GetProperties().Value.SmbProperties.PermissionKey;
      EXPECT_TRUE(result3.HasValue());
      EXPECT_EQ(result1.Value(), result3.Value());
    }

    {
      // Set permission with SetProperties works
      Files::Shares::Models::FileHttpHeaders httpHeaders;
      httpHeaders.ContentType = "application/x-binary";
      httpHeaders.ContentLanguage = "en-US";
      httpHeaders.ContentDisposition = "attachment";
      httpHeaders.CacheControl = "no-cache";
      httpHeaders.ContentEncoding = "identity";

      Files::Shares::Models::FileSmbProperties properties;
      properties.Attributes = Files::Shares::Models::FileAttributes::System
          | Files::Shares::Models::FileAttributes::NotContentIndexed;
      properties.CreatedOn = std::chrono::system_clock::now();
      properties.LastWrittenOn = std::chrono::system_clock::now();
      properties.PermissionKey = "";
      auto client1 = m_fileShareDirectoryClient->GetFileClient(RandomString() + "a1");
      auto client2 = m_fileShareDirectoryClient->GetFileClient(RandomString() + "a2");

      EXPECT_NO_THROW(client1.Create(1024));
      EXPECT_NO_THROW(client2.Create(1024));
      Files::Shares::SetFilePropertiesOptions options1;
      Files::Shares::SetFilePropertiesOptions options2;
      options1.Permission = permission;
      options2.Permission = permission;
      EXPECT_NO_THROW(client1.SetProperties(httpHeaders, properties, options1));
      EXPECT_NO_THROW(client2.SetProperties(httpHeaders, properties, options2));
      auto result1 = client1.GetProperties().Value.SmbProperties.PermissionKey;
      auto result2 = client1.GetProperties().Value.SmbProperties.PermissionKey;
      EXPECT_TRUE(result1.HasValue());
      EXPECT_TRUE(result2.HasValue());
      EXPECT_EQ(result1.Value(), result2.Value());

      auto client3 = m_fileShareDirectoryClient->GetFileClient(RandomString() + "a3");
      Files::Shares::CreateFileOptions options3;
      options3.SmbProperties.PermissionKey = result1;
      std::string permissionKey;
      EXPECT_NO_THROW(
          permissionKey = client3.Create(1024, options3).Value.SmbProperties.PermissionKey.Value());
      auto result3 = client3.GetProperties().Value.SmbProperties.PermissionKey;
      EXPECT_TRUE(result3.HasValue());
      EXPECT_EQ(permissionKey, result3.Value());
    }
  }

  TEST_F(FileShareFileClientTest, FileSmbProperties)
  {
    Files::Shares::Models::FileSmbProperties properties;
    properties.Attributes = Files::Shares::Models::FileAttributes::System
        | Files::Shares::Models::FileAttributes::NotContentIndexed;
    properties.CreatedOn = std::chrono::system_clock::now();
    properties.LastWrittenOn = std::chrono::system_clock::now();
    properties.ChangedOn = std::chrono::system_clock::now();
    properties.PermissionKey = m_fileClient->GetProperties().Value.SmbProperties.PermissionKey;
    {
      // Create directory with SmbProperties works
      auto client1 = m_fileShareDirectoryClient->GetFileClient(RandomString() + "1");
      auto client2 = m_fileShareDirectoryClient->GetFileClient(RandomString() + "2");
      Files::Shares::CreateFileOptions options1;
      Files::Shares::CreateFileOptions options2;
      options1.SmbProperties = properties;
      options2.SmbProperties = properties;

      EXPECT_NO_THROW(client1.Create(1024, options1));
      EXPECT_NO_THROW(client2.Create(1024, options2));
      auto directoryProperties1 = client1.GetProperties();
      auto directoryProperties2 = client2.GetProperties();
      EXPECT_EQ(
          directoryProperties2.Value.SmbProperties.CreatedOn.Value(),
          directoryProperties1.Value.SmbProperties.CreatedOn.Value());
      EXPECT_EQ(
          directoryProperties2.Value.SmbProperties.LastWrittenOn.Value(),
          directoryProperties1.Value.SmbProperties.LastWrittenOn.Value());
      EXPECT_EQ(
          directoryProperties2.Value.SmbProperties.ChangedOn.Value(),
          directoryProperties1.Value.SmbProperties.ChangedOn.Value());
      EXPECT_EQ(
          directoryProperties2.Value.SmbProperties.Attributes,
          directoryProperties1.Value.SmbProperties.Attributes);
    }

    {
      // SetProperties works
      Files::Shares::Models::FileHttpHeaders httpHeaders;
      httpHeaders.ContentType = "application/x-binary";
      httpHeaders.ContentLanguage = "en-US";
      httpHeaders.ContentDisposition = "attachment";
      httpHeaders.CacheControl = "no-cache";
      httpHeaders.ContentEncoding = "identity";

      auto client1 = m_fileShareDirectoryClient->GetFileClient(RandomString() + "3");
      auto client2 = m_fileShareDirectoryClient->GetFileClient(RandomString() + "4");

      EXPECT_NO_THROW(client1.Create(1024));
      EXPECT_NO_THROW(client2.Create(1024));
      EXPECT_NO_THROW(client1.SetProperties(httpHeaders, properties));
      EXPECT_NO_THROW(client2.SetProperties(httpHeaders, properties));
      auto directoryProperties1 = client1.GetProperties();
      auto directoryProperties2 = client2.GetProperties();
      EXPECT_EQ(
          directoryProperties2.Value.SmbProperties.CreatedOn.Value(),
          directoryProperties1.Value.SmbProperties.CreatedOn.Value());
      EXPECT_EQ(
          directoryProperties2.Value.SmbProperties.LastWrittenOn.Value(),
          directoryProperties1.Value.SmbProperties.LastWrittenOn.Value());
      EXPECT_EQ(
          directoryProperties2.Value.SmbProperties.ChangedOn.Value(),
          directoryProperties1.Value.SmbProperties.ChangedOn.Value());
      EXPECT_EQ(
          directoryProperties2.Value.SmbProperties.Attributes,
          directoryProperties1.Value.SmbProperties.Attributes);
    }
  }

  TEST_F(FileShareFileClientTest, SmbPropertiesDefaultValue)
  {
    auto fileClient = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString());
    fileClient.Create(1024);
    auto smbProperties = fileClient.GetProperties().Value.SmbProperties;
    EXPECT_EQ(smbProperties.Attributes, Files::Shares::Models::FileAttributes::Archive);
    ASSERT_TRUE(smbProperties.CreatedOn.HasValue());
    EXPECT_TRUE(IsValidTime(smbProperties.CreatedOn.Value()));
    ASSERT_TRUE(smbProperties.LastWrittenOn.HasValue());
    EXPECT_TRUE(IsValidTime(smbProperties.LastWrittenOn.Value()));
    ASSERT_TRUE(smbProperties.ChangedOn.HasValue());
    EXPECT_TRUE(IsValidTime(smbProperties.ChangedOn.Value()));

    fileClient.SetProperties(
        Files::Shares::Models::FileHttpHeaders(), Files::Shares::Models::FileSmbProperties());

    auto smbProperties2 = fileClient.GetProperties().Value.SmbProperties;
    EXPECT_EQ(smbProperties2.PermissionKey.Value(), smbProperties.PermissionKey.Value());
    EXPECT_EQ(smbProperties2.Attributes, smbProperties.Attributes);
    EXPECT_EQ(smbProperties2.CreatedOn.Value(), smbProperties.CreatedOn.Value());
    EXPECT_EQ(smbProperties2.LastWrittenOn.Value(), smbProperties.LastWrittenOn.Value());
    EXPECT_NE(smbProperties2.ChangedOn.Value(), smbProperties.ChangedOn.Value());
  }

  TEST_F(FileShareFileClientTest, HandlesFunctionalityWorks)
  {
    auto result = m_fileClient->ListHandles();
    EXPECT_TRUE(result.FileHandles.empty());
    EXPECT_FALSE(result.NextPageToken.HasValue());

    for (auto pageResult = m_fileClient->ListHandles(); pageResult.HasPage();
         pageResult.MoveToNextPage())
    {
    }

    EXPECT_NO_THROW(m_fileClient->ForceCloseAllHandles());

    for (auto pageResult = m_fileClient->ForceCloseAllHandles(); pageResult.HasPage();
         pageResult.MoveToNextPage())
    {
    }
  }

  TEST_F(FileShareFileClientTest, LeaseRelated)
  {
    {
      std::string leaseId1 = RandomUUID();
      auto lastModified = m_fileClient->GetProperties().Value.LastModified;
      Files::Shares::ShareLeaseClient leaseClient(*m_fileClient, leaseId1);
      auto aLease
          = leaseClient.Acquire(Files::Shares::ShareLeaseClient::InfiniteLeaseDuration).Value;
      EXPECT_TRUE(aLease.ETag.HasValue());
      EXPECT_GE(aLease.LastModified, lastModified);
      EXPECT_EQ(aLease.LeaseId, leaseId1);
      lastModified = m_fileClient->GetProperties().Value.LastModified;
      aLease = leaseClient.Acquire(Files::Shares::ShareLeaseClient::InfiniteLeaseDuration).Value;
      EXPECT_TRUE(aLease.ETag.HasValue());
      EXPECT_GE(aLease.LastModified, lastModified);
      EXPECT_EQ(aLease.LeaseId, leaseId1);

      auto properties = m_fileClient->GetProperties().Value;
      EXPECT_EQ(properties.LeaseState.Value(), Files::Shares::Models::LeaseState::Leased);
      EXPECT_EQ(properties.LeaseStatus.Value(), Files::Shares::Models::LeaseStatus::Locked);

      std::string leaseId2 = RandomUUID();
      EXPECT_NE(leaseId1, leaseId2);
      lastModified = m_fileClient->GetProperties().Value.LastModified;
      auto cLease = leaseClient.Change(leaseId2).Value;
      EXPECT_TRUE(cLease.ETag.HasValue());
      EXPECT_GE(cLease.LastModified, lastModified);
      EXPECT_EQ(cLease.LeaseId, leaseId2);
      EXPECT_EQ(leaseClient.GetLeaseId(), leaseId2);

      lastModified = m_fileClient->GetProperties().Value.LastModified;
      auto fileInfo = leaseClient.Release().Value;
      EXPECT_TRUE(fileInfo.ETag.HasValue());
      EXPECT_GE(fileInfo.LastModified, lastModified);
    }

    {

      Files::Shares::ShareLeaseClient leaseClient(*m_fileClient, RandomUUID());
      auto aLease
          = leaseClient.Acquire(Files::Shares::ShareLeaseClient::InfiniteLeaseDuration).Value;
      auto lastModified = m_fileClient->GetProperties().Value.LastModified;
      auto brokenLease = leaseClient.Break().Value;
      EXPECT_TRUE(brokenLease.ETag.HasValue());
      EXPECT_GE(brokenLease.LastModified, lastModified);
    }
  }

  TEST_F(FileShareFileClientTest, ConcurrentUpload_LIVEONLY_)
  {
    const auto blobContent = RandomBuffer(static_cast<size_t>(8_MB));

    auto testUploadFromBuffer = [&](int concurrency,
                                    int64_t bufferSize,
                                    Azure::Nullable<int64_t> singleUploadThreshold = {},
                                    Azure::Nullable<int64_t> chunkSize = {}) {
      Files::Shares::UploadFileFromOptions options;
      options.TransferOptions.Concurrency = concurrency;
      if (singleUploadThreshold.HasValue())
      {
        options.TransferOptions.SingleUploadThreshold = singleUploadThreshold.Value();
      }
      if (chunkSize.HasValue())
      {
        options.TransferOptions.ChunkSize = chunkSize.Value();
      }

      auto fileClient = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString());
      EXPECT_NO_THROW(
          fileClient.UploadFrom(blobContent.data(), static_cast<size_t>(bufferSize), options));
      std::vector<uint8_t> downloadBuffer(static_cast<size_t>(bufferSize), '\x00');
      fileClient.DownloadTo(downloadBuffer.data(), downloadBuffer.size());
      std::vector<uint8_t> expectedData(
          blobContent.begin(), blobContent.begin() + static_cast<size_t>(bufferSize));
      EXPECT_EQ(downloadBuffer, expectedData);
    };

    auto testUploadFromFile = [&](int concurrency,
                                  int64_t fileSize,
                                  Azure::Nullable<int64_t> singleUploadThreshold = {},
                                  Azure::Nullable<int64_t> chunkSize = {}) {
      Files::Shares::UploadFileFromOptions options;
      options.TransferOptions.Concurrency = concurrency;
      if (singleUploadThreshold.HasValue())
      {
        options.TransferOptions.SingleUploadThreshold = singleUploadThreshold.Value();
      }
      if (chunkSize.HasValue())
      {
        options.TransferOptions.ChunkSize = chunkSize.Value();
      }

      const std::string tempFileName = RandomString();
      WriteFile(
          tempFileName,
          std::vector<uint8_t>(
              blobContent.begin(), blobContent.begin() + static_cast<size_t>(fileSize)));
      auto fileClient = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString());
      EXPECT_NO_THROW(fileClient.UploadFrom(tempFileName, options));
      DeleteFile(tempFileName);
      std::vector<uint8_t> downloadBuffer(static_cast<size_t>(fileSize), '\x00');
      fileClient.DownloadTo(downloadBuffer.data(), downloadBuffer.size());
      std::vector<uint8_t> expectedData(
          blobContent.begin(), blobContent.begin() + static_cast<size_t>(fileSize));
      EXPECT_EQ(downloadBuffer, expectedData);
    };

    for (int c : {1, 2, 4})
    {
      for (int i = 0; i < 16; ++i)
      {
        // random range
        int64_t fileSize = RandomInt(1, 1_MB);
        testUploadFromBuffer(c, fileSize, 4_KB, 40_KB);
        testUploadFromFile(c, fileSize, 2_KB, 162_KB);
        testUploadFromBuffer(c, fileSize, 0, 127_KB);
        testUploadFromFile(c, fileSize, 0, 253_KB);
      }
    }
  }

  TEST_F(FileShareFileClientTest, ConcurrentDownload_LIVEONLY_)
  {
    auto fileContent = RandomBuffer(8 * 1024 * 1024);
    m_fileClient->UploadFrom(fileContent.data(), 8 * 1024 * 1024);
    auto testDownloadToBuffer = [&](int concurrency,
                                    int64_t downloadSize,
                                    Azure::Nullable<int64_t> offset = {},
                                    Azure::Nullable<int64_t> length = {},
                                    Azure::Nullable<int64_t> initialChunkSize = {},
                                    Azure::Nullable<int64_t> chunkSize = {}) {
      std::vector<uint8_t> downloadBuffer;
      std::vector<uint8_t> expectedData = fileContent;
      int64_t fileSize = fileContent.size();
      int64_t actualDownloadSize = std::min(downloadSize, fileSize);
      if (offset.HasValue() && length.HasValue())
      {
        actualDownloadSize = std::min(length.Value(), fileSize - offset.Value());
        if (actualDownloadSize >= 0)
        {
          expectedData.assign(
              fileContent.begin() + static_cast<ptrdiff_t>(offset.Value()),
              fileContent.begin() + static_cast<ptrdiff_t>(offset.Value() + actualDownloadSize));
        }
        else
        {
          expectedData.clear();
        }
      }
      else if (offset.HasValue())
      {
        actualDownloadSize = fileSize - offset.Value();
        if (actualDownloadSize >= 0)
        {
          expectedData.assign(
              fileContent.begin() + static_cast<ptrdiff_t>(offset.Value()), fileContent.end());
        }
        else
        {
          expectedData.clear();
        }
      }
      downloadBuffer.resize(static_cast<size_t>(downloadSize), '\x00');
      Files::Shares::DownloadFileToOptions options;
      options.TransferOptions.Concurrency = concurrency;
      if (offset.HasValue())
      {
        options.Range = Core::Http::HttpRange();
        options.Range.Value().Offset = offset.Value();
        options.Range.Value().Length = length;
      }

      if (initialChunkSize.HasValue())
      {
        options.TransferOptions.InitialChunkSize = initialChunkSize.Value();
      }
      if (chunkSize.HasValue())
      {
        options.TransferOptions.ChunkSize = chunkSize.Value();
      }
      if (actualDownloadSize > 0)
      {
        auto res = m_fileClient->DownloadTo(downloadBuffer.data(), downloadBuffer.size(), options);
        EXPECT_EQ(res.Value.ContentRange.Length.Value(), actualDownloadSize);
        downloadBuffer.resize(static_cast<size_t>(res.Value.ContentRange.Length.Value()));
        EXPECT_EQ(downloadBuffer, expectedData);
      }
      else
      {
        EXPECT_THROW(
            m_fileClient->DownloadTo(downloadBuffer.data(), downloadBuffer.size(), options),
            StorageException);
      }
    };
    auto testDownloadToFile = [&](int concurrency,
                                  int64_t downloadSize,
                                  Azure::Nullable<int64_t> offset = {},
                                  Azure::Nullable<int64_t> length = {},
                                  Azure::Nullable<int64_t> initialChunkSize = {},
                                  Azure::Nullable<int64_t> chunkSize = {}) {
      std::string tempFilename = RandomString();
      std::vector<uint8_t> expectedData = fileContent;
      int64_t fileSize = fileContent.size();
      int64_t actualDownloadSize = std::min(downloadSize, fileSize);
      if (offset.HasValue() && length.HasValue())
      {
        actualDownloadSize = std::min(length.Value(), fileSize - offset.Value());
        if (actualDownloadSize >= 0)
        {
          expectedData.assign(
              fileContent.begin() + static_cast<ptrdiff_t>(offset.Value()),
              fileContent.begin() + static_cast<ptrdiff_t>(offset.Value() + actualDownloadSize));
        }
        else
        {
          expectedData.clear();
        }
      }
      else if (offset.HasValue())
      {
        actualDownloadSize = fileSize - offset.Value();
        if (actualDownloadSize >= 0)
        {
          expectedData.assign(
              fileContent.begin() + static_cast<ptrdiff_t>(offset.Value()), fileContent.end());
        }
        else
        {
          expectedData.clear();
        }
      }
      Files::Shares::DownloadFileToOptions options;
      options.TransferOptions.Concurrency = concurrency;
      if (offset.HasValue())
      {
        options.Range = Core::Http::HttpRange();
        options.Range.Value().Offset = offset.Value();
        options.Range.Value().Length = length;
      }
      if (initialChunkSize.HasValue())
      {
        options.TransferOptions.InitialChunkSize = initialChunkSize.Value();
      }
      if (chunkSize.HasValue())
      {
        options.TransferOptions.ChunkSize = chunkSize.Value();
      }
      if (actualDownloadSize > 0)
      {
        auto res = m_fileClient->DownloadTo(tempFilename, options);
        EXPECT_EQ(res.Value.ContentRange.Length.Value(), actualDownloadSize);
        EXPECT_EQ(ReadFile(tempFilename), expectedData);
      }
      else
      {
        EXPECT_THROW(m_fileClient->DownloadTo(tempFilename, options), StorageException);
      }
      DeleteFile(tempFilename);
    };

    const int64_t fileSize = fileContent.size();
    std::vector<std::future<void>> futures;
    for (int c : {1, 2, 4})
    {
      // random range
      std::mt19937_64 random_generator(std::random_device{}());
      for (int i = 0; i < 16; ++i)
      {
        std::uniform_int_distribution<int64_t> offsetDistribution(0, fileContent.size() - 1);
        int64_t offset = offsetDistribution(random_generator);
        std::uniform_int_distribution<int64_t> lengthDistribution(1, 64_KB);
        int64_t length = lengthDistribution(random_generator);
        futures.emplace_back(std::async(
            std::launch::async, testDownloadToBuffer, c, fileSize, offset, length, 4_KB, 4_KB));
        futures.emplace_back(std::async(
            std::launch::async, testDownloadToFile, c, fileSize, offset, length, 4_KB, 4_KB));
      }

      // buffer not big enough
      Files::Shares::DownloadFileToOptions options;
      options.TransferOptions.Concurrency = c;
      options.Range = Core::Http::HttpRange();
      options.Range.Value().Offset = 1;
      for (int64_t length : {1ULL, 2ULL, 4_KB, 5_KB, 8_KB, 11_KB, 20_KB})
      {
        std::vector<uint8_t> downloadBuffer;
        downloadBuffer.resize(static_cast<size_t>(length - 1));
        options.Range.Value().Length = length;
        EXPECT_THROW(
            m_fileClient->DownloadTo(
                downloadBuffer.data(), static_cast<size_t>(length - 1), options),
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
    auto rangeSize = 128;
    auto numOfChunks = 3;
    std::vector<uint8_t> rangeContent = RandomBuffer(rangeSize);
    auto memBodyStream = Core::IO::MemoryBodyStream(rangeContent);
    {
      // Simple upload/download.
      auto fileClient = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString());
      fileClient.Create(static_cast<int64_t>(numOfChunks) * rangeSize);
      for (int32_t i = 0; i < numOfChunks; ++i)
      {
        memBodyStream.Rewind();
        EXPECT_NO_THROW(fileClient.UploadRange(static_cast<int64_t>(rangeSize) * i, memBodyStream));
      }

      for (int32_t i = 0; i < numOfChunks; ++i)
      {
        Files::Shares::DownloadFileOptions downloadOptions;
        downloadOptions.Range = Core::Http::HttpRange();
        downloadOptions.Range.Value().Offset = static_cast<int64_t>(rangeSize) * i;
        downloadOptions.Range.Value().Length = rangeSize;
        Files::Shares::Models::DownloadFileResult result;
        EXPECT_NO_THROW(result = fileClient.Download(downloadOptions).Value);
        auto resultBuffer = result.BodyStream->ReadToEnd(Core::Context());
        EXPECT_EQ(rangeContent, resultBuffer);
        EXPECT_EQ(downloadOptions.Range.Value().Length.Value(), result.ContentRange.Length.Value());
        EXPECT_EQ(downloadOptions.Range.Value().Offset, result.ContentRange.Offset);
        EXPECT_EQ(static_cast<int64_t>(numOfChunks) * rangeSize, result.FileSize);
      }
    }
    // last write time
    {
      memBodyStream.Rewind();
      auto fileClient = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString());
      fileClient.Create(static_cast<int64_t>(numOfChunks) * rangeSize);
      auto lastWriteTimeBeforeUpload
          = fileClient.GetProperties().Value.SmbProperties.LastWrittenOn.Value();
      Files::Shares::UploadFileRangeOptions uploadOptions;
      uploadOptions.FileLastWrittenMode
          = Azure::Storage::Files::Shares::Models::FileLastWrittenMode::Now;
      EXPECT_NO_THROW(fileClient.UploadRange(0, memBodyStream, uploadOptions));
      auto lastWriteTimeAfterUpload
          = fileClient.GetProperties().Value.SmbProperties.LastWrittenOn.Value();
      EXPECT_NE(lastWriteTimeBeforeUpload, lastWriteTimeAfterUpload);
    }
    {
      memBodyStream.Rewind();
      auto fileClient = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString());
      fileClient.Create(static_cast<int64_t>(numOfChunks) * rangeSize);
      auto lastWriteTimeBeforeUpload
          = fileClient.GetProperties().Value.SmbProperties.LastWrittenOn.Value();
      Files::Shares::UploadFileRangeOptions uploadOptions;
      uploadOptions.FileLastWrittenMode
          = Azure::Storage::Files::Shares::Models::FileLastWrittenMode::Preserve;
      EXPECT_NO_THROW(fileClient.UploadRange(0, memBodyStream, uploadOptions));
      auto lastWriteTimeAfterUpload
          = fileClient.GetProperties().Value.SmbProperties.LastWrittenOn.Value();
      EXPECT_EQ(lastWriteTimeBeforeUpload, lastWriteTimeAfterUpload);
    }
    {
      // MD5 works.
      memBodyStream.Rewind();
      Azure::Core::Cryptography::Md5Hash instance;
      auto md5 = instance.Final(rangeContent.data(), rangeContent.size());
      auto fileClient = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString() + "2");
      Files::Shares::UploadFileRangeOptions uploadOptions;
      fileClient.Create(static_cast<int64_t>(numOfChunks) * rangeSize);
      ContentHash hash;
      hash.Value = md5;
      hash.Algorithm = HashAlgorithm::Md5;
      uploadOptions.TransactionalContentHash = hash;
      EXPECT_NO_THROW(fileClient.UploadRange(0, memBodyStream, uploadOptions));
      hash.Value = Azure::Core::Convert::Base64Decode(DummyMd5);
      uploadOptions.TransactionalContentHash = hash;
      memBodyStream.Rewind();
      EXPECT_THROW(fileClient.UploadRange(0, memBodyStream, uploadOptions), StorageException);
    }
  }

  TEST_F(FileShareFileClientTest, CopyRelated)
  {
    size_t fileSize = 128;
    std::vector<uint8_t> fileContent = RandomBuffer(fileSize);
    auto memBodyStream = Core::IO::MemoryBodyStream(fileContent);
    {
      // Simple copy works.
      auto fileClient = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString() + "1");
      fileClient.Create(fileSize);

      auto destFileClient
          = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString() + "2");
      auto copyOperation = destFileClient.StartCopy(fileClient.GetUrl());
      EXPECT_EQ(
          copyOperation.GetRawResponse().GetStatusCode(),
          Azure::Core::Http::HttpStatusCode::Accepted);
      auto fileProperties = copyOperation.PollUntilDone(std::chrono::milliseconds(1000)).Value;
      EXPECT_EQ(fileProperties.CopyStatus.Value(), Files::Shares::Models::CopyStatus::Success);
    }

    {
      // Copy mode with override and empty permission throws error..
      auto fileClient = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString() + "3");
      fileClient.Create(fileSize);

      auto destFileClient
          = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString() + "4");
    }
  }

  TEST_F(FileShareFileClientTest, RangeRelated)
  {
    size_t fileSize = 1024 * 3;
    auto fileContent = RandomBuffer(fileSize);
    auto memBodyStream = Core::IO::MemoryBodyStream(fileContent);
    auto halfContent
        = std::vector<uint8_t>(fileContent.begin(), fileContent.begin() + fileSize / 2);
    halfContent.resize(fileSize);
    auto fileClient = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString());
    fileClient.Create(fileSize);
    EXPECT_NO_THROW(fileClient.UploadRange(0, memBodyStream));
    EXPECT_NO_THROW(fileClient.ClearRange(fileSize / 2, fileSize / 2));
    std::vector<uint8_t> downloadContent(static_cast<size_t>(fileSize), '\x00');
    EXPECT_NO_THROW(fileClient.DownloadTo(downloadContent.data(), static_cast<size_t>(fileSize)));
    EXPECT_EQ(halfContent, downloadContent);

    EXPECT_NO_THROW(fileClient.ClearRange(512, 512));
    Files::Shares::Models::GetFileRangeListResult result;
    EXPECT_NO_THROW(result = fileClient.GetRangeList().Value);
    EXPECT_EQ(2U, result.Ranges.size());
    EXPECT_EQ(0, result.Ranges[0].Offset);
    EXPECT_TRUE(result.Ranges[0].Length.HasValue());
    EXPECT_EQ(512, result.Ranges[0].Length.Value());
    EXPECT_EQ(1024, result.Ranges[1].Offset);
    EXPECT_TRUE(result.Ranges[1].Length.HasValue());
    EXPECT_EQ(static_cast<int32_t>(fileSize / 2) - 1024, result.Ranges[1].Length.Value());
  }

  TEST_F(FileShareFileClientTest, PreviousRangeWithSnapshot)
  {
    size_t fileSize = 1024 * 10;
    std::vector<uint8_t> fileContent = RandomBuffer(fileSize);
    auto memBodyStream = Core::IO::MemoryBodyStream(fileContent);
    auto halfContent
        = std::vector<uint8_t>(fileContent.begin(), fileContent.begin() + fileSize / 2);
    halfContent.resize(fileSize);
    auto fileClient = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString());
    fileClient.Create(fileSize);
    EXPECT_NO_THROW(fileClient.UploadRange(0, memBodyStream));
    EXPECT_NO_THROW(fileClient.ClearRange(fileSize / 2, fileSize / 2));
    std::vector<uint8_t> downloadContent(static_cast<size_t>(fileSize), '\x00');
    EXPECT_NO_THROW(fileClient.DownloadTo(downloadContent.data(), static_cast<size_t>(fileSize)));
    EXPECT_EQ(halfContent, downloadContent);

    auto snapshot1 = m_shareClient->CreateSnapshot().Value.Snapshot;
    EXPECT_NO_THROW(fileClient.ClearRange(500, 2048));
    auto snapshot2 = m_shareClient->CreateSnapshot().Value.Snapshot;
    Files::Shares::Models::GetFileRangeListResult result;
    Files::Shares::GetFileRangeListOptions options;
    EXPECT_NO_THROW(result = fileClient.GetRangeListDiff(snapshot1, options).Value);
    EXPECT_EQ(2U, result.Ranges.size());
    EXPECT_EQ(0, result.Ranges[0].Offset);
    EXPECT_TRUE(result.Ranges[0].Length.HasValue());
    EXPECT_EQ(512, result.Ranges[0].Length.Value());
    EXPECT_EQ(2048, result.Ranges[1].Offset);
    EXPECT_TRUE(result.Ranges[1].Length.HasValue());
    EXPECT_EQ(512, result.Ranges[1].Length.Value());
    EXPECT_NO_THROW(fileClient.ClearRange(3096, 2048));
    auto snapshot3 = m_shareClient->CreateSnapshot().Value.Snapshot;
    EXPECT_NO_THROW(result = fileClient.GetRangeListDiff(snapshot1, options).Value);
    EXPECT_EQ(4U, result.Ranges.size());
    EXPECT_EQ(0, result.Ranges[0].Offset);
    EXPECT_TRUE(result.Ranges[0].Length.HasValue());
    EXPECT_EQ(512, result.Ranges[0].Length.Value());
    EXPECT_EQ(2048, result.Ranges[1].Offset);
    EXPECT_TRUE(result.Ranges[1].Length.HasValue());
    EXPECT_EQ(512, result.Ranges[1].Length.Value());
    EXPECT_EQ(3072, result.Ranges[2].Offset);
    EXPECT_TRUE(result.Ranges[2].Length.HasValue());
    EXPECT_EQ(512, result.Ranges[2].Length.Value());
    EXPECT_EQ(5120, result.Ranges[3].Offset);
    EXPECT_TRUE(result.Ranges[3].Length.HasValue());
    EXPECT_EQ(512, result.Ranges[3].Length.Value());

    EXPECT_EQ(2U, result.ClearRanges.size());
    EXPECT_EQ(512, result.ClearRanges[0].Offset);
    EXPECT_TRUE(result.ClearRanges[0].Length.HasValue());
    EXPECT_EQ(1536, result.ClearRanges[0].Length.Value());
    EXPECT_EQ(3584, result.ClearRanges[1].Offset);
    EXPECT_TRUE(result.ClearRanges[1].Length.HasValue());
    EXPECT_EQ(1536, result.ClearRanges[1].Length.Value());
  }

  TEST_F(FileShareFileClientTest, StorageExceptionAdditionalInfo)
  {
    auto options = InitStorageClientOptions<Azure::Storage::Files::Shares::ShareClientOptions>();
    class InvalidQueryParameterPolicy final : public Azure::Core::Http::Policies::HttpPolicy {
    public:
      ~InvalidQueryParameterPolicy() override {}

      std::unique_ptr<HttpPolicy> Clone() const override
      {
        return std::make_unique<InvalidQueryParameterPolicy>(*this);
      }

      std::unique_ptr<Core::Http::RawResponse> Send(
          Core::Http::Request& request,
          Core::Http::Policies::NextHttpPolicy nextPolicy,
          Core::Context const& context) const override
      {
        request.GetUrl().AppendQueryParameter("comp", "lease1");
        return nextPolicy.Send(request, context);
      }
    };
    options.PerOperationPolicies.emplace_back(std::make_unique<InvalidQueryParameterPolicy>());
    auto fileClient = Azure::Storage::Files::Shares::ShareFileClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_shareName, RandomString(), options);
    try
    {
      fileClient.Create(1024);
    }
    catch (const StorageException& e)
    {
      EXPECT_NE(e.StatusCode, Azure::Core::Http::HttpStatusCode::None);
      EXPECT_FALSE(e.ReasonPhrase.empty());
      EXPECT_FALSE(e.ClientRequestId.empty());
      EXPECT_FALSE(e.RequestId.empty());
      EXPECT_FALSE(e.ErrorCode.empty());
      EXPECT_FALSE(e.Message.empty());
      EXPECT_FALSE(e.AdditionalInformation.empty());
      return;
    }
    FAIL();
  }

  TEST_F(FileShareFileClientTest, UploadRangeFromUri)
  {
    size_t fileSize = 1 * 1024;
    std::string fileName = RandomString() + "file";
    std::vector<uint8_t> fileContent = RandomBuffer(fileSize);
    auto memBodyStream = Core::IO::MemoryBodyStream(fileContent);
    auto sourceFileClient = m_shareClient->GetRootDirectoryClient().GetFileClient(fileName);
    sourceFileClient.Create(fileSize);
    EXPECT_NO_THROW(sourceFileClient.UploadRange(0, memBodyStream));

    auto destFileClient
        = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString() + "f2");
    destFileClient.Create(fileSize * 4);
    Azure::Core::Http::HttpRange sourceRange;
    Azure::Core::Http::HttpRange destRange;
    sourceRange.Length = fileSize;
    destRange.Offset = fileSize;
    destRange.Length = fileSize;

    // Get the SAS of the file
    Sas::ShareSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSasBuilder.StartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    fileSasBuilder.ExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);
    fileSasBuilder.ShareName = m_shareName;
    fileSasBuilder.FilePath = fileName;
    fileSasBuilder.Resource = Sas::ShareSasResource::File;
    fileSasBuilder.SetPermissions(Sas::ShareSasPermissions::Read);
    std::string sourceSas = fileSasBuilder.GenerateSasToken(
        *_internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential);

    Files::Shares::Models::UploadFileRangeFromUriResult uploadResult;
    EXPECT_NO_THROW(
        uploadResult = destFileClient
                           .UploadRangeFromUri(
                               destRange.Offset, sourceFileClient.GetUrl() + sourceSas, sourceRange)
                           .Value);

    Files::Shares::Models::DownloadFileResult result;
    Files::Shares::DownloadFileOptions downloadOptions;
    downloadOptions.Range = destRange;
    EXPECT_NO_THROW(result = destFileClient.Download(downloadOptions).Value);
    auto resultBuffer = result.BodyStream->ReadToEnd(Core::Context());
    EXPECT_EQ(fileContent, resultBuffer);
    Files::Shares::Models::GetFileRangeListResult getRangeResult;
    EXPECT_NO_THROW(getRangeResult = destFileClient.GetRangeList().Value);
    EXPECT_EQ(1U, getRangeResult.Ranges.size());
    EXPECT_EQ(static_cast<int64_t>(fileSize), getRangeResult.Ranges[0].Offset);
    EXPECT_TRUE(getRangeResult.Ranges[0].Length.HasValue());
    EXPECT_EQ(static_cast<int64_t>(fileSize), getRangeResult.Ranges[0].Length.Value());

    // last write time
    {
      auto lastWriteTimeBeforeUpload
          = destFileClient.GetProperties().Value.SmbProperties.LastWrittenOn.Value();
      Files::Shares::UploadFileRangeFromUriOptions uploadRangeOptions;
      uploadRangeOptions.FileLastWrittenMode
          = Azure::Storage::Files::Shares::Models::FileLastWrittenMode::Now;
      EXPECT_NO_THROW(
          uploadResult = destFileClient
                             .UploadRangeFromUri(
                                 destRange.Offset,
                                 sourceFileClient.GetUrl() + sourceSas,
                                 sourceRange,
                                 uploadRangeOptions)
                             .Value);
      auto lastWriteTimeAfterUpload
          = destFileClient.GetProperties().Value.SmbProperties.LastWrittenOn.Value();
      EXPECT_NE(lastWriteTimeBeforeUpload, lastWriteTimeAfterUpload);
    }
    {
      auto lastWriteTimeBeforeUpload
          = destFileClient.GetProperties().Value.SmbProperties.LastWrittenOn.Value();
      Files::Shares::UploadFileRangeFromUriOptions uploadRangeOptions;
      uploadRangeOptions.FileLastWrittenMode
          = Azure::Storage::Files::Shares::Models::FileLastWrittenMode::Preserve;
      EXPECT_NO_THROW(
          uploadResult = destFileClient
                             .UploadRangeFromUri(
                                 destRange.Offset,
                                 sourceFileClient.GetUrl() + sourceSas,
                                 sourceRange,
                                 uploadRangeOptions)
                             .Value);
      auto lastWriteTimeAfterUpload
          = destFileClient.GetProperties().Value.SmbProperties.LastWrittenOn.Value();
      EXPECT_EQ(lastWriteTimeBeforeUpload, lastWriteTimeAfterUpload);
    }
    // source access condition works.
    {
      Files::Shares::UploadFileRangeFromUriOptions uploadRangeOptions;
      uploadRangeOptions.SourceAccessCondition.IfNoneMatchContentHash
          = uploadResult.TransactionalContentHash;
      EXPECT_THROW(
          uploadResult = destFileClient
                             .UploadRangeFromUri(
                                 destRange.Offset,
                                 sourceFileClient.GetUrl() + sourceSas,
                                 sourceRange,
                                 uploadRangeOptions)
                             .Value,
          StorageException);
      uploadRangeOptions.SourceAccessCondition.IfNoneMatchContentHash.Value().Value
          = Azure::Core::Convert::Base64Decode(DummyCrc64);

      EXPECT_NO_THROW(
          uploadResult = destFileClient
                             .UploadRangeFromUri(
                                 destRange.Offset,
                                 sourceFileClient.GetUrl() + sourceSas,
                                 sourceRange,
                                 uploadRangeOptions)
                             .Value);
    }
    {
      Files::Shares::UploadFileRangeFromUriOptions uploadRangeOptions;
      uploadRangeOptions.SourceAccessCondition.IfMatchContentHash
          = uploadResult.TransactionalContentHash;
      EXPECT_NO_THROW(
          uploadResult = destFileClient
                             .UploadRangeFromUri(
                                 destRange.Offset,
                                 sourceFileClient.GetUrl() + sourceSas,
                                 sourceRange,
                                 uploadRangeOptions)
                             .Value);

      uploadRangeOptions.SourceAccessCondition.IfMatchContentHash.Value().Value
          = Azure::Core::Convert::Base64Decode(DummyCrc64);
      EXPECT_THROW(
          uploadResult = destFileClient
                             .UploadRangeFromUri(
                                 destRange.Offset,
                                 sourceFileClient.GetUrl() + sourceSas,
                                 sourceRange,

                                 uploadRangeOptions)
                             .Value,
          StorageException);
    }
    {
      Files::Shares::UploadFileRangeFromUriOptions uploadRangeOptions;
      uploadRangeOptions.TransactionalContentHash = uploadResult.TransactionalContentHash;
      EXPECT_NO_THROW(
          uploadResult = destFileClient
                             .UploadRangeFromUri(
                                 destRange.Offset,
                                 sourceFileClient.GetUrl() + sourceSas,
                                 sourceRange,
                                 uploadRangeOptions)
                             .Value);
      uploadRangeOptions.TransactionalContentHash.Value().Value
          = Azure::Core::Convert::Base64Decode(DummyCrc64);
      EXPECT_THROW(
          uploadResult = destFileClient
                             .UploadRangeFromUri(
                                 destRange.Offset,
                                 sourceFileClient.GetUrl() + sourceSas,
                                 sourceRange,
                                 uploadRangeOptions)
                             .Value,
          StorageException);
    }
  }

}}} // namespace Azure::Storage::Test
