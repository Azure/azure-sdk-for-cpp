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

  void DataLakeFileSystemClientTest::SetUp()
  {
    DataLakeServiceClientTest::SetUp();

    m_fileSystemName = GetLowercaseIdentifier();
    m_fileSystemClient = std::make_shared<Files::DataLake::DataLakeFileSystemClient>(
        m_dataLakeServiceClient->GetFileSystemClient(m_fileSystemName));
    while (true)
    {
      try
      {
        m_fileSystemClient->CreateIfNotExists();
        break;
      }
      catch (StorageException& e)
      {
        if (e.ErrorCode != "ContainerBeingDeleted")
        {
          throw;
        }
        SUCCEED() << "Container is being deleted. Will try again after 3 seconds.";
        std::this_thread::sleep_for(std::chrono::seconds(3));
      }
    }

    m_resourceCleanupFunctions.push_back(
        [fileSystemClient = *m_fileSystemClient]() { fileSystemClient.DeleteIfExists(); });
  }

  Files::DataLake::DataLakeFileSystemClient
  DataLakeFileSystemClientTest::GetFileSystemClientForTest(
      const std::string& fileSystemName,
      Files::DataLake::DataLakeClientOptions clientOptions)
  {
    InitClientOptions(clientOptions);
    auto fsClient = Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
        AdlsGen2ConnectionString(), fileSystemName, clientOptions);
    m_resourceCleanupFunctions.push_back([fsClient]() { fsClient.DeleteIfExists(); });
    return fsClient;
  }

  TEST_F(DataLakeFileSystemClientTest, CreateDeleteFileSystems)
  {
    auto fsClient = GetFileSystemClientForTest(LowercaseRandomString());
    EXPECT_THROW(fsClient.Delete(), StorageException);
    EXPECT_NO_THROW(fsClient.Create());
    EXPECT_NO_THROW(fsClient.CreateIfNotExists());
    EXPECT_THROW(fsClient.Create(), StorageException);
    EXPECT_NO_THROW(fsClient.Delete());
    EXPECT_NO_THROW(fsClient.DeleteIfExists());
  }

  TEST_F(DataLakeFileSystemClientTest, CreateDeleteFileSystemsWithAccessCondition)
  {
    {
      auto fsClient = GetFileSystemClientForTest(LowercaseRandomString());
      fsClient.Create();
      auto properties = fsClient.GetProperties().Value;

      Files::DataLake::DeleteFileSystemOptions deleteOptions;
      deleteOptions.AccessConditions.IfModifiedSince
          = properties.LastModified + std::chrono::seconds(5);
      EXPECT_THROW(fsClient.Delete(deleteOptions), StorageException);
      deleteOptions.AccessConditions.IfModifiedSince
          = properties.LastModified - std::chrono::seconds(5);
      EXPECT_NO_THROW(fsClient.Delete(deleteOptions));
    }
    {
      auto fsClient = GetFileSystemClientForTest(LowercaseRandomString());
      fsClient.Create();
      auto properties = fsClient.GetProperties().Value;

      Files::DataLake::DeleteFileSystemOptions deleteOptions;
      deleteOptions.AccessConditions.IfUnmodifiedSince
          = properties.LastModified - std::chrono::seconds(5);
      EXPECT_THROW(fsClient.Delete(deleteOptions), StorageException);
      deleteOptions.AccessConditions.IfUnmodifiedSince
          = properties.LastModified + std::chrono::seconds(5);
      EXPECT_NO_THROW(fsClient.Delete(deleteOptions));
    }
    {
      auto leaseId = RandomUUID();
      auto dummyLeaseId = RandomUUID();
      auto fsClient = GetFileSystemClientForTest(LowercaseRandomString());
      fsClient.Create();

      Files::DataLake::DataLakeLeaseClient leaseClient(fsClient, leaseId);
      leaseClient.Acquire(std::chrono::seconds(30));
      EXPECT_THROW(fsClient.Delete(), StorageException);
      Files::DataLake::DeleteFileSystemOptions deleteOptions;
      deleteOptions.AccessConditions.LeaseId = dummyLeaseId;
      EXPECT_THROW(fsClient.Delete(deleteOptions), StorageException);
      deleteOptions.AccessConditions.LeaseId = leaseId;
      EXPECT_NO_THROW(fsClient.Delete(deleteOptions));
    }
  }

  TEST_F(DataLakeFileSystemClientTest, FileSystemMetadata)
  {
    {
      auto metadata = RandomMetadata();
      // Set/Get Metadata works
      EXPECT_NO_THROW(m_fileSystemClient->SetMetadata(metadata));
      EXPECT_EQ(metadata, m_fileSystemClient->GetProperties().Value.Metadata);
      EXPECT_NO_THROW(m_fileSystemClient->SetMetadata({}));
      EXPECT_TRUE(m_fileSystemClient->GetProperties().Value.Metadata.empty());
    }

    {
      auto fsClient = GetFileSystemClientForTest(LowercaseRandomString());
      Files::DataLake::CreateFileSystemOptions options;
      options.Metadata = RandomMetadata();
      fsClient.Create(options);
      EXPECT_EQ(fsClient.GetProperties().Value.Metadata, options.Metadata);
    }
  }

  TEST_F(DataLakeFileSystemClientTest, GetDataLakeFileSystemPropertiesResult)
  {
    auto metadata1 = RandomMetadata();
    // Get Metadata via properties works
    EXPECT_NO_THROW(m_fileSystemClient->SetMetadata(metadata1));
    auto properties = m_fileSystemClient->GetProperties().Value;
    EXPECT_EQ(metadata1, properties.Metadata);
    EXPECT_TRUE(IsValidTime(properties.LastModified));
    EXPECT_TRUE(properties.ETag.HasValue());
    EXPECT_FALSE(properties.DefaultEncryptionScope.empty());
    EXPECT_FALSE(properties.PreventEncryptionScopeOverride);
  }

  TEST_F(DataLakeFileSystemClientTest, ListPaths)
  {
    std::set<std::string> paths;
    const std::string dir1 = RandomString();
    const std::string dir2 = RandomString();

    std::set<std::string> rootPaths;
    rootPaths.emplace(dir1);
    rootPaths.emplace(dir2);

    {
      auto dirClient = m_fileSystemClient->GetDirectoryClient(dir1);
      for (int i = 0; i < 3; ++i)
      {
        std::string filename = RandomString();
        auto fileClient = dirClient.GetFileClient(filename);
        fileClient.CreateIfNotExists();
        paths.emplace(dir1 + "/" + filename);
      }

      dirClient = m_fileSystemClient->GetDirectoryClient(dir2);
      for (int i = 0; i < 4; ++i)
      {
        std::string filename = RandomString();
        auto fileClient = dirClient.GetFileClient(filename);
        fileClient.CreateIfNotExists();
        paths.emplace(dir2 + "/" + filename);
      }
      std::string filename = RandomString();
      auto fileClient = m_fileSystemClient->GetFileClient(filename);
      fileClient.CreateIfNotExists();
      paths.emplace(filename);
      rootPaths.emplace(filename);
    }

    {
      // Normal list recursively.
      std::set<std::string> results;
      for (auto page = m_fileSystemClient->ListPaths(true); page.HasPage(); page.MoveToNextPage())
      {
        for (auto& path : page.Paths)
        {
          results.insert(path.Name);
        }
      }

      for (const auto& path : paths)
      {
        EXPECT_NE(results.find(path), results.end());
      }
    }
    {
      // non-recursive
      std::set<std::string> results;
      for (auto page = m_fileSystemClient->ListPaths(false); page.HasPage(); page.MoveToNextPage())
      {
        for (auto& path : page.Paths)
        {
          results.insert(path.Name);
        }
      }

      for (const auto& path : rootPaths)
      {
        EXPECT_NE(results.find(path), results.end());
      }
      EXPECT_LT(results.size(), paths.size());
    }
    {
      // List max result
      Files::DataLake::ListPathsOptions options;
      options.PageSizeHint = 2;
      auto response = m_fileSystemClient->ListPaths(true, options);
      EXPECT_LE(2U, response.Paths.size());
    }
  }

  TEST_F(DataLakeFileSystemClientTest, UnencodedPathDirectoryFileNameWorks)
  {
    const std::string non_ascii_word = "\xE6\xB5\x8B\xE8\xAF\x95";
    const std::string encoded_non_ascii_word = "%E6%B5%8B%E8%AF%95";
    std::string baseName = "a b c / !@#$%^&*(?/<>,.;:'\"[]{}|`~\\) def" + non_ascii_word;
    {
      std::string pathName = baseName + RandomString();
      auto fileClient = m_fileSystemClient->GetFileClient(pathName);
      EXPECT_NO_THROW(fileClient.Create());
      auto fileUrl = fileClient.GetUrl();
      EXPECT_EQ(fileUrl, m_fileSystemClient->GetUrl() + "/" + _internal::UrlEncodePath(pathName));
    }
    {
      std::string directoryName = baseName + RandomString() + "1";
      auto directoryClient = m_fileSystemClient->GetDirectoryClient(directoryName);
      EXPECT_NO_THROW(directoryClient.Create());
      auto directoryUrl = directoryClient.GetUrl();
      EXPECT_EQ(
          directoryUrl,
          m_fileSystemClient->GetUrl() + "/" + _internal::UrlEncodePath(directoryName));
    }
    {
      std::string fileName = baseName + RandomString() + "2";
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
      auto fileSystemName = LowercaseRandomString() + "1";
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
              AdlsGen2ConnectionString(), LowercaseRandomString())
              .GetUrl(),
          credential,
          options);

      EXPECT_NO_THROW(clientSecretClient->Create());
      EXPECT_NO_THROW(clientSecretClient->Delete());
    }
  }

  TEST_F(DataLakeFileSystemClientTest, CustomerProvidedKey)
  {
    auto getRandomCustomerProvidedKey = [&]() {
      Files::DataLake::EncryptionKey key;
      std::vector<uint8_t> aes256Key = RandomBuffer(32);
      key.Key = Azure::Core::Convert::Base64Encode(aes256Key);
      key.KeyHash = Azure::Core::Cryptography::_internal::Sha256Hash().Final(
          aes256Key.data(), aes256Key.size());
      key.Algorithm = Blobs::Models::EncryptionAlgorithmType::Aes256;
      return key;
    };

    auto buffer = RandomBuffer(10);
    Azure::Core::IO::MemoryBodyStream bodyStream(buffer.data(), buffer.size());

    auto customerProvidedKey = getRandomCustomerProvidedKey();
    Files::DataLake::DataLakeClientOptions clientOptionsWithCPK;
    clientOptionsWithCPK.CustomerProvidedKey = customerProvidedKey;
    auto fileSystemClientWithCPK
        = GetFileSystemClientForTest(m_fileSystemName, clientOptionsWithCPK);
    auto fileSystemClientWithoutCPK = GetFileSystemClientForTest(m_fileSystemName);

    // fileSystem works
    {
      // Rename File
      const std::string filename1 = RandomString() + "file1";
      const std::string filename2 = RandomString() + "file2";
      const std::string filename3 = RandomString() + "file3";
      const std::string filename4 = RandomString() + "file4";

      auto oldFileClient = fileSystemClientWithCPK.GetFileClient(filename1);
      oldFileClient.Create();
      auto newFileClient = fileSystemClientWithCPK.RenameFile(filename1, filename2).Value;
      auto properties = newFileClient.GetProperties().Value;
      EXPECT_EQ(customerProvidedKey.KeyHash, properties.EncryptionKeySha256.Value());
      auto newFileClientWithoutEncryptionKey = fileSystemClientWithoutCPK.GetFileClient(filename2);
      EXPECT_THROW(newFileClientWithoutEncryptionKey.GetProperties(), StorageException);
      EXPECT_NO_THROW(fileSystemClientWithoutCPK.RenameFile(filename2, filename3));

      // Rename Directory
      const std::string baseName = RandomString();
      const std::string oldDirectoryName = baseName + "dir1";
      const std::string newDirectoryName = baseName + "dir2";
      const std::string newDirectoryName2 = baseName + "dir3";

      auto oldDirectoryClient = fileSystemClientWithCPK.GetDirectoryClient(oldDirectoryName);
      oldDirectoryClient.Create();
      oldDirectoryClient.GetFileClient(baseName + "file3").Create();
      oldDirectoryClient.GetSubdirectoryClient(baseName + "dir4").Create();

      auto newDirectoryClient
          = fileSystemClientWithCPK.RenameDirectory(oldDirectoryName, newDirectoryName).Value;
      properties = newDirectoryClient.GetProperties().Value;
      EXPECT_TRUE(properties.EncryptionKeySha256.HasValue());
      EXPECT_EQ(customerProvidedKey.KeyHash, properties.EncryptionKeySha256.Value());
      auto newDirectoryClientWithoutEncryptionKey
          = fileSystemClientWithoutCPK.GetDirectoryClient(newDirectoryName);
      EXPECT_THROW(newDirectoryClientWithoutEncryptionKey.GetProperties(), StorageException);
      EXPECT_NO_THROW(
          fileSystemClientWithoutCPK.RenameDirectory(newDirectoryName, newDirectoryName2));

      auto createResult = fileSystemClientWithCPK.GetFileClient(filename4).Create().Value;
      EXPECT_TRUE(createResult.EncryptionKeySha256.HasValue());
      EXPECT_EQ(customerProvidedKey.KeyHash, createResult.EncryptionKeySha256.Value());
    }

    // path works
    {
      const std::string pathName = "path";
      const std::string pathName2 = "path2";

      auto pathClientWithCPK
          = Files::DataLake::DataLakePathClient(fileSystemClientWithCPK.GetFileClient(pathName));
      auto pathClientWithoutCPK
          = Files::DataLake::DataLakePathClient(fileSystemClientWithoutCPK.GetFileClient(pathName));
      auto pathClient2WithCPK
          = Files::DataLake::DataLakePathClient(fileSystemClientWithCPK.GetFileClient(pathName2));

      EXPECT_NO_THROW(pathClientWithCPK.Create(Files::DataLake::Models::PathResourceType::File));
      EXPECT_NO_THROW(pathClientWithCPK.SetMetadata(RandomMetadata()));
      auto properties = pathClientWithCPK.GetProperties().Value;
      EXPECT_TRUE(properties.EncryptionKeySha256.HasValue());
      EXPECT_EQ(customerProvidedKey.KeyHash, properties.EncryptionKeySha256.Value());

      EXPECT_THROW(pathClientWithoutCPK.SetMetadata(RandomMetadata()), StorageException);
      EXPECT_THROW(pathClientWithoutCPK.GetProperties(), StorageException);
      EXPECT_NO_THROW(pathClientWithoutCPK.GetAccessControlList());
      EXPECT_NO_THROW(
          pathClientWithoutCPK.SetHttpHeaders(Files::DataLake::Models::PathHttpHeaders()));
      EXPECT_NO_THROW(pathClientWithoutCPK.SetPermissions("rwxrw-rw-"));

      auto createResult
          = pathClient2WithCPK.Create(Files::DataLake::Models::PathResourceType::File).Value;
      EXPECT_TRUE(createResult.EncryptionKeySha256.HasValue());
      EXPECT_EQ(customerProvidedKey.KeyHash, createResult.EncryptionKeySha256.Value());
    }

    // file works
    {
      const std::string fileName = "file";
      const std::string fileName2 = "file2";
      auto fileClientWithCPK = fileSystemClientWithCPK.GetFileClient(fileName);
      auto fileClientWithoutCPK = fileSystemClientWithoutCPK.GetFileClient(fileName);
      auto fileClient2WithCPK = fileSystemClientWithCPK.GetFileClient(fileName2);

      // upload test
      EXPECT_NO_THROW(fileClientWithCPK.Create());
      EXPECT_NO_THROW(fileClientWithCPK.UploadFrom(buffer.data(), buffer.size()));
      auto result = fileClientWithCPK.Download();
      auto downloaded = ReadBodyStream(result.Value.Body);
      EXPECT_EQ(buffer, downloaded);
      EXPECT_NO_THROW(fileClientWithCPK.Delete());
      // append test
      EXPECT_NO_THROW(fileClientWithCPK.Create());
      bodyStream.Rewind();
      EXPECT_NO_THROW(fileClientWithCPK.Append(bodyStream, 0));
      bodyStream.Rewind();
      EXPECT_THROW(fileClientWithoutCPK.Append(bodyStream, buffer.size()), StorageException);
      EXPECT_NO_THROW(fileClientWithCPK.Flush(buffer.size()));
      result = fileClientWithCPK.Download();
      downloaded = ReadBodyStream(result.Value.Body);
      EXPECT_EQ(buffer, downloaded);
      EXPECT_NO_THROW(fileClientWithCPK.SetMetadata(RandomMetadata()));
      auto properties = fileClientWithCPK.GetProperties().Value;
      EXPECT_TRUE(properties.EncryptionKeySha256.HasValue());
      EXPECT_EQ(customerProvidedKey.KeyHash, properties.EncryptionKeySha256.Value());
      EXPECT_THROW(fileClientWithoutCPK.Flush(buffer.size()), StorageException);
      EXPECT_THROW(fileClientWithoutCPK.Download(), StorageException);

      auto createResult = fileClient2WithCPK.Create().Value;
      EXPECT_TRUE(createResult.EncryptionKeySha256.HasValue());
      EXPECT_EQ(customerProvidedKey.KeyHash, createResult.EncryptionKeySha256.Value());
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

      auto directoryClientWithCPK = fileSystemClientWithCPK.GetDirectoryClient(directoryName);
      auto directoryClientWithoutCPK = fileSystemClientWithoutCPK.GetDirectoryClient(directoryName);

      // create subdirectory/file
      EXPECT_NO_THROW(directoryClientWithCPK.Create());
      auto subdirectoryClientWithCPK
          = directoryClientWithCPK.GetSubdirectoryClient(subdirectoryName1);
      EXPECT_NO_THROW(subdirectoryClientWithCPK.Create());
      auto fileClientWithCPK = directoryClientWithCPK.GetFileClient(fileName1);
      EXPECT_NO_THROW(fileClientWithCPK.Create());
      auto subdirectoryProperties = subdirectoryClientWithCPK.GetProperties().Value;
      EXPECT_EQ(customerProvidedKey.KeyHash, subdirectoryProperties.EncryptionKeySha256.Value());
      auto fileProperties = fileClientWithCPK.GetProperties();
      EXPECT_EQ(customerProvidedKey.KeyHash, fileProperties.Value.EncryptionKeySha256.Value());

      // rename file
      auto newFileClient
          = directoryClientWithCPK.RenameFile(fileName1, directoryName + "/" + fileName2).Value;
      auto newFileProperties = newFileClient.GetProperties().Value;
      EXPECT_EQ(customerProvidedKey.KeyHash, newFileProperties.EncryptionKeySha256.Value());
      auto newFileClientWithoutCPK
          = fileSystemClientWithoutCPK.GetFileClient(directoryName + "/" + fileName2);
      EXPECT_THROW(newFileClientWithoutCPK.GetProperties(), StorageException);
      EXPECT_NO_THROW(
          directoryClientWithoutCPK.RenameFile(fileName2, directoryName + "/" + fileName3));

      auto newSubdirectoryClientWithCPK
          = directoryClientWithCPK
                .RenameSubdirectory(subdirectoryName1, directoryName + "/" + subdirectoryName2)
                .Value;
      auto newSubdirectoryProperties = newSubdirectoryClientWithCPK.GetProperties().Value;
      EXPECT_EQ(customerProvidedKey.KeyHash, newSubdirectoryProperties.EncryptionKeySha256.Value());
      auto newsubdirectoryClientWithoutCPK
          = fileSystemClientWithoutCPK.GetDirectoryClient(directoryName + "/" + subdirectoryName2);
      EXPECT_THROW(newsubdirectoryClientWithoutCPK.GetProperties(), StorageException);
      EXPECT_NO_THROW(directoryClientWithoutCPK.RenameSubdirectory(
          subdirectoryName2, directoryName + "/" + subdirectoryName3));

      auto directoryClient2WithCPK = fileSystemClientWithCPK.GetDirectoryClient(directoryName2);
      auto createResult = directoryClient2WithCPK.Create().Value;
      EXPECT_TRUE(createResult.EncryptionKeySha256.HasValue());
      EXPECT_EQ(customerProvidedKey.KeyHash, createResult.EncryptionKeySha256.Value());
    }
  }

  TEST_F(DataLakeFileSystemClientTest, EncryptionScope)
  {
    auto const& testEncryptionScope = GetTestEncryptionScope();
    // without EncryptionScope
    {
      auto properties = m_fileSystemClient->GetProperties().Value;
      EXPECT_EQ(properties.DefaultEncryptionScope, AccountEncryptionKey);
      EXPECT_EQ(properties.PreventEncryptionScopeOverride, false);
    }
    // with EncryptionScope
    {
      std::string fileSystemName = LowercaseRandomString() + "1";
      std::string pathName = RandomString() + "1";
      auto fileSystemClient = GetFileSystemClientForTest(fileSystemName);
      Files::DataLake::CreateFileSystemOptions createOptions;
      createOptions.DefaultEncryptionScope = testEncryptionScope;
      createOptions.PreventEncryptionScopeOverride = true;
      EXPECT_NO_THROW(fileSystemClient.Create(createOptions));
      auto properties = fileSystemClient.GetProperties().Value;
      EXPECT_EQ(properties.DefaultEncryptionScope, createOptions.DefaultEncryptionScope.Value());
      EXPECT_EQ(
          properties.PreventEncryptionScopeOverride,
          createOptions.PreventEncryptionScopeOverride.Value());
      Files::DataLake::ListFileSystemsOptions listFileSystemOptions;
      listFileSystemOptions.Prefix = fileSystemName;
      auto fileSystems
          = m_dataLakeServiceClient->ListFileSystems(listFileSystemOptions).FileSystems;
      for (auto& fileSystem : fileSystems)
      {
        if (fileSystem.Name == fileSystemName)
        {
          EXPECT_EQ(
              fileSystem.Details.DefaultEncryptionScope,
              createOptions.DefaultEncryptionScope.Value());
          EXPECT_EQ(
              fileSystem.Details.PreventEncryptionScopeOverride,
              createOptions.PreventEncryptionScopeOverride.Value());
        }
      }
    }
  }

  TEST_F(DataLakeFileSystemClientTest, GetSetAccessPolicy)
  {
    {
      auto fileSystem = GetFileSystemClientForTest(LowercaseRandomString());
      fileSystem.CreateIfNotExists();

      Files::DataLake::SetFileSystemAccessPolicyOptions options;
      options.AccessType = Files::DataLake::Models::PublicAccessType::None;
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
      if (m_testContext.IsLiveMode())
      {
        EXPECT_EQ(ret2.Value.SignedIdentifiers, options.SignedIdentifiers);
      }

      options.AccessType = Files::DataLake::Models::PublicAccessType::FileSystem;
      EXPECT_NO_THROW(fileSystem.SetAccessPolicy(options));
      ret2 = fileSystem.GetAccessPolicy();
      EXPECT_EQ(ret2.Value.AccessType, options.AccessType);

      options.AccessType = Files::DataLake::Models::PublicAccessType::None;
      EXPECT_NO_THROW(fileSystem.SetAccessPolicy(options));
      ret2 = fileSystem.GetAccessPolicy();
      EXPECT_EQ(ret2.Value.AccessType, options.AccessType);
    }
    {
      auto fileSystem = GetFileSystemClientForTest(LowercaseRandomString());
      Files::DataLake::CreateFileSystemOptions options;
      options.AccessType = Files::DataLake::Models::PublicAccessType::FileSystem;
      fileSystem.Create(options);
      auto ret = fileSystem.GetAccessPolicy();
      EXPECT_EQ(Files::DataLake::Models::PublicAccessType::FileSystem, ret.Value.AccessType);
    }
    {
      auto fileSystem = GetFileSystemClientForTest(LowercaseRandomString());
      Files::DataLake::CreateFileSystemOptions options;
      options.AccessType = Files::DataLake::Models::PublicAccessType::Path;
      fileSystem.Create(options);
      auto ret = fileSystem.GetAccessPolicy();
      EXPECT_EQ(Files::DataLake::Models::PublicAccessType::Path, ret.Value.AccessType);
    }
    {
      auto fileSystem = GetFileSystemClientForTest(LowercaseRandomString());
      Files::DataLake::CreateFileSystemOptions options;
      options.AccessType = Files::DataLake::Models::PublicAccessType::Path;
      fileSystem.Create(options);
      auto ret = fileSystem.GetAccessPolicy();
      EXPECT_EQ(Files::DataLake::Models::PublicAccessType::Path, ret.Value.AccessType);
    }
  }

  TEST_F(DataLakeFileSystemClientTest, RenameFile)
  {
    const std::string oldFilename = RandomString() + "1";
    const std::string newFilename = RandomString() + "2";

    auto oldFileClient = m_fileSystemClient->GetFileClient(oldFilename);
    oldFileClient.Create();

    auto newFileClient = m_fileSystemClient->RenameFile(oldFilename, newFilename).Value;

    EXPECT_NO_THROW(newFileClient.GetProperties());
    EXPECT_NO_THROW(m_fileSystemClient->GetFileClient(newFilename).GetProperties());
    EXPECT_THROW(oldFileClient.GetProperties(), StorageException);

    const std::string newFileSystemName = LowercaseRandomString() + "1";
    const std::string newFilename2 = LowercaseRandomString() + "3";

    auto newFileSystem = GetFileSystemClientForTest(newFileSystemName);
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
    const std::string baseName = RandomString();
    const std::string oldDirectoryName = baseName + "1";
    const std::string newDirectoryName = baseName + "2";

    auto oldDirectoryClient = m_fileSystemClient->GetDirectoryClient(oldDirectoryName);
    oldDirectoryClient.Create();
    oldDirectoryClient.GetFileClient(baseName + "3").Create();
    oldDirectoryClient.GetSubdirectoryClient(baseName + "4").Create();

    auto newDirectoryClient
        = m_fileSystemClient->RenameDirectory(oldDirectoryName, newDirectoryName).Value;

    EXPECT_NO_THROW(newDirectoryClient.GetProperties());
    EXPECT_NO_THROW(m_fileSystemClient->GetDirectoryClient(newDirectoryName).GetProperties());
    EXPECT_THROW(oldDirectoryClient.GetProperties(), StorageException);

    const std::string newFileSystemName = LowercaseRandomString();
    const std::string newDirectoryName2 = baseName + "5";

    auto newFileSystem = GetFileSystemClientForTest(newFileSystemName);
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

  TEST_F(DataLakeFileSystemClientTest, RenameFileSasAuthentication)
  {
    const std::string baseName = RandomString();
    const std::string sourceFilename = baseName + "1";
    const std::string destinationFilename = baseName + "2";
    auto fileClient = m_fileSystemClient->GetFileClient(sourceFilename);
    fileClient.CreateIfNotExists();

    Files::DataLake::DataLakeClientOptions options;
    InitClientOptions(options);
    Files::DataLake::DataLakeFileSystemClient fileSystemClientSas(
        Files::DataLake::_detail::GetDfsUrlFromUrl(m_fileSystemClient->GetUrl()) + GetSas(),
        options);
    fileSystemClientSas.RenameFile(sourceFilename, destinationFilename);
    EXPECT_THROW(
        m_fileSystemClient->GetFileClient(sourceFilename).GetProperties(), StorageException);
    EXPECT_NO_THROW(m_fileSystemClient->GetFileClient(destinationFilename).GetProperties());

    const std::string sourceDirectoryName = baseName + "3";
    const std::string destinationDirectoryName = baseName + "4";
    auto directoryClient = m_fileSystemClient->GetDirectoryClient(sourceDirectoryName);
    directoryClient.CreateIfNotExists();

    fileSystemClientSas.RenameDirectory(sourceDirectoryName, destinationDirectoryName);
    EXPECT_THROW(
        m_fileSystemClient->GetDirectoryClient(sourceDirectoryName).GetProperties(),
        StorageException);
    EXPECT_NO_THROW(
        m_fileSystemClient->GetDirectoryClient(destinationDirectoryName).GetProperties());
  }

  TEST_F(DataLakeFileSystemClientTest, ListDeletedPaths)
  {
    const std::string deletedFilename = RandomString() + "_file_deleted";
    const std::string nonDeletedFilename = RandomString() + "_file";
    const std::string deletedDirectoryName = RandomString() + "_dir_deleted";
    const std::string nonDeletedDirectoryName = RandomString() + "_dir";

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
      std::set<std::string> paths;
      for (auto pageResult = m_fileSystemClient->ListDeletedPaths(); pageResult.HasPage();
           pageResult.MoveToNextPage())
      {
        for (const auto& p : pageResult.DeletedPaths)
        {
          paths.insert(p.Name);
        }
      }
      EXPECT_NE(paths.find(deletedDirectoryName), paths.end());
      EXPECT_NE(paths.find(deletedFilename), paths.end());
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
        EXPECT_LE(pageResult.DeletedPaths.size(), 1);
      }
      EXPECT_EQ(2, paths.size());
    }
    // prefix works
    {
      const std::string directoryName = RandomString() + "_prefix";
      const std::string filename = "file";

      auto directoryClient = m_fileSystemClient->GetDirectoryClient(directoryName);
      directoryClient.Create();
      auto fileClient = directoryClient.GetFileClient(filename);
      fileClient.Create();
      fileClient.Delete();

      Files::DataLake::ListDeletedPathsOptions options;
      options.Prefix = directoryName;
      std::vector<Files::DataLake::Models::PathDeletedItem> paths;
      for (auto pageResult = m_fileSystemClient->ListDeletedPaths(options); pageResult.HasPage();
           pageResult.MoveToNextPage())
      {
        paths.insert(paths.end(), pageResult.DeletedPaths.begin(), pageResult.DeletedPaths.end());
      }
      EXPECT_EQ(1, paths.size());
      EXPECT_EQ(directoryName + "/" + filename, paths[0].Name);
    }
  }

  TEST_F(DataLakeFileSystemClientTest, Undelete)
  {
    const std::string directoryName = RandomString() + "_dir";
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
