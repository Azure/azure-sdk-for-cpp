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
#include <azure/storage/common/internal/file_io.hpp>
#include <azure/storage/common/internal/shared_key_policy.hpp>

namespace Azure { namespace Storage { namespace Files { namespace DataLake { namespace Models {

  bool operator==(const PathHttpHeaders& lhs, const PathHttpHeaders& rhs)
  {
    return lhs.ContentType == rhs.ContentType && lhs.ContentEncoding == rhs.ContentEncoding
        && lhs.ContentLanguage == rhs.ContentLanguage && lhs.CacheControl == rhs.CacheControl
        && lhs.ContentDisposition == rhs.ContentDisposition;
  }

}}}}} // namespace Azure::Storage::Files::DataLake::Models

namespace Azure { namespace Storage { namespace Test {

  std::shared_ptr<Files::DataLake::DataLakeFileClient> DataLakeFileClientTest::m_fileClient;
  std::string DataLakeFileClientTest::m_fileName;

  void DataLakeFileClientTest::SetUpTestSuite()
  {
    DataLakeFileSystemClientTest::SetUpTestSuite();
    m_fileName = RandomString();
    m_fileClient = std::make_shared<Files::DataLake::DataLakeFileClient>(
        m_fileSystemClient->GetFileClient(m_fileName));
    m_fileClient->Create();
  }

  void DataLakeFileClientTest::TearDownTestSuite()
  {
    m_fileSystemClient->GetFileClient(m_fileName).Delete();
    DataLakeFileSystemClientTest::TearDownTestSuite();
  }

  TEST_F(DataLakeFileClientTest, CreateDeleteFiles)
  {
    {
      // Normal create/delete.
      std::vector<Files::DataLake::DataLakeFileClient> fileClient;
      for (int32_t i = 0; i < 5; ++i)
      {
        auto client = m_fileSystemClient->GetFileClient(RandomString());
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
        auto client = m_fileSystemClient->GetFileClient(RandomString());
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
        auto client = m_fileSystemClient->GetFileClient(RandomString());
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
      auto client = m_fileSystemClient->GetFileClient(RandomString());
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
      auto client = Files::DataLake::DataLakeFileClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(), LowercaseRandomString(), RandomString());
      bool deleted = false;
      EXPECT_NO_THROW(deleted = client.DeleteIfExists().Value.Deleted);
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
      auto client1 = m_fileSystemClient->GetFileClient(RandomString());
      auto client2 = m_fileSystemClient->GetFileClient(RandomString());
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
      auto httpHeader = GetInterestingHttpHeaders();
      std::vector<Files::DataLake::DataLakeFileClient> fileClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetFileClient(RandomString());
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
    auto buffer = RandomBuffer(bufferSize);
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

  TEST_F(DataLakeFileClientTest, FileReadReturns)
  {
    const int32_t bufferSize = 4 * 1024; // 4KB data size
    auto buffer = RandomBuffer(bufferSize);
    auto bufferStream = std::make_unique<Azure::Core::IO::MemoryBodyStream>(
        Azure::Core::IO::MemoryBodyStream(buffer));
    auto newFileName = RandomString();
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

    std::string tempFilename = RandomString();
    EXPECT_NO_THROW(fileClient.DownloadTo(tempFilename));
    EXPECT_TRUE(ReadFile(tempFilename).empty());
    DeleteFile(tempFilename);

    std::vector<uint8_t> buff;
    EXPECT_NO_THROW(fileClient.DownloadTo(buff.data(), 0));
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

  TEST_F(DataLakeFileClientTest, ConcurrentUploadDownload)
  {
    std::vector<uint8_t> fileContent = RandomBuffer(static_cast<size_t>(8_MB));

    auto testUploadFromBuffer = [&](int concurrency, int64_t fileSize) {
      auto fileClient = m_fileSystemClient->GetFileClient(RandomString());

      Azure::Storage::Files::DataLake::UploadFileFromOptions options;
      options.TransferOptions.ChunkSize = 1_MB;
      options.TransferOptions.Concurrency = concurrency;
      options.HttpHeaders = GetInterestingHttpHeaders();
      options.Metadata = RandomMetadata();
      auto res = fileClient.UploadFrom(fileContent.data(), static_cast<size_t>(fileSize), options);
      auto lastModified = fileClient.GetProperties().Value.LastModified;
      EXPECT_TRUE(res.Value.ETag.HasValue());
      EXPECT_TRUE(IsValidTime(res.Value.LastModified));
      EXPECT_EQ(res.Value.LastModified, lastModified);
      auto properties = fileClient.GetProperties().Value;
      EXPECT_EQ(properties.FileSize, fileSize);
      EXPECT_EQ(properties.HttpHeaders, options.HttpHeaders);
      EXPECT_EQ(properties.Metadata, options.Metadata);
      EXPECT_EQ(properties.ETag, res.Value.ETag);
      EXPECT_TRUE(IsValidTime(res.Value.LastModified));
      EXPECT_EQ(properties.LastModified, res.Value.LastModified);
      std::vector<uint8_t> downloadContent(static_cast<size_t>(fileSize), '\x00');
      fileClient.DownloadTo(downloadContent.data(), static_cast<size_t>(fileSize));
      EXPECT_EQ(
          downloadContent,
          std::vector<uint8_t>(
              fileContent.begin(), fileContent.begin() + static_cast<size_t>(fileSize)));
    };

    auto testUploadFromFile = [&](int concurrency, int64_t fileSize) {
      auto fileClient = m_fileSystemClient->GetFileClient(RandomString());

      Azure::Storage::Files::DataLake::UploadFileFromOptions options;
      options.TransferOptions.ChunkSize = 1_MB;
      options.TransferOptions.Concurrency = concurrency;
      options.HttpHeaders = GetInterestingHttpHeaders();
      options.Metadata = RandomMetadata();

      std::string tempFilename = RandomString();
      {
        Azure::Storage::_internal::FileWriter fileWriter(tempFilename);
        fileWriter.Write(fileContent.data(), static_cast<size_t>(fileSize), 0);
      }
      auto res = fileClient.UploadFrom(tempFilename, options);
      auto lastModified = fileClient.GetProperties().Value.LastModified;
      EXPECT_TRUE(res.Value.ETag.HasValue());
      EXPECT_TRUE(IsValidTime(res.Value.LastModified));
      EXPECT_EQ(res.Value.LastModified, lastModified);
      auto properties = fileClient.GetProperties().Value;
      EXPECT_EQ(properties.FileSize, fileSize);
      EXPECT_EQ(properties.HttpHeaders, options.HttpHeaders);
      EXPECT_EQ(properties.Metadata, options.Metadata);
      EXPECT_EQ(properties.ETag, res.Value.ETag);
      EXPECT_EQ(properties.LastModified, res.Value.LastModified);
      std::vector<uint8_t> downloadContent(static_cast<size_t>(fileSize), '\x00');
      fileClient.DownloadTo(downloadContent.data(), static_cast<size_t>(fileSize));
      EXPECT_EQ(
          downloadContent,
          std::vector<uint8_t>(
              fileContent.begin(), fileContent.begin() + static_cast<size_t>(fileSize)));
      std::string tempFileDestinationName = RandomString();
      fileClient.DownloadTo(tempFileDestinationName);
      Azure::Storage::_internal::FileReader fileReader(tempFileDestinationName);
      auto size = fileReader.GetFileSize();
      EXPECT_EQ(fileSize, size);
      DeleteFile(tempFileDestinationName);
      DeleteFile(tempFilename);
    };

    std::vector<std::future<void>> futures;
    for (int c : {1, 2, 5})
    {
      for (int64_t l :
           {0ULL, 1ULL, 2ULL, 2_KB, 4_KB, 999_KB, 1_MB, 2_MB - 1, 3_MB, 5_MB, 8_MB - 1234, 8_MB})
      {
        ASSERT_GE(fileContent.size(), static_cast<size_t>(l));
        futures.emplace_back(std::async(std::launch::async, testUploadFromBuffer, c, l));
        futures.emplace_back(std::async(std::launch::async, testUploadFromFile, c, l));
      }
    }
    for (auto& f : futures)
    {
      f.get();
    }
  }

  TEST_F(DataLakeFileClientTest, ConstructorsWorks)
  {
    {
      // Create from connection string validates static creator function and shared key
      // constructor.
      auto fileName = RandomString();
      auto connectionStringClient
          = Azure::Storage::Files::DataLake::DataLakeFileClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(), m_fileSystemName, fileName);
      EXPECT_NO_THROW(connectionStringClient.Create());
      EXPECT_NO_THROW(connectionStringClient.Delete());
    }

    {
      // Create from client secret credential.
      auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
          AadTenantId(), AadClientId(), AadClientSecret());

      auto clientSecretClient = Azure::Storage::Files::DataLake::DataLakeFileClient(
          Azure::Storage::Files::DataLake::_detail::GetDfsUrlFromUrl(
              Azure::Storage::Files::DataLake::DataLakeFileClient::CreateFromConnectionString(
                  AdlsGen2ConnectionString(), m_fileSystemName, RandomString())
                  .GetUrl()),
          credential);

      EXPECT_NO_THROW(clientSecretClient.Create());
      EXPECT_NO_THROW(clientSecretClient.Delete());
    }

    {
      // Create from Anonymous credential.
      std::vector<uint8_t> blobContent;
      blobContent.resize(static_cast<size_t>(1_MB));
      RandomBuffer(reinterpret_cast<char*>(&blobContent[0]), blobContent.size());
      auto objectName = RandomString();
      auto containerClient = Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(), m_fileSystemName);
      Azure::Storage::Blobs::SetBlobContainerAccessPolicyOptions options;
      options.AccessType = Azure::Storage::Blobs::Models::PublicAccessType::Blob;
      containerClient.SetAccessPolicy(options);
      auto blobClient = containerClient.GetBlockBlobClient(objectName);
      auto memoryStream = Azure::Core::IO::MemoryBodyStream(blobContent.data(), blobContent.size());
      EXPECT_NO_THROW(blobClient.Upload(memoryStream));

      auto anonymousClient = Azure::Storage::Files::DataLake::DataLakeFileClient(
          Azure::Storage::Files::DataLake::DataLakeFileClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(), m_fileSystemName, objectName)
              .GetUrl());

      std::this_thread::sleep_for(std::chrono::seconds(30));

      EXPECT_NO_THROW(anonymousClient.Download());
    }
  }
}}} // namespace Azure::Storage::Test
