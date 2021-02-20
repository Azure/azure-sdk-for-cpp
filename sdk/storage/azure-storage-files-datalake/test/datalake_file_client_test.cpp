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
#include <azure/storage/common/file_io.hpp>
#include <azure/storage/common/shared_key_policy.hpp>
#include <azure/storage/files/datalake/datalake_utilities.hpp>

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
    m_fileName = RandomString(10);
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
        EXPECT_FALSE(response->IsDirectory);
        Files::DataLake::DeleteDataLakeFileOptions options1;
        options1.AccessConditions.IfModifiedSince = response->LastModified;
        EXPECT_TRUE(IsValidTime(response->LastModified));
        EXPECT_THROW(client.Delete(options1), StorageException);
        Files::DataLake::DeleteDataLakeFileOptions options2;
        options2.AccessConditions.IfUnmodifiedSince = response->LastModified;
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
        Files::DataLake::DeleteDataLakeFileOptions options1;
        options1.AccessConditions.IfNoneMatch = response->ETag;
        EXPECT_THROW(client.Delete(options1), StorageException);
        Files::DataLake::DeleteDataLakeFileOptions options2;
        options2.AccessConditions.IfMatch = response->ETag;
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
      EXPECT_NO_THROW(created = client.Create()->Created);
      EXPECT_TRUE(created);
      EXPECT_NO_THROW(created = client.CreateIfNotExists()->Created);
      EXPECT_FALSE(created);
      EXPECT_NO_THROW(deleted = client.Delete()->Deleted);
      EXPECT_TRUE(deleted);
      EXPECT_NO_THROW(deleted = client.DeleteIfExists()->Deleted);
      EXPECT_FALSE(deleted);
    }
    {
      auto client = Files::DataLake::DataLakeFileClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(), LowercaseRandomString(), RandomString());
      bool deleted = false;
      EXPECT_NO_THROW(deleted = client.DeleteIfExists()->Deleted);
      EXPECT_FALSE(deleted);
    }
  }

  TEST_F(DataLakeFileClientTest, RenameFiles)
  {
    {
      // Normal create/rename/delete.
      auto fileName = RandomString();
      EXPECT_NO_THROW(m_fileSystemClient->GetFileClient(fileName).Create());
      std::shared_ptr<Files::DataLake::DataLakeFileClient> ret;
      EXPECT_NO_THROW(
          ret = std::make_shared<Files::DataLake::DataLakeFileClient>(
              m_fileSystemClient->RenameFile(fileName, RandomString()).ExtractValue()));
      EXPECT_THROW(m_fileSystemClient->GetFileClient(fileName).Delete(), StorageException);
      EXPECT_NO_THROW(ret->Delete());
    }
    {
      // Normal rename with last modified access condition.
      auto fileName = RandomString();
      auto fileClient = m_fileSystemClient->GetFileClient(fileName);
      EXPECT_NO_THROW(fileClient.Create());
      auto response = fileClient.GetProperties();
      Files::DataLake::RenameDataLakeFileOptions options1;
      options1.SourceAccessConditions.IfModifiedSince = response->LastModified;
      EXPECT_THROW(
          m_fileSystemClient->RenameFile(fileName, RandomString(), options1), StorageException);
      Files::DataLake::RenameDataLakeFileOptions options2;
      options2.SourceAccessConditions.IfUnmodifiedSince = response->LastModified;
      std::shared_ptr<Files::DataLake::DataLakeFileClient> ret;
      EXPECT_NO_THROW(
          ret = std::make_shared<Files::DataLake::DataLakeFileClient>(
              m_fileSystemClient->RenameFile(fileName, RandomString(), options2).ExtractValue()));
      EXPECT_THROW(m_fileSystemClient->GetFileClient(fileName).Delete(), StorageException);
      EXPECT_NO_THROW(ret->Delete());
    }
    {
      // Normal rename with if match access condition.
      auto fileName = RandomString();
      auto fileClient = m_fileSystemClient->GetFileClient(fileName);
      EXPECT_NO_THROW(fileClient.Create());
      auto response = fileClient.GetProperties();
      Files::DataLake::RenameDataLakeFileOptions options1;
      options1.SourceAccessConditions.IfNoneMatch = response->ETag;
      EXPECT_THROW(
          m_fileSystemClient->RenameFile(fileName, RandomString(), options1), StorageException);
      Files::DataLake::RenameDataLakeFileOptions options2;
      options2.SourceAccessConditions.IfMatch = response->ETag;
      std::shared_ptr<Files::DataLake::DataLakeFileClient> ret;
      EXPECT_NO_THROW(
          ret = std::make_shared<Files::DataLake::DataLakeFileClient>(
              m_fileSystemClient->RenameFile(fileName, RandomString(), options2).ExtractValue()));
      EXPECT_THROW(m_fileSystemClient->GetFileClient(fileName).Delete(), StorageException);
      EXPECT_THROW(fileClient.GetProperties(), StorageException);
      EXPECT_NO_THROW(ret->Delete());
    }
    {
      // Rename to a destination file system.
      auto fileName = RandomString();
      auto fileClient = m_fileSystemClient->GetFileClient(fileName);
      EXPECT_NO_THROW(fileClient.Create());
      {
        // Rename to a non-existing file system will fail but will not change URI.
        Files::DataLake::RenameDataLakeFileOptions options;
        options.DestinationFileSystem = LowercaseRandomString();
        EXPECT_THROW(
            m_fileSystemClient->RenameFile(fileName, RandomString(), options), StorageException);
        EXPECT_NO_THROW(fileClient.GetProperties());
      }
      {
        // Rename to an existing file system will succeed and changes URI.
        auto newfileSystemName = LowercaseRandomString(10);
        auto newfileSystemClient = std::make_shared<Files::DataLake::DataLakeFileSystemClient>(
            Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
                AdlsGen2ConnectionString(), newfileSystemName));
        newfileSystemClient->Create();
        Files::DataLake::RenameDataLakeFileOptions options;
        options.DestinationFileSystem = newfileSystemName;
        std::shared_ptr<Files::DataLake::DataLakeFileClient> ret;
        EXPECT_NO_THROW(
            ret = std::make_shared<Files::DataLake::DataLakeFileClient>(
                m_fileSystemClient->RenameFile(fileName, RandomString(), options).ExtractValue()));
        EXPECT_THROW(fileClient.GetProperties(), StorageException);
        EXPECT_NO_THROW(ret->Delete());
      }
    }
  }

  TEST_F(DataLakeFileClientTest, FileMetadata)
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
      // Create path with metadata works
      auto client1 = m_fileSystemClient->GetFileClient(RandomString());
      auto client2 = m_fileSystemClient->GetFileClient(RandomString());
      Files::DataLake::CreateDataLakeFileOptions options1;
      Files::DataLake::CreateDataLakeFileOptions options2;
      options1.Metadata = metadata1;
      options2.Metadata = metadata2;

      EXPECT_NO_THROW(client1.Create(options1));
      EXPECT_NO_THROW(client2.Create(options2));
      auto result = client1.GetProperties()->Metadata;
      EXPECT_EQ(metadata1, result);
      result = client2.GetProperties()->Metadata;
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
      EXPECT_EQ(metadata1, result->Metadata);
      EXPECT_NO_THROW(m_fileClient->SetMetadata(metadata2));
      result = m_fileClient->GetProperties();
      EXPECT_EQ(metadata2, result->Metadata);
    }

    {
      // Last modified Etag works.
      auto properties1 = m_fileClient->GetProperties();
      auto properties2 = m_fileClient->GetProperties();
      EXPECT_EQ(properties1->ETag, properties2->ETag);
      EXPECT_TRUE(IsValidTime(properties1->LastModified));
      EXPECT_EQ(properties1->LastModified, properties2->LastModified);

      // This operation changes ETag/LastModified.
      EXPECT_NO_THROW(m_fileClient->SetMetadata(metadata1));

      auto properties3 = m_fileClient->GetProperties();
      EXPECT_NE(properties1->ETag, properties3->ETag);
    }

    {
      // Http headers works.
      auto httpHeader = GetInterestingHttpHeaders();
      std::vector<Files::DataLake::DataLakeFileClient> fileClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetFileClient(RandomString());
        Files::DataLake::CreateDataLakeFileOptions options;
        options.HttpHeaders = httpHeader;
        EXPECT_NO_THROW(client.Create(options));
        fileClient.emplace_back(std::move(client));
      }
      for (const auto& client : fileClient)
      {
        auto result = client.GetProperties();
        EXPECT_EQ(httpHeader.CacheControl, result->HttpHeaders.CacheControl);
        EXPECT_EQ(httpHeader.ContentDisposition, result->HttpHeaders.ContentDisposition);
        EXPECT_EQ(httpHeader.ContentLanguage, result->HttpHeaders.ContentLanguage);
        EXPECT_EQ(httpHeader.ContentType, result->HttpHeaders.ContentType);
        EXPECT_NO_THROW(client.Delete());
      }
    }
  }

  TEST_F(DataLakeFileClientTest, FileDataActions)
  {
    const int32_t bufferSize = 4 * 1024; // 4KB data size
    auto buffer = RandomBuffer(bufferSize);
    auto bufferStream = std::make_unique<Azure::Core::Http::MemoryBodyStream>(
        Azure::Core::Http::MemoryBodyStream(buffer));
    auto properties1 = m_fileClient->GetProperties();

    // Append
    m_fileClient->Append(bufferStream.get(), 0);
    auto properties2 = m_fileClient->GetProperties();
    // Append does not change etag because not committed yet.
    EXPECT_EQ(properties1->ETag, properties2->ETag);
    EXPECT_TRUE(IsValidTime(properties1->LastModified));
    EXPECT_EQ(properties1->LastModified, properties2->LastModified);

    // Flush
    m_fileClient->Flush(bufferSize);
    auto properties3 = m_fileClient->GetProperties();
    EXPECT_NE(properties2->ETag, properties3->ETag);

    // Read
    auto result = m_fileClient->Download();
    auto downloaded = ReadBodyStream(result->Body);
    EXPECT_EQ(buffer, downloaded);
  }

  TEST_F(DataLakeFileClientTest, FileReadReturns)
  {
    const int32_t bufferSize = 4 * 1024; // 4KB data size
    auto buffer = RandomBuffer(bufferSize);
    auto bufferStream = std::make_unique<Azure::Core::Http::MemoryBodyStream>(
        Azure::Core::Http::MemoryBodyStream(buffer));
    auto newFileName = RandomString(10);
    auto newFileClient = std::make_shared<Files::DataLake::DataLakeFileClient>(
        m_fileSystemClient->GetFileClient(newFileName));
    newFileClient->Create();
    auto properties1 = newFileClient->GetProperties();

    // Append
    newFileClient->Append(bufferStream.get(), 0);
    auto properties2 = newFileClient->GetProperties();
    // Append does not change etag because not committed yet.
    EXPECT_EQ(properties1->ETag, properties2->ETag);
    EXPECT_TRUE(IsValidTime(properties1->LastModified));
    EXPECT_EQ(properties1->LastModified, properties2->LastModified);

    // Flush
    newFileClient->Flush(bufferSize);
    auto properties3 = newFileClient->GetProperties();
    EXPECT_NE(properties2->ETag, properties3->ETag);

    // Read
    auto result = newFileClient->Download();
    auto downloaded = ReadBodyStream(result->Body);
    EXPECT_EQ(buffer, downloaded);
    EXPECT_EQ(bufferSize, result->FileSize);
    EXPECT_EQ(bufferSize, result->ContentRange.Length.GetValue());
    EXPECT_EQ(0, result->ContentRange.Offset);

    // Read Range
    {
      auto firstHalf = std::vector<uint8_t>(buffer.begin(), buffer.begin() + (bufferSize / 2));
      Files::DataLake::DownloadDataLakeFileOptions options;
      options.Range = Azure::Core::Http::Range();
      options.Range.GetValue().Offset = 0;
      options.Range.GetValue().Length = bufferSize / 2;
      result = newFileClient->Download(options);
      downloaded = ReadBodyStream(result->Body);
      EXPECT_EQ(firstHalf.size(), downloaded.size());
      EXPECT_EQ(firstHalf, downloaded);
      EXPECT_EQ(bufferSize, result->FileSize);
      EXPECT_EQ(bufferSize / 2, result->ContentRange.Length.GetValue());
      EXPECT_EQ(0, result->ContentRange.Offset);
    }
    {
      auto secondHalf = std::vector<uint8_t>(buffer.begin() + bufferSize / 2, buffer.end());
      Files::DataLake::DownloadDataLakeFileOptions options;
      options.Range = Azure::Core::Http::Range();
      options.Range.GetValue().Offset = bufferSize / 2;
      options.Range.GetValue().Length = bufferSize / 2;
      result = newFileClient->Download(options);
      downloaded = ReadBodyStream(result->Body);
      EXPECT_EQ(secondHalf, downloaded);
      EXPECT_EQ(bufferSize, result->FileSize);
      EXPECT_EQ(bufferSize / 2, result->ContentRange.Length.GetValue());
      EXPECT_EQ(bufferSize / 2, result->ContentRange.Offset);
    }
    {
      // Read with last modified access condition.
      auto response = newFileClient->GetProperties();
      Files::DataLake::DownloadDataLakeFileOptions options1;
      options1.AccessConditions.IfModifiedSince = response->LastModified;
      EXPECT_TRUE(IsValidTime(response->LastModified));
      EXPECT_THROW(newFileClient->Download(options1), StorageException);
      Files::DataLake::DownloadDataLakeFileOptions options2;
      options2.AccessConditions.IfUnmodifiedSince = response->LastModified;
      EXPECT_NO_THROW(result = newFileClient->Download(options2));
      downloaded = ReadBodyStream(result->Body);
      EXPECT_EQ(buffer, downloaded);
    }
    {
      // Read with if match access condition.
      auto response = newFileClient->GetProperties();
      Files::DataLake::DownloadDataLakeFileOptions options1;
      options1.AccessConditions.IfNoneMatch = response->ETag;
      EXPECT_THROW(newFileClient->Download(options1), StorageException);
      Files::DataLake::DownloadDataLakeFileOptions options2;
      options2.AccessConditions.IfMatch = response->ETag;
      EXPECT_NO_THROW(result = newFileClient->Download(options2));
      downloaded = ReadBodyStream(result->Body);
      EXPECT_EQ(buffer, downloaded);
    }
  }

  TEST_F(DataLakeFileClientTest, ScheduleForDeletion)
  {
    {
      auto client = m_fileSystemClient->GetFileClient(RandomString());
      EXPECT_NO_THROW(client.Create());
      EXPECT_NO_THROW(client.ScheduleDeletion(
          Files::DataLake::ScheduleDataLakeFileExpiryOriginType::NeverExpire));
    }
    {
      auto client = m_fileSystemClient->GetFileClient(RandomString());
      EXPECT_NO_THROW(client.Create());
      Files::DataLake::ScheduleDataLakeFileDeletionOptions options;
      EXPECT_THROW(
          client.ScheduleDeletion(
              Files::DataLake::ScheduleDataLakeFileExpiryOriginType::RelativeToNow, options),
          StorageException);
      options.TimeToExpire = std::chrono::milliseconds(1000);
      EXPECT_NO_THROW(client.ScheduleDeletion(
          Files::DataLake::ScheduleDataLakeFileExpiryOriginType::RelativeToNow, options));
    }
    {
      auto client = m_fileSystemClient->GetFileClient(RandomString());
      EXPECT_NO_THROW(client.Create());
      Files::DataLake::ScheduleDataLakeFileDeletionOptions options;
      EXPECT_THROW(
          client.ScheduleDeletion(
              Files::DataLake::ScheduleDataLakeFileExpiryOriginType::Absolute, options),
          StorageException);
      options.TimeToExpire = std::chrono::milliseconds(1000);
      EXPECT_THROW(
          client.ScheduleDeletion(
              Files::DataLake::ScheduleDataLakeFileExpiryOriginType::Absolute, options),
          StorageException);
      options.ExpiresOn = Azure::Core::DateTime::Parse(
          "Wed, 29 Sep 2100 09:53:03 GMT", Azure::Core::DateTime::DateFormat::Rfc1123);
      options.TimeToExpire = Azure::Core::Nullable<std::chrono::milliseconds>();
      EXPECT_NO_THROW(client.ScheduleDeletion(
          Files::DataLake::ScheduleDataLakeFileExpiryOriginType::Absolute, options));
    }
  }

  TEST_F(DataLakeFileClientTest, ConcurrentUploadDownload)
  {
    std::vector<uint8_t> fileContent = RandomBuffer(static_cast<std::size_t>(8_MB));

    auto testUploadFromBuffer = [&](int concurrency, int64_t fileSize) {
      auto fileClient = m_fileSystemClient->GetFileClient(RandomString());

      Azure::Storage::Files::DataLake::UploadDataLakeFileFromOptions options;
      options.TransferOptions.ChunkSize = 1_MB;
      options.TransferOptions.Concurrency = concurrency;
      options.HttpHeaders = GetInterestingHttpHeaders();
      options.Metadata = RandomMetadata();
      auto res
          = fileClient.UploadFrom(fileContent.data(), static_cast<std::size_t>(fileSize), options);
      auto lastModified = fileClient.GetProperties()->LastModified;
      EXPECT_TRUE(res->ETag.HasValue());
      EXPECT_TRUE(IsValidTime(res->LastModified));
      EXPECT_EQ(res->LastModified, lastModified);
      auto properties = *fileClient.GetProperties();
      EXPECT_EQ(properties.FileSize, fileSize);
      EXPECT_EQ(properties.HttpHeaders, options.HttpHeaders);
      EXPECT_EQ(properties.Metadata, options.Metadata);
      EXPECT_EQ(properties.ETag, res->ETag);
      EXPECT_TRUE(IsValidTime(res->LastModified));
      EXPECT_EQ(properties.LastModified, res->LastModified);
      std::vector<uint8_t> downloadContent(static_cast<std::size_t>(fileSize), '\x00');
      fileClient.DownloadTo(downloadContent.data(), static_cast<std::size_t>(fileSize));
      EXPECT_EQ(
          downloadContent,
          std::vector<uint8_t>(
              fileContent.begin(), fileContent.begin() + static_cast<std::size_t>(fileSize)));
    };

    auto testUploadFromFile = [&](int concurrency, int64_t fileSize) {
      auto fileClient = m_fileSystemClient->GetFileClient(RandomString());

      Azure::Storage::Files::DataLake::UploadDataLakeFileFromOptions options;
      options.TransferOptions.ChunkSize = 1_MB;
      options.TransferOptions.Concurrency = concurrency;
      options.HttpHeaders = GetInterestingHttpHeaders();
      options.Metadata = RandomMetadata();

      std::string tempFilename = RandomString();
      {
        Azure::Storage::Details::FileWriter fileWriter(tempFilename);
        fileWriter.Write(fileContent.data(), fileSize, 0);
      }
      auto res = fileClient.UploadFrom(tempFilename, options);
      auto lastModified = fileClient.GetProperties()->LastModified;
      EXPECT_TRUE(res->ETag.HasValue());
      EXPECT_TRUE(IsValidTime(res->LastModified));
      EXPECT_EQ(res->LastModified, lastModified);
      auto properties = *fileClient.GetProperties();
      EXPECT_EQ(properties.FileSize, fileSize);
      EXPECT_EQ(properties.HttpHeaders, options.HttpHeaders);
      EXPECT_EQ(properties.Metadata, options.Metadata);
      EXPECT_EQ(properties.ETag, res->ETag);
      EXPECT_EQ(properties.LastModified, res->LastModified);
      std::vector<uint8_t> downloadContent(static_cast<std::size_t>(fileSize), '\x00');
      fileClient.DownloadTo(downloadContent.data(), static_cast<std::size_t>(fileSize));
      EXPECT_EQ(
          downloadContent,
          std::vector<uint8_t>(
              fileContent.begin(), fileContent.begin() + static_cast<std::size_t>(fileSize)));
      std::string tempFileDestinationName = RandomString();
      fileClient.DownloadTo(tempFileDestinationName);
      Azure::Storage::Details::FileReader fileReader(tempFileDestinationName);
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

  TEST_F(DataLakeFileClientTest, ConstructorsWorks)
  {
    {
      // Create from connection string validates static creator function and shared key
      // constructor.
      auto fileName = RandomString(10);
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
          Azure::Storage::Files::DataLake::Details::GetDfsUrlFromUrl(
              Azure::Storage::Files::DataLake::DataLakeFileClient::CreateFromConnectionString(
                  AdlsGen2ConnectionString(), m_fileSystemName, RandomString(10))
                  .GetUrl()),
          credential);

      EXPECT_NO_THROW(clientSecretClient.Create());
      EXPECT_NO_THROW(clientSecretClient.Delete());
    }

    {
      // Create from Anonymous credential.
      std::vector<uint8_t> blobContent;
      blobContent.resize(static_cast<std::size_t>(1_MB));
      RandomBuffer(reinterpret_cast<char*>(&blobContent[0]), blobContent.size());
      auto objectName = RandomString(10);
      auto containerClient = Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(), m_fileSystemName);
      Azure::Storage::Blobs::SetBlobContainerAccessPolicyOptions options;
      options.AccessType = Azure::Storage::Blobs::Models::PublicAccessType::Blob;
      containerClient.SetAccessPolicy(options);
      auto blobClient = containerClient.GetBlockBlobClient(objectName);
      auto memoryStream
          = Azure::Core::Http::MemoryBodyStream(blobContent.data(), blobContent.size());
      EXPECT_NO_THROW(blobClient.Upload(&memoryStream));

      auto anonymousClient = Azure::Storage::Files::DataLake::DataLakeFileClient(
          Azure::Storage::Files::DataLake::DataLakeFileClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(), m_fileSystemName, objectName)
              .GetUrl());

      std::this_thread::sleep_for(std::chrono::seconds(30));

      EXPECT_NO_THROW(anonymousClient.Download());
    }
  }
}}} // namespace Azure::Storage::Test
