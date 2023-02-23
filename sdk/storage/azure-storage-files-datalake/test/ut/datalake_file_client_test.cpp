// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake_file_client_test.hpp"

#include <algorithm>
#include <future>
#include <random>
#include <thread>
#include <vector>

#include <azure/identity/client_secret_credential.hpp>
#include <azure/storage/blobs.hpp>
#include <azure/storage/common/internal/shared_key_policy.hpp>

namespace Azure { namespace Storage { namespace Blobs { namespace Models {

  bool operator==(const BlobHttpHeaders& lhs, const BlobHttpHeaders& rhs)
  {
    return lhs.ContentType == rhs.ContentType && lhs.ContentEncoding == rhs.ContentEncoding
        && lhs.ContentLanguage == rhs.ContentLanguage && lhs.CacheControl == rhs.CacheControl
        && lhs.ContentDisposition == rhs.ContentDisposition;
  }

}}}} // namespace Azure::Storage::Blobs::Models

namespace Azure { namespace Storage { namespace Test {

  void DataLakeFileClientTest::SetUp()
  {
    DataLakeFileSystemClientTest::SetUp();
    CHECK_SKIP_TEST();
    m_fileName = GetFileSystemValidName();
    m_fileClient = std::make_shared<Files::DataLake::DataLakeFileClient>(
        m_fileSystemClient->GetFileClient(m_fileName));
    m_fileClient->CreateIfNotExists();
  }

  void DataLakeFileClientTest::TearDown()
  {
    CHECK_SKIP_TEST();
    m_fileSystemClient->GetFileClient(m_fileName).Delete();
    DataLakeFileSystemClientTest::TearDown();
  }

  TEST_F(DataLakeFileClientTest, CreateDeleteFiles)
  {
    {
      // Normal create/delete.
      std::vector<Files::DataLake::DataLakeFileClient> fileClient;
      for (int32_t i = 0; i < 5; ++i)
      {
        auto client = m_fileSystemClient->GetFileClient("file" + std::to_string(i));
        EXPECT_NO_THROW(client.Create());
        fileClient.emplace_back(std::move(client));
      }
      for (const auto& client : fileClient)
      {
        EXPECT_NO_THROW(client.Delete());
      }
    }
    {
      // Normal delete with last modified access condition.
      std::vector<Files::DataLake::DataLakeFileClient> fileClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetFileClient("fileCondition" + std::to_string(i));
        EXPECT_NO_THROW(client.Create());
        fileClient.emplace_back(std::move(client));
      }
      for (const auto& client : fileClient)
      {
        auto response = client.GetProperties();
        EXPECT_FALSE(response.Value.IsDirectory);
        Files::DataLake::DeleteFileOptions options1;
        options1.AccessConditions.IfModifiedSince = response.Value.LastModified;
        EXPECT_TRUE(IsValidTime(response.Value.LastModified));
        EXPECT_THROW(client.Delete(options1), StorageException);
        Files::DataLake::DeleteFileOptions options2;
        options2.AccessConditions.IfUnmodifiedSince = response.Value.LastModified;
        EXPECT_NO_THROW(client.Delete(options2));
      }
    }
    {
      // Normal delete with if match access condition.
      std::vector<Files::DataLake::DataLakeFileClient> fileClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetFileClient("fileMatch" + std::to_string(i));
        EXPECT_NO_THROW(client.Create());
        fileClient.emplace_back(std::move(client));
      }
      for (const auto& client : fileClient)
      {
        auto response = client.GetProperties();
        Files::DataLake::DeleteFileOptions options1;
        options1.AccessConditions.IfNoneMatch = response.Value.ETag;
        EXPECT_THROW(client.Delete(options1), StorageException);
        Files::DataLake::DeleteFileOptions options2;
        options2.AccessConditions.IfMatch = response.Value.ETag;
        EXPECT_NO_THROW(client.Delete(options2));
      }
    }
  }

  TEST_F(DataLakeFileClientTest, CreateDeleteIfExistsFiles)
  {
    {
      auto client = m_fileSystemClient->GetFileClient("aver");
      bool created = false;
      bool deleted = false;
      EXPECT_NO_THROW(created = client.Create().Value.Created);
      EXPECT_TRUE(created);
      EXPECT_NO_THROW(created = client.CreateIfNotExists().Value.Created);
      EXPECT_FALSE(created);
      EXPECT_NO_THROW(deleted = client.Delete().Value.Deleted);
      EXPECT_TRUE(deleted);
      EXPECT_NO_THROW(deleted = client.DeleteIfExists().Value.Deleted);
      EXPECT_FALSE(deleted);
    }
    {
      std::string testName(GetTestNameLowerCase());
      auto client = Files::DataLake::DataLakeFileClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(),
          testName + "a",
          testName + "b",
          InitClientOptions<Files::DataLake::DataLakeClientOptions>());
      bool deleted = false;
      EXPECT_NO_THROW(deleted = client.DeleteIfExists().Value.Deleted);
      EXPECT_FALSE(deleted);
    }
  }

  TEST_F(DataLakeFileClientTest, FileMetadata)
  {
    auto metadata1 = GetMetadata();
    auto metadata2 = GetMetadata();
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
      // Create path with metadata works
      auto client1 = m_fileSystemClient->GetFileClient("path1");
      auto client2 = m_fileSystemClient->GetFileClient("path2");
      Files::DataLake::CreateFileOptions options1;
      Files::DataLake::CreateFileOptions options2;
      options1.Metadata = metadata1;
      options2.Metadata = metadata2;

      EXPECT_NO_THROW(client1.Create(options1));
      EXPECT_NO_THROW(client2.Create(options2));
      auto result = client1.GetProperties().Value.Metadata;
      EXPECT_EQ(metadata1, result);
      result = client2.GetProperties().Value.Metadata;
      EXPECT_EQ(metadata2, result);
      EXPECT_NO_THROW(client1.Delete());
      EXPECT_NO_THROW(client2.Delete());
    }
  }

  TEST_F(DataLakeFileClientTest, FileProperties)
  {
    auto metadata1 = GetMetadata();
    auto metadata2 = GetMetadata();
    {
      // Get Metadata via properties works
      EXPECT_NO_THROW(m_fileClient->SetMetadata(metadata1));
      auto result = m_fileClient->GetProperties();
      EXPECT_EQ(metadata1, result.Value.Metadata);
      EXPECT_NO_THROW(m_fileClient->SetMetadata(metadata2));
      result = m_fileClient->GetProperties();
      EXPECT_EQ(metadata2, result.Value.Metadata);
    }

    {
      // Last modified Etag works.
      auto properties1 = m_fileClient->GetProperties();
      auto properties2 = m_fileClient->GetProperties();
      EXPECT_EQ(properties1.Value.ETag, properties2.Value.ETag);
      EXPECT_TRUE(IsValidTime(properties1.Value.LastModified));
      EXPECT_EQ(properties1.Value.LastModified, properties2.Value.LastModified);

      // This operation changes ETag/LastModified.
      EXPECT_NO_THROW(m_fileClient->SetMetadata(metadata1));

      auto properties3 = m_fileClient->GetProperties();
      EXPECT_NE(properties1.Value.ETag, properties3.Value.ETag);
    }

    {
      // HTTP headers works.
      auto httpHeader = GetInterestingHttpHeaders();
      std::vector<Files::DataLake::DataLakeFileClient> fileClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetFileClient("client" + std::to_string(i));
        Files::DataLake::CreateFileOptions options;
        options.HttpHeaders = httpHeader;
        EXPECT_NO_THROW(client.Create(options));
        fileClient.emplace_back(std::move(client));
      }
      for (const auto& client : fileClient)
      {
        auto result = client.GetProperties();
        EXPECT_EQ(httpHeader.CacheControl, result.Value.HttpHeaders.CacheControl);
        EXPECT_EQ(httpHeader.ContentDisposition, result.Value.HttpHeaders.ContentDisposition);
        EXPECT_EQ(httpHeader.ContentLanguage, result.Value.HttpHeaders.ContentLanguage);
        EXPECT_EQ(httpHeader.ContentType, result.Value.HttpHeaders.ContentType);
        EXPECT_NO_THROW(client.Delete());
      }
    }
  }

  TEST_F(DataLakeFileClientTest, FileDataActions)
  {
    const int32_t bufferSize = 4 * 1024; // 4KB data size
    std::vector<uint8_t> buffer(bufferSize, 'x');
    auto bufferStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        Azure::Core::IO::MemoryBodyStream(buffer));
    auto properties1 = m_fileClient->GetProperties();

    // Append
    m_fileClient->Append(*bufferStream, 0);
    auto properties2 = m_fileClient->GetProperties();
    // Append does not change etag because not committed yet.
    EXPECT_EQ(properties1.Value.ETag, properties2.Value.ETag);
    EXPECT_TRUE(IsValidTime(properties1.Value.LastModified));
    EXPECT_EQ(properties1.Value.LastModified, properties2.Value.LastModified);

    // Flush
    m_fileClient->Flush(bufferSize);
    auto properties3 = m_fileClient->GetProperties();
    EXPECT_NE(properties2.Value.ETag, properties3.Value.ETag);

    // Read
    auto result = m_fileClient->Download();
    auto downloaded = ReadBodyStream(result.Value.Body);
    EXPECT_EQ(buffer, downloaded);
  }

  TEST_F(DataLakeFileClientTest, AppendFileWithFlush)
  {
    const int32_t bufferSize = 4 * 1024; // 4KB data size
    std::vector<uint8_t> buffer(bufferSize, 'x');
    auto bufferStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        Azure::Core::IO::MemoryBodyStream(buffer));

    // Append with flush option
    {
      auto client = m_fileSystemClient->GetFileClient(GetTestNameLowerCase() + "_flush_true");
      client.Create();
      auto properties1 = client.GetProperties();
      Files::DataLake::AppendFileOptions options;
      options.Flush = true;
      bufferStream->Rewind();
      client.Append(*bufferStream, 0, options);
      auto properties2 = client.GetProperties();
      EXPECT_NE(properties1.Value.ETag, properties2.Value.ETag);
      EXPECT_EQ(bufferSize, properties2.Value.FileSize);
    }
    {
      auto client = m_fileSystemClient->GetFileClient(GetTestNameLowerCase() + "_flush_false");
      client.Create();
      auto properties1 = client.GetProperties();
      Files::DataLake::AppendFileOptions options;
      options.Flush = false;
      bufferStream->Rewind();
      client.Append(*bufferStream, 0, options);
      auto properties2 = client.GetProperties();
      EXPECT_EQ(properties1.Value.ETag, properties2.Value.ETag);
      EXPECT_EQ(0ll, properties2.Value.FileSize);
    }
  }

  TEST_F(DataLakeFileClientTest, AppendFileWithLease)
  {
    const int32_t bufferSize = 4 * 1024; // 4KB data size
    std::vector<uint8_t> buffer(bufferSize, 'x');
    auto bufferStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        Azure::Core::IO::MemoryBodyStream(buffer));

    // Append Lease Acquire
    {
      auto client = m_fileSystemClient->GetFileClient(GetTestNameLowerCase() + "_acquire");
      client.Create();
      Files::DataLake::AppendFileOptions options;
      options.LeaseAction = Files::DataLake::Models::LeaseAction::Acquire;
      options.LeaseId = Files::DataLake::DataLakeLeaseClient::CreateUniqueLeaseId();
      options.LeaseDuration = std::chrono::seconds(20);
      bufferStream->Rewind();
      client.Append(*bufferStream, 0, options);
      auto properties = client.GetProperties();
      EXPECT_TRUE(properties.Value.LeaseStatus.HasValue());
      EXPECT_EQ(Files::DataLake::Models::LeaseStatus::Locked, properties.Value.LeaseStatus.Value());
      EXPECT_TRUE(properties.Value.LeaseState.HasValue());
      EXPECT_EQ(Files::DataLake::Models::LeaseState::Leased, properties.Value.LeaseState.Value());
      EXPECT_TRUE(properties.Value.LeaseDuration.HasValue());
      EXPECT_EQ(
          Files::DataLake::Models::LeaseDurationType::Fixed,
          properties.Value.LeaseDuration.Value());
    }
    // Append Lease AutoRenew
    {
      auto client = m_fileSystemClient->GetFileClient(GetTestNameLowerCase() + "_auto_renew");
      client.Create();
      const std::string leaseId = Files::DataLake::DataLakeLeaseClient::CreateUniqueLeaseId();
      Files::DataLake::DataLakeLeaseClient leaseClient(client, leaseId);
      leaseClient.Acquire(std::chrono::seconds(20));
      Files::DataLake::AppendFileOptions options;
      options.LeaseAction = Files::DataLake::Models::LeaseAction::AutoRenew;
      options.AccessConditions.LeaseId = leaseId;
      bufferStream->Rewind();
      auto response = client.Append(*bufferStream, 0, options);
      EXPECT_TRUE(response.Value.IsLeaseRenewed.HasValue());
      auto properties = client.GetProperties();
      EXPECT_TRUE(properties.Value.LeaseStatus.HasValue());
      EXPECT_EQ(Files::DataLake::Models::LeaseStatus::Locked, properties.Value.LeaseStatus.Value());
      EXPECT_TRUE(properties.Value.LeaseState.HasValue());
      EXPECT_EQ(Files::DataLake::Models::LeaseState::Leased, properties.Value.LeaseState.Value());
      EXPECT_TRUE(properties.Value.LeaseDuration.HasValue());
      EXPECT_EQ(
          Files::DataLake::Models::LeaseDurationType::Fixed,
          properties.Value.LeaseDuration.Value());
    }
    // Append Lease Release
    {
      auto client = m_fileSystemClient->GetFileClient(GetTestNameLowerCase() + "_release");
      client.Create();
      const std::string leaseId = Files::DataLake::DataLakeLeaseClient::CreateUniqueLeaseId();
      Files::DataLake::DataLakeLeaseClient leaseClient(client, leaseId);
      leaseClient.Acquire(std::chrono::seconds(20));
      Files::DataLake::AppendFileOptions options;
      options.LeaseAction = Files::DataLake::Models::LeaseAction::Release;
      options.AccessConditions.LeaseId = leaseId;
      options.Flush = true;
      bufferStream->Rewind();
      client.Append(*bufferStream, 0, options);
      auto properties = client.GetProperties();
      EXPECT_TRUE(properties.Value.LeaseStatus.HasValue());
      EXPECT_EQ(
          Files::DataLake::Models::LeaseStatus::Unlocked, properties.Value.LeaseStatus.Value());
      EXPECT_TRUE(properties.Value.LeaseState.HasValue());
      EXPECT_EQ(
          Files::DataLake::Models::LeaseState::Available, properties.Value.LeaseState.Value());
    }
    // Append Lease AcquireRelease
    {
      auto client = m_fileSystemClient->GetFileClient(GetTestNameLowerCase() + "_acquire_release");
      client.Create();
      Files::DataLake::AppendFileOptions options;
      options.LeaseAction = Files::DataLake::Models::LeaseAction::AcquireRelease;
      options.LeaseId = Files::DataLake::DataLakeLeaseClient::CreateUniqueLeaseId();
      options.LeaseDuration = std::chrono::seconds(20);
      options.Flush = true;
      bufferStream->Rewind();
      client.Append(*bufferStream, 0, options);
      auto properties = client.GetProperties();
      EXPECT_TRUE(properties.Value.LeaseStatus.HasValue());
      EXPECT_EQ(
          Files::DataLake::Models::LeaseStatus::Unlocked, properties.Value.LeaseStatus.Value());
      EXPECT_TRUE(properties.Value.LeaseState.HasValue());
      EXPECT_EQ(
          Files::DataLake::Models::LeaseState::Available, properties.Value.LeaseState.Value());
    }
  }

  TEST_F(DataLakeFileClientTest, FlushFileWithLease)
  {
    const int32_t bufferSize = 4 * 1024; // 4KB data size
    std::vector<uint8_t> buffer(bufferSize, 'x');
    auto bufferStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        Azure::Core::IO::MemoryBodyStream(buffer));

    // Flush Lease Acquire
    {
      auto client = m_fileSystemClient->GetFileClient(GetTestNameLowerCase() + "_acquire");
      client.Create();
      bufferStream->Rewind();
      client.Append(*bufferStream, 0);
      Files::DataLake::FlushFileOptions options;
      options.LeaseAction = Files::DataLake::Models::LeaseAction::Acquire;
      options.LeaseId = Files::DataLake::DataLakeLeaseClient::CreateUniqueLeaseId();
      options.LeaseDuration = std::chrono::seconds(20);
      client.Flush(bufferSize, options);
      auto properties = client.GetProperties();
      EXPECT_TRUE(properties.Value.LeaseStatus.HasValue());
      EXPECT_EQ(Files::DataLake::Models::LeaseStatus::Locked, properties.Value.LeaseStatus.Value());
      EXPECT_TRUE(properties.Value.LeaseState.HasValue());
      EXPECT_EQ(Files::DataLake::Models::LeaseState::Leased, properties.Value.LeaseState.Value());
      EXPECT_TRUE(properties.Value.LeaseDuration.HasValue());
      EXPECT_EQ(
          Files::DataLake::Models::LeaseDurationType::Fixed,
          properties.Value.LeaseDuration.Value());
    }
    // Flush Lease AutoRenew
    {
      auto client = m_fileSystemClient->GetFileClient(GetTestNameLowerCase() + "_auto_renew");
      client.Create();
      const std::string leaseId = Files::DataLake::DataLakeLeaseClient::CreateUniqueLeaseId();
      Files::DataLake::AppendFileOptions options;
      options.LeaseAction = Files::DataLake::Models::LeaseAction::Acquire;
      options.LeaseId = leaseId;
      options.LeaseDuration = std::chrono::seconds(20);
      bufferStream->Rewind();
      client.Append(*bufferStream, 0, options);
      Files::DataLake::FlushFileOptions flushOptions;
      flushOptions.LeaseAction = Files::DataLake::Models::LeaseAction::AutoRenew;
      flushOptions.AccessConditions.LeaseId = leaseId;
      auto response = client.Flush(bufferSize, flushOptions);
      EXPECT_TRUE(response.Value.IsLeaseRenewed.HasValue());
      auto properties = client.GetProperties();
      EXPECT_TRUE(properties.Value.LeaseStatus.HasValue());
      EXPECT_EQ(Files::DataLake::Models::LeaseStatus::Locked, properties.Value.LeaseStatus.Value());
      EXPECT_TRUE(properties.Value.LeaseState.HasValue());
      EXPECT_EQ(Files::DataLake::Models::LeaseState::Leased, properties.Value.LeaseState.Value());
      EXPECT_TRUE(properties.Value.LeaseDuration.HasValue());
      EXPECT_EQ(
          Files::DataLake::Models::LeaseDurationType::Fixed,
          properties.Value.LeaseDuration.Value());
    }
    // Flush Lease Release
    {
      auto client = m_fileSystemClient->GetFileClient(GetTestNameLowerCase() + "_release");
      client.Create();
      const std::string leaseId = Files::DataLake::DataLakeLeaseClient::CreateUniqueLeaseId();
      Files::DataLake::AppendFileOptions options;
      options.LeaseAction = Files::DataLake::Models::LeaseAction::Acquire;
      options.LeaseId = leaseId;
      options.LeaseDuration = std::chrono::seconds(20);
      bufferStream->Rewind();
      client.Append(*bufferStream, 0, options);
      Files::DataLake::FlushFileOptions flushOptions;
      flushOptions.LeaseAction = Files::DataLake::Models::LeaseAction::Release;
      flushOptions.AccessConditions.LeaseId = leaseId;
      client.Flush(bufferSize, flushOptions);
      auto properties = client.GetProperties();
      EXPECT_TRUE(properties.Value.LeaseStatus.HasValue());
      EXPECT_EQ(
          Files::DataLake::Models::LeaseStatus::Unlocked, properties.Value.LeaseStatus.Value());
      EXPECT_TRUE(properties.Value.LeaseState.HasValue());
      EXPECT_EQ(
          Files::DataLake::Models::LeaseState::Available, properties.Value.LeaseState.Value());
    }
    // Flush Lease AcquireRelease
    {
      auto client = m_fileSystemClient->GetFileClient(GetTestNameLowerCase() + "_acquire_release");
      client.Create();
      bufferStream->Rewind();
      client.Append(*bufferStream, 0);
      Files::DataLake::FlushFileOptions options;
      options.LeaseAction = Files::DataLake::Models::LeaseAction::AcquireRelease;
      options.LeaseId = Files::DataLake::DataLakeLeaseClient::CreateUniqueLeaseId();
      options.LeaseDuration = std::chrono::seconds(20);
      client.Flush(bufferSize, options);
      auto properties = client.GetProperties();
      EXPECT_TRUE(properties.Value.LeaseStatus.HasValue());
      EXPECT_EQ(
          Files::DataLake::Models::LeaseStatus::Unlocked, properties.Value.LeaseStatus.Value());
      EXPECT_TRUE(properties.Value.LeaseState.HasValue());
      EXPECT_EQ(
          Files::DataLake::Models::LeaseState::Available, properties.Value.LeaseState.Value());
    }
  }

  TEST_F(DataLakeFileClientTest, DISABLED_CreateWithEncryptionContext)
  {
    std::string encryptionContext = "encryptionContext";
    const std::string fileName = RandomString();
    auto fileClient = m_fileSystemClient->GetFileClient(fileName);
    Files::DataLake::CreateFileOptions options;
    options.EncryptionContext = encryptionContext;
    // Assert Create
    EXPECT_NO_THROW(fileClient.Create(options));
    // Assert GetProperties
    auto properties = fileClient.GetProperties();
    EXPECT_TRUE(properties.Value.EncryptionContext.HasValue());
    EXPECT_EQ(encryptionContext, properties.Value.EncryptionContext.Value());
    // Assert Download
    auto downloadResult = fileClient.Download();
    EXPECT_TRUE(downloadResult.Value.Details.EncryptionContext.HasValue());
    EXPECT_EQ(encryptionContext, downloadResult.Value.Details.EncryptionContext.Value());
    // Assert ListPaths
    auto paths = m_fileSystemClient->ListPaths(false).Paths;
    auto iter = std::find_if(
        paths.begin(), paths.end(), [&fileName](const Files::DataLake::Models::PathItem& path) {
          return path.Name == fileName;
        });
    EXPECT_NE(paths.end(), iter);
    EXPECT_TRUE(iter->EncryptionContext.HasValue());
    EXPECT_EQ(encryptionContext, iter->EncryptionContext.Value());
  }

  TEST_F(DataLakeFileClientTest, FileReadReturns)
  {
    const int32_t bufferSize = 4 * 1024; // 4KB data size
    std::vector<uint8_t> buffer(bufferSize, 'x');
    auto bufferStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        Azure::Core::IO::MemoryBodyStream(buffer));
    std::string newFileName("fileForTest");
    auto newFileClient = std::make_shared<Files::DataLake::DataLakeFileClient>(
        m_fileSystemClient->GetFileClient(newFileName));
    newFileClient->Create();
    auto properties1 = newFileClient->GetProperties();

    // Append
    newFileClient->Append(*bufferStream, 0);
    auto properties2 = newFileClient->GetProperties();
    // Append does not change etag because not committed yet.
    EXPECT_EQ(properties1.Value.ETag, properties2.Value.ETag);
    EXPECT_TRUE(IsValidTime(properties1.Value.LastModified));
    EXPECT_EQ(properties1.Value.LastModified, properties2.Value.LastModified);

    // Flush
    newFileClient->Flush(bufferSize);
    auto properties3 = newFileClient->GetProperties();
    EXPECT_NE(properties2.Value.ETag, properties3.Value.ETag);

    // Read
    auto result = newFileClient->Download();
    auto downloaded = ReadBodyStream(result.Value.Body);
    EXPECT_EQ(buffer, downloaded);
    EXPECT_EQ(bufferSize, result.Value.FileSize);
    EXPECT_EQ(bufferSize, result.Value.ContentRange.Length.Value());
    EXPECT_EQ(0, result.Value.ContentRange.Offset);

    // Read Range
    {
      auto firstHalf = std::vector<uint8_t>(buffer.begin(), buffer.begin() + (bufferSize / 2));
      Files::DataLake::DownloadFileOptions options;
      options.Range = Azure::Core::Http::HttpRange();
      options.Range.Value().Offset = 0;
      options.Range.Value().Length = bufferSize / 2;
      result = newFileClient->Download(options);
      downloaded = ReadBodyStream(result.Value.Body);
      EXPECT_EQ(firstHalf.size(), downloaded.size());
      EXPECT_EQ(firstHalf, downloaded);
      EXPECT_EQ(bufferSize, result.Value.FileSize);
      EXPECT_EQ(bufferSize / 2, result.Value.ContentRange.Length.Value());
      EXPECT_EQ(0, result.Value.ContentRange.Offset);
    }
    {
      auto secondHalf = std::vector<uint8_t>(buffer.begin() + bufferSize / 2, buffer.end());
      Files::DataLake::DownloadFileOptions options;
      options.Range = Azure::Core::Http::HttpRange();
      options.Range.Value().Offset = bufferSize / 2;
      options.Range.Value().Length = bufferSize / 2;
      result = newFileClient->Download(options);
      downloaded = ReadBodyStream(result.Value.Body);
      EXPECT_EQ(secondHalf, downloaded);
      EXPECT_EQ(bufferSize, result.Value.FileSize);
      EXPECT_EQ(bufferSize / 2, result.Value.ContentRange.Length.Value());
      EXPECT_EQ(bufferSize / 2, result.Value.ContentRange.Offset);
    }
    {
      // Read with last modified access condition.
      auto response = newFileClient->GetProperties();
      Files::DataLake::DownloadFileOptions options1;
      options1.AccessConditions.IfModifiedSince = response.Value.LastModified;
      EXPECT_TRUE(IsValidTime(response.Value.LastModified));
      EXPECT_THROW(newFileClient->Download(options1), StorageException);
      Files::DataLake::DownloadFileOptions options2;
      options2.AccessConditions.IfUnmodifiedSince = response.Value.LastModified;
      EXPECT_NO_THROW(result = newFileClient->Download(options2));
      downloaded = ReadBodyStream(result.Value.Body);
      EXPECT_EQ(buffer, downloaded);
    }
    {
      // Read with if match access condition.
      auto response = newFileClient->GetProperties();
      Files::DataLake::DownloadFileOptions options1;
      options1.AccessConditions.IfNoneMatch = response.Value.ETag;
      EXPECT_THROW(newFileClient->Download(options1), StorageException);
      Files::DataLake::DownloadFileOptions options2;
      options2.AccessConditions.IfMatch = response.Value.ETag;
      EXPECT_NO_THROW(result = newFileClient->Download(options2));
      downloaded = ReadBodyStream(result.Value.Body);
      EXPECT_EQ(buffer, downloaded);
    }
  }

  TEST_F(DataLakeFileClientTest, ReadEmptyFile)
  {
    auto fileClient = m_fileSystemClient->GetFileClient(GetTestNameLowerCase());
    fileClient.Create();

    auto res = fileClient.Download();
    EXPECT_EQ(res.Value.Body->Length(), 0);

    std::string tempFilename(GetTestNameLowerCase());
    EXPECT_NO_THROW(fileClient.DownloadTo(tempFilename));
    EXPECT_TRUE(ReadFile(tempFilename).empty());
    DeleteFile(tempFilename);

    std::vector<uint8_t> buff;
    EXPECT_NO_THROW(fileClient.DownloadTo(buff.data(), 0));
    fileClient.Delete();
  }

  TEST_F(DataLakeFileClientTest, DownloadNonExistingToFile)
  {
    const auto testName(GetTestName());
    auto fileClient = m_fileSystemClient->GetFileClient(testName);

    EXPECT_THROW(fileClient.DownloadTo(testName), StorageException);
    EXPECT_THROW(ReadFile(testName), std::runtime_error);
  }

  TEST_F(DataLakeFileClientTest, ScheduleForDeletion)
  {
    {
      auto client = m_fileSystemClient->GetFileClient(GetTestNameLowerCase());
      auto createResponse = client.Create();
      auto scheduleDeletionResponse
          = client.ScheduleDeletion(Files::DataLake::ScheduleFileExpiryOriginType::NeverExpire);
      EXPECT_EQ(scheduleDeletionResponse.Value.ETag, createResponse.Value.ETag);
      EXPECT_EQ(scheduleDeletionResponse.Value.LastModified, createResponse.Value.LastModified);
    }
    {
      auto client = m_fileSystemClient->GetFileClient(GetTestNameLowerCase() + "1");
      EXPECT_NO_THROW(client.Create());
      Files::DataLake::ScheduleFileDeletionOptions options;
      EXPECT_THROW(
          client.ScheduleDeletion(
              Files::DataLake::ScheduleFileExpiryOriginType::RelativeToNow, options),
          StorageException);
      options.TimeToExpire = std::chrono::milliseconds(1000);
      EXPECT_NO_THROW(client.ScheduleDeletion(
          Files::DataLake::ScheduleFileExpiryOriginType::RelativeToNow, options));
    }
    {
      auto client = m_fileSystemClient->GetFileClient(GetTestNameLowerCase() + "2");
      EXPECT_NO_THROW(client.Create());
      Files::DataLake::ScheduleFileDeletionOptions options;
      EXPECT_THROW(
          client.ScheduleDeletion(Files::DataLake::ScheduleFileExpiryOriginType::Absolute, options),
          StorageException);
      options.TimeToExpire = std::chrono::milliseconds(1000);
      EXPECT_THROW(
          client.ScheduleDeletion(Files::DataLake::ScheduleFileExpiryOriginType::Absolute, options),
          StorageException);
      options.ExpiresOn = Azure::DateTime::Parse(
          "Wed, 29 Sep 2100 09:53:03 GMT", Azure::DateTime::DateFormat::Rfc1123);
      options.TimeToExpire = Azure::Nullable<std::chrono::milliseconds>();
      EXPECT_NO_THROW(client.ScheduleDeletion(
          Files::DataLake::ScheduleFileExpiryOriginType::Absolute, options));
    }
  }

  namespace {

    struct FileConcurrentUploadParameter
    {
      int Concurrency;
      int64_t FileSize;
    };

    class UploadFile : public DataLakeFileClientTest,
                       public ::testing::WithParamInterface<FileConcurrentUploadParameter> {
    };

    std::string GetUploadSuffix(const testing::TestParamInfo<UploadFile::ParamType>& info)
    {
      // Can't use empty spaces or underscores (_) as per google test documentation
      // http://google.github.io/googletest/advanced.html#specifying-names-for-value-parameterized-test-parameters
      auto const& p = info.param;
      std::string suffix("c" + std::to_string(p.Concurrency) + "s" + std::to_string(p.FileSize));
      return suffix;
    }

    std::vector<FileConcurrentUploadParameter> GetUploadParameters()
    {
      std::vector<FileConcurrentUploadParameter> testParametes;
      for (int c : {1, 2, 5})
      {
        for (int64_t l :
             {0ULL, 1ULL, 2ULL, 2_KB, 4_KB, 999_KB, 1_MB, 2_MB - 1, 3_MB, 5_MB, 8_MB - 1234, 8_MB})
        {
          testParametes.emplace_back(FileConcurrentUploadParameter({c, l}));
        }
      }
      return testParametes;
    }
  } // namespace

  TEST_P(UploadFile, fromBuffer)
  {
    UploadFile::ParamType const& p(GetParam());
    std::vector<uint8_t> fileContent(static_cast<size_t>(8_MB), 'x');
    auto fileClient = m_fileSystemClient->GetFileClient(GetTestNameLowerCase());

    Azure::Storage::Files::DataLake::UploadFileFromOptions options;
    options.TransferOptions.ChunkSize = 1_MB;
    options.TransferOptions.Concurrency = p.Concurrency;
    options.HttpHeaders = GetInterestingHttpHeaders();
    options.Metadata = GetMetadata();
    auto res = fileClient.UploadFrom(fileContent.data(), static_cast<size_t>(p.FileSize), options);
    auto lastModified = fileClient.GetProperties().Value.LastModified;
    EXPECT_TRUE(res.Value.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(res.Value.LastModified));
    EXPECT_EQ(res.Value.LastModified, lastModified);
    auto properties = fileClient.GetProperties().Value;
    EXPECT_EQ(properties.FileSize, p.FileSize);
    EXPECT_EQ(properties.HttpHeaders, options.HttpHeaders);
    EXPECT_EQ(properties.Metadata, options.Metadata);
    EXPECT_EQ(properties.ETag, res.Value.ETag);
    EXPECT_TRUE(IsValidTime(res.Value.LastModified));
    EXPECT_EQ(properties.LastModified, res.Value.LastModified);
    std::vector<uint8_t> downloadContent(static_cast<size_t>(p.FileSize), '\x00');
    fileClient.DownloadTo(downloadContent.data(), static_cast<size_t>(p.FileSize));
    EXPECT_EQ(
        downloadContent,
        std::vector<uint8_t>(
            fileContent.begin(), fileContent.begin() + static_cast<size_t>(p.FileSize)));
    fileClient.Delete();
  }

  TEST_P(UploadFile, fromFile)
  {
    UploadFile::ParamType const& p(GetParam());
    std::vector<uint8_t> fileContent(static_cast<size_t>(p.FileSize), 'x');
    auto fileClient = m_fileSystemClient->GetFileClient(GetTestNameLowerCase());

    Azure::Storage::Files::DataLake::UploadFileFromOptions options;
    options.TransferOptions.ChunkSize = 1_MB;
    options.TransferOptions.Concurrency = p.Concurrency;
    options.HttpHeaders = GetInterestingHttpHeaders();
    options.Metadata = GetMetadata();

    std::string tempFilename = GetTestNameLowerCase();
    WriteFile(tempFilename, fileContent);
    auto res = fileClient.UploadFrom(tempFilename, options);
    auto lastModified = fileClient.GetProperties().Value.LastModified;
    EXPECT_TRUE(res.Value.ETag.HasValue());
    EXPECT_TRUE(IsValidTime(res.Value.LastModified));
    EXPECT_EQ(res.Value.LastModified, lastModified);
    auto properties = fileClient.GetProperties().Value;
    EXPECT_EQ(properties.FileSize, p.FileSize);
    EXPECT_EQ(properties.HttpHeaders, options.HttpHeaders);
    EXPECT_EQ(properties.Metadata, options.Metadata);
    EXPECT_EQ(properties.ETag, res.Value.ETag);
    EXPECT_EQ(properties.LastModified, res.Value.LastModified);
    std::vector<uint8_t> downloadContent(static_cast<size_t>(p.FileSize), '\x00');
    fileClient.DownloadTo(downloadContent.data(), static_cast<size_t>(p.FileSize));
    EXPECT_EQ(
        downloadContent,
        std::vector<uint8_t>(
            fileContent.begin(), fileContent.begin() + static_cast<size_t>(p.FileSize)));
    std::string tempFileDestinationName = RandomString();
    fileClient.DownloadTo(tempFileDestinationName);
    EXPECT_EQ(ReadFile(tempFileDestinationName), fileContent);
    DeleteFile(tempFileDestinationName);
    DeleteFile(tempFilename);
    fileClient.Delete();
  }

  INSTANTIATE_TEST_SUITE_P(
      withParam,
      UploadFile,
      testing::ValuesIn(GetUploadParameters()),
      GetUploadSuffix);

  TEST_F(DataLakeFileClientTest, ConstructorsWorks)
  {
    {
      // Create from connection string validates static creator function and shared key
      // constructor.
      auto fileName = GetTestNameLowerCase();
      auto connectionStringClient
          = Azure::Storage::Files::DataLake::DataLakeFileClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(),
              m_fileSystemName,
              fileName,
              InitClientOptions<Azure::Storage::Files::DataLake::DataLakeClientOptions>());
      EXPECT_NO_THROW(connectionStringClient.Create());
      EXPECT_NO_THROW(connectionStringClient.Delete());
    }

    {
      // Create from client secret credential.
      std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential
          = std::make_shared<Azure::Identity::ClientSecretCredential>(
              AadTenantId(), AadClientId(), AadClientSecret());
      Azure::Storage::Files::DataLake::DataLakeClientOptions options;

      auto clientSecretClient = InitTestClient<
          Azure::Storage::Files::DataLake::DataLakeFileClient,
          Azure::Storage::Files::DataLake::DataLakeClientOptions>(
          Azure::Storage::Files::DataLake::_detail::GetDfsUrlFromUrl(
              Azure::Storage::Files::DataLake::DataLakeFileClient::CreateFromConnectionString(
                  AdlsGen2ConnectionString(), m_fileSystemName, "credential")
                  .GetUrl()),
          credential,
          options);

      EXPECT_NO_THROW(clientSecretClient->Create());
      EXPECT_NO_THROW(clientSecretClient->Delete());
    }

    {
      // Create from Anonymous credential.
      std::vector<uint8_t> blobContent(static_cast<size_t>(1_MB), 'x');

      auto objectName = "testObject";
      auto containerClient = Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(),
          m_fileSystemName,
          InitClientOptions<Azure::Storage::Blobs::BlobClientOptions>());
      Azure::Storage::Blobs::SetBlobContainerAccessPolicyOptions options;
      options.AccessType = Azure::Storage::Blobs::Models::PublicAccessType::Blob;
      containerClient.SetAccessPolicy(options);
      auto blobClient = containerClient.GetBlockBlobClient(objectName);
      auto memoryStream = Azure::Core::IO::MemoryBodyStream(blobContent.data(), blobContent.size());
      EXPECT_NO_THROW(blobClient.Upload(memoryStream));

      auto anonymousClient = Azure::Storage::Files::DataLake::DataLakeFileClient(
          Azure::Storage::Files::DataLake::DataLakeFileClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(), m_fileSystemName, objectName)
              .GetUrl(),
          InitClientOptions<Azure::Storage::Files::DataLake::DataLakeClientOptions>());

      TestSleep(std::chrono::seconds(30));

      EXPECT_NO_THROW(anonymousClient.Download());
    }
  }
}}} // namespace Azure::Storage::Test
