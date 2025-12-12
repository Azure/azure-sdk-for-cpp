// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "share_file_client_test.hpp"

#include <azure/core/cryptography/hash.hpp>
#include <azure/storage/blobs/blob_container_client.hpp>
#include <azure/storage/blobs/block_blob_client.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/file_io.hpp>
#include <azure/storage/common/storage_common.hpp>

#include <algorithm>
#include <chrono>
#include <future>
#include <random>

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
        auto shareClient = m_shareServiceClient->GetShareClient(LowercaseRandomString());
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

  TEST_F(FileShareFileClientTest, CreateWithHttpHeaders)
  {
    auto fileName = RandomString();
    Files::Shares::ShareFileClient client = m_fileShareDirectoryClient->GetFileClient(fileName);

    std::vector<uint8_t> emptyContent;
    Azure::Core::Cryptography::Md5Hash instance;
    auto md5 = instance.Final(emptyContent.data(), 0L);

    Files::Shares::Models::FileHttpHeaders httpHeaders;
    httpHeaders.ContentType = "application/x-binary";
    httpHeaders.ContentLanguage = "en-US";
    httpHeaders.ContentDisposition = "attachment";
    httpHeaders.CacheControl = "no-cache";
    httpHeaders.ContentEncoding = "identity";
    httpHeaders.ContentHash.Algorithm = HashAlgorithm::Md5;
    httpHeaders.ContentHash.Value = md5;

    Files::Shares::CreateFileOptions options;
    options.HttpHeaders = httpHeaders;

    EXPECT_NO_THROW(client.Create(1024, options));
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

  TEST_F(FileShareFileClientTest, HandlesFunctionalityWorks_PLAYBACKONLY_)
  {
    auto shareClient = m_shareServiceClient->GetShareClient("myshare");
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient("myfile.txt");
    Files::Shares::ListFileHandlesOptions options;
    options.PageSizeHint = 1;
    std::unordered_set<std::string> handles;
    for (auto pageResult = fileClient.ListHandles(options); pageResult.HasPage();
         pageResult.MoveToNextPage())
    {
      ASSERT_EQ(1L, pageResult.FileHandles.size());
      handles.insert(pageResult.FileHandles[0].HandleId);
    }
    EXPECT_EQ(handles.size(), 2);

    EXPECT_NO_THROW(fileClient.ForceCloseAllHandles());

    auto result = fileClient.ListHandles();
    EXPECT_TRUE(result.FileHandles.empty());
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
      int64_t actualDownloadSize = (std::min)(downloadSize, fileSize);
      if (offset.HasValue() && length.HasValue())
      {
        actualDownloadSize = (std::min)(length.Value(), fileSize - offset.Value());
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
      int64_t actualDownloadSize = (std::min)(downloadSize, fileSize);
      if (offset.HasValue() && length.HasValue())
      {
        actualDownloadSize = (std::min)(length.Value(), fileSize - offset.Value());
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

      // Range download without Length
      {
        Files::Shares::DownloadFileOptions downloadOptions;
        downloadOptions.Range = Core::Http::HttpRange();
        downloadOptions.Range.Value().Offset = static_cast<int64_t>(rangeSize) * (numOfChunks - 1);
        Files::Shares::Models::DownloadFileResult result;
        EXPECT_NO_THROW(result = fileClient.Download(downloadOptions).Value);
        auto resultBuffer = result.BodyStream->ReadToEnd(Core::Context());
        EXPECT_EQ(rangeContent, resultBuffer);
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

      Files::Shares::DownloadFileOptions downloadOptions;
      downloadOptions.Range = Core::Http::HttpRange();
      downloadOptions.Range.Value().Offset = 0;
      downloadOptions.Range.Value().Length = rangeSize;
      downloadOptions.RangeHashAlgorithm = HashAlgorithm::Md5;
      Files::Shares::Models::DownloadFileResult result;
      EXPECT_NO_THROW(result = fileClient.Download(downloadOptions).Value);
      auto resultBuffer = result.BodyStream->ReadToEnd(Core::Context());
      EXPECT_EQ(rangeContent, resultBuffer);
      EXPECT_EQ(downloadOptions.Range.Value().Length.Value(), result.ContentRange.Length.Value());
      EXPECT_EQ(downloadOptions.Range.Value().Offset, result.ContentRange.Offset);
      EXPECT_EQ(static_cast<int64_t>(numOfChunks) * rangeSize, result.FileSize);
      EXPECT_TRUE(result.TransactionalContentHash.HasValue());
      EXPECT_EQ(md5, result.TransactionalContentHash.Value().Value);
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

  TEST_F(FileShareFileClientTest, CopyWithProperties)
  {
    size_t fileSize = 128;
    std::vector<uint8_t> fileContent = RandomBuffer(fileSize);
    auto memBodyStream = Core::IO::MemoryBodyStream(fileContent);
    {
      // Simple copy works.
      auto fileClient = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString() + "1");
      fileClient.Create(fileSize);

      auto sourceProperties = fileClient.GetProperties().Value;

      Files::Shares::StartFileCopyOptions options;
      options.SmbProperties.Attributes = sourceProperties.SmbProperties.Attributes;
      options.SmbProperties.CreatedOn = sourceProperties.SmbProperties.CreatedOn;
      options.SmbProperties.ChangedOn = sourceProperties.SmbProperties.ChangedOn;
      options.SmbProperties.LastWrittenOn = sourceProperties.SmbProperties.LastWrittenOn;
      options.PermissionCopyMode = Files::Shares::Models::PermissionCopyMode::Override;
      options.SmbProperties.PermissionKey = sourceProperties.SmbProperties.PermissionKey;

      auto destFileClient
          = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString() + "2");
      auto copyOperation = destFileClient.StartCopy(fileClient.GetUrl(), options);
      EXPECT_EQ(
          copyOperation.GetRawResponse().GetStatusCode(),
          Azure::Core::Http::HttpStatusCode::Accepted);
      auto fileProperties = copyOperation.PollUntilDone(std::chrono::milliseconds(1000)).Value;
      EXPECT_EQ(fileProperties.CopyStatus.Value(), Files::Shares::Models::CopyStatus::Success);
    }
  }

  TEST_F(FileShareFileClientTest, CopyWithSmbPropertyFlags)
  {
    auto sddlPermission
        = "O:S-1-5-21-2127521184-1604012920-1887927527-21560751G:S-1-5-21-2127521184-1604012920-"
          "1887927527-513D:(A;;FA;;;SY)(A;;FA;;;BA)(A;;0x1200a9;;;S-1-5-21-397955417-626881126-"
          "188441444-3053964)";

    Files::Shares::CreateFileOptions createOptions;
    createOptions.SmbProperties.Attributes = Files::Shares::Models::FileAttributes::System
        | Files::Shares::Models::FileAttributes::NotContentIndexed;
    auto fileClient = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString() + "1");
    fileClient.Create(128, createOptions);
    auto sourceProperties = fileClient.GetProperties().Value;

    auto fileProperties = m_fileClient->GetProperties().Value;

    // None scenario
    {
      Files::Shares::StartFileCopyOptions options;
      options.SmbPropertiesToCopy = Files::Shares::CopyableFileSmbPropertyFlags::None;
      options.SmbProperties.Attributes = Files::Shares::Models::FileAttributes::ReadOnly;
      options.SmbProperties.CreatedOn = fileProperties.SmbProperties.CreatedOn;
      options.SmbProperties.ChangedOn = fileProperties.SmbProperties.ChangedOn;
      options.SmbProperties.LastWrittenOn = fileProperties.SmbProperties.LastWrittenOn;
      options.PermissionCopyMode = Files::Shares::Models::PermissionCopyMode::Override;
      options.Permission = sddlPermission;

      auto destFileClient
          = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString() + "2");
      auto copyOperation = destFileClient.StartCopy(fileClient.GetUrl(), options);
      EXPECT_EQ(
          copyOperation.GetRawResponse().GetStatusCode(),
          Azure::Core::Http::HttpStatusCode::Accepted);
      auto destProperties = copyOperation.PollUntilDone(std::chrono::milliseconds(1000)).Value;
      EXPECT_EQ(destProperties.CopyStatus.Value(), Files::Shares::Models::CopyStatus::Success);
      destProperties = destFileClient.GetProperties().Value;
      EXPECT_EQ(
          destProperties.SmbProperties.Attributes,
          options.SmbProperties.Attributes | Files::Shares::Models::FileAttributes::Archive);
      EXPECT_EQ(
          destProperties.SmbProperties.CreatedOn.Value(), options.SmbProperties.CreatedOn.Value());
      EXPECT_EQ(
          destProperties.SmbProperties.ChangedOn.Value(), options.SmbProperties.ChangedOn.Value());
      EXPECT_EQ(
          destProperties.SmbProperties.LastWrittenOn.Value(),
          options.SmbProperties.LastWrittenOn.Value());
      auto destPermissionKey = destProperties.SmbProperties.PermissionKey.Value();
      std::string destPermission = m_shareClient->GetPermission(destPermissionKey).Value;
      EXPECT_EQ(destPermission, sddlPermission);
    }

    // Source scenario
    {
      Files::Shares::StartFileCopyOptions options;
      options.SmbPropertiesToCopy = Files::Shares::CopyableFileSmbPropertyFlags::All;
      options.SmbProperties.Attributes = Files::Shares::Models::FileAttributes::ReadOnly;
      options.SmbProperties.CreatedOn = fileProperties.SmbProperties.CreatedOn;
      options.SmbProperties.ChangedOn = fileProperties.SmbProperties.ChangedOn;
      options.SmbProperties.LastWrittenOn = fileProperties.SmbProperties.LastWrittenOn;
      options.PermissionCopyMode = Files::Shares::Models::PermissionCopyMode::Override;
      options.SmbProperties.PermissionKey = "";

      auto destFileClient
          = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString() + "2");
      auto copyOperation = destFileClient.StartCopy(fileClient.GetUrl(), options);
      EXPECT_EQ(
          copyOperation.GetRawResponse().GetStatusCode(),
          Azure::Core::Http::HttpStatusCode::Accepted);
      auto destProperties = copyOperation.PollUntilDone(std::chrono::milliseconds(1000)).Value;
      EXPECT_EQ(destProperties.CopyStatus.Value(), Files::Shares::Models::CopyStatus::Success);
      destProperties = destFileClient.GetProperties().Value;
      EXPECT_EQ(destProperties.SmbProperties.Attributes, sourceProperties.SmbProperties.Attributes);
      EXPECT_EQ(
          destProperties.SmbProperties.CreatedOn.Value(),
          sourceProperties.SmbProperties.CreatedOn.Value());
      EXPECT_EQ(
          destProperties.SmbProperties.ChangedOn.Value(),
          sourceProperties.SmbProperties.ChangedOn.Value());
      EXPECT_EQ(
          destProperties.SmbProperties.LastWrittenOn.Value(),
          sourceProperties.SmbProperties.LastWrittenOn.Value());
      EXPECT_EQ(
          destProperties.SmbProperties.PermissionKey.Value(),
          sourceProperties.SmbProperties.PermissionKey.Value());
    }

    // Part Source Scenario
    {

      Files::Shares::StartFileCopyOptions options;
      options.SmbPropertiesToCopy = Files::Shares::CopyableFileSmbPropertyFlags::FileAttributes
          | Files::Shares::CopyableFileSmbPropertyFlags::Permission;
      options.SmbProperties.Attributes = Files::Shares::Models::FileAttributes::ReadOnly;
      options.SmbProperties.CreatedOn = fileProperties.SmbProperties.CreatedOn;
      options.SmbProperties.ChangedOn = fileProperties.SmbProperties.ChangedOn;
      options.SmbProperties.LastWrittenOn = fileProperties.SmbProperties.LastWrittenOn;
      options.PermissionCopyMode = Files::Shares::Models::PermissionCopyMode::Override;
      options.SmbProperties.PermissionKey = "";

      auto destFileClient
          = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString() + "2");
      auto copyOperation = destFileClient.StartCopy(fileClient.GetUrl(), options);
      EXPECT_EQ(
          copyOperation.GetRawResponse().GetStatusCode(),
          Azure::Core::Http::HttpStatusCode::Accepted);
      auto destProperties = copyOperation.PollUntilDone(std::chrono::milliseconds(1000)).Value;
      EXPECT_EQ(destProperties.CopyStatus.Value(), Files::Shares::Models::CopyStatus::Success);
      destProperties = destFileClient.GetProperties().Value;
      EXPECT_EQ(destProperties.SmbProperties.Attributes, sourceProperties.SmbProperties.Attributes);
      EXPECT_EQ(
          destProperties.SmbProperties.CreatedOn.Value(), options.SmbProperties.CreatedOn.Value());
      EXPECT_EQ(
          destProperties.SmbProperties.ChangedOn.Value(), options.SmbProperties.ChangedOn.Value());
      EXPECT_EQ(
          destProperties.SmbProperties.LastWrittenOn.Value(),
          options.SmbProperties.LastWrittenOn.Value());
      EXPECT_EQ(
          destProperties.SmbProperties.PermissionKey.Value(),
          sourceProperties.SmbProperties.PermissionKey.Value());
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

  TEST_F(FileShareFileClientTest, GetRangeListWithRange)
  {
    size_t rangeSize = 128;
    std::vector<uint8_t> rangeContent = RandomBuffer(rangeSize);
    auto memBodyStream = Core::IO::MemoryBodyStream(rangeContent);

    auto fileClient = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString());
    fileClient.Create(rangeSize);

    EXPECT_NO_THROW(fileClient.UploadRange(0, memBodyStream));

    // GetRangeList with Range
    {
      Files::Shares::GetFileRangeListOptions options;
      options.Range = Core::Http::HttpRange();
      options.Range.Value().Offset = 0;
      options.Range.Value().Length = 128;
      Files::Shares::Models::GetFileRangeListResult result;

      EXPECT_NO_THROW(result = fileClient.GetRangeList(options).Value);
      EXPECT_EQ(1U, result.Ranges.size());
      EXPECT_EQ(0, result.Ranges[0].Offset);
      EXPECT_TRUE(result.Ranges[0].Length.HasValue());

      options.Range.Value().Length.Reset();
      EXPECT_NO_THROW(result = fileClient.GetRangeList(options).Value);
      EXPECT_EQ(1U, result.Ranges.size());
      EXPECT_EQ(0, result.Ranges[0].Offset);
      EXPECT_TRUE(result.Ranges[0].Length.HasValue());
    }

    // GetRangeListDiff with Range
    {
      // sleep for 1 second to make sure the previous operation is finished
      TestSleep(std::chrono::milliseconds(1000));
      auto snapshot = m_shareClient->CreateSnapshot().Value.Snapshot;
      EXPECT_NO_THROW(fileClient.ClearRange(64, 64));
      Files::Shares::GetFileRangeListOptions options;
      options.Range = Core::Http::HttpRange();
      options.Range.Value().Offset = 64;
      options.Range.Value().Length = 64;
      Files::Shares::Models::GetFileRangeListResult result;

      EXPECT_NO_THROW(result = fileClient.GetRangeListDiff(snapshot, options).Value);
      EXPECT_EQ(1U, result.Ranges.size());
      EXPECT_EQ(64, result.Ranges[0].Offset);
      EXPECT_TRUE(result.Ranges[0].Length.HasValue());

      options.Range.Value().Length.Reset();
      EXPECT_NO_THROW(result = fileClient.GetRangeListDiff(snapshot, options).Value);
      EXPECT_EQ(1U, result.Ranges.size());
      EXPECT_EQ(64, result.Ranges[0].Offset);
      EXPECT_TRUE(result.Ranges[0].Length.HasValue());
    }
  }

  TEST_F(FileShareFileClientTest, GetRangeListDiffWithRename)
  {
    size_t rangeSize = 128;
    std::vector<uint8_t> rangeContent = RandomBuffer(rangeSize);
    auto memBodyStream = Core::IO::MemoryBodyStream(rangeContent);
    std::string sourceFileName = RandomString();
    std::string destFileName = RandomString();

    auto fileClient = m_shareClient->GetRootDirectoryClient().GetFileClient(sourceFileName);
    fileClient.Create(rangeSize);

    EXPECT_NO_THROW(fileClient.UploadRange(0, memBodyStream));

    // sleep for 1 second to make sure the previous operation is finished
    TestSleep(std::chrono::milliseconds(1000));
    auto snapshot = m_shareClient->CreateSnapshot().Value.Snapshot;
    EXPECT_NO_THROW(fileClient.ClearRange(64, 64));

    fileClient
        = m_shareClient->GetRootDirectoryClient().RenameFile(sourceFileName, destFileName).Value;

    Files::Shares::GetFileRangeListOptions options;
    options.Range = Core::Http::HttpRange();
    options.Range.Value().Offset = 64;
    options.Range.Value().Length = 64;
    Files::Shares::Models::GetFileRangeListResult result;

    // SupportRename == true
    options.IncludeRenames = true;
    EXPECT_NO_THROW(result = fileClient.GetRangeListDiff(snapshot, options).Value);
    EXPECT_EQ(1U, result.Ranges.size());
    EXPECT_EQ(64, result.Ranges[0].Offset);
    EXPECT_TRUE(result.Ranges[0].Length.HasValue());

    // SupportRename == false
    options.IncludeRenames = false;
    EXPECT_THROW(fileClient.GetRangeListDiff(snapshot, options), StorageException);
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
    auto fileClient = Azure::Storage::Files::Shares::ShareFileClient(
        GetShareFileUrl(m_shareName, RandomString()), GetTestCredential(), options);
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

  TEST_F(FileShareFileClientTest, UploadRangeFromUri_LIVEONLY_)
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

  TEST_F(FileShareFileClientTest, SourceOAuthUploadRangeFromUri_LIVEONLY_)
  {
    size_t fileSize = 1 * 1024;
    std::string containerName = LowercaseRandomString();
    std::string blobName = RandomString();
    std::vector<uint8_t> blobContent = RandomBuffer(fileSize);
    auto memBodyStream = Core::IO::MemoryBodyStream(blobContent);

    auto containerClient = Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
        StandardStorageConnectionString(),
        containerName,
        InitStorageClientOptions<Storage::Blobs::BlobClientOptions>());
    containerClient.Create();
    auto sourceBlobClient = containerClient.GetBlockBlobClient(blobName);
    sourceBlobClient.Upload(memBodyStream);

    auto destFileClient
        = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString() + "f2");
    destFileClient.Create(fileSize * 4);
    Azure::Core::Http::HttpRange sourceRange;
    Azure::Core::Http::HttpRange destRange;
    sourceRange.Length = fileSize;
    destRange.Length = fileSize;

    // Get oauth token of source file
    Azure::Core::Credentials::TokenRequestContext requestContext;
    requestContext.Scopes = {Storage::_internal::StorageScope};
    auto oauthToken = GetTestCredential()->GetToken(requestContext, Azure::Core::Context());

    Files::Shares::UploadFileRangeFromUriOptions options;
    options.SourceAuthorization = "Bearer " + oauthToken.Token;
    Files::Shares::Models::UploadFileRangeFromUriResult uploadResult;
    EXPECT_NO_THROW(
        uploadResult
        = destFileClient
              .UploadRangeFromUri(destRange.Offset, sourceBlobClient.GetUrl(), sourceRange, options)
              .Value);

    Files::Shares::Models::DownloadFileResult result;
    Files::Shares::DownloadFileOptions downloadOptions;
    downloadOptions.Range = destRange;
    EXPECT_NO_THROW(result = destFileClient.Download(downloadOptions).Value);
    auto resultBuffer = result.BodyStream->ReadToEnd(Core::Context());
    EXPECT_EQ(blobContent, resultBuffer);
    Files::Shares::Models::GetFileRangeListResult getRangeResult;
    EXPECT_NO_THROW(getRangeResult = destFileClient.GetRangeList().Value);
    EXPECT_EQ(1U, getRangeResult.Ranges.size());
    EXPECT_TRUE(getRangeResult.Ranges[0].Length.HasValue());
    EXPECT_EQ(static_cast<int64_t>(fileSize), getRangeResult.Ranges[0].Length.Value());

    EXPECT_NO_THROW(containerClient.Delete());
  }

  TEST_F(FileShareFileClientTest, DestinationOAuthUploadRangeFromUri_PLAYBACKONLY_)
  {
    size_t fileSize = 1 * 1024;
    std::string containerName = LowercaseRandomString();
    std::string blobName = RandomString();
    std::vector<uint8_t> blobContent = RandomBuffer(fileSize);
    auto memBodyStream = Core::IO::MemoryBodyStream(blobContent);

    auto containerClient = Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
        StandardStorageConnectionString(),
        containerName,
        InitStorageClientOptions<Storage::Blobs::BlobClientOptions>());
    containerClient.Create();
    auto sourceBlobClient = containerClient.GetBlockBlobClient(blobName);
    sourceBlobClient.Upload(memBodyStream);

    auto oauthCredential = GetTestCredential();
    auto clientOptions = InitStorageClientOptions<Files::Shares::ShareClientOptions>();
    clientOptions.ShareTokenIntent = Files::Shares::Models::ShareTokenIntent::Backup;

    auto destFileClient
        = Files::Shares::ShareClient(m_shareClient->GetUrl(), oauthCredential, clientOptions)
              .GetRootDirectoryClient()
              .GetFileClient(RandomString());

    destFileClient.Create(fileSize * 4);
    Azure::Core::Http::HttpRange sourceRange;
    Azure::Core::Http::HttpRange destRange;
    sourceRange.Length = fileSize;
    destRange.Length = fileSize;

    // Get oauth token of source file
    Azure::Core::Credentials::TokenRequestContext requestContext;
    requestContext.Scopes = {Storage::_internal::StorageScope};
    auto oauthToken = oauthCredential->GetToken(requestContext, Azure::Core::Context());

    Files::Shares::UploadFileRangeFromUriOptions options;
    options.SourceAuthorization = "Bearer " + oauthToken.Token;
    Files::Shares::Models::UploadFileRangeFromUriResult uploadResult;
    EXPECT_NO_THROW(
        uploadResult
        = destFileClient
              .UploadRangeFromUri(destRange.Offset, sourceBlobClient.GetUrl(), sourceRange, options)
              .Value);

    Files::Shares::Models::DownloadFileResult result;
    Files::Shares::DownloadFileOptions downloadOptions;
    downloadOptions.Range = destRange;
    EXPECT_NO_THROW(result = destFileClient.Download(downloadOptions).Value);
    auto resultBuffer = result.BodyStream->ReadToEnd(Core::Context());
    EXPECT_EQ(blobContent, resultBuffer);
    Files::Shares::Models::GetFileRangeListResult getRangeResult;
    EXPECT_NO_THROW(getRangeResult = destFileClient.GetRangeList().Value);
    EXPECT_EQ(1U, getRangeResult.Ranges.size());
    EXPECT_TRUE(getRangeResult.Ranges[0].Length.HasValue());
    EXPECT_EQ(static_cast<int64_t>(fileSize), getRangeResult.Ranges[0].Length.Value());

    EXPECT_NO_THROW(containerClient.Delete());
  }

  TEST_F(FileShareFileClientTest, UploadFromWithOptions)
  {
    auto fileClient = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString());

    size_t fileSize = 512;
    std::vector<uint8_t> content(RandomBuffer(fileSize));
    auto memBodyStream = Core::IO::MemoryBodyStream(content);

    Azure::Core::Cryptography::Md5Hash instance;
    auto md5 = instance.Final(content.data(), fileSize);

    Files::Shares::UploadFileFromOptions options;

    options.SmbProperties.Attributes = Files::Shares::Models::FileAttributes::Hidden;
    options.SmbProperties.CreatedOn = std::chrono::system_clock::now();
    options.SmbProperties.LastWrittenOn = std::chrono::system_clock::now();
    options.SmbProperties.PermissionKey = "";
    options.SmbProperties.ChangedOn = std::chrono::system_clock::now();
    options.HttpHeaders.ContentType = "application/x-binary";
    options.HttpHeaders.ContentLanguage = "en-US";
    options.HttpHeaders.ContentDisposition = "attachment";
    options.HttpHeaders.CacheControl = "no-cache";
    options.HttpHeaders.ContentEncoding = "identity";
    options.HttpHeaders.ContentHash.Value = md5;

    // UploadFrom buffer
    EXPECT_NO_THROW(fileClient.UploadFrom(content.data(), fileSize, options));

    // UploadFrom file
    const std::string tempFilename = "file" + RandomString();
    WriteFile(tempFilename, content);
    EXPECT_NO_THROW(fileClient.UploadFrom(tempFilename, options));
  }

  TEST_F(FileShareFileClientTest, AllowTrailingDot)
  {
    const std::string fileName = RandomString();
    const std::string fileNameWithTrailingDot = fileName + ".";
    const std::string shareName = m_shareName;
    auto options = InitStorageClientOptions<Files::Shares::ShareClientOptions>();
    options.ShareTokenIntent = Files::Shares::Models::ShareTokenIntent::Backup;

    auto testTrailingDot = [&](Nullable<bool> allowTrailingDot) {
      options.AllowTrailingDot = allowTrailingDot;

      auto shareServiceClient
          = Files::Shares::ShareServiceClient(GetShareServiceUrl(), GetTestCredential(), options);
      auto shareClient = shareServiceClient.GetShareClient(shareName);
      auto rootDirectoryClient = shareClient.GetRootDirectoryClient();
      auto fileClient = rootDirectoryClient.GetFileClient(fileNameWithTrailingDot);

      size_t fileSize = 512;
      std::vector<uint8_t> content(RandomBuffer(fileSize));
      auto memBodyStream = Core::IO::MemoryBodyStream(content);

      // Create
      auto createResult = fileClient.Create(fileSize).Value;

      // ListFilesAndDirectories
      bool isFound = false;
      for (auto page = rootDirectoryClient.ListFilesAndDirectories(); page.HasPage();
           page.MoveToNextPage())
      {
        auto files = page.Files;
        auto iter = std::find_if(
            files.begin(),
            files.end(),
            [&allowTrailingDot, &fileName, &fileNameWithTrailingDot](
                const Files::Shares::Models::FileItem& file) {
              std::string name = allowTrailingDot.HasValue() && allowTrailingDot.Value()
                  ? fileNameWithTrailingDot
                  : fileName;
              return file.Name == name;
            });
        if (iter != files.end())
        {
          isFound = true;
          break;
        }
      }
      EXPECT_TRUE(isFound);

      // GetProperties
      auto properties = fileClient.GetProperties().Value;
      EXPECT_EQ(createResult.LastModified, properties.LastModified);
      EXPECT_EQ(createResult.ETag, properties.ETag);

      // ListHandles
      auto handles = fileClient.ListHandles().FileHandles;
      EXPECT_EQ(handles.size(), 0L);

      // Download
      EXPECT_NO_THROW(fileClient.Download());

      // SetProperties
      EXPECT_NO_THROW(fileClient.SetProperties(
          Files::Shares::Models::FileHttpHeaders(), Files::Shares::Models::FileSmbProperties()));

      // SetMetadata
      EXPECT_NO_THROW(fileClient.SetMetadata(RandomMetadata()));

      // ForceCloseHandles
      auto closeHandlesResult = fileClient.ForceCloseAllHandles();
      EXPECT_EQ(closeHandlesResult.NumberOfHandlesClosed, 0);
      EXPECT_EQ(closeHandlesResult.NumberOfHandlesFailedToClose, 0);

      // UploadRange
      EXPECT_NO_THROW(fileClient.UploadRange(0L, memBodyStream));

      // GetRangeList
      auto range = fileClient.GetRangeList().Value;
      EXPECT_EQ(range.Ranges.size(), 1L);

      // GetRangeListDiff
      auto snapshot = m_shareClient->CreateSnapshot().Value.Snapshot;
      EXPECT_NO_THROW(fileClient.GetRangeListDiff(snapshot));

      // ClearRange
      EXPECT_NO_THROW(fileClient.ClearRange(0, fileSize));

      // UploadFrom buffer
      EXPECT_NO_THROW(fileClient.UploadFrom(content.data(), fileSize));

      // UploadFrom file
      const std::string tempFilename = "file" + RandomString();
      WriteFile(tempFilename, content);
      EXPECT_NO_THROW(fileClient.UploadFrom(tempFilename));

      // Delete
      EXPECT_NO_THROW(fileClient.Delete());
    };

    // allowTrailingDot not set
    testTrailingDot(Nullable<bool>());
    // allowTrailingDot = true
    testTrailingDot(true);
    // allowTrailingDot = false
    testTrailingDot(false);
  }

  TEST_F(FileShareFileClientTest, CopyAllowTrailingDot_LIVEONLY_)
  {
    const std::string fileName = RandomString();
    const std::string fileNameWithTrailingDot = fileName + ".";
    const std::string connectionString = StandardStorageConnectionString();
    const std::string shareName = m_shareName;
    auto options = InitStorageClientOptions<Files::Shares::ShareClientOptions>();
    options.ShareTokenIntent = Files::Shares::Models::ShareTokenIntent::Backup;

    auto testTrailingDot = [&](Nullable<bool> allowTrailingDot,
                               Nullable<bool> allowSourceTrailingDot) {
      options.AllowTrailingDot = allowTrailingDot;
      options.AllowSourceTrailingDot = allowSourceTrailingDot;

      auto shareServiceClient
          = Files::Shares::ShareServiceClient(GetShareServiceUrl(), GetTestCredential(), options);
      auto shareClient = shareServiceClient.GetShareClient(shareName);
      auto rootDirectoryClient = shareClient.GetRootDirectoryClient();
      auto fileClient = rootDirectoryClient.GetFileClient(fileNameWithTrailingDot);

      size_t fileSize = 1 * 1024;
      std::vector<uint8_t> content(RandomBuffer(fileSize));
      auto memBodyStream = Core::IO::MemoryBodyStream(content);

      auto createResult = fileClient.Create(fileSize).Value;
      fileClient.UploadRange(0, memBodyStream);

      bool allowTarget = allowTrailingDot.HasValue() && allowTrailingDot.Value();
      bool allowSource = allowSourceTrailingDot.HasValue() && allowSourceTrailingDot.Value();

      {
        const std::string destFileNameWithTrailingDot = fileName + "_dest" + ".";
        auto destFileClient = rootDirectoryClient.GetFileClient(destFileNameWithTrailingDot);

        // StartCopy
        if (allowTarget == allowSource)
        {
          auto copyOperation = destFileClient.StartCopy(fileClient.GetUrl());
          EXPECT_EQ(
              copyOperation.GetRawResponse().GetStatusCode(),
              Azure::Core::Http::HttpStatusCode::Accepted);
          copyOperation.Poll();
          EXPECT_TRUE(copyOperation.Value().CopyId.HasValue());

          // AbortCopy
          // This exception is intentionally. It is difficult to test AbortCopyAsync() in a
          // deterministic way.
          try
          {
            destFileClient.AbortCopy(copyOperation.Value().CopyId.Value());
          }
          catch (StorageException& e)
          {
            EXPECT_EQ(e.ErrorCode, "NoPendingCopyOperation");
          }

          EXPECT_NO_THROW(destFileClient.Delete());
        }
        else
        {
          EXPECT_THROW(destFileClient.StartCopy(fileClient.GetUrl()), StorageException);
        }
      }

      {
        // uploadRange
        const std::string destFileNameWithTrailingDot = fileName + "_dest2" + ".";
        auto destFileClient = rootDirectoryClient.GetFileClient(destFileNameWithTrailingDot);
        destFileClient.Create(fileSize);
        Azure::Core::Http::HttpRange sourceRange;
        Azure::Core::Http::HttpRange destRange;
        sourceRange.Length = fileSize;
        destRange.Offset = 0;
        destRange.Length = fileSize;

        // Get the SAS of the file
        Sas::ShareSasBuilder fileSasBuilder;
        fileSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
        fileSasBuilder.StartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
        fileSasBuilder.ExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);
        fileSasBuilder.ShareName = shareName;
        fileSasBuilder.FilePath = allowSource ? fileNameWithTrailingDot : fileName;
        fileSasBuilder.Resource = Sas::ShareSasResource::File;
        fileSasBuilder.SetPermissions(Sas::ShareSasPermissions::Read);
        std::string sourceSas = fileSasBuilder.GenerateSasToken(
            *_internal::ParseConnectionString(connectionString).KeyCredential);

        if (allowTarget == allowSource)
        {
          EXPECT_NO_THROW(destFileClient.UploadRangeFromUri(
              destRange.Offset, fileClient.GetUrl() + sourceSas, sourceRange));
        }
        else
        {
          EXPECT_THROW(
              destFileClient.UploadRangeFromUri(
                  destRange.Offset, fileClient.GetUrl() + sourceSas, sourceRange),
              StorageException);
        }

        EXPECT_NO_THROW(destFileClient.Delete());
      }

      // Delete
      EXPECT_NO_THROW(fileClient.Delete());
    };

    // allowTrailingDot not set, allowSourceTrailingDot not set
    testTrailingDot(Nullable<bool>(), Nullable<bool>());
    // allowTrailingDot = true, allowSourceTrailingDot =true
    testTrailingDot(true, true);
    // allowTrailingDot = true, allowSourceTrailingDot = false
    testTrailingDot(true, false);
    // allowTrailingDot = false, allowSourceTrailingDot = true
    testTrailingDot(false, true);
    // allowTrailingDot = false, allowSourceTrailingDot = false
    testTrailingDot(false, false);
  }

  TEST_F(FileShareFileClientTest, LeaseAllowTrailingDot)
  {
    const std::string fileNameWithTrailingDot = RandomString() + ".";
    const std::string connectionString = StandardStorageConnectionString();
    const std::string shareName = m_shareName;
    auto options = InitStorageClientOptions<Files::Shares::ShareClientOptions>();
    options.ShareTokenIntent = Files::Shares::Models::ShareTokenIntent::Backup;

    auto testTrailingDot = [&](Nullable<bool> allowTrailingDot) {
      options.AllowTrailingDot = allowTrailingDot;

      auto shareServiceClient
          = Files::Shares::ShareServiceClient(GetShareServiceUrl(), GetTestCredential(), options);
      auto shareClient = shareServiceClient.GetShareClient(shareName);
      auto rootDirectoryClient = shareClient.GetRootDirectoryClient();
      auto fileClient = rootDirectoryClient.GetFileClient(fileNameWithTrailingDot);
      std::string leaseId1 = RandomUUID();
      Files::Shares::ShareLeaseClient leaseClient(fileClient, leaseId1);

      size_t fileSize = 512;
      fileClient.Create(fileSize);

      // Acquire
      EXPECT_NO_THROW(leaseClient.Acquire(Files::Shares::ShareLeaseClient::InfiniteLeaseDuration));

      // Change
      std::string leaseId2 = RandomUUID();
      EXPECT_NO_THROW(leaseClient.Change(leaseId2));

      // Break
      EXPECT_NO_THROW(leaseClient.Break());

      // Release
      EXPECT_NO_THROW(leaseClient.Release());
    };

    // allowTrailingDot not set
    testTrailingDot(Nullable<bool>());
    // allowTrailingDot = true
    testTrailingDot(true);
    // allowTrailingDot = false
    testTrailingDot(false);
  }

  TEST_F(FileShareFileClientTest, OAuth_PLAYBACKONLY_)
  {
    const std::string fileName = RandomString();

    // Create from client secret credential.
    auto credential = GetTestCredential();
    auto options = InitStorageClientOptions<Files::Shares::ShareClientOptions>();
    options.ShareTokenIntent = Files::Shares::Models::ShareTokenIntent::Backup;

    auto shareClient = Files::Shares::ShareClient(m_shareClient->GetUrl(), credential, options);
    auto rootDirectoryClient = shareClient.GetRootDirectoryClient();
    auto fileClient = rootDirectoryClient.GetFileClient(fileName);
    std::string leaseId1 = RandomUUID();
    Files::Shares::ShareLeaseClient leaseClient(fileClient, leaseId1);

    size_t fileSize = 512;
    std::vector<uint8_t> content(RandomBuffer(fileSize));
    auto memBodyStream = Core::IO::MemoryBodyStream(content);

    // Create
    EXPECT_NO_THROW(fileClient.Create(fileSize));

    // GetProperties
    EXPECT_NO_THROW(fileClient.GetProperties());

    // ListHandles
    EXPECT_NO_THROW(fileClient.ListHandles());

    // Download
    EXPECT_NO_THROW(fileClient.Download());

    // SetProperties
    EXPECT_NO_THROW(fileClient.SetProperties(
        Files::Shares::Models::FileHttpHeaders(), Files::Shares::Models::FileSmbProperties()));

    // SetMetadata
    EXPECT_NO_THROW(fileClient.SetMetadata(RandomMetadata()));

    // ForceCloseHandles
    EXPECT_NO_THROW(fileClient.ForceCloseAllHandles());

    // UploadRange
    EXPECT_NO_THROW(fileClient.UploadRange(0L, memBodyStream));

    // GetRangeList
    EXPECT_NO_THROW(fileClient.GetRangeList());

    // GetRangeListDiff
    auto snapshot = m_shareClient->CreateSnapshot().Value.Snapshot;
    EXPECT_NO_THROW(fileClient.GetRangeListDiff(snapshot));

    // ClearRange
    EXPECT_NO_THROW(fileClient.ClearRange(0, fileSize));

    // UploadFrom buffer
    EXPECT_NO_THROW(fileClient.UploadFrom(content.data(), fileSize));

    // UploadFrom file
    const std::string tempFilename = "file" + RandomString();
    WriteFile(tempFilename, content);
    EXPECT_NO_THROW(fileClient.UploadFrom(tempFilename));

    // Acquire
    EXPECT_NO_THROW(leaseClient.Acquire(Files::Shares::ShareLeaseClient::InfiniteLeaseDuration));

    // Change
    std::string leaseId2 = RandomUUID();
    EXPECT_NO_THROW(leaseClient.Change(leaseId2));

    // Break
    EXPECT_NO_THROW(leaseClient.Break());

    // Release
    EXPECT_NO_THROW(leaseClient.Release());

    // Delete
    EXPECT_NO_THROW(fileClient.Delete());

    // OAuth Constructor
    auto fileClient1
        = Files::Shares::ShareFileClient(m_fileClient->GetUrl(), GetTestCredential(), options);
    EXPECT_NO_THROW(fileClient1.GetProperties());
  }

  TEST_F(FileShareFileClientTest, OAuthCopy_PLAYBACKONLY_)
  {
    const std::string fileName = RandomString();

    // Create from client secret credential.
    auto credential = GetTestCredential();
    auto options = InitStorageClientOptions<Files::Shares::ShareClientOptions>();
    options.ShareTokenIntent = Files::Shares::Models::ShareTokenIntent::Backup;

    auto shareClient = Files::Shares::ShareClient(m_shareClient->GetUrl(), credential, options);
    auto rootDirectoryClient = shareClient.GetRootDirectoryClient();
    auto fileClient = rootDirectoryClient.GetFileClient(fileName);

    size_t fileSize = 1 * 1024 * 1024;
    std::vector<uint8_t> content(RandomBuffer(fileSize));
    auto memBodyStream = Core::IO::MemoryBodyStream(content);

    auto createResult = fileClient.Create(fileSize).Value;
    fileClient.UploadRange(0, memBodyStream);

    const std::string destFileName = fileName + "_dest";
    auto destFileClient = rootDirectoryClient.GetFileClient(destFileName);

    // StartCopy
    auto copyOperation = destFileClient.StartCopy(fileClient.GetUrl());
    EXPECT_EQ(
        copyOperation.GetRawResponse().GetStatusCode(),
        Azure::Core::Http::HttpStatusCode::Accepted);
    copyOperation.Poll();
    EXPECT_TRUE(copyOperation.Value().CopyId.HasValue());

    // AbortCopy
    // This exception is intentionally. It is difficult to test AbortCopyAsync() in a
    // deterministic way.
    try
    {
      destFileClient.AbortCopy(copyOperation.Value().CopyId.Value());
    }
    catch (StorageException& e)
    {
      EXPECT_EQ(e.ErrorCode, "NoPendingCopyOperation");
    }
    EXPECT_NO_THROW(destFileClient.Delete());
  }

  // cspell:ignore myshare myfile
  TEST_F(FileShareFileClientTest, ListHandlesAccessRights_PLAYBACKONLY_)
  {
    auto shareClient = m_shareServiceClient->GetShareClient("myshare");
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient("myfile");
    auto fileHandles = fileClient.ListHandles().FileHandles;
    Files::Shares::Models::ShareFileHandleAccessRights allAccessRights
        = Files::Shares::Models::ShareFileHandleAccessRights::Read
        | Files::Shares::Models::ShareFileHandleAccessRights::Write
        | Files::Shares::Models::ShareFileHandleAccessRights::Delete;
    EXPECT_EQ(fileHandles.size(), 1L);
    EXPECT_TRUE(fileHandles[0].AccessRights.HasValue());
    EXPECT_EQ(allAccessRights, fileHandles[0].AccessRights.Value());
  }

  TEST_F(FileShareFileClientTest, ListHandlesWithClientName_PLAYBACKONLY_)
  {
    auto shareClient = m_shareServiceClient->GetShareClient("myshare");
    auto fileClient
        = shareClient.GetRootDirectoryClient().GetSubdirectoryClient("dir1").GetFileClient(
            "test.txt");
    auto fileHandles = fileClient.ListHandles().FileHandles;
    EXPECT_EQ(fileHandles.size(), 1L);
    EXPECT_FALSE(fileHandles[0].ClientName.empty());
  }

  TEST_F(FileShareFileClientTest, WithShareSnapshot)
  {
    const std::string timestamp1 = "2001-01-01T01:01:01.1111000Z";
    const std::string timestamp2 = "2022-02-02T02:02:02.2222000Z";

    auto client1 = m_fileClient->WithShareSnapshot(timestamp1);
    EXPECT_FALSE(client1.GetUrl().find("snapshot=" + timestamp1) == std::string::npos);
    EXPECT_TRUE(client1.GetUrl().find("snapshot=" + timestamp2) == std::string::npos);
    client1 = client1.WithShareSnapshot(timestamp2);
    EXPECT_TRUE(client1.GetUrl().find("snapshot=" + timestamp1) == std::string::npos);
    EXPECT_FALSE(client1.GetUrl().find("snapshot=" + timestamp2) == std::string::npos);
    client1 = client1.WithShareSnapshot("");
    EXPECT_TRUE(client1.GetUrl().find("snapshot=" + timestamp1) == std::string::npos);
    EXPECT_TRUE(client1.GetUrl().find("snapshot=" + timestamp2) == std::string::npos);
  }

  TEST_F(FileShareFileClientTest, Audience_PLAYBACKONLY_)
  {
    auto credential = GetTestCredential();
    auto clientOptions = InitStorageClientOptions<Files::Shares::ShareClientOptions>();
    clientOptions.ShareTokenIntent = Files::Shares::Models::ShareTokenIntent::Backup;

    // audience by default
    auto fileClient
        = Files::Shares::ShareFileClient(m_fileClient->GetUrl(), credential, clientOptions);
    EXPECT_NO_THROW(fileClient.GetProperties());

    // default audience
    clientOptions.Audience = Files::Shares::ShareAudience::DefaultAudience;
    fileClient = Files::Shares::ShareFileClient(m_fileClient->GetUrl(), credential, clientOptions);
    EXPECT_NO_THROW(fileClient.GetProperties());

    // service audience
    auto accountName = StandardStorageAccountName();
    clientOptions.Audience
        = Files::Shares::ShareAudience::CreateShareServiceAccountAudience(accountName);
    fileClient = Files::Shares::ShareFileClient(m_fileClient->GetUrl(), credential, clientOptions);
    EXPECT_NO_THROW(fileClient.GetProperties());

    // custom audience
    auto fileUrl = Azure::Core::Url(fileClient.GetUrl());
    clientOptions.Audience
        = Files::Shares::ShareAudience(fileUrl.GetScheme() + "://" + fileUrl.GetHost());
    fileClient = Files::Shares::ShareFileClient(m_fileClient->GetUrl(), credential, clientOptions);
    EXPECT_NO_THROW(fileClient.GetProperties());

    fileClient = Files::Shares::ShareServiceClient(
                     m_shareServiceClient->GetUrl(), credential, clientOptions)
                     .GetShareClient(m_shareName)
                     .GetRootDirectoryClient()
                     .GetSubdirectoryClient(m_directoryName)
                     .GetFileClient(m_fileName);
    EXPECT_NO_THROW(fileClient.GetProperties());

    // error audience
    clientOptions.Audience = Files::Shares::ShareAudience("https://disk.compute.azure.com");
    fileClient = Files::Shares::ShareFileClient(m_fileClient->GetUrl(), credential, clientOptions);
    EXPECT_THROW(fileClient.GetProperties(), StorageException);

    fileClient = Files::Shares::ShareServiceClient(
                     m_shareServiceClient->GetUrl(), credential, clientOptions)
                     .GetShareClient(m_shareName)
                     .GetRootDirectoryClient()
                     .GetSubdirectoryClient(m_directoryName)
                     .GetFileClient(m_fileName);
    EXPECT_THROW(fileClient.GetProperties(), StorageException);
  }

  TEST(ShareFileHandleAccessRightsTest, ShareFileHandleAccessRights)
  {
    Files::Shares::Models::ShareFileHandleAccessRights accessRightsA
        = Files::Shares::Models::ShareFileHandleAccessRights::Read
        | Files::Shares::Models::ShareFileHandleAccessRights::Write;
    Files::Shares::Models::ShareFileHandleAccessRights accessRightsB
        = Files::Shares::Models::ShareFileHandleAccessRights::Write
        | Files::Shares::Models::ShareFileHandleAccessRights::Delete;

    Files::Shares::Models::ShareFileHandleAccessRights orAccessRights
        = Files::Shares::Models::ShareFileHandleAccessRights::Read
        | Files::Shares::Models::ShareFileHandleAccessRights::Write
        | Files::Shares::Models::ShareFileHandleAccessRights::Delete;
    Files::Shares::Models::ShareFileHandleAccessRights andAccessRights
        = Files::Shares::Models::ShareFileHandleAccessRights::Write;
    Files::Shares::Models::ShareFileHandleAccessRights xorAccessRights
        = Files::Shares::Models::ShareFileHandleAccessRights::Read
        | Files::Shares::Models::ShareFileHandleAccessRights::Delete;

    EXPECT_EQ(orAccessRights, accessRightsA | accessRightsB);
    EXPECT_EQ(andAccessRights, accessRightsA & accessRightsB);
    EXPECT_EQ(xorAccessRights, accessRightsA ^ accessRightsB);
  }

  TEST_F(FileShareFileClientTest, FilePermissionFormat_PLAYBACKONLY_)
  {
    auto sddlPermission
        = "O:S-1-5-21-2127521184-1604012920-1887927527-21560751G:S-1-5-21-2127521184-1604012920-"
          "1887927527-513D:AI(A;;FA;;;SY)(A;;FA;;;BA)(A;;0x1200a9;;;S-1-5-21-397955417-626881126-"
          "188441444-3053964)S:NO_ACCESS_CONTROL";
    auto sddlPermissionNoControlFlag
        = "O:S-1-5-21-2127521184-1604012920-1887927527-21560751G:S-1-5-21-2127521184-1604012920-"
          "1887927527-513D:(A;;FA;;;SY)(A;;FA;;;BA)(A;;0x1200a9;;;S-1-5-21-397955417-626881126-"
          "188441444-3053964)";
    auto binaryPermission = "AQAUhGwAAACIAAAAAAAAABQAAAACAFgAAwAAAAAAFAD/"
                            "AR8AAQEAAAAAAAUSAAAAAAAYAP8BHwABAgAAAAAABSAAAAAgAgAAAAAkAKkAEgABBQAAAA"
                            "AABRUAAABZUbgXZnJdJWRjOwuMmS4AAQUAAAAAAAUVAAAAoGXPfnhLm1/nfIdwr/"
                            "1IAQEFAAAAAAAFFQAAAKBlz354S5tf53yHcAECAAA=";
    auto binaryPermissionNoControlFlag
        = "AQAEgGwAAACIAAAAAAAAABQAAAACAFgAAwAAAAAAFAD/"
          "AR8AAQEAAAAAAAUSAAAAAAAYAP8BHwABAgAAAAAABSAAAAAgAgAAAAAkAKkAEgABBQAAAAAABRUAAABZUbgXZnJd"
          "JWRjOwuMmS4AAQUAAAAAAAUVAAAAoGXPfnhLm1/nfIdwr/"
          "1IAQEFAAAAAAAFFQAAAKBlz354S5tf53yHcAECAAA=";
    // sddl format
    {
      auto permissionFormat = Files::Shares::Models::FilePermissionFormat::Sddl;
      auto fileClient
          = m_shareClient->GetRootDirectoryClient().GetFileClient(LowercaseRandomString());

      // Create
      Files::Shares::CreateFileOptions options;
      options.FilePermissionFormat = permissionFormat;
      options.Permission = sddlPermission;
      auto permissionKey = fileClient.Create(1, options).Value.SmbProperties.PermissionKey.Value();
      Files::Shares::GetSharePermissionOptions getOptions;
      getOptions.FilePermissionFormat = permissionFormat;
      auto permission = m_shareClient->GetPermission(permissionKey, getOptions).Value;
      EXPECT_EQ(sddlPermissionNoControlFlag, permission);

      // Set Properties
      Files::Shares::SetFilePropertiesOptions setOptions;
      setOptions.FilePermissionFormat = permissionFormat;
      setOptions.Permission = sddlPermission;
      fileClient.SetProperties(
          Files::Shares::Models::FileHttpHeaders(),
          Files::Shares::Models::FileSmbProperties(),
          setOptions);
      permissionKey = fileClient.GetProperties().Value.SmbProperties.PermissionKey.Value();
      permission = m_shareClient->GetPermission(permissionKey, getOptions).Value;
      EXPECT_EQ(sddlPermission, permission);

      // Upload From
      size_t fileSize = 512;
      std::vector<uint8_t> content(RandomBuffer(fileSize));
      auto memBodyStream = Core::IO::MemoryBodyStream(content);

      Files::Shares::UploadFileFromOptions uploadFromOptions;
      uploadFromOptions.FilePermission = sddlPermission;
      uploadFromOptions.FilePermissionFormat = permissionFormat;

      // UploadFrom buffer
      auto fileClient2
          = m_shareClient->GetRootDirectoryClient().GetFileClient(LowercaseRandomString());
      EXPECT_NO_THROW(fileClient2.UploadFrom(content.data(), fileSize, uploadFromOptions));
      permissionKey = fileClient2.GetProperties().Value.SmbProperties.PermissionKey.Value();
      permission = m_shareClient->GetPermission(permissionKey, getOptions).Value;
      EXPECT_EQ(sddlPermissionNoControlFlag, permission);

      // UploadFrom file
      auto fileClient3
          = m_shareClient->GetRootDirectoryClient().GetFileClient(LowercaseRandomString());
      const std::string tempFilename = "file" + RandomString();
      WriteFile(tempFilename, content);
      EXPECT_NO_THROW(fileClient3.UploadFrom(tempFilename, uploadFromOptions));
      permissionKey = fileClient3.GetProperties().Value.SmbProperties.PermissionKey.Value();
      permission = m_shareClient->GetPermission(permissionKey, getOptions).Value;
      EXPECT_EQ(sddlPermissionNoControlFlag, permission);
    }
    // binary format
    {
      auto permissionFormat = Files::Shares::Models::FilePermissionFormat::Binary;
      auto fileClient = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString());

      // Create
      Files::Shares::CreateFileOptions options;
      options.FilePermissionFormat = permissionFormat;
      options.Permission = binaryPermission;
      auto permissionKey = fileClient.Create(1, options).Value.SmbProperties.PermissionKey.Value();
      Files::Shares::GetSharePermissionOptions getOptions;
      getOptions.FilePermissionFormat = permissionFormat;
      auto permission = m_shareClient->GetPermission(permissionKey, getOptions).Value;
      EXPECT_EQ(binaryPermissionNoControlFlag, permission);

      // Set Properties
      Files::Shares::SetFilePropertiesOptions setOptions;
      setOptions.FilePermissionFormat = permissionFormat;
      setOptions.Permission = binaryPermission;
      fileClient.SetProperties(
          Files::Shares::Models::FileHttpHeaders(),
          Files::Shares::Models::FileSmbProperties(),
          setOptions);
      permissionKey = fileClient.GetProperties().Value.SmbProperties.PermissionKey.Value();
      permission = m_shareClient->GetPermission(permissionKey, getOptions).Value;
      EXPECT_EQ(binaryPermission, permission);

      // Upload From
      size_t fileSize = 512;
      std::vector<uint8_t> content(RandomBuffer(fileSize));
      auto memBodyStream = Core::IO::MemoryBodyStream(content);

      Files::Shares::UploadFileFromOptions uploadFromOptions;
      uploadFromOptions.FilePermission = binaryPermission;
      uploadFromOptions.FilePermissionFormat = permissionFormat;

      // UploadFrom buffer
      auto fileClient2
          = m_shareClient->GetRootDirectoryClient().GetFileClient(LowercaseRandomString());
      EXPECT_NO_THROW(fileClient2.UploadFrom(content.data(), fileSize, uploadFromOptions));
      permissionKey = fileClient2.GetProperties().Value.SmbProperties.PermissionKey.Value();
      permission = m_shareClient->GetPermission(permissionKey, getOptions).Value;
      EXPECT_EQ(binaryPermissionNoControlFlag, permission);

      // UploadFrom file
      auto fileClient3
          = m_shareClient->GetRootDirectoryClient().GetFileClient(LowercaseRandomString());
      const std::string tempFilename = "file" + RandomString();
      WriteFile(tempFilename, content);
      EXPECT_NO_THROW(fileClient3.UploadFrom(tempFilename, uploadFromOptions));
      permissionKey = fileClient3.GetProperties().Value.SmbProperties.PermissionKey.Value();
      permission = m_shareClient->GetPermission(permissionKey, getOptions).Value;
      EXPECT_EQ(binaryPermissionNoControlFlag, permission);
    }
  }

  TEST_F(FileShareFileClientTest, FilePermissionFormatForCopy_PLAYBACKONLY_)
  {
    auto sddlPermission
        = "O:S-1-5-21-2127521184-1604012920-1887927527-21560751G:S-1-5-21-2127521184-1604012920-"
          "1887927527-513D:AI(A;;FA;;;SY)(A;;FA;;;BA)(A;;0x1200a9;;;S-1-5-21-397955417-626881126-"
          "188441444-3053964)S:NO_ACCESS_CONTROL";
    auto sddlPermissionNoControlFlag
        = "O:S-1-5-21-2127521184-1604012920-1887927527-21560751G:S-1-5-21-2127521184-1604012920-"
          "1887927527-513D:(A;;FA;;;SY)(A;;FA;;;BA)(A;;0x1200a9;;;S-1-5-21-397955417-626881126-"
          "188441444-3053964)";
    auto binaryPermission
        = "AQAEgIgAAACUAAAAAAAAABQAAAACAHQABQAAAAAAGAD/AR8AAQIAAAAAAAUgAAAAIAIAAAAAFAD/"
          "AR8AAQEAAAAAAAUSAAAAAAAYAKkAEgABAgAAAAAABSAAAAAhAgAAAAAUAL8BEwABAQAAAAAABQsAAAAAABQA/"
          "wEfAAEBAAAAAAAFEgAAAAEBAAAAAAAFEgAAAAEBAAAAAAAFEgAAAA==";
    auto binaryPermissionNoControlFlag
        = "AQAEgGwAAACIAAAAAAAAABQAAAACAFgAAwAAAAAAFAD/"
          "AR8AAQEAAAAAAAUSAAAAAAAYAP8BHwABAgAAAAAABSAAAAAgAgAAAAAkAKkAEgABBQAAAAAABRUAAABZUbgXZnJd"
          "JWRjOwuMmS4AAQUAAAAAAAUVAAAAoGXPfnhLm1/nfIdwr/"
          "1IAQEFAAAAAAAFFQAAAKBlz354S5tf53yHcAECAAA=";

    size_t fileSize = 128;
    // source client
    auto sourceClient = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString() + "1");
    sourceClient.Create(fileSize);

    // sddl format
    {
      auto permissionFormat = Files::Shares::Models::FilePermissionFormat::Sddl;
      auto fileClient
          = m_shareClient->GetRootDirectoryClient().GetFileClient(LowercaseRandomString());

      Files::Shares::StartFileCopyOptions options;
      options.FilePermissionFormat = permissionFormat;
      options.Permission = sddlPermission;
      options.PermissionCopyMode = Files::Shares::Models::PermissionCopyMode::Override;
      auto copyOperation = fileClient.StartCopy(sourceClient.GetUrl(), options);
      EXPECT_EQ(
          copyOperation.GetRawResponse().GetStatusCode(),
          Azure::Core::Http::HttpStatusCode::Accepted);
      auto fileProperties = copyOperation.PollUntilDone(std::chrono::milliseconds(1000)).Value;
      EXPECT_EQ(fileProperties.CopyStatus.Value(), Files::Shares::Models::CopyStatus::Success);
      auto permissionKey = fileProperties.SmbProperties.PermissionKey.Value();
      Files::Shares::GetSharePermissionOptions getOptions;
      getOptions.FilePermissionFormat = permissionFormat;
      auto permission = m_shareClient->GetPermission(permissionKey, getOptions).Value;
      EXPECT_EQ(sddlPermissionNoControlFlag, permission);
    }
    // binary format
    {
      auto permissionFormat = Files::Shares::Models::FilePermissionFormat::Binary;
      auto fileClient = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString());

      Files::Shares::StartFileCopyOptions options;
      options.FilePermissionFormat = permissionFormat;
      options.Permission = binaryPermissionNoControlFlag;
      options.PermissionCopyMode = Files::Shares::Models::PermissionCopyMode::Override;
      auto copyOperation = fileClient.StartCopy(sourceClient.GetUrl(), options);
      EXPECT_EQ(
          copyOperation.GetRawResponse().GetStatusCode(),
          Azure::Core::Http::HttpStatusCode::Accepted);
      auto fileProperties = copyOperation.PollUntilDone(std::chrono::milliseconds(1000)).Value;
      EXPECT_EQ(fileProperties.CopyStatus.Value(), Files::Shares::Models::CopyStatus::Success);
      auto permissionKey = fileProperties.SmbProperties.PermissionKey.Value();
      Files::Shares::GetSharePermissionOptions getOptions;
      getOptions.FilePermissionFormat = permissionFormat;
      auto permission = m_shareClient->GetPermission(permissionKey, getOptions).Value;
      EXPECT_EQ(binaryPermissionNoControlFlag, permission);
    }

    // source mode
    std::string permissionKey;
    {
      auto permissionFormat = Files::Shares::Models::FilePermissionFormat::Binary;
      auto fileClient = m_shareClient->GetRootDirectoryClient().GetFileClient(RandomString());

      Files::Shares::StartFileCopyOptions options;
      options.FilePermissionFormat = permissionFormat;
      // Permission and PermissionFormat are ignored when PermissionCopyMode is Source.
      options.Permission = binaryPermission;
      options.PermissionCopyMode = Files::Shares::Models::PermissionCopyMode::Source;
      auto copyOperation = fileClient.StartCopy(sourceClient.GetUrl(), options);
      EXPECT_EQ(
          copyOperation.GetRawResponse().GetStatusCode(),
          Azure::Core::Http::HttpStatusCode::Accepted);
      auto fileProperties = copyOperation.PollUntilDone(std::chrono::milliseconds(1000)).Value;
      EXPECT_EQ(fileProperties.CopyStatus.Value(), Files::Shares::Models::CopyStatus::Success);
      permissionKey = fileProperties.SmbProperties.PermissionKey.Value();
      Files::Shares::GetSharePermissionOptions getOptions;
      getOptions.FilePermissionFormat = permissionFormat;
      auto permission = m_shareClient->GetPermission(permissionKey, getOptions).Value;
      EXPECT_EQ(binaryPermission, permission);
    }
  }

  TEST_F(FileShareFileClientTest, PremiumPosixProperties_PLAYBACKONLY_)
  {
    auto shareServiceClient = *m_premiumShareServiceClient;

    auto shareName = LowercaseRandomString();
    auto shareClient = GetPremiumShareClientForTest(shareName);
    Files::Shares::CreateShareOptions shareOptions;
    shareOptions.EnabledProtocols = Files::Shares::Models::ShareProtocols::Nfs;
    EXPECT_NO_THROW(shareClient.Create(shareOptions));
    auto otherProperties = m_fileClient->GetProperties().Value;

    auto fileName = LowercaseRandomString();
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);

    std::string octalMode = "0777";

    // Create a file
    Files::Shares::CreateFileOptions createOptions;
    createOptions.PosixProperties.FileMode
        = Files::Shares::Models::NfsFileMode::ParseOctalFileMode(octalMode);
    createOptions.PosixProperties.Group = "123";
    createOptions.PosixProperties.Owner = "456";
    createOptions.PosixProperties.NfsFileType = Files::Shares::Models::NfsFileType::Regular;
    Files::Shares::Models::CreateFileResult createResult;
    EXPECT_NO_THROW(createResult = fileClient.Create(256, createOptions).Value);
    EXPECT_TRUE(createResult.PosixProperties.FileMode.HasValue());
    EXPECT_EQ(createResult.PosixProperties.FileMode.Value().ToOctalFileMode(), octalMode);
    EXPECT_TRUE(createResult.PosixProperties.Group.HasValue());
    EXPECT_EQ(
        createResult.PosixProperties.Group.Value(), createOptions.PosixProperties.Group.Value());
    EXPECT_TRUE(createResult.PosixProperties.FileMode.HasValue());
    EXPECT_EQ(
        createResult.PosixProperties.Owner.Value(), createOptions.PosixProperties.Owner.Value());
    EXPECT_TRUE(createResult.PosixProperties.NfsFileType.HasValue());
    EXPECT_EQ(
        createResult.PosixProperties.NfsFileType.Value(),
        Files::Shares::Models::NfsFileType::Regular);

    // Set Properties
    Files::Shares::SetFilePropertiesOptions setOptions;
    setOptions.PosixProperties.FileMode
        = Files::Shares::Models::NfsFileMode::ParseOctalFileMode("0707");
    setOptions.PosixProperties.Group = "123";
    setOptions.PosixProperties.Owner = "456";
    Files::Shares::Models::SetFilePropertiesResult setResult;
    EXPECT_NO_THROW(
        setResult = fileClient
                        .SetProperties(
                            Files::Shares::Models::FileHttpHeaders(),
                            Files::Shares::Models::FileSmbProperties(),
                            setOptions)
                        .Value);
    EXPECT_TRUE(setResult.PosixProperties.FileMode.HasValue());
    EXPECT_EQ(setResult.PosixProperties.FileMode.Value().ToOctalFileMode(), "0707");
    EXPECT_TRUE(setResult.PosixProperties.Group.HasValue());
    EXPECT_EQ(setResult.PosixProperties.Group.Value(), setOptions.PosixProperties.Group.Value());
    EXPECT_TRUE(setResult.PosixProperties.Owner.HasValue());
    EXPECT_EQ(setResult.PosixProperties.Owner.Value(), setOptions.PosixProperties.Owner.Value());
    EXPECT_TRUE(setResult.PosixProperties.LinkCount.HasValue());

    // Get Properties
    Files::Shares::Models::FileProperties properties;
    EXPECT_NO_THROW(properties = fileClient.GetProperties().Value);
    EXPECT_TRUE(properties.PosixProperties.FileMode.HasValue());
    EXPECT_EQ(properties.PosixProperties.FileMode.Value().ToOctalFileMode(), "0707");
    EXPECT_TRUE(properties.PosixProperties.Group.HasValue());
    EXPECT_EQ(properties.PosixProperties.Group.Value(), setOptions.PosixProperties.Group.Value());
    EXPECT_TRUE(properties.PosixProperties.Owner.HasValue());
    EXPECT_EQ(properties.PosixProperties.Owner.Value(), setOptions.PosixProperties.Owner.Value());
    EXPECT_TRUE(properties.PosixProperties.LinkCount.HasValue());
    EXPECT_TRUE(properties.PosixProperties.NfsFileType.HasValue());
    EXPECT_EQ(
        properties.PosixProperties.NfsFileType.Value(),
        Files::Shares::Models::NfsFileType::Regular);

    // Download
    Files::Shares::Models::DownloadFileResult downloadResult;
    EXPECT_NO_THROW(downloadResult = fileClient.Download().Value);
    EXPECT_TRUE(downloadResult.Details.PosixProperties.FileMode.HasValue());
    EXPECT_EQ(downloadResult.Details.PosixProperties.FileMode.Value().ToOctalFileMode(), "0707");
    EXPECT_TRUE(downloadResult.Details.PosixProperties.Group.HasValue());
    EXPECT_EQ(
        downloadResult.Details.PosixProperties.Group.Value(),
        setOptions.PosixProperties.Group.Value());
    EXPECT_TRUE(downloadResult.Details.PosixProperties.Owner.HasValue());
    EXPECT_EQ(
        downloadResult.Details.PosixProperties.Owner.Value(),
        setOptions.PosixProperties.Owner.Value());
    EXPECT_TRUE(downloadResult.Details.PosixProperties.LinkCount.HasValue());

    // DownloadTo
    Files::Shares::Models::DownloadFileToResult downloadToResult;
    std::string tempFilename = RandomString() + "1";
    EXPECT_NO_THROW(downloadToResult = fileClient.DownloadTo(tempFilename).Value);
    EXPECT_TRUE(downloadToResult.Details.PosixProperties.FileMode.HasValue());
    EXPECT_EQ(downloadToResult.Details.PosixProperties.FileMode.Value().ToOctalFileMode(), "0707");
    EXPECT_TRUE(downloadToResult.Details.PosixProperties.Group.HasValue());
    EXPECT_EQ(
        downloadToResult.Details.PosixProperties.Group.Value(),
        setOptions.PosixProperties.Group.Value());
    EXPECT_TRUE(downloadToResult.Details.PosixProperties.Owner.HasValue());
    EXPECT_EQ(
        downloadToResult.Details.PosixProperties.Owner.Value(),
        setOptions.PosixProperties.Owner.Value());
    EXPECT_TRUE(downloadToResult.Details.PosixProperties.LinkCount.HasValue());
    DeleteFile(tempFilename);

    std::vector<uint8_t> buff(256);
    EXPECT_NO_THROW(downloadToResult = fileClient.DownloadTo(buff.data(), 256).Value);
    EXPECT_TRUE(downloadToResult.Details.PosixProperties.FileMode.HasValue());
    EXPECT_EQ(downloadToResult.Details.PosixProperties.FileMode.Value().ToOctalFileMode(), "0707");
    EXPECT_TRUE(downloadToResult.Details.PosixProperties.Group.HasValue());
    EXPECT_EQ(
        downloadToResult.Details.PosixProperties.Group.Value(),
        setOptions.PosixProperties.Group.Value());
    EXPECT_TRUE(downloadToResult.Details.PosixProperties.Owner.HasValue());
    EXPECT_EQ(
        downloadToResult.Details.PosixProperties.Owner.Value(),
        setOptions.PosixProperties.Owner.Value());
    EXPECT_TRUE(downloadToResult.Details.PosixProperties.LinkCount.HasValue());

    // Create SymbolicLink
    std::string sourceUrl = fileClient.GetUrl();
    auto symbolicLinkClient
        = shareClient.GetRootDirectoryClient().GetFileClient(LowercaseRandomString());
    Files::Shares::CreateSymbolicLinkOptions createSymbolicLinkOptions;
    createSymbolicLinkOptions.CreatedOn = otherProperties.SmbProperties.CreatedOn;
    createSymbolicLinkOptions.LastWrittenOn = otherProperties.SmbProperties.LastWrittenOn;

    createSymbolicLinkOptions.Metadata = RandomMetadata();
    createSymbolicLinkOptions.Group = "123";
    createSymbolicLinkOptions.Owner = "456";
    Files::Shares::Models::CreateFileSymbolicLinkResult createSymbolicLinkResult;
    EXPECT_NO_THROW(
        createSymbolicLinkResult
        = symbolicLinkClient.CreateSymbolicLink(sourceUrl, createSymbolicLinkOptions).Value);
    EXPECT_TRUE(createSymbolicLinkResult.PosixProperties.FileMode.HasValue());
    EXPECT_EQ(
        createSymbolicLinkResult.PosixProperties.FileMode.Value().ToOctalFileMode(), octalMode);
    EXPECT_TRUE(createSymbolicLinkResult.PosixProperties.Group.HasValue());
    EXPECT_EQ(
        createSymbolicLinkResult.PosixProperties.Group.Value(),
        createSymbolicLinkOptions.Group.Value());
    EXPECT_TRUE(createSymbolicLinkResult.PosixProperties.FileMode.HasValue());
    EXPECT_EQ(
        createSymbolicLinkResult.PosixProperties.Owner.Value(),
        createSymbolicLinkOptions.Owner.Value());
    EXPECT_TRUE(createSymbolicLinkResult.PosixProperties.NfsFileType.HasValue());
    EXPECT_EQ(
        createSymbolicLinkResult.PosixProperties.NfsFileType.Value(),
        Files::Shares::Models::NfsFileType::SymLink);
    EXPECT_EQ(
        createSymbolicLinkResult.SmbProperties.CreatedOn.Value(),
        createSymbolicLinkOptions.CreatedOn.Value());
    EXPECT_EQ(
        createSymbolicLinkResult.SmbProperties.LastWrittenOn.Value(),
        createSymbolicLinkOptions.LastWrittenOn.Value());
    EXPECT_TRUE(createSymbolicLinkResult.SmbProperties.ChangedOn.HasValue());
    EXPECT_TRUE(!createSymbolicLinkResult.SmbProperties.FileId.empty());
    EXPECT_TRUE(!createSymbolicLinkResult.SmbProperties.ParentFileId.empty());
    EXPECT_TRUE(createSymbolicLinkResult.ETag.HasValue());

    // Get SymbolicLink
    Files::Shares::Models::GetFileSymbolicLinkResult getSymbolicLinkResult;
    EXPECT_NO_THROW(getSymbolicLinkResult = symbolicLinkClient.GetSymbolicLink().Value);
    EXPECT_TRUE(getSymbolicLinkResult.ETag.HasValue());
    if (!m_testContext.IsPlaybackMode())
    {
      EXPECT_EQ(Azure::Core::Url::Decode(getSymbolicLinkResult.LinkText), sourceUrl);
    }

    // Create HardLink
    auto hardLinkClient
        = shareClient.GetRootDirectoryClient().GetFileClient(LowercaseRandomString());
    Files::Shares::CreateHardLinkOptions createHardLinkOptions;
    Files::Shares::Models::CreateFileHardLinkResult createFileHardLinkResult;
    EXPECT_NO_THROW(
        createFileHardLinkResult
        = hardLinkClient.CreateHardLink(fileName, createHardLinkOptions).Value);
    EXPECT_TRUE(createFileHardLinkResult.PosixProperties.FileMode.HasValue());
    EXPECT_EQ(createFileHardLinkResult.PosixProperties.FileMode.Value().ToOctalFileMode(), "0707");
    EXPECT_TRUE(createFileHardLinkResult.PosixProperties.Group.HasValue());
    EXPECT_EQ(
        createFileHardLinkResult.PosixProperties.Group.Value(),
        createOptions.PosixProperties.Group.Value());
    EXPECT_TRUE(createFileHardLinkResult.PosixProperties.FileMode.HasValue());
    EXPECT_EQ(
        createFileHardLinkResult.PosixProperties.Owner.Value(),
        createOptions.PosixProperties.Owner.Value());
    EXPECT_TRUE(createFileHardLinkResult.PosixProperties.NfsFileType.HasValue());
    EXPECT_EQ(
        createFileHardLinkResult.PosixProperties.NfsFileType.Value(),
        Files::Shares::Models::NfsFileType::Regular);
    EXPECT_TRUE(createFileHardLinkResult.SmbProperties.CreatedOn.HasValue());
    EXPECT_TRUE(createFileHardLinkResult.SmbProperties.LastWrittenOn.HasValue());
    EXPECT_TRUE(createFileHardLinkResult.SmbProperties.ChangedOn.HasValue());
    EXPECT_TRUE(!createFileHardLinkResult.SmbProperties.FileId.empty());
    EXPECT_TRUE(!createFileHardLinkResult.SmbProperties.ParentFileId.empty());
    EXPECT_TRUE(createFileHardLinkResult.ETag.HasValue());

    // Delete
    Files::Shares::Models::DeleteFileResult deleteResult;
    EXPECT_NO_THROW(deleteResult = fileClient.Delete().Value);
    EXPECT_TRUE(deleteResult.LinkCount.HasValue());

    // Upload
    size_t fileSize = 512;
    std::vector<uint8_t> content(RandomBuffer(fileSize));
    auto memBodyStream = Core::IO::MemoryBodyStream(content);
    tempFilename = "file" + RandomString();
    WriteFile(tempFilename, content);

    Files::Shares::UploadFileFromOptions uploadOptions;
    uploadOptions.PosixProperties.FileMode
        = Files::Shares::Models::NfsFileMode::ParseOctalFileMode(octalMode);
    uploadOptions.PosixProperties.Group = "123";
    uploadOptions.PosixProperties.Owner = "456";
    uploadOptions.PosixProperties.NfsFileType = Files::Shares::Models::NfsFileType::Regular;

    // From buffer
    fileClient = shareClient.GetRootDirectoryClient().GetFileClient(LowercaseRandomString());
    EXPECT_NO_THROW(fileClient.UploadFrom(content.data(), fileSize, uploadOptions));
    properties = fileClient.GetProperties().Value;
    EXPECT_TRUE(properties.PosixProperties.FileMode.HasValue());
    EXPECT_EQ(properties.PosixProperties.FileMode.Value().ToOctalFileMode(), octalMode);
    EXPECT_TRUE(properties.PosixProperties.Group.HasValue());
    EXPECT_EQ(
        properties.PosixProperties.Group.Value(), uploadOptions.PosixProperties.Group.Value());
    EXPECT_TRUE(properties.PosixProperties.Owner.HasValue());
    EXPECT_EQ(
        properties.PosixProperties.Owner.Value(), uploadOptions.PosixProperties.Owner.Value());
    EXPECT_TRUE(properties.PosixProperties.LinkCount.HasValue());
    EXPECT_TRUE(properties.PosixProperties.NfsFileType.HasValue());
    EXPECT_EQ(
        properties.PosixProperties.NfsFileType.Value(),
        Files::Shares::Models::NfsFileType::Regular);

    // From file
    fileClient = shareClient.GetRootDirectoryClient().GetFileClient(LowercaseRandomString());
    EXPECT_NO_THROW(fileClient.UploadFrom(tempFilename, uploadOptions));
    properties = fileClient.GetProperties().Value;
    EXPECT_TRUE(properties.PosixProperties.FileMode.HasValue());
    EXPECT_EQ(properties.PosixProperties.FileMode.Value().ToOctalFileMode(), octalMode);
    EXPECT_TRUE(properties.PosixProperties.Group.HasValue());
    EXPECT_EQ(
        properties.PosixProperties.Group.Value(), uploadOptions.PosixProperties.Group.Value());
    EXPECT_TRUE(properties.PosixProperties.Owner.HasValue());
    EXPECT_EQ(
        properties.PosixProperties.Owner.Value(), uploadOptions.PosixProperties.Owner.Value());
    EXPECT_TRUE(properties.PosixProperties.LinkCount.HasValue());
    EXPECT_TRUE(properties.PosixProperties.NfsFileType.HasValue());
    EXPECT_EQ(
        properties.PosixProperties.NfsFileType.Value(),
        Files::Shares::Models::NfsFileType::Regular);
  }

  TEST_F(FileShareFileClientTest, PremiumPosixPropertiesForCopy_PLAYBACKONLY_)
  {
    auto shareServiceClient = *m_premiumShareServiceClient;

    auto shareName = LowercaseRandomString();
    auto shareClient = GetPremiumShareClientForTest(shareName);
    Files::Shares::CreateShareOptions shareOptions;
    shareOptions.EnabledProtocols = Files::Shares::Models::ShareProtocols::Nfs;
    EXPECT_NO_THROW(shareClient.Create(shareOptions));

    auto sourceName = LowercaseRandomString();
    auto sourceClient = shareClient.GetRootDirectoryClient().GetFileClient(sourceName);

    std::string sourceMode = "0777";

    // Create a file
    Files::Shares::CreateFileOptions createOptions;
    createOptions.PosixProperties.FileMode
        = Files::Shares::Models::NfsFileMode::ParseOctalFileMode(sourceMode);
    createOptions.PosixProperties.Group = "123";
    createOptions.PosixProperties.Owner = "456";
    createOptions.PosixProperties.NfsFileType = Files::Shares::Models::NfsFileType::Regular;
    EXPECT_NO_THROW(sourceClient.Create(256, createOptions));

    // Copy with override
    auto destFileClient
        = shareClient.GetRootDirectoryClient().GetFileClient(LowercaseRandomString());
    Files::Shares::StartFileCopyOptions copyOptions;
    copyOptions.SmbPropertiesToCopy = Files::Shares::CopyableFileSmbPropertyFlags::None;
    copyOptions.PosixProperties.FileMode
        = Files::Shares::Models::NfsFileMode::ParseOctalFileMode("0757");
    copyOptions.ModeCopyMode = Files::Shares::Models::ModeCopyMode::Override;
    copyOptions.PosixProperties.Group = "888";
    copyOptions.PosixProperties.Owner = "999";
    copyOptions.OwnerCopyMode = Files::Shares::Models::OwnerCopyMode::Override;
    auto copyOperation = destFileClient.StartCopy(sourceClient.GetUrl(), copyOptions);
    EXPECT_EQ(
        copyOperation.GetRawResponse().GetStatusCode(),
        Azure::Core::Http::HttpStatusCode::Accepted);
    auto properties = copyOperation.PollUntilDone(std::chrono::milliseconds(1000)).Value;
    EXPECT_EQ(properties.CopyStatus.Value(), Files::Shares::Models::CopyStatus::Success);
    EXPECT_TRUE(properties.PosixProperties.FileMode.HasValue());
    EXPECT_EQ(properties.PosixProperties.FileMode.Value().ToOctalFileMode(), "0757");
    EXPECT_TRUE(properties.PosixProperties.Group.HasValue());
    EXPECT_EQ(properties.PosixProperties.Group.Value(), copyOptions.PosixProperties.Group.Value());
    EXPECT_TRUE(properties.PosixProperties.Owner.HasValue());
    EXPECT_EQ(properties.PosixProperties.Owner.Value(), copyOptions.PosixProperties.Owner.Value());

    // Copy with source
    copyOptions = Files::Shares::StartFileCopyOptions();
    copyOptions.SmbPropertiesToCopy = Files::Shares::CopyableFileSmbPropertyFlags::None;
    copyOptions.ModeCopyMode = Files::Shares::Models::ModeCopyMode::Source;
    copyOptions.OwnerCopyMode = Files::Shares::Models::OwnerCopyMode::Source;
    destFileClient = shareClient.GetRootDirectoryClient().GetFileClient(LowercaseRandomString());
    copyOperation = destFileClient.StartCopy(sourceClient.GetUrl(), copyOptions);
    EXPECT_EQ(
        copyOperation.GetRawResponse().GetStatusCode(),
        Azure::Core::Http::HttpStatusCode::Accepted);
    properties = copyOperation.PollUntilDone(std::chrono::milliseconds(1000)).Value;
    EXPECT_EQ(properties.CopyStatus.Value(), Files::Shares::Models::CopyStatus::Success);
    EXPECT_TRUE(properties.PosixProperties.FileMode.HasValue());
    EXPECT_EQ(properties.PosixProperties.FileMode.Value().ToOctalFileMode(), sourceMode);
    EXPECT_TRUE(properties.PosixProperties.Group.HasValue());
    EXPECT_EQ(
        properties.PosixProperties.Group.Value(), createOptions.PosixProperties.Group.Value());
    EXPECT_TRUE(properties.PosixProperties.Owner.HasValue());
    EXPECT_EQ(
        properties.PosixProperties.Owner.Value(), createOptions.PosixProperties.Owner.Value());

    // Copy with source/override
    copyOptions = Files::Shares::StartFileCopyOptions();
    copyOptions.SmbPropertiesToCopy = Files::Shares::CopyableFileSmbPropertyFlags::None;
    copyOptions.ModeCopyMode = Files::Shares::Models::ModeCopyMode::Override;
    copyOptions.PosixProperties.FileMode
        = Files::Shares::Models::NfsFileMode::ParseOctalFileMode("0767");
    copyOptions.OwnerCopyMode = Files::Shares::Models::OwnerCopyMode::Source;
    destFileClient = shareClient.GetRootDirectoryClient().GetFileClient(LowercaseRandomString());
    copyOperation = destFileClient.StartCopy(sourceClient.GetUrl(), copyOptions);
    EXPECT_EQ(
        copyOperation.GetRawResponse().GetStatusCode(),
        Azure::Core::Http::HttpStatusCode::Accepted);
    properties = copyOperation.PollUntilDone(std::chrono::milliseconds(1000)).Value;
    EXPECT_EQ(properties.CopyStatus.Value(), Files::Shares::Models::CopyStatus::Success);
    EXPECT_TRUE(properties.PosixProperties.FileMode.HasValue());
    EXPECT_EQ(properties.PosixProperties.FileMode.Value().ToOctalFileMode(), "0767");
    EXPECT_TRUE(properties.PosixProperties.Group.HasValue());
    EXPECT_EQ(
        properties.PosixProperties.Group.Value(), createOptions.PosixProperties.Group.Value());
    EXPECT_TRUE(properties.PosixProperties.Owner.HasValue());
    EXPECT_EQ(
        properties.PosixProperties.Owner.Value(), createOptions.PosixProperties.Owner.Value());

    // Copy without PosixProperties
    copyOptions = Files::Shares::StartFileCopyOptions();
    copyOptions.SmbPropertiesToCopy = Files::Shares::CopyableFileSmbPropertyFlags::None;
    destFileClient = shareClient.GetRootDirectoryClient().GetFileClient(LowercaseRandomString());
    copyOperation = destFileClient.StartCopy(sourceClient.GetUrl(), copyOptions);
    EXPECT_EQ(
        copyOperation.GetRawResponse().GetStatusCode(),
        Azure::Core::Http::HttpStatusCode::Accepted);
    properties = copyOperation.PollUntilDone(std::chrono::milliseconds(1000)).Value;
    EXPECT_EQ(properties.CopyStatus.Value(), Files::Shares::Models::CopyStatus::Success);
    EXPECT_TRUE(properties.PosixProperties.FileMode.HasValue());
    EXPECT_EQ(properties.PosixProperties.FileMode.Value().ToOctalFileMode(), "0664");
    EXPECT_TRUE(properties.PosixProperties.Group.HasValue());
    EXPECT_EQ(properties.PosixProperties.Group.Value(), "0");
    EXPECT_TRUE(properties.PosixProperties.Owner.HasValue());
    EXPECT_EQ(properties.PosixProperties.Owner.Value(), "0");

    // Copy with invalid input
    copyOptions = Files::Shares::StartFileCopyOptions();
    copyOptions.SmbPropertiesToCopy = Files::Shares::CopyableFileSmbPropertyFlags::None;
    copyOptions.ModeCopyMode = Files::Shares::Models::ModeCopyMode::Source;
    copyOptions.PosixProperties.FileMode
        = Files::Shares::Models::NfsFileMode::ParseOctalFileMode("0767");
    copyOptions.OwnerCopyMode = Files::Shares::Models::OwnerCopyMode::Source;
    destFileClient = shareClient.GetRootDirectoryClient().GetFileClient(LowercaseRandomString());
    EXPECT_THROW(destFileClient.StartCopy(sourceClient.GetUrl(), copyOptions), StorageException);

    copyOptions = Files::Shares::StartFileCopyOptions();
    copyOptions.SmbPropertiesToCopy = Files::Shares::CopyableFileSmbPropertyFlags::None;
    copyOptions.ModeCopyMode = Files::Shares::Models::ModeCopyMode::Override;
    copyOptions.OwnerCopyMode = Files::Shares::Models::OwnerCopyMode::Source;
    destFileClient = shareClient.GetRootDirectoryClient().GetFileClient(LowercaseRandomString());
    EXPECT_THROW(destFileClient.StartCopy(sourceClient.GetUrl(), copyOptions), StorageException);

    copyOptions = Files::Shares::StartFileCopyOptions();
    copyOptions.SmbPropertiesToCopy = Files::Shares::CopyableFileSmbPropertyFlags::None;
    copyOptions.ModeCopyMode = Files::Shares::Models::ModeCopyMode::Source;
    copyOptions.OwnerCopyMode = Files::Shares::Models::OwnerCopyMode::Override;
    destFileClient = shareClient.GetRootDirectoryClient().GetFileClient(LowercaseRandomString());
    EXPECT_THROW(destFileClient.StartCopy(sourceClient.GetUrl(), copyOptions), StorageException);

    copyOptions = Files::Shares::StartFileCopyOptions();
    copyOptions.SmbPropertiesToCopy = Files::Shares::CopyableFileSmbPropertyFlags::None;
    copyOptions.ModeCopyMode = Files::Shares::Models::ModeCopyMode::Source;
    copyOptions.OwnerCopyMode = Files::Shares::Models::OwnerCopyMode::Source;
    copyOptions.PosixProperties.Group = "888";
    destFileClient = shareClient.GetRootDirectoryClient().GetFileClient(LowercaseRandomString());
    EXPECT_THROW(destFileClient.StartCopy(sourceClient.GetUrl(), copyOptions), StorageException);
  }

  TEST_F(FileShareFileClientTest, StructuredMessageTest)
  {
    const size_t contentSize = 2 * 1024 + 512;
    auto content = RandomBuffer(contentSize);
    auto bodyStream = Azure::Core::IO::MemoryBodyStream(content.data(), content.size());
    const std::string tempFileName = RandomString();
    WriteFile(tempFileName, content);
    Files::Shares::TransferValidationOptions validationOptions;
    validationOptions.Algorithm = StorageChecksumAlgorithm::Crc64;

    // UploadRange
    auto fileClient = m_fileShareDirectoryClient->GetFileClient("uploadrange_" + RandomString());
    fileClient.Create(contentSize);
    Files::Shares::UploadFileRangeOptions uploadRangeOptions;
    uploadRangeOptions.ValidationOptions = validationOptions;
    Files::Shares::Models::UploadFileRangeResult uploadRangeResult;
    EXPECT_NO_THROW(
        uploadRangeResult = fileClient.UploadRange(0, bodyStream, uploadRangeOptions).Value);
    EXPECT_TRUE(uploadRangeResult.StructuredBodyType.HasValue());

    // Download
    Files::Shares::DownloadFileOptions downloadOptions;
    downloadOptions.ValidationOptions = validationOptions;
    Files::Shares::Models::DownloadFileResult downloadResult;
    EXPECT_NO_THROW(downloadResult = fileClient.Download(downloadOptions).Value);
    auto downloadedData = downloadResult.BodyStream->ReadToEnd();
    EXPECT_EQ(content, downloadedData);
    EXPECT_TRUE(downloadResult.StructuredContentLength.HasValue());
    EXPECT_EQ(downloadResult.StructuredContentLength.Value(), contentSize);
    EXPECT_TRUE(downloadResult.StructuredBodyType.HasValue());
    EXPECT_EQ(downloadResult.FileSize, contentSize);
    // partial download
    downloadOptions.Range = Core::Http::HttpRange();
    downloadOptions.Range.Value().Length = contentSize / 2;
    EXPECT_NO_THROW(downloadResult = fileClient.Download(downloadOptions).Value);
    downloadedData = downloadResult.BodyStream->ReadToEnd();
    EXPECT_EQ(
        downloadedData, std::vector<uint8_t>(content.begin(), content.begin() + contentSize / 2));
    EXPECT_TRUE(downloadResult.StructuredContentLength.HasValue());
    EXPECT_EQ(downloadResult.StructuredContentLength.Value(), contentSize / 2);
    EXPECT_TRUE(downloadResult.StructuredBodyType.HasValue());
    EXPECT_EQ(downloadResult.FileSize, contentSize);
    downloadOptions.Range.Reset();

    // UploadFrom DownloadTo
    Files::Shares::UploadFileFromOptions uploadFromOptions;
    uploadFromOptions.ValidationOptions = validationOptions;
    Files::Shares::Models::UploadFileFromResult uploadFromResult;
    Files::Shares::DownloadFileToOptions downloadToOptions;
    downloadToOptions.ValidationOptions = validationOptions;
    Files::Shares::Models::DownloadFileToResult downloadToResult;

    // From stream
    fileClient = m_fileShareDirectoryClient->GetFileClient("uploadfromstream_" + RandomString());
    EXPECT_NO_THROW(
        uploadFromResult
        = fileClient.UploadFrom(content.data(), contentSize, uploadFromOptions).Value);
    auto downloadBuffer = std::vector<uint8_t>(contentSize, '\x00');
    EXPECT_NO_THROW(
        downloadToResult
        = fileClient.DownloadTo(downloadBuffer.data(), contentSize, downloadToOptions).Value);
    EXPECT_EQ(downloadBuffer, content);
    // partial downloadTo
    downloadToOptions.Range = Core::Http::HttpRange();
    downloadToOptions.Range.Value().Length = contentSize / 2;
    downloadBuffer.resize(static_cast<size_t>(contentSize / 2), '\x00');
    EXPECT_NO_THROW(
        downloadToResult
        = fileClient.DownloadTo(downloadBuffer.data(), contentSize / 2, downloadToOptions).Value);
    EXPECT_EQ(
        downloadBuffer, std::vector<uint8_t>(content.begin(), content.begin() + contentSize / 2));
    downloadToOptions.Range.Reset();

    // From file
    fileClient = m_fileShareDirectoryClient->GetFileClient("uploadfromfile_" + RandomString());
    EXPECT_NO_THROW(
        uploadFromResult = fileClient.UploadFrom(tempFileName, uploadFromOptions).Value);
    auto downloadFileName = RandomString();
    EXPECT_NO_THROW(
        downloadToResult = fileClient.DownloadTo(downloadFileName, downloadToOptions).Value);
  }

}}} // namespace Azure::Storage::Test
