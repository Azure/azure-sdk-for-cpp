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
    if (shouldSkipTest())
    {
      return;
    }
    m_fileName = RandomString();
    m_fileClient = std::make_shared<Files::DataLake::DataLakeFileClient>(
        m_fileSystemClient->GetFileClient(m_fileName));
    m_fileClient->CreateIfNotExists();
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
      auto fileClient = m_fileSystemClient->GetFileClient(RandomString());
      bool deleted = false;
      EXPECT_NO_THROW(deleted = fileClient.DeleteIfExists().Value.Deleted);
      EXPECT_FALSE(deleted);
    }
  }

  TEST_F(DataLakeFileClientTest, FileMetadata)
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
    auto metadata1 = RandomMetadata();
    auto metadata2 = RandomMetadata();
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
      auto httpHeaders = Files::DataLake::Models::PathHttpHeaders();
      httpHeaders.ContentType = "application/x-binary";
      httpHeaders.ContentLanguage = "en-US";
      httpHeaders.ContentDisposition = "attachment";
      httpHeaders.CacheControl = "no-cache";
      httpHeaders.ContentEncoding = "identity";
      std::vector<Files::DataLake::DataLakeFileClient> fileClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetFileClient("client" + std::to_string(i));
        Files::DataLake::CreateFileOptions options;
        options.HttpHeaders = httpHeaders;
        EXPECT_NO_THROW(client.Create(options));
        fileClient.emplace_back(std::move(client));
      }
      for (const auto& client : fileClient)
      {
        auto result = client.GetProperties();
        EXPECT_EQ(httpHeaders.ContentType, result.Value.HttpHeaders.ContentType);
        EXPECT_EQ(httpHeaders.ContentLanguage, result.Value.HttpHeaders.ContentLanguage);
        EXPECT_EQ(httpHeaders.ContentDisposition, result.Value.HttpHeaders.ContentDisposition);
        EXPECT_EQ(httpHeaders.CacheControl, result.Value.HttpHeaders.CacheControl);
        EXPECT_EQ(httpHeaders.ContentEncoding, result.Value.HttpHeaders.ContentEncoding);
        EXPECT_NO_THROW(client.Delete());
      }
    }
  }

  TEST_F(DataLakeFileClientTest, FileDataActions)
  {
    const int32_t bufferSize = 10;
    std::vector<uint8_t> buffer = RandomBuffer(bufferSize);
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
    const int32_t bufferSize = 1;
    auto buffer = RandomBuffer(bufferSize);
    auto bufferStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        Azure::Core::IO::MemoryBodyStream(buffer));

    // Append with flush option
    {
      auto client = m_fileSystemClient->GetFileClient(RandomString());
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
      auto client = m_fileSystemClient->GetFileClient(RandomString());
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
    const int32_t bufferSize = 1;
    auto buffer = RandomBuffer(bufferSize);
    auto bufferStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        Azure::Core::IO::MemoryBodyStream(buffer));

    // Append Lease Acquire
    {
      auto client = m_fileSystemClient->GetFileClient(RandomString() + "_acquire");
      client.Create();
      Files::DataLake::AppendFileOptions options;
      options.LeaseAction = Files::DataLake::Models::LeaseAction::Acquire;
      options.LeaseId = RandomUUID();
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
      auto client = m_fileSystemClient->GetFileClient(RandomString() + "_auto_renew");
      client.Create();
      const std::string leaseId = RandomUUID();
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
      auto client = m_fileSystemClient->GetFileClient(RandomString() + "_release");
      client.Create();
      const std::string leaseId = RandomUUID();
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
      auto client = m_fileSystemClient->GetFileClient(RandomString() + "_acquire_release");
      client.Create();
      Files::DataLake::AppendFileOptions options;
      options.LeaseAction = Files::DataLake::Models::LeaseAction::AcquireRelease;
      options.LeaseId = RandomUUID();
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
    const int32_t bufferSize = 1;
    auto buffer = RandomBuffer(bufferSize);
    auto bufferStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        Azure::Core::IO::MemoryBodyStream(buffer));

    // Flush Lease Acquire
    {
      auto client = m_fileSystemClient->GetFileClient(RandomString() + "_acquire");
      client.Create();
      bufferStream->Rewind();
      client.Append(*bufferStream, 0);
      Files::DataLake::FlushFileOptions options;
      options.LeaseAction = Files::DataLake::Models::LeaseAction::Acquire;
      options.LeaseId = RandomUUID();
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
      auto client = m_fileSystemClient->GetFileClient(RandomString() + "_auto_renew");
      client.Create();
      const std::string leaseId = RandomUUID();
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
      auto client = m_fileSystemClient->GetFileClient(RandomString() + "_release");
      client.Create();
      const std::string leaseId = RandomUUID();
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
      auto client = m_fileSystemClient->GetFileClient(RandomString() + "_acquire_release");
      client.Create();
      bufferStream->Rewind();
      client.Append(*bufferStream, 0);
      Files::DataLake::FlushFileOptions options;
      options.LeaseAction = Files::DataLake::Models::LeaseAction::AcquireRelease;
      options.LeaseId = RandomUUID();
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

  TEST_F(DataLakeFileClientTest, FileReadReturns)
  {
    const int32_t bufferSize = 20;
    std::vector<uint8_t> buffer = RandomBuffer(bufferSize);
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
    auto fileClient = m_fileSystemClient->GetFileClient(RandomString());
    fileClient.Create();

    auto res = fileClient.Download();
    EXPECT_EQ(res.Value.Body->Length(), 0);

    std::string tempFilename(RandomString());
    EXPECT_NO_THROW(fileClient.DownloadTo(tempFilename));
    EXPECT_TRUE(ReadFile(tempFilename).empty());
    DeleteFile(tempFilename);

    std::vector<uint8_t> buff;
    EXPECT_NO_THROW(fileClient.DownloadTo(buff.data(), 0));
    fileClient.Delete();
  }

  TEST_F(DataLakeFileClientTest, DownloadNonExistingToFile)
  {
    const auto testName = RandomString();
    auto fileClient = m_fileSystemClient->GetFileClient(testName);

    EXPECT_THROW(fileClient.DownloadTo(testName), StorageException);
    EXPECT_THROW(ReadFile(testName), std::runtime_error);
  }

  TEST_F(DataLakeFileClientTest, ScheduleForDeletion)
  {
    {
      auto client = m_fileSystemClient->GetFileClient(RandomString());
      auto createResponse = client.Create();
      auto scheduleDeletionResponse
          = client.ScheduleDeletion(Files::DataLake::ScheduleFileExpiryOriginType::NeverExpire);
      EXPECT_EQ(scheduleDeletionResponse.Value.ETag, createResponse.Value.ETag);
      EXPECT_EQ(scheduleDeletionResponse.Value.LastModified, createResponse.Value.LastModified);
    }
    {
      auto client = m_fileSystemClient->GetFileClient(RandomString());
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
      auto client = m_fileSystemClient->GetFileClient(RandomString());
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

  TEST_F(DataLakeFileClientTest, ConcurrentDownload_LIVEONLY_)
  {
    auto fileClient = *m_fileClient;
    const auto blobContent = RandomBuffer(static_cast<size_t>(8_MB));
    fileClient.UploadFrom(blobContent.data(), blobContent.size());

    auto testDownloadToBuffer = [&](int concurrency,
                                    int64_t downloadSize,
                                    Azure::Nullable<int64_t> offset = {},
                                    Azure::Nullable<int64_t> length = {},
                                    Azure::Nullable<int64_t> initialChunkSize = {},
                                    Azure::Nullable<int64_t> chunkSize = {}) {
      std::vector<uint8_t> downloadBuffer;
      std::vector<uint8_t> expectedData = blobContent;
      int64_t blobSize = blobContent.size();
      int64_t actualDownloadSize = std::min(downloadSize, blobSize);
      if (offset.HasValue() && length.HasValue())
      {
        actualDownloadSize = std::min(length.Value(), blobSize - offset.Value());
        if (actualDownloadSize >= 0)
        {
          expectedData.assign(
              blobContent.begin() + static_cast<ptrdiff_t>(offset.Value()),
              blobContent.begin() + static_cast<ptrdiff_t>(offset.Value() + actualDownloadSize));
        }
        else
        {
          expectedData.clear();
        }
      }
      else if (offset.HasValue())
      {
        actualDownloadSize = blobSize - offset.Value();
        if (actualDownloadSize >= 0)
        {
          expectedData.assign(
              blobContent.begin() + static_cast<ptrdiff_t>(offset.Value()), blobContent.end());
        }
        else
        {
          expectedData.clear();
        }
      }
      downloadBuffer.resize(static_cast<size_t>(downloadSize), '\x00');
      Files::DataLake::DownloadFileToOptions options;
      options.TransferOptions.Concurrency = concurrency;
      if (offset.HasValue() || length.HasValue())
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
        auto res = fileClient.DownloadTo(downloadBuffer.data(), downloadBuffer.size(), options);
        EXPECT_EQ(res.Value.FileSize, blobSize);
        EXPECT_EQ(res.Value.ContentRange.Length.Value(), actualDownloadSize);
        EXPECT_EQ(res.Value.ContentRange.Offset, offset.HasValue() ? offset.Value() : 0);
        downloadBuffer.resize(static_cast<size_t>(res.Value.ContentRange.Length.Value()));
        EXPECT_EQ(downloadBuffer, expectedData);
      }
      else
      {
        EXPECT_THROW(
            fileClient.DownloadTo(downloadBuffer.data(), downloadBuffer.size(), options),
            StorageException);
      }
    };

    auto testDownloadToFile = [&](int concurrency,
                                  int64_t downloadSize,
                                  Azure::Nullable<int64_t> offset = {},
                                  Azure::Nullable<int64_t> length = {},
                                  Azure::Nullable<int64_t> initialChunkSize = {},
                                  Azure::Nullable<int64_t> chunkSize = {}) {
      std::string tempFilename = RandomString() + "file" + std::to_string(concurrency);
      if (offset)
      {
        tempFilename.append(std::to_string(offset.Value()));
      }
      std::vector<uint8_t> expectedData = blobContent;
      int64_t blobSize = blobContent.size();
      int64_t actualDownloadSize = std::min(downloadSize, blobSize);
      if (offset.HasValue() && length.HasValue())
      {
        actualDownloadSize = std::min(length.Value(), blobSize - offset.Value());
        if (actualDownloadSize >= 0)
        {
          expectedData.assign(
              blobContent.begin() + static_cast<ptrdiff_t>(offset.Value()),
              blobContent.begin() + static_cast<ptrdiff_t>(offset.Value() + actualDownloadSize));
        }
        else
        {
          expectedData.clear();
        }
      }
      else if (offset.HasValue())
      {
        actualDownloadSize = blobSize - offset.Value();
        if (actualDownloadSize >= 0)
        {
          expectedData.assign(
              blobContent.begin() + static_cast<ptrdiff_t>(offset.Value()), blobContent.end());
        }
        else
        {
          expectedData.clear();
        }
      }
      Files::DataLake::DownloadFileToOptions options;
      options.TransferOptions.Concurrency = concurrency;
      if (offset.HasValue() || length.HasValue())
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
        auto res = fileClient.DownloadTo(tempFilename, options);
        EXPECT_EQ(res.Value.FileSize, blobSize);
        EXPECT_EQ(res.Value.ContentRange.Length.Value(), actualDownloadSize);
        EXPECT_EQ(res.Value.ContentRange.Offset, offset.HasValue() ? offset.Value() : 0);
        EXPECT_EQ(ReadFile(tempFilename), expectedData);
      }
      else
      {
        EXPECT_THROW(fileClient.DownloadTo(tempFilename, options), StorageException);
      }
      DeleteFile(tempFilename);
    };

    const int64_t blobSize = blobContent.size();
    for (int c : {1, 2, 4})
    {
      std::vector<std::future<void>> futures;
      // random range
      for (int i = 0; i < 16; ++i)
      {
        int64_t offset = RandomInt(0, blobContent.size() - 1);
        int64_t length = RandomInt(1, 64_KB);
        futures.emplace_back(std::async(
            std::launch::async, testDownloadToBuffer, c, blobSize, offset, length, 8_KB, 4_KB));
        futures.emplace_back(std::async(
            std::launch::async, testDownloadToFile, c, blobSize, offset, length, 4_KB, 7_KB));
      }

      // buffer not big enough
      Files::DataLake::DownloadFileToOptions options;
      options.TransferOptions.Concurrency = c;
      options.Range = Core::Http::HttpRange();
      options.Range.Value().Offset = 1;
      for (int64_t length : {1ULL, 2ULL, 4_KB, 5_KB, 8_KB, 11_KB, 20_KB})
      {
        std::vector<uint8_t> downloadBuffer;
        downloadBuffer.resize(static_cast<size_t>(length - 1));
        options.Range.Value().Length = length;
        EXPECT_THROW(
            fileClient.DownloadTo(downloadBuffer.data(), static_cast<size_t>(length - 1), options),
            std::runtime_error);
      }
      for (auto& f : futures)
      {
        f.get();
      }
    }
  }

  TEST_F(DataLakeFileClientTest, ConcurrentUpload_LIVEONLY_)
  {
    const auto blobContent = RandomBuffer(static_cast<size_t>(8_MB));

    auto testUploadFromBuffer = [&](int concurrency,
                                    int64_t bufferSize,
                                    Azure::Nullable<int64_t> singleUploadThreshold = {},
                                    Azure::Nullable<int64_t> chunkSize = {}) {
      Files::DataLake::UploadFileFromOptions options;
      options.TransferOptions.Concurrency = concurrency;
      if (singleUploadThreshold.HasValue())
      {
        options.TransferOptions.SingleUploadThreshold = singleUploadThreshold.Value();
      }
      if (chunkSize.HasValue())
      {
        options.TransferOptions.ChunkSize = chunkSize.Value();
      }

      auto fileClient = m_fileSystemClient->GetFileClient(RandomString());
      EXPECT_NO_THROW(
          fileClient.UploadFrom(blobContent.data(), static_cast<size_t>(bufferSize), options));
      std::vector<uint8_t> downloadBuffer(static_cast<size_t>(bufferSize), '\x00');
      Files::DataLake::DownloadFileToOptions downloadOptions;
      downloadOptions.TransferOptions.Concurrency = 1;
      ASSERT_NO_THROW(
          fileClient.DownloadTo(downloadBuffer.data(), downloadBuffer.size(), downloadOptions));
      std::vector<uint8_t> expectedData(
          blobContent.begin(), blobContent.begin() + static_cast<size_t>(bufferSize));
      EXPECT_EQ(downloadBuffer, expectedData);
    };

    auto testUploadFromFile = [&](int concurrency,
                                  int64_t fileSize,
                                  Azure::Nullable<int64_t> singleUploadThreshold = {},
                                  Azure::Nullable<int64_t> chunkSize = {}) {
      Files::DataLake::UploadFileFromOptions options;
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
      auto fileClient = m_fileSystemClient->GetFileClient(RandomString());
      EXPECT_NO_THROW(fileClient.UploadFrom(tempFileName, options));
      DeleteFile(tempFileName);
      std::vector<uint8_t> downloadBuffer(static_cast<size_t>(fileSize), '\x00');
      Files::DataLake::DownloadFileToOptions downloadOptions;
      downloadOptions.TransferOptions.Concurrency = 1;
      ASSERT_NO_THROW(
          fileClient.DownloadTo(downloadBuffer.data(), downloadBuffer.size(), downloadOptions));
      std::vector<uint8_t> expectedData(
          blobContent.begin(), blobContent.begin() + static_cast<size_t>(fileSize));
      EXPECT_EQ(downloadBuffer, expectedData);
    };

    for (int c : {1, 2, 4})
    {
      std::vector<std::future<void>> futures;
      // random range
      for (int i = 0; i < 16; ++i)
      {
        int64_t fileSize = RandomInt(1, 1_MB);
        futures.emplace_back(
            std::async(std::launch::async, testUploadFromBuffer, c, fileSize, 4_KB, 56_KB));
        futures.emplace_back(
            std::async(std::launch::async, testUploadFromFile, c, fileSize, 2_KB, 172_KB));
        futures.emplace_back(
            std::async(std::launch::async, testUploadFromBuffer, c, fileSize, 0, 109_KB));
        futures.emplace_back(
            std::async(std::launch::async, testUploadFromFile, c, fileSize, 0, 256_KB));
      }
      for (auto& f : futures)
      {
        f.get();
      }
    }
  }

}}} // namespace Azure::Storage::Test
