// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "datalake_file_system_client_test.hpp"

#include <algorithm>

#include <azure/core/internal/cryptography/sha_hash.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <azure/storage/common/crypt.hpp>

namespace Azure { namespace Storage { namespace Blobs { namespace Models {

  bool operator==(const SignedIdentifier& lhs, const SignedIdentifier& rhs);

}}}} // namespace Azure::Storage::Blobs::Models

namespace Azure { namespace Storage { namespace Test {

  const size_t PathTestSize = 5;

  std::string DataLakeFileSystemClientTest::GetSas()
  {
    Sas::DataLakeSasBuilder sasBuilder;
    sasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    sasBuilder.ExpiresOn = std::chrono::system_clock::now() + std::chrono::hours(72);
    sasBuilder.FileSystemName = m_fileSystemName;
    sasBuilder.Resource = Sas::DataLakeSasResource::FileSystem;
    sasBuilder.SetPermissions(Sas::DataLakeFileSystemSasPermissions::All);
    return sasBuilder.GenerateSasToken(
        *_internal::ParseConnectionString(AdlsGen2ConnectionString()).KeyCredential);
  }

  void DataLakeFileSystemClientTest::CreateDirectoryList()
  {
    std::string const directoryName(GetFileSystemValidName());
    std::string const prefix(directoryName.begin(), directoryName.end() - 2);
    m_directoryA = prefix + "a";
    m_directoryB = prefix + "b";
    m_pathNameSetA.clear();
    m_pathNameSetB.clear();
    for (size_t i = 0; i < PathTestSize; ++i)
    {
      {
        auto name = m_directoryA + "/" + std::to_string(i);
        m_fileSystemClient->GetFileClient(name).Create();
        m_pathNameSetA.emplace_back(std::move(name));
      }
      {
        auto name = m_directoryB + "/" + std::to_string(i);
        m_fileSystemClient->GetFileClient(name).Create();
        m_pathNameSetB.emplace_back(std::move(name));
      }
    }
  }

  void DataLakeFileSystemClientTest::SetUp()
  {
    DataLakeServiceClientTest::SetUp();
    CHECK_SKIP_TEST();

    m_fileSystemName = GetFileSystemValidName();
    m_fileSystemClient = std::make_shared<Files::DataLake::DataLakeFileSystemClient>(
        m_dataLakeServiceClient->GetFileSystemClient(m_fileSystemName));
    m_fileSystemClient->CreateIfNotExists();
  }

  void DataLakeFileSystemClientTest::TearDown()
  {
    CHECK_SKIP_TEST();
    m_fileSystemClient->Delete();
    DataLakeServiceClientTest::TearDown();
  }

  std::vector<Files::DataLake::Models::PathItem> DataLakeFileSystemClientTest::ListAllPaths(
      bool recursive,
      const std::string& directory)
  {
    std::vector<Files::DataLake::Models::PathItem> result;
    std::string continuation;
    Files::DataLake::ListPathsOptions options;
    if (directory.empty())
    {
      for (auto pageResult = m_fileSystemClient->ListPaths(recursive, options);
           pageResult.HasPage();
           pageResult.MoveToNextPage())
      {
        result.insert(result.end(), pageResult.Paths.begin(), pageResult.Paths.end());
      }
    }
    else
    {
      auto directoryClient = m_fileSystemClient->GetDirectoryClient(directory);
      for (auto pageResult = directoryClient.ListPaths(recursive, options); pageResult.HasPage();
           pageResult.MoveToNextPage())
      {
        result.insert(result.end(), pageResult.Paths.begin(), pageResult.Paths.end());
      }
    }

    return result;
  }

  Files::DataLake::Models::PathHttpHeaders DataLakeFileSystemClientTest::GetInterestingHttpHeaders()
  {
    static Files::DataLake::Models::PathHttpHeaders result = []() {
      Files::DataLake::Models::PathHttpHeaders ret;
      ret.CacheControl = std::string("no-cache");
      ret.ContentDisposition = std::string("attachment");
      ret.ContentEncoding = std::string("deflate");
      ret.ContentLanguage = std::string("en-US");
      ret.ContentType = std::string("application/octet-stream");
      return ret;
    }();
    return result;
  }

  TEST_F(DataLakeFileSystemClientTest, CreateDeleteFileSystems)
  {
    std::string fileSystemName("prefix");
    {
      // Normal create/delete.
      std::vector<Files::DataLake::DataLakeFileSystemClient> fileSystemClient;
      for (int32_t i = 0; i < 5; ++i)
      {
        auto client = Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
            AdlsGen2ConnectionString(),
            fileSystemName + std::to_string(i),
            InitClientOptions<Files::DataLake::DataLakeClientOptions>());
        EXPECT_NO_THROW(client.Create());
        fileSystemClient.emplace_back(std::move(client));
      }
      for (const auto& client : fileSystemClient)
      {
        EXPECT_NO_THROW(client.Delete());
      }
    }
    {
      std::string fileSystemNameAccessCondition(fileSystemName + "a");
      // Normal delete with access condition.
      std::vector<Files::DataLake::DataLakeFileSystemClient> fileSystemClient;
      for (int32_t i = 0; i < 5; ++i)
      {
        auto client = Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
            AdlsGen2ConnectionString(),
            fileSystemNameAccessCondition + std::to_string(i),
            InitClientOptions<Files::DataLake::DataLakeClientOptions>());
        EXPECT_NO_THROW(client.Create());
        fileSystemClient.emplace_back(std::move(client));
      }
      for (const auto& client : fileSystemClient)
      {
        auto response = client.GetProperties();
        Files::DataLake::DeleteFileSystemOptions options1;
        options1.AccessConditions.IfModifiedSince = response.Value.LastModified;
        EXPECT_THROW(client.Delete(options1), StorageException);
        Files::DataLake::DeleteFileSystemOptions options2;
        options2.AccessConditions.IfUnmodifiedSince = response.Value.LastModified;
        EXPECT_NO_THROW(client.Delete(options2));
      }
    }
    {
      // CreateIfNotExists & DeleteIfExists.
      {
        std::string fileSystemNameCreate(fileSystemName + "c");
        auto client = Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
            AdlsGen2ConnectionString(),
            fileSystemNameCreate,
            InitClientOptions<Files::DataLake::DataLakeClientOptions>());
        EXPECT_NO_THROW(client.Create());
        EXPECT_NO_THROW(client.CreateIfNotExists());
        EXPECT_NO_THROW(client.Delete());
        EXPECT_NO_THROW(client.DeleteIfExists());
      }
      {
        std::string fileSystemNameCreateIf(fileSystemName + "ci");
        auto client = Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
            AdlsGen2ConnectionString(),
            fileSystemNameCreateIf,
            InitClientOptions<Files::DataLake::DataLakeClientOptions>());
        EXPECT_NO_THROW(client.CreateIfNotExists());
        EXPECT_THROW(client.Create(), StorageException);
        EXPECT_NO_THROW(client.DeleteIfExists());
      }
      {
        std::string fileSystemNameCreateIfNot(fileSystemName + "cid");
        auto client = Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
            AdlsGen2ConnectionString(),
            fileSystemNameCreateIfNot,
            InitClientOptions<Files::DataLake::DataLakeClientOptions>());
        auto created = client.Create().Value.Created;
        EXPECT_TRUE(created);
        auto createResult = client.CreateIfNotExists();
        EXPECT_FALSE(createResult.Value.Created);
        EXPECT_FALSE(createResult.Value.ETag.HasValue());
        EXPECT_EQ(DateTime(), createResult.Value.LastModified);
        auto deleted = client.Delete().Value.Deleted;
        EXPECT_TRUE(deleted);
      }
      {
        std::string fileSystemNameDelete(fileSystemName + "d");
        auto client = Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
            AdlsGen2ConnectionString(),
            fileSystemNameDelete,
            InitClientOptions<Files::DataLake::DataLakeClientOptions>());
        auto deleteResult = client.DeleteIfExists();
        EXPECT_FALSE(deleteResult.Value.Deleted);
      }
    }
  }

  TEST_F(DataLakeFileSystemClientTest, FileSystemMetadata)
  {
    auto metadata1 = GetMetadata();
    auto metadata2 = GetMetadata();
    {
      // Set/Get Metadata works
      EXPECT_NO_THROW(m_fileSystemClient->SetMetadata(metadata1));
      auto result = m_fileSystemClient->GetProperties().Value.Metadata;
      EXPECT_EQ(metadata1, result);
      EXPECT_NO_THROW(m_fileSystemClient->SetMetadata(metadata2));
      result = m_fileSystemClient->GetProperties().Value.Metadata;
      EXPECT_EQ(metadata2, result);
    }

    {
      // Create file system with metadata works
      auto options = InitClientOptions<Files::DataLake::DataLakeClientOptions>();
      auto client1 = Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(), m_fileSystemName + "1", options);
      auto client2 = Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(), m_fileSystemName + "2", options);
      Files::DataLake::CreateFileSystemOptions options1;
      Files::DataLake::CreateFileSystemOptions options2;
      options1.Metadata = metadata1;
      options2.Metadata = metadata2;

      EXPECT_NO_THROW(client1.Create(options1));
      EXPECT_NO_THROW(client2.Create(options2));
      auto result = client1.GetProperties().Value.Metadata;
      EXPECT_EQ(metadata1, result);
      result = client2.GetProperties().Value.Metadata;
      EXPECT_EQ(metadata2, result);
      client1.DeleteIfExists();
      client2.DeleteIfExists();
    }
  }

  TEST_F(DataLakeFileSystemClientTest, GetDataLakeFileSystemPropertiesResult)
  {
    auto metadata1 = GetMetadata();
    auto metadata2 = GetMetadata();
    {
      // Get Metadata via properties works
      EXPECT_NO_THROW(m_fileSystemClient->SetMetadata(metadata1));
      auto result = m_fileSystemClient->GetProperties();
      EXPECT_EQ(metadata1, result.Value.Metadata);
      EXPECT_NO_THROW(m_fileSystemClient->SetMetadata(metadata2));
      result = m_fileSystemClient->GetProperties();
      EXPECT_EQ(metadata2, result.Value.Metadata);
    }

    {
      // Last modified Etag works.
      auto properties1 = m_fileSystemClient->GetProperties();
      auto properties2 = m_fileSystemClient->GetProperties();
      EXPECT_EQ(properties1.Value.ETag, properties2.Value.ETag);
      EXPECT_EQ(properties1.Value.LastModified, properties2.Value.LastModified);

      // This operation changes ETag/LastModified.
      EXPECT_NO_THROW(m_fileSystemClient->SetMetadata(metadata1));

      auto properties3 = m_fileSystemClient->GetProperties();
      EXPECT_NE(properties1.Value.ETag, properties3.Value.ETag);
    }
  }

  TEST_F(DataLakeFileSystemClientTest, ListPaths)
  {
    CreateDirectoryList();
    {
      // Normal list recursively.
      auto result = ListAllPaths(true);
      for (const auto& name : m_pathNameSetA)
      {
        auto iter = std::find_if(
            result.begin(), result.end(), [&name](const Files::DataLake::Models::PathItem& path) {
              return path.Name == name;
            });
        EXPECT_NE(result.end(), iter);
        EXPECT_EQ(iter->Name, name);
        EXPECT_EQ(iter->Name.substr(0U, m_directoryA.size()), m_directoryA);
        EXPECT_TRUE(iter->CreatedOn.HasValue());
        EXPECT_FALSE(iter->ExpiresOn.HasValue());
      }
      for (const auto& name : m_pathNameSetB)
      {
        auto iter = std::find_if(
            result.begin(), result.end(), [&name](const Files::DataLake::Models::PathItem& path) {
              return path.Name == name;
            });
        EXPECT_NE(result.end(), iter);
        EXPECT_EQ(iter->Name, name);
        EXPECT_EQ(iter->Name.substr(0U, m_directoryB.size()), m_directoryB);
        EXPECT_TRUE(iter->CreatedOn.HasValue());
        EXPECT_FALSE(iter->ExpiresOn.HasValue());
      }
    }
    {
      // List with directory.
      auto result = ListAllPaths(true, m_directoryA);
      for (const auto& name : m_pathNameSetA)
      {
        auto iter = std::find_if(
            result.begin(), result.end(), [&name](const Files::DataLake::Models::PathItem& path) {
              return path.Name == name;
            });
        EXPECT_NE(result.end(), iter);
        EXPECT_EQ(iter->Name, name);
        EXPECT_EQ(iter->Name.substr(0U, m_directoryA.size()), m_directoryA);
        EXPECT_TRUE(iter->CreatedOn.HasValue());
        EXPECT_FALSE(iter->ExpiresOn.HasValue());
      }
      for (const auto& name : m_pathNameSetB)
      {
        auto iter = std::find_if(
            result.begin(), result.end(), [&name](const Files::DataLake::Models::PathItem& path) {
              return path.Name == name;
            });
        EXPECT_EQ(result.end(), iter);
      }
    }
    {
      // List max result
      Files::DataLake::ListPathsOptions options;
      options.PageSizeHint = 2;
      auto response = m_fileSystemClient->ListPaths(true, options);
      EXPECT_LE(2U, response.Paths.size());
    }
    {
      // check expiry time
      const std::string filename = GetTestNameLowerCase() + "check_expiry";
      auto client = m_fileSystemClient->GetFileClient(GetTestNameLowerCase() + "check_expiry");
      Files::DataLake::CreateFileOptions createOptions;
      createOptions.ScheduleDeletionOptions.ExpiresOn = Azure::DateTime::Parse(
          "Wed, 29 Sep 2100 09:53:03 GMT", Azure::DateTime::DateFormat::Rfc1123);
      client.Create(createOptions);

      auto result = ListAllPaths(false);
      auto iter = std::find_if(
          result.begin(), result.end(), [&filename](const Files::DataLake::Models::PathItem& path) {
            return path.Name == filename;
          });
      EXPECT_TRUE(iter->ExpiresOn.HasValue());
      EXPECT_EQ(createOptions.ScheduleDeletionOptions.ExpiresOn.Value(), iter->ExpiresOn.Value());
    }
  }

  TEST_F(DataLakeFileSystemClientTest, UnencodedPathDirectoryFileNameWorks)
  {
    const std::string non_ascii_word = "\xE6\xB5\x8B\xE8\xAF\x95";
    const std::string encoded_non_ascii_word = "%E6%B5%8B%E8%AF%95";
    std::string baseName = "a b c / !@#$%^&*(?/<>,.;:'\"[]{}|`~\\) def" + non_ascii_word;
    {
      std::string pathName = baseName + GetTestNameLowerCase();
      auto fileClient = m_fileSystemClient->GetFileClient(pathName);
      EXPECT_NO_THROW(fileClient.Create());
      auto fileUrl = fileClient.GetUrl();
      EXPECT_EQ(fileUrl, m_fileSystemClient->GetUrl() + "/" + _internal::UrlEncodePath(pathName));
    }
    {
      std::string directoryName = baseName + GetTestNameLowerCase() + "1";
      auto directoryClient = m_fileSystemClient->GetDirectoryClient(directoryName);
      EXPECT_NO_THROW(directoryClient.Create());
      auto directoryUrl = directoryClient.GetUrl();
      EXPECT_EQ(
          directoryUrl,
          m_fileSystemClient->GetUrl() + "/" + _internal::UrlEncodePath(directoryName));
    }
    {
      std::string fileName = baseName + GetTestNameLowerCase() + "2";
      auto fileClient = m_fileSystemClient->GetFileClient(fileName);
      EXPECT_NO_THROW(fileClient.Create());
      auto fileUrl = fileClient.GetUrl();
      EXPECT_EQ(fileUrl, m_fileSystemClient->GetUrl() + "/" + _internal::UrlEncodePath(fileName));
    }
  }

  TEST_F(DataLakeFileSystemClientTest, ConstructorsWorks)
  {
    {
      // Create from connection string validates static creator function and shared key constructor.
      auto fileSystemName = GetTestNameLowerCase() + "1";
      auto connectionStringClient
          = Azure::Storage::Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(),
              fileSystemName,
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
          Azure::Storage::Files::DataLake::DataLakeFileSystemClient,
          Azure::Storage::Files::DataLake::DataLakeClientOptions>(
          Azure::Storage::Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(), GetTestNameLowerCase())
              .GetUrl(),
          credential,
          options);

      EXPECT_NO_THROW(clientSecretClient->Create());
      EXPECT_NO_THROW(clientSecretClient->Delete());
    }
  }

  TEST_F(DataLakeFileSystemClientTest, CustomerProvidedKey_LIVEONLY_)
  {

    auto getRandomCustomerProvidedKey = [&]() {
      Files::DataLake::EncryptionKey key;
      std::vector<uint8_t> aes256Key;
      aes256Key.resize(32);
      RandomBuffer(&aes256Key[0], aes256Key.size());
      key.Key = Azure::Core::Convert::Base64Encode(aes256Key);
      key.KeyHash = Azure::Core::Cryptography::_internal::Sha256Hash().Final(
          aes256Key.data(), aes256Key.size());
      key.Algorithm = Blobs::Models::EncryptionAlgorithmType::Aes256;
      return key;
    };

    const int32_t bufferSize = 1024; // 1KB data size
    auto buffer = std::make_shared<std::vector<uint8_t>>(bufferSize, 'x');
    Azure::Core::IO::MemoryBodyStream bodyStream(buffer->data(), buffer->size());

    auto customerProvidedKey
        = std::make_shared<Files::DataLake::EncryptionKey>(getRandomCustomerProvidedKey());
    Files::DataLake::DataLakeClientOptions options;
    options.CustomerProvidedKey = *customerProvidedKey;
    auto fileServiceClient = std::make_shared<Files::DataLake::DataLakeServiceClient>(
        Files::DataLake::DataLakeServiceClient::CreateFromConnectionString(
            AdlsGen2ConnectionString(), options));
    auto fileSystemClient = std::make_shared<Files::DataLake::DataLakeFileSystemClient>(
        fileServiceClient->GetFileSystemClient(m_fileSystemName));

    // fileSystem works
    {
      auto fileSystemClientWithoutEncryptionKey
          = Azure::Storage::Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(), m_fileSystemName);
      // Rename File
      const std::string filename1 = GetTestName() + "file1";
      const std::string filename2 = GetTestName() + "file2";
      const std::string filename3 = GetTestName() + "file3";
      const std::string filename4 = GetTestName() + "file4";

      auto oldFileClient = fileSystemClient->GetFileClient(filename1);
      oldFileClient.Create();
      auto newFileClient = fileSystemClient->RenameFile(filename1, filename2).Value;
      auto properties = std::make_shared<Files::DataLake::Models::PathProperties>(
          newFileClient.GetProperties().Value);
      EXPECT_EQ(customerProvidedKey->KeyHash, properties->EncryptionKeySha256.Value());
      auto newFileClientWithoutEncryptionKey
          = Files::DataLake::DataLakeFileClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(), m_fileSystemName, filename2);
      EXPECT_THROW(newFileClientWithoutEncryptionKey.GetProperties(), StorageException);
      EXPECT_NO_THROW(fileSystemClientWithoutEncryptionKey.RenameFile(filename2, filename3));

      // Rename Directory
      const std::string testName(GetTestName());
      const std::string oldDirectoryName = testName + "dir1";
      const std::string newDirectoryName = testName + "dir2";
      const std::string newDirectoryName2 = testName + "dir3";

      auto oldDirectoryClient = fileSystemClient->GetDirectoryClient(oldDirectoryName);
      oldDirectoryClient.Create();
      oldDirectoryClient.GetFileClient(testName + "file3").Create();
      oldDirectoryClient.GetSubdirectoryClient(testName + "dir4").Create();

      auto newDirectoryClient
          = fileSystemClient->RenameDirectory(oldDirectoryName, newDirectoryName).Value;
      properties = std::make_shared<Files::DataLake::Models::PathProperties>(
          newDirectoryClient.GetProperties().Value);
      EXPECT_TRUE(properties->EncryptionKeySha256.HasValue());
      EXPECT_EQ(customerProvidedKey->KeyHash, properties->EncryptionKeySha256.Value());
      auto newDirectoryClientWithoutEncryptionKey
          = Files::DataLake::DataLakeDirectoryClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(), m_fileSystemName, newDirectoryName);
      EXPECT_THROW(newDirectoryClientWithoutEncryptionKey.GetProperties(), StorageException);
      EXPECT_NO_THROW(fileSystemClientWithoutEncryptionKey.RenameDirectory(
          newDirectoryName, newDirectoryName2));

      auto fileSystemClientWithEncryptionKey
          = Azure::Storage::Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(), m_fileSystemName, options);
      auto created = std::make_shared<Files::DataLake::Models::CreatePathResult>(
          fileSystemClientWithEncryptionKey.GetFileClient(filename4).Create().Value);
      EXPECT_TRUE(created->EncryptionKeySha256.HasValue());
      EXPECT_EQ(customerProvidedKey->KeyHash, created->EncryptionKeySha256.Value());
    }

    // path works
    {
      const std::string pathName = "path";
      const std::string pathName2 = "path2";

      auto pathClient = Files::DataLake::DataLakePathClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(), m_fileSystemName, pathName, options);
      EXPECT_NO_THROW(pathClient.Create(Files::DataLake::Models::PathResourceType::File));
      EXPECT_NO_THROW(pathClient.SetMetadata(GetMetadata()));
      auto properties = std::make_shared<Files::DataLake::Models::PathProperties>(
          pathClient.GetProperties().Value);
      EXPECT_TRUE(properties->EncryptionKeySha256.HasValue());
      EXPECT_EQ(customerProvidedKey->KeyHash, properties->EncryptionKeySha256.Value());
      auto pathClientWithoutEncryptionKey
          = Files::DataLake::DataLakePathClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(), m_fileSystemName, pathName);
      EXPECT_THROW(pathClientWithoutEncryptionKey.SetMetadata(GetMetadata()), StorageException);
      EXPECT_THROW(pathClientWithoutEncryptionKey.GetProperties(), StorageException);
      EXPECT_NO_THROW(pathClientWithoutEncryptionKey.GetAccessControlList());
      EXPECT_NO_THROW(pathClientWithoutEncryptionKey.SetHttpHeaders(
          Files::DataLake::Models::PathHttpHeaders()));
      EXPECT_NO_THROW(pathClientWithoutEncryptionKey.SetPermissions("rwxrw-rw-"));

      auto pathClientWithEncryptionKey
          = Files::DataLake::DataLakePathClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(), m_fileSystemName, pathName2, options);
      auto created
          = pathClientWithEncryptionKey.Create(Files::DataLake::Models::PathResourceType::File)
                .Value;
      EXPECT_TRUE(created.EncryptionKeySha256.HasValue());
      EXPECT_EQ(customerProvidedKey->KeyHash, created.EncryptionKeySha256.Value());
    }

    // file works
    {
      const std::string fileName = "file";
      const std::string fileName2 = "file2";
      auto fileClient = fileSystemClient->GetFileClient(fileName);
      auto fileClientWithoutEncryptionKey
          = Files::DataLake::DataLakeFileClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(), m_fileSystemName, fileName);
      // upload test
      EXPECT_NO_THROW(fileClient.Create());
      EXPECT_NO_THROW(fileClient.UploadFrom(buffer->data(), bufferSize));
      auto result = fileClient.Download();
      auto downloaded = std::make_shared<std::vector<uint8_t>>(ReadBodyStream(result.Value.Body));
      EXPECT_EQ(*buffer, *downloaded);
      EXPECT_NO_THROW(fileClient.Delete());
      // append test
      EXPECT_NO_THROW(fileClient.Create());
      bodyStream.Rewind();
      EXPECT_NO_THROW(fileClient.Append(bodyStream, 0));
      bodyStream.Rewind();
      EXPECT_THROW(fileClientWithoutEncryptionKey.Append(bodyStream, bufferSize), StorageException);
      EXPECT_NO_THROW(fileClient.Flush(bufferSize));
      result = fileClient.Download();
      downloaded = std::make_shared<std::vector<uint8_t>>(ReadBodyStream(result.Value.Body));
      EXPECT_EQ(*buffer, *downloaded);
      EXPECT_NO_THROW(fileClient.SetMetadata(GetMetadata()));
      auto properties = std::make_shared<Files::DataLake::Models::PathProperties>(
          fileClient.GetProperties().Value);
      EXPECT_TRUE(properties->EncryptionKeySha256.HasValue());
      EXPECT_EQ(customerProvidedKey->KeyHash, properties->EncryptionKeySha256.Value());
      EXPECT_THROW(fileClientWithoutEncryptionKey.Flush(bufferSize), StorageException);
      EXPECT_THROW(fileClientWithoutEncryptionKey.Download(), StorageException);

      auto fileClientWithEncryptionKey
          = Files::DataLake::DataLakeFileClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(), m_fileSystemName, fileName2, options);
      auto created = std::make_shared<Files::DataLake::Models::CreatePathResult>(
          fileClientWithEncryptionKey.Create().Value);
      EXPECT_TRUE(created->EncryptionKeySha256.HasValue());
      EXPECT_EQ(customerProvidedKey->KeyHash, created->EncryptionKeySha256.Value());
    }
    // directory works
    {
      const std::string directoryName = "directory";
      const std::string directoryName2 = "directory2";
      const std::string subdirectoryName1 = "subdirectory1";
      const std::string subdirectoryName2 = "subdirectory2";
      const std::string subdirectoryName3 = "subdirectory3";
      const std::string fileName1 = "file1";
      const std::string fileName2 = "file2";
      const std::string fileName3 = "file3";
      auto directoryClient = fileSystemClient->GetDirectoryClient(directoryName);
      auto directoryClientWithoutEncryptionKey
          = Files::DataLake::DataLakeDirectoryClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(), m_fileSystemName, directoryName);
      // create subdirectory/file
      EXPECT_NO_THROW(directoryClient.Create());
      auto subdirectoryClient = directoryClient.GetSubdirectoryClient(subdirectoryName1);
      EXPECT_NO_THROW(subdirectoryClient.Create());
      auto fileClient = directoryClient.GetFileClient(fileName1);
      EXPECT_NO_THROW(fileClient.Create());
      auto subdirectoryProperties = std::make_shared<Files::DataLake::Models::PathProperties>(
          subdirectoryClient.GetProperties().Value);
      EXPECT_EQ(customerProvidedKey->KeyHash, subdirectoryProperties->EncryptionKeySha256.Value());
      auto fileProperties = fileClient.GetProperties();
      EXPECT_EQ(customerProvidedKey->KeyHash, fileProperties.Value.EncryptionKeySha256.Value());

      // rename file
      auto newFileClient
          = directoryClient.RenameFile(fileName1, directoryName + "/" + fileName2).Value;
      auto newFileProperties = std::make_shared<Files::DataLake::Models::PathProperties>(
          newFileClient.GetProperties().Value);
      EXPECT_EQ(customerProvidedKey->KeyHash, newFileProperties->EncryptionKeySha256.Value());
      auto newFileClientWithoutEncryptionKey
          = Files::DataLake::DataLakeFileClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(), m_fileSystemName, directoryName + "/" + fileName2);
      EXPECT_THROW(newFileClientWithoutEncryptionKey.GetProperties(), StorageException);
      EXPECT_NO_THROW(directoryClientWithoutEncryptionKey.RenameFile(
          fileName2, directoryName + "/" + fileName3));

      auto newSubdirectoryClient
          = directoryClient
                .RenameSubdirectory(subdirectoryName1, directoryName + "/" + subdirectoryName2)
                .Value;
      auto newSubdirectoryProperties = std::make_shared<Files::DataLake::Models::PathProperties>(
          newSubdirectoryClient.GetProperties().Value);
      EXPECT_EQ(
          customerProvidedKey->KeyHash, newSubdirectoryProperties->EncryptionKeySha256.Value());
      auto newsubdirectoryClientWithoutEncryptionKey
          = Files::DataLake::DataLakeDirectoryClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(),
              m_fileSystemName,
              directoryName + "/" + subdirectoryName2);
      EXPECT_THROW(newsubdirectoryClientWithoutEncryptionKey.GetProperties(), StorageException);
      EXPECT_NO_THROW(directoryClientWithoutEncryptionKey.RenameSubdirectory(
          subdirectoryName2, directoryName + "/" + subdirectoryName3));

      auto directoryClientWithEncryptionKey
          = Files::DataLake::DataLakeDirectoryClient::CreateFromConnectionString(
              AdlsGen2ConnectionString(), m_fileSystemName, directoryName2, options);
      auto created = std::make_shared<Files::DataLake::Models::CreatePathResult>(
          directoryClientWithEncryptionKey.Create().Value);
      EXPECT_TRUE(created->EncryptionKeySha256.HasValue());
      EXPECT_EQ(customerProvidedKey->KeyHash, created->EncryptionKeySha256.Value());
    }
  }

  TEST_F(DataLakeFileSystemClientTest, GetSetAccessPolicy_LIVEONLY_)
  {
    CHECK_SKIP_TEST();
    {
      auto fileSystem = Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(),
          GetTestNameLowerCase(),
          InitClientOptions<Files::DataLake::DataLakeClientOptions>());
      fileSystem.Create();

      Files::DataLake::SetFileSystemAccessPolicyOptions options;
      options.AccessType = Files::DataLake::Models::PublicAccessType::Path;
      {
        Files::DataLake::Models::SignedIdentifier identifier;
        identifier.Id = std::string(64, 'a');
        identifier.StartsOn = std::chrono::system_clock::now() - std::chrono::minutes(1);
        identifier.ExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(1);
        identifier.Permissions = "r";
        options.SignedIdentifiers.emplace_back(identifier);
      }
      {
        Files::DataLake::Models::SignedIdentifier identifier;
        identifier.Id = std::string(64, 'b');
        identifier.StartsOn = std::chrono::system_clock::now() - std::chrono::minutes(2);
        identifier.ExpiresOn.Reset();
        /* cspell:disable-next-line */
        identifier.Permissions = "racwdxlt";
        options.SignedIdentifiers.emplace_back(identifier);
      }
      {
        Files::DataLake::Models::SignedIdentifier identifier;
        identifier.Id = std::string(64, 'c');
        identifier.Permissions = "r";
        options.SignedIdentifiers.emplace_back(identifier);
      }
      {
        Files::DataLake::Models::SignedIdentifier identifier;
        identifier.Id = std::string(64, 'd');
        identifier.StartsOn = std::chrono::system_clock::now() - std::chrono::minutes(1);
        identifier.ExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(1);
        options.SignedIdentifiers.emplace_back(identifier);
      }

      auto ret = fileSystem.SetAccessPolicy(options);
      EXPECT_TRUE(ret.Value.ETag.HasValue());
      EXPECT_TRUE(IsValidTime(ret.Value.LastModified));

      auto ret2 = fileSystem.GetAccessPolicy();
      EXPECT_EQ(ret2.Value.AccessType, options.AccessType);
      ASSERT_EQ(ret2.Value.SignedIdentifiers.size(), options.SignedIdentifiers.size());
      for (size_t i = 0; i < ret2.Value.SignedIdentifiers.size(); ++i)
      {
        EXPECT_EQ(ret2.Value.SignedIdentifiers[i], options.SignedIdentifiers[i]);
      }

      options.AccessType = Files::DataLake::Models::PublicAccessType::FileSystem;
      EXPECT_NO_THROW(fileSystem.SetAccessPolicy(options));
      ret2 = fileSystem.GetAccessPolicy();
      EXPECT_EQ(ret2.Value.AccessType, options.AccessType);

      options.AccessType = Files::DataLake::Models::PublicAccessType::None;
      EXPECT_NO_THROW(fileSystem.SetAccessPolicy(options));
      ret2 = fileSystem.GetAccessPolicy();
      EXPECT_EQ(ret2.Value.AccessType, options.AccessType);

      fileSystem.Delete();
    }
    {
      auto fileSystem = Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(),
          GetTestNameLowerCase() + "1",
          InitClientOptions<Files::DataLake::DataLakeClientOptions>());
      Files::DataLake::CreateFileSystemOptions options;
      options.AccessType = Files::DataLake::Models::PublicAccessType::FileSystem;
      fileSystem.Create(options);
      auto ret = fileSystem.GetAccessPolicy();
      EXPECT_EQ(Files::DataLake::Models::PublicAccessType::FileSystem, ret.Value.AccessType);
      fileSystem.Delete();
    }
    {
      auto fileSystem = Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(),
          GetTestNameLowerCase() + "2",
          InitClientOptions<Files::DataLake::DataLakeClientOptions>());
      Files::DataLake::CreateFileSystemOptions options;
      options.AccessType = Files::DataLake::Models::PublicAccessType::Path;
      fileSystem.Create(options);
      auto ret = fileSystem.GetAccessPolicy();
      EXPECT_EQ(Files::DataLake::Models::PublicAccessType::Path, ret.Value.AccessType);
      fileSystem.Delete();
    }
    {
      auto fileSystem = Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(),
          GetTestNameLowerCase() + "3",
          InitClientOptions<Files::DataLake::DataLakeClientOptions>());
      Files::DataLake::CreateFileSystemOptions options;
      options.AccessType = Files::DataLake::Models::PublicAccessType::Path;
      fileSystem.Create(options);
      auto ret = fileSystem.GetAccessPolicy();
      EXPECT_EQ(Files::DataLake::Models::PublicAccessType::Path, ret.Value.AccessType);
      fileSystem.Delete();
    }
  }

  TEST_F(DataLakeFileSystemClientTest, RenameFile)
  {
    const std::string oldFilename = GetTestName() + "1";
    const std::string newFilename = GetTestName() + "2";

    auto oldFileClient = m_fileSystemClient->GetFileClient(oldFilename);
    oldFileClient.Create();

    auto newFileClient = m_fileSystemClient->RenameFile(oldFilename, newFilename).Value;

    EXPECT_NO_THROW(newFileClient.GetProperties());
    EXPECT_NO_THROW(m_fileSystemClient->GetFileClient(newFilename).GetProperties());
    EXPECT_THROW(oldFileClient.GetProperties(), StorageException);

    const std::string newFileSystemName = GetTestNameLowerCase() + "1";
    const std::string newFilename2 = GetTestName() + "3";

    auto newFileSystem = m_dataLakeServiceClient->GetFileSystemClient(newFileSystemName);
    newFileSystem.Create();

    Files::DataLake::RenameFileOptions options;
    options.DestinationFileSystem = newFileSystemName;
    auto newFileClient2 = m_fileSystemClient->RenameFile(newFilename, newFilename2, options).Value;

    EXPECT_NO_THROW(newFileClient2.GetProperties());
    EXPECT_NO_THROW(newFileSystem.GetFileClient(newFilename2).GetProperties());
    newFileSystem.Delete();
    EXPECT_THROW(newFileClient.GetProperties(), StorageException);
  }

  TEST_F(DataLakeFileSystemClientTest, RenameDirectory)
  {
    const std::string testName(GetTestName());
    const std::string oldDirectoryName = testName + "1";
    const std::string newDirectoryName = testName + "2";

    auto oldDirectoryClient = m_fileSystemClient->GetDirectoryClient(oldDirectoryName);
    oldDirectoryClient.Create();
    oldDirectoryClient.GetFileClient(testName + "3").Create();
    oldDirectoryClient.GetSubdirectoryClient(testName + "4").Create();

    auto newDirectoryClient
        = m_fileSystemClient->RenameDirectory(oldDirectoryName, newDirectoryName).Value;

    EXPECT_NO_THROW(newDirectoryClient.GetProperties());
    EXPECT_NO_THROW(m_fileSystemClient->GetDirectoryClient(newDirectoryName).GetProperties());
    EXPECT_THROW(oldDirectoryClient.GetProperties(), StorageException);

    const std::string newFileSystemName = GetTestNameLowerCase();
    const std::string newDirectoryName2 = testName + "5";

    auto newFileSystem = m_dataLakeServiceClient->GetFileSystemClient(newFileSystemName);
    newFileSystem.Create();

    Files::DataLake::RenameDirectoryOptions options;
    options.DestinationFileSystem = newFileSystemName;
    auto newDirectoryClient2
        = m_fileSystemClient->RenameDirectory(newDirectoryName, newDirectoryName2, options).Value;

    EXPECT_NO_THROW(newDirectoryClient2.GetProperties());
    EXPECT_NO_THROW(newFileSystem.GetDirectoryClient(newDirectoryName2).GetProperties());
    newFileSystem.Delete();
    EXPECT_THROW(newDirectoryClient.GetProperties(), StorageException);
  }

  TEST_F(DataLakeFileSystemClientTest, RenameFileSasAuthentication_LIVEONLY_)
  {
    const std::string testName(GetTestName());
    const std::string sourceFilename = testName + "1";
    const std::string destinationFilename = testName + "2";
    auto fileClient = m_fileSystemClient->GetFileClient(sourceFilename);
    fileClient.CreateIfNotExists();

    Files::DataLake::DataLakeFileSystemClient fileSystemClientSas(
        Files::DataLake::_detail::GetDfsUrlFromUrl(m_fileSystemClient->GetUrl()) + GetSas());
    fileSystemClientSas.RenameFile(sourceFilename, destinationFilename);
    EXPECT_THROW(
        m_fileSystemClient->GetFileClient(sourceFilename).GetProperties(), StorageException);
    EXPECT_NO_THROW(m_fileSystemClient->GetFileClient(destinationFilename).GetProperties());

    const std::string sourceDirectoryName = testName + "3";
    const std::string destinationDirectoryName = testName + "4";
    auto directoryClient = m_fileSystemClient->GetDirectoryClient(sourceDirectoryName);
    directoryClient.CreateIfNotExists();

    fileSystemClientSas.RenameDirectory(sourceDirectoryName, destinationDirectoryName);
    EXPECT_THROW(
        m_fileSystemClient->GetDirectoryClient(sourceDirectoryName).GetProperties(),
        StorageException);
    EXPECT_NO_THROW(
        m_fileSystemClient->GetDirectoryClient(destinationDirectoryName).GetProperties());
  }

  TEST_F(DataLakeFileSystemClientTest, DISABLED_ListDeletedPaths)
  {
    const std::string deletedFilename = GetTestNameLowerCase() + "_file_deleted";
    const std::string nonDeletedFilename = GetTestNameLowerCase() + "_file";
    const std::string deletedDirectoryName = GetTestNameLowerCase() + "_dir_deleted";
    const std::string nonDeletedDirectoryName = GetTestNameLowerCase() + "_dir";

    auto deletedFileClient = m_fileSystemClient->GetFileClient(deletedFilename);
    auto nonDeletedFileClient = m_fileSystemClient->GetFileClient(nonDeletedFilename);
    auto deletedDirectoryClient = m_fileSystemClient->GetDirectoryClient(deletedDirectoryName);
    auto nonDeletedDirectoryClient
        = m_fileSystemClient->GetDirectoryClient(nonDeletedDirectoryName);

    deletedFileClient.Create();
    deletedFileClient.Delete();
    nonDeletedFileClient.Create();
    deletedDirectoryClient.Create();
    deletedDirectoryClient.DeleteEmpty();
    nonDeletedDirectoryClient.Create();

    {
      std::vector<Files::DataLake::Models::PathDeletedItem> paths;
      EXPECT_NO_THROW(paths = std::move(m_fileSystemClient->ListDeletedPaths().DeletedPaths));
      EXPECT_EQ(2, paths.size());
      EXPECT_EQ(deletedDirectoryName, paths[0].Name);
      EXPECT_EQ(deletedFilename, paths[1].Name);
    }
    //  List max result
    {
      Files::DataLake::ListDeletedPathsOptions options;
      options.PageSizeHint = 1;
      std::vector<Files::DataLake::Models::PathDeletedItem> paths;
      for (auto pageResult = m_fileSystemClient->ListDeletedPaths(options); pageResult.HasPage();
           pageResult.MoveToNextPage())
      {
        paths.insert(paths.end(), pageResult.DeletedPaths.begin(), pageResult.DeletedPaths.end());
        EXPECT_EQ(1, pageResult.DeletedPaths.size());
      }
      EXPECT_EQ(2, paths.size());
    }
    // prefix works
    {
      const std::string directoryName = GetTestNameLowerCase() + "_prefix";
      const std::string filename = "file";

      auto directoryClient = m_fileSystemClient->GetDirectoryClient(directoryName);
      directoryClient.Create();
      auto fileClient = directoryClient.GetFileClient(filename);
      fileClient.Create();
      fileClient.Delete();

      Files::DataLake::ListDeletedPathsOptions options;
      options.Prefix = directoryName;
      std::vector<Files::DataLake::Models::PathDeletedItem> paths;
      EXPECT_NO_THROW(
          paths = std::move(m_fileSystemClient->ListDeletedPaths(options).DeletedPaths));
      EXPECT_EQ(1, paths.size());
      EXPECT_EQ(directoryName + "/" + filename, paths[0].Name);
    }
  }

  TEST_F(DataLakeFileSystemClientTest, DISABLED_Undelete)
  {
    const std::string directoryName = GetTestNameLowerCase() + "_dir";
    const std::string subFileName = "sub_file";

    auto directoryClient = m_fileSystemClient->GetDirectoryClient(directoryName);
    directoryClient.Create();
    auto subFileClient = m_fileSystemClient->GetFileClient(directoryName + "/" + subFileName);
    subFileClient.Create();
    Files::DataLake::Models::PathProperties properties = directoryClient.GetProperties().Value;
    Files::DataLake::Models::PathProperties subFileProperties = subFileClient.GetProperties().Value;

    // recursive works
    {
      directoryClient.DeleteRecursive();

      auto paths = m_fileSystemClient->ListDeletedPaths().DeletedPaths;
      const std::string deletionId = paths[0].DeletionId;

      auto restoredClient = m_fileSystemClient->UndeletePath(directoryName, deletionId).Value;

      paths = m_fileSystemClient->ListDeletedPaths().DeletedPaths;
      EXPECT_EQ(0, paths.size());
      Files::DataLake::Models::PathProperties restoredProperties;
      EXPECT_NO_THROW(restoredProperties = restoredClient.GetProperties().Value);
      EXPECT_TRUE(restoredProperties.IsDirectory);
      EXPECT_EQ(properties.ETag, restoredProperties.ETag);
      Files::DataLake::Models::PathProperties restoredSubFileProperties;
      EXPECT_NO_THROW(restoredSubFileProperties = subFileClient.GetProperties().Value);
      EXPECT_TRUE(!restoredSubFileProperties.IsDirectory);
      EXPECT_EQ(subFileProperties.ETag, restoredSubFileProperties.ETag);
    }
    // not recursive works
    {
      subFileClient.Delete();
      directoryClient.DeleteEmpty();

      auto paths = m_fileSystemClient->ListDeletedPaths().DeletedPaths;
      std::string deletionId = paths[0].DeletionId;

      // restore directory
      auto restoredClient = m_fileSystemClient->UndeletePath(directoryName, deletionId).Value;
      paths = m_fileSystemClient->ListDeletedPaths().DeletedPaths;
      EXPECT_EQ(1, paths.size()); // not restore subFile
      Files::DataLake::Models::PathProperties restoredProperties;
      EXPECT_NO_THROW(restoredProperties = restoredClient.GetProperties().Value);
      EXPECT_TRUE(restoredProperties.IsDirectory);
      EXPECT_EQ(properties.ETag, restoredProperties.ETag);
      EXPECT_THROW(subFileClient.GetProperties(), StorageException);

      // restore file
      deletionId = paths[0].DeletionId;
      restoredClient
          = m_fileSystemClient->UndeletePath(directoryName + "/" + subFileName, deletionId).Value;
      paths = m_fileSystemClient->ListDeletedPaths().DeletedPaths;
      EXPECT_EQ(0, paths.size());
      EXPECT_NO_THROW(restoredProperties = restoredClient.GetProperties().Value);
      EXPECT_FALSE(restoredProperties.IsDirectory);
      EXPECT_EQ(subFileProperties.ETag, restoredProperties.ETag);
    }
  }

}}} // namespace Azure::Storage::Test
