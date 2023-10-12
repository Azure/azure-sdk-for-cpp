// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "share_directory_client_test.hpp"

#include <algorithm>
#include <chrono>

namespace Azure { namespace Storage { namespace Test {

  void FileShareDirectoryClientTest::SetUp()
  {
    FileShareClientTest::SetUp();
    if (shouldSkipTest())
    {
      return;
    }
    m_directoryName = RandomString();
    m_fileShareDirectoryClient = std::make_shared<Files::Shares::ShareDirectoryClient>(
        m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(m_directoryName));
    m_fileShareDirectoryClient->Create();
  }

  TEST_F(FileShareDirectoryClientTest, Constructors)
  {
    auto clientOptions = InitStorageClientOptions<Files::Shares::ShareClientOptions>();
    {
      auto directoryClient = Files::Shares::ShareDirectoryClient::CreateFromConnectionString(
          StandardStorageConnectionString(), m_shareName, m_directoryName, clientOptions);
      EXPECT_NO_THROW(directoryClient.GetProperties());
    }
    {
      auto credential
          = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;
      Files::Shares::ShareDirectoryClient directoryClient(
          m_fileShareDirectoryClient->GetUrl(), credential, clientOptions);
      EXPECT_NO_THROW(directoryClient.GetProperties());
    }
    {
      auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
      auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

      auto keyCredential
          = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

      Sas::ShareSasBuilder shareSasBuilder;
      shareSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
      shareSasBuilder.StartsOn = sasStartsOn;
      shareSasBuilder.ExpiresOn = sasExpiresOn;
      shareSasBuilder.ShareName = m_shareName;
      shareSasBuilder.Resource = Sas::ShareSasResource::Share;
      shareSasBuilder.SetPermissions(Sas::ShareSasPermissions::All);
      auto sasToken = shareSasBuilder.GenerateSasToken(*keyCredential);

      auto directoryClient = Files::Shares::ShareDirectoryClient(
          m_fileShareDirectoryClient->GetUrl() + sasToken, clientOptions);
      EXPECT_NO_THROW(directoryClient.GetProperties());
    }
  }

  TEST_F(FileShareDirectoryClientTest, CreateDeleteDirectories)
  {
    {
      // Normal create/delete.
      Files::Shares::ShareDirectoryClient client
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(RandomString());
      EXPECT_NO_THROW(client.Create());
      EXPECT_NO_THROW(client.Delete());
    }

    {
      // Create directory that already exist throws.
      Files::Shares::ShareDirectoryClient client
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(RandomString());
      EXPECT_NO_THROW(client.Create());
      EXPECT_THROW(client.Create(), StorageException);
    }
    {
      // CreateIfNotExists & DeleteIfExists.
      {
        auto client = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(RandomString());
        EXPECT_NO_THROW(client.Create());
        EXPECT_NO_THROW(client.CreateIfNotExists());
        EXPECT_NO_THROW(client.Delete());
        EXPECT_NO_THROW(client.DeleteIfExists());
      }
      {
        auto client = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(RandomString());
        EXPECT_NO_THROW(client.CreateIfNotExists());
        EXPECT_THROW(client.Create(), StorageException);
        EXPECT_NO_THROW(client.DeleteIfExists());
      }
      {
        auto client = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(RandomString());
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
        auto client = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(RandomString());
        auto deleteResult = client.DeleteIfExists();
        EXPECT_FALSE(deleteResult.Value.Deleted);
      }
      {
        auto client = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(RandomString());
        auto deleteResult = client.DeleteIfExists();
        EXPECT_FALSE(deleteResult.Value.Deleted);
      }
      {
        auto client = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(RandomString());
        client = client.GetSubdirectoryClient(RandomString());
        auto deleteResult = client.DeleteIfExists();
        EXPECT_FALSE(deleteResult.Value.Deleted);
      }
    }
  }

  TEST_F(FileShareDirectoryClientTest, RenameFile)
  {
    const std::string baseName = RandomString();
    const std::string baseDirectoryName = baseName + "1";
    auto rootDirectoryClient = m_shareClient->GetRootDirectoryClient();
    auto baseDirectoryClient
        = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(baseDirectoryName);
    baseDirectoryClient.Create();
    // base test
    {
      const std::string oldFilename = baseName + "2";
      auto oldFileClient = baseDirectoryClient.GetFileClient(oldFilename);
      oldFileClient.Create(512);
      const std::string newFilename = baseName + "3";
      auto newFileClient
          = baseDirectoryClient.RenameFile(oldFilename, baseDirectoryName + "/" + newFilename)
                .Value;
      EXPECT_NO_THROW(newFileClient.GetProperties());
      EXPECT_THROW(oldFileClient.GetProperties(), StorageException);
    }

    // overwrite
    {
      const std::string oldFilename = baseName + "4";
      auto oldFileClient = baseDirectoryClient.GetFileClient(oldFilename);
      oldFileClient.Create(512);
      const std::string newFilename = baseName + "5";
      auto newFileClient = baseDirectoryClient.GetFileClient(newFilename);
      newFileClient.Create(512);
      EXPECT_THROW(
          baseDirectoryClient.RenameFile(oldFilename, baseDirectoryName + "/" + newFilename),
          StorageException);

      Files::Shares::RenameFileOptions options;
      options.ReplaceIfExists = true;
      EXPECT_NO_THROW(
          newFileClient
          = baseDirectoryClient
                .RenameFile(oldFilename, baseDirectoryName + "/" + newFilename, options)
                .Value);
      EXPECT_NO_THROW(newFileClient.GetProperties());
      EXPECT_THROW(oldFileClient.GetProperties(), StorageException);
    }
    // overwrite readOnly
    {
      const std::string oldFilename = baseName + "6";
      auto oldFileClient = baseDirectoryClient.GetFileClient(oldFilename);
      oldFileClient.Create(512);
      const std::string newFilename = baseName + "7";
      Files::Shares::CreateFileOptions createOptions;
      Files::Shares::Models::FileSmbProperties properties;
      properties.Attributes = Files::Shares::Models::FileAttributes::ReadOnly;
      createOptions.SmbProperties = properties;
      auto newFileClient = baseDirectoryClient.GetFileClient(newFilename);
      newFileClient.Create(512, createOptions);
      Files::Shares::RenameFileOptions renameOptions;
      renameOptions.ReplaceIfExists = true;
      EXPECT_THROW(
          baseDirectoryClient.RenameFile(
              oldFilename, baseDirectoryName + "/" + newFilename, renameOptions),
          StorageException);

      renameOptions.IgnoreReadOnly = true;
      EXPECT_NO_THROW(
          newFileClient
          = baseDirectoryClient
                .RenameFile(oldFilename, baseDirectoryName + "/" + newFilename, renameOptions)
                .Value);
      EXPECT_NO_THROW(newFileClient.GetProperties());
      EXPECT_THROW(oldFileClient.GetProperties(), StorageException);
    }
    // with options
    {
      std::string permission
          = "O:S-1-5-21-2127521184-1604012920-1887927527-21560751G:S-1-5-21-"
            "2127521184-1604012920-1887927527-513D:AI(A;;FA;;;SY)(A;;FA;;;BA)(A;;"
            "0x1200a9;;;S-1-5-21-397955417-626881126-188441444-3053964)";

      const std::string oldFilename = baseName + "8";
      auto oldFileClient = baseDirectoryClient.GetFileClient(oldFilename);
      oldFileClient.Create(512);
      const std::string newFilename = baseName + "9";
      Files::Shares::RenameFileOptions renameOptions;
      renameOptions.Metadata = RandomMetadata();
      renameOptions.FilePermission = permission;
      Files::Shares::Models::FileSmbProperties properties;
      properties.ChangedOn = std::chrono::system_clock::now();
      properties.CreatedOn = std::chrono::system_clock::now();
      properties.LastWrittenOn = std::chrono::system_clock::now();
      properties.Attributes = Files::Shares::Models::FileAttributes::None;
      renameOptions.SmbProperties = properties;
      renameOptions.ContentType = "application/x-binary";
      auto newFileClient
          = baseDirectoryClient
                .RenameFile(oldFilename, baseDirectoryName + "/" + newFilename, renameOptions)
                .Value;
      Files::Shares::Models::FileProperties newProperties;
      EXPECT_NO_THROW(newProperties = newFileClient.GetProperties().Value);
      EXPECT_THROW(oldFileClient.GetProperties(), StorageException);
      EXPECT_EQ(renameOptions.Metadata, newProperties.Metadata);
      EXPECT_EQ(properties.Attributes, newProperties.SmbProperties.Attributes);
      EXPECT_EQ(renameOptions.ContentType.Value(), newProperties.HttpHeaders.ContentType);
    }

    // diff directory
    {
      const std::string oldSubdirectoryName = baseName + "10";
      auto oldSubdirectoryClient = baseDirectoryClient.GetSubdirectoryClient(oldSubdirectoryName);
      oldSubdirectoryClient.Create();
      const std::string oldFilename = baseName + "11";
      auto oldFileClient = oldSubdirectoryClient.GetFileClient(oldFilename);
      oldFileClient.Create(512);

      const std::string otherDirectoryname = baseName + "12";
      auto otherDirectoryClient = rootDirectoryClient.GetSubdirectoryClient(otherDirectoryname);
      otherDirectoryClient.Create();
      const std::string newFilename = baseName + "13";
      auto newFileClient
          = baseDirectoryClient
                .RenameFile(
                    oldSubdirectoryName + "/" + oldFilename, otherDirectoryname + "/" + newFilename)
                .Value;
      EXPECT_NO_THROW(newFileClient.GetProperties());
      EXPECT_THROW(oldFileClient.GetProperties(), StorageException);
    }
    // root directory
    {
      const std::string oldFilename = baseName + "14";
      auto oldFileClient = baseDirectoryClient.GetFileClient(oldFilename);
      oldFileClient.Create(512);
      const std::string newFilename = baseName + "15";
      auto newFileClient = baseDirectoryClient.RenameFile(oldFilename, newFilename).Value;
      EXPECT_NO_THROW(newFileClient.GetProperties());
      EXPECT_THROW(oldFileClient.GetProperties(), StorageException);
    }
    // lease
    {
      const std::string oldFilename = baseName + "16";
      auto oldFileClient = baseDirectoryClient.GetFileClient(oldFilename);
      oldFileClient.Create(512);
      const std::string oldLeaseId = RandomUUID();
      Files::Shares::ShareLeaseClient oldLeaseClient(oldFileClient, oldLeaseId);
      oldLeaseClient.Acquire(Files::Shares::ShareLeaseClient::InfiniteLeaseDuration);
      const std::string newFilename = baseName + "17";
      auto newFileClient = baseDirectoryClient.GetFileClient(newFilename);
      newFileClient.Create(512);
      const std::string newLeaseId = RandomUUID();
      Files::Shares::ShareLeaseClient newLeaseClient(newFileClient, newLeaseId);
      newLeaseClient.Acquire(Files::Shares::ShareLeaseClient::InfiniteLeaseDuration);

      Files::Shares::RenameFileOptions options;
      options.ReplaceIfExists = true;
      EXPECT_THROW(
          baseDirectoryClient.RenameFile(
              oldFilename, baseDirectoryName + "/" + newFilename, options),
          StorageException);
      options.SourceAccessConditions.LeaseId = oldLeaseId;
      options.AccessConditions.LeaseId = newLeaseId;
      EXPECT_NO_THROW(baseDirectoryClient.RenameFile(
          oldFilename, baseDirectoryName + "/" + newFilename, options));
      Files::Shares::ShareLeaseClient renamedLeaseClient(newFileClient, oldLeaseId);
      renamedLeaseClient.Release();
    }
  }

  TEST_F(FileShareDirectoryClientTest, RenameSubdirectory)
  {
    const std::string baseName = RandomString();
    const std::string baseDirectoryName = baseName + "1";
    auto rootDirectoryClient = m_shareClient->GetRootDirectoryClient();
    auto baseDirectoryClient
        = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(baseDirectoryName);
    baseDirectoryClient.Create();
    // base test
    {
      const std::string oldSubdirectoryName = baseName + "2";
      auto oldSubdirectoryClient = baseDirectoryClient.GetSubdirectoryClient(oldSubdirectoryName);
      oldSubdirectoryClient.Create();
      oldSubdirectoryClient.GetFileClient(baseName + "File1").Create(512);
      const std::string newSubdirectoryName = baseName + "3";
      auto newSubdirectoryClient
          = baseDirectoryClient
                .RenameSubdirectory(
                    oldSubdirectoryName, baseDirectoryName + "/" + newSubdirectoryName)
                .Value;
      EXPECT_NO_THROW(newSubdirectoryClient.GetProperties());
      EXPECT_THROW(oldSubdirectoryClient.GetProperties(), StorageException);
    }

    // overwrite
    {
      const std::string oldSubdirectoryName = baseName + "4";
      auto oldSubdirectoryClient = baseDirectoryClient.GetSubdirectoryClient(oldSubdirectoryName);
      oldSubdirectoryClient.Create();
      const std::string existFilename = baseName + "5";
      auto existFileClient = baseDirectoryClient.GetFileClient(existFilename);
      existFileClient.Create(512);
      EXPECT_THROW(
          baseDirectoryClient.RenameSubdirectory(
              oldSubdirectoryName, baseDirectoryName + "/" + existFilename),
          StorageException);

      Files::Shares::RenameDirectoryOptions options;
      options.ReplaceIfExists = true;
      EXPECT_NO_THROW(baseDirectoryClient.RenameSubdirectory(
          oldSubdirectoryName, baseDirectoryName + "/" + existFilename, options));
      EXPECT_THROW(oldSubdirectoryClient.GetProperties(), StorageException);
    }
    // overwrite readOnly
    {
      const std::string oldSubdirectoryName = baseName + "6";
      auto oldSubdirectoryClient = baseDirectoryClient.GetSubdirectoryClient(oldSubdirectoryName);
      oldSubdirectoryClient.Create();
      const std::string existFilename = baseName + "7";
      Files::Shares::CreateFileOptions createOptions;
      Files::Shares::Models::FileSmbProperties properties;
      properties.Attributes = Files::Shares::Models::FileAttributes::ReadOnly;
      createOptions.SmbProperties = properties;
      auto existFileClient = baseDirectoryClient.GetFileClient(existFilename);
      existFileClient.Create(512, createOptions);
      Files::Shares::RenameDirectoryOptions renameOptions;
      renameOptions.ReplaceIfExists = true;
      EXPECT_THROW(
          baseDirectoryClient.RenameSubdirectory(
              oldSubdirectoryName, baseDirectoryName + "/" + existFilename, renameOptions),
          StorageException);

      renameOptions.IgnoreReadOnly = true;
      EXPECT_NO_THROW(baseDirectoryClient.RenameSubdirectory(
          oldSubdirectoryName, baseDirectoryName + "/" + existFilename, renameOptions));
      EXPECT_THROW(oldSubdirectoryClient.GetProperties(), StorageException);
    }
    // with options
    {
      std::string permission
          = "O:S-1-5-21-2127521184-1604012920-1887927527-21560751G:S-1-5-21-"
            "2127521184-1604012920-1887927527-513D:AI(A;;FA;;;SY)(A;;FA;;;BA)(A;;"
            "0x1200a9;;;S-1-5-21-397955417-626881126-188441444-3053964)";

      const std::string oldSubdirectoryName = baseName + "8";
      auto oldSubdirectoryClient = baseDirectoryClient.GetSubdirectoryClient(oldSubdirectoryName);
      oldSubdirectoryClient.Create();
      const std::string newSubdirectoryName = baseName + "9";
      Files::Shares::RenameDirectoryOptions renameOptions;
      renameOptions.Metadata = RandomMetadata();
      renameOptions.FilePermission = permission;
      Files::Shares::Models::FileSmbProperties properties;
      properties.ChangedOn = std::chrono::system_clock::now();
      properties.CreatedOn = std::chrono::system_clock::now();
      properties.LastWrittenOn = std::chrono::system_clock::now();
      renameOptions.SmbProperties = properties;
      auto newSubdirectoryClient = baseDirectoryClient
                                       .RenameSubdirectory(
                                           oldSubdirectoryName,
                                           baseDirectoryName + "/" + newSubdirectoryName,
                                           renameOptions)
                                       .Value;
      Files::Shares::Models::DirectoryProperties newProperties;
      EXPECT_NO_THROW(newProperties = newSubdirectoryClient.GetProperties().Value);
      EXPECT_THROW(oldSubdirectoryClient.GetProperties(), StorageException);
      EXPECT_EQ(renameOptions.Metadata, newProperties.Metadata);
    }

    // diff directory
    {
      const std::string oldMiddleDirectoryName = baseName + "10";
      auto oldMiddleDirectoryClient
          = baseDirectoryClient.GetSubdirectoryClient(oldMiddleDirectoryName);
      oldMiddleDirectoryClient.Create();
      const std::string oldSubdirectoryName = baseName + "11";
      auto oldSubdirectoryClient
          = oldMiddleDirectoryClient.GetSubdirectoryClient(oldSubdirectoryName);
      oldSubdirectoryClient.Create();

      const std::string otherDirectoryName = baseName + "12";
      auto otherDirectoryClient = rootDirectoryClient.GetSubdirectoryClient(otherDirectoryName);
      otherDirectoryClient.Create();
      const std::string newSubdirectoryName = baseName + "13";
      auto newSubdirectoryClient = baseDirectoryClient
                                       .RenameSubdirectory(
                                           oldMiddleDirectoryName + "/" + oldSubdirectoryName,
                                           otherDirectoryName + "/" + newSubdirectoryName)
                                       .Value;
      EXPECT_NO_THROW(newSubdirectoryClient.GetProperties());
      EXPECT_THROW(oldSubdirectoryClient.GetProperties(), StorageException);
    }
    // root directory
    {
      const std::string oldSubdirectoryName = baseName + "14";
      auto oldSubdirectoryClient = baseDirectoryClient.GetSubdirectoryClient(oldSubdirectoryName);
      oldSubdirectoryClient.Create();
      oldSubdirectoryClient.GetFileClient(baseName + "File1").Create(512);
      const std::string newSubdirectoryName = baseName + "15";
      auto newSubdirectoryClient
          = baseDirectoryClient.RenameSubdirectory(oldSubdirectoryName, newSubdirectoryName).Value;
      EXPECT_NO_THROW(newSubdirectoryClient.GetProperties());
      EXPECT_THROW(oldSubdirectoryClient.GetProperties(), StorageException);
    }
  }

  TEST_F(FileShareDirectoryClientTest, DirectoryMetadata)
  {
    auto metadata1 = RandomMetadata();
    auto metadata2 = RandomMetadata();
    {
      // Set/Get Metadata works
      EXPECT_NO_THROW(m_fileShareDirectoryClient->SetMetadata(metadata1));
      auto result = m_fileShareDirectoryClient->GetProperties().Value.Metadata;
      EXPECT_EQ(metadata1, result);
      EXPECT_NO_THROW(m_fileShareDirectoryClient->SetMetadata(metadata2));
      result = m_fileShareDirectoryClient->GetProperties().Value.Metadata;
      EXPECT_EQ(metadata2, result);
    }

    {
      // Create directory with metadata works
      auto client1
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(RandomString() + "meta1");
      auto client2
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(RandomString() + "meta2");
      Files::Shares::CreateDirectoryOptions options1;
      Files::Shares::CreateDirectoryOptions options2;
      options1.Metadata = metadata1;
      options2.Metadata = metadata2;

      EXPECT_NO_THROW(client1.Create(options1));
      EXPECT_NO_THROW(client2.Create(options2));
      auto result = client1.GetProperties().Value.Metadata;
      EXPECT_EQ(metadata1, result);
      result = client2.GetProperties().Value.Metadata;
      EXPECT_EQ(metadata2, result);
      client1.Delete();
      client2.Delete();
    }
  }

  TEST_F(FileShareDirectoryClientTest, DirectoryPermission)
  {
    std::string permission = "O:S-1-5-21-2127521184-1604012920-1887927527-21560751G:S-1-5-21-"
                             "2127521184-1604012920-1887927527-513D:AI(A;;FA;;;SY)(A;;FA;;;BA)(A;;"
                             "0x1200a9;;;S-1-5-21-397955417-626881126-188441444-3053964)";

    {
      // Create directory with permission/permission key works
      auto client1
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(RandomString() + "1");
      auto client2
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(RandomString() + "2");
      Files::Shares::CreateDirectoryOptions options1;
      Files::Shares::CreateDirectoryOptions options2;
      options1.DirectoryPermission = permission;
      options2.DirectoryPermission = permission;

      EXPECT_NO_THROW(client1.Create(options1));
      EXPECT_NO_THROW(client2.Create(options2));
      auto result1 = client1.GetProperties().Value.SmbProperties.PermissionKey;
      auto result2 = client2.GetProperties().Value.SmbProperties.PermissionKey;
      EXPECT_EQ(result1.Value(), result2.Value());

      auto client3
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(RandomString() + "3");
      Files::Shares::CreateDirectoryOptions options3;
      options3.SmbProperties.PermissionKey = result1;
      EXPECT_NO_THROW(client3.Create(options3));
      auto result3 = client3.GetProperties().Value.SmbProperties.PermissionKey;
      EXPECT_EQ(result1.Value(), result3.Value());
    }

    {
      // Set permission with SetProperties works
      Files::Shares::Models::FileSmbProperties properties;
      properties.Attributes = Files::Shares::Models::FileAttributes::Directory
          | Files::Shares::Models::FileAttributes::NotContentIndexed;
      properties.CreatedOn = std::chrono::system_clock::now();
      properties.LastWrittenOn = std::chrono::system_clock::now();
      properties.PermissionKey = "";
      auto client1
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(RandomString() + "4");
      auto client2
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(RandomString() + "5");

      EXPECT_NO_THROW(client1.Create());
      EXPECT_NO_THROW(client2.Create());
      Files::Shares::SetDirectoryPropertiesOptions options1;
      Files::Shares::SetDirectoryPropertiesOptions options2;
      options1.FilePermission = permission;
      options2.FilePermission = permission;
      EXPECT_NO_THROW(client1.SetProperties(properties, options1));
      EXPECT_NO_THROW(client2.SetProperties(properties, options2));
      auto result1 = client1.GetProperties().Value.SmbProperties.PermissionKey;
      auto result2 = client2.GetProperties().Value.SmbProperties.PermissionKey;
      EXPECT_EQ(result1.Value(), result2.Value());

      auto client3
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(RandomString() + "6");
      Files::Shares::CreateDirectoryOptions options3;
      options3.SmbProperties.PermissionKey = result1;
      std::string permissionKey;
      EXPECT_NO_THROW(
          permissionKey = client3.Create(options3).Value.SmbProperties.PermissionKey.Value());
      auto result3 = client3.GetProperties().Value.SmbProperties.PermissionKey.Value();
      EXPECT_EQ(permissionKey, result3);
    }
  }

  TEST_F(FileShareDirectoryClientTest, DirectorySmbProperties)
  {
    Files::Shares::Models::FileSmbProperties properties;
    properties.Attributes = Files::Shares::Models::FileAttributes::Directory
        | Files::Shares::Models::FileAttributes::NotContentIndexed;
    properties.CreatedOn = std::chrono::system_clock::now();
    properties.LastWrittenOn = std::chrono::system_clock::now();
    properties.ChangedOn = std::chrono::system_clock::now();
    properties.PermissionKey
        = m_fileShareDirectoryClient->GetProperties().Value.SmbProperties.PermissionKey;
    {
      // Create directory with SmbProperties works
      auto client1
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(RandomString() + "1");
      auto client2
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(RandomString() + "2");
      Files::Shares::CreateDirectoryOptions options1;
      Files::Shares::CreateDirectoryOptions options2;
      options1.SmbProperties = properties;
      options2.SmbProperties = properties;

      EXPECT_NO_THROW(client1.Create(options1));
      EXPECT_NO_THROW(client2.Create(options2));
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
      auto client1
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(RandomString() + "3");
      auto client2
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(RandomString() + "4");

      EXPECT_NO_THROW(client1.Create());
      EXPECT_NO_THROW(client2.Create());
      EXPECT_NO_THROW(client1.SetProperties(properties));
      EXPECT_NO_THROW(client2.SetProperties(properties));
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

  TEST_F(FileShareDirectoryClientTest, SmbPropertiesDefaultValue)
  {
    auto directoryClient
        = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(RandomString());
    directoryClient.Create();
    auto smbProperties = directoryClient.GetProperties().Value.SmbProperties;
    EXPECT_EQ(smbProperties.Attributes, Files::Shares::Models::FileAttributes::Directory);
    ASSERT_TRUE(smbProperties.CreatedOn.HasValue());
    EXPECT_TRUE(IsValidTime(smbProperties.CreatedOn.Value()));
    ASSERT_TRUE(smbProperties.LastWrittenOn.HasValue());
    EXPECT_TRUE(IsValidTime(smbProperties.LastWrittenOn.Value()));
    ASSERT_TRUE(smbProperties.ChangedOn.HasValue());
    EXPECT_TRUE(IsValidTime(smbProperties.ChangedOn.Value()));

    directoryClient.SetProperties(Files::Shares::Models::FileSmbProperties());

    auto smbProperties2 = directoryClient.GetProperties().Value.SmbProperties;
    EXPECT_EQ(smbProperties2.PermissionKey.Value(), smbProperties.PermissionKey.Value());
    EXPECT_EQ(smbProperties2.Attributes, smbProperties.Attributes);
    EXPECT_EQ(smbProperties2.CreatedOn.Value(), smbProperties.CreatedOn.Value());
    EXPECT_EQ(smbProperties2.LastWrittenOn.Value(), smbProperties.LastWrittenOn.Value());
    EXPECT_NE(smbProperties2.ChangedOn.Value(), smbProperties.ChangedOn.Value());
  }

  TEST_F(FileShareDirectoryClientTest, ListFilesAndDirectoriesMultiPageTest)
  {
    auto dirClient = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(RandomString());
    dirClient.Create();
    std::set<std::string> nameSet;
    for (size_t i = 0; i < 5; ++i)
    {
      auto dirname = RandomString();
      auto subdirClient = dirClient.GetSubdirectoryClient(dirname);
      subdirClient.Create();
      auto filename = RandomString();
      auto fileClient = dirClient.GetFileClient(filename);
      fileClient.Create(1024);
      nameSet.insert(dirname);
      nameSet.insert(filename);
    }

    Files::Shares::ListFilesAndDirectoriesOptions listOptions;
    listOptions.PageSizeHint = 3;
    std::set<std::string> listedNameSet;
    int numPages = 0;
    for (auto page = dirClient.ListFilesAndDirectories(listOptions); page.HasPage();
         page.MoveToNextPage())
    {
      ++numPages;
      for (const auto& i : page.Directories)
      {
        listedNameSet.insert(i.Name);
      }
      for (const auto& i : page.Files)
      {
        listedNameSet.insert(i.Name);
      }
    }
    EXPECT_EQ(nameSet, listedNameSet);
    EXPECT_GT(numPages, 1);
  }

  TEST_F(FileShareDirectoryClientTest, ListFilesAndDirectoriesSinglePageTest)
  {
    // Setup
    auto directoryNameA = "dirA";
    auto directoryNameB = "dirB";
    std::vector<std::string> directoryNameSetA;
    std::vector<std::string> directoryNameSetB;
    std::vector<std::string> fileNameSetA;
    std::vector<std::string> fileNameSetB;
    std::string prefix = RandomString();
    auto clientA = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(directoryNameA);
    clientA.Create();
    auto clientB = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(directoryNameB);
    clientB.Create();
    for (size_t i = 0; i < 5; ++i)
    {
      {
        auto directoryName = RandomString() + std::to_string(i);
        auto fileName = RandomString() + std::to_string(i);
        EXPECT_NO_THROW(clientA.GetSubdirectoryClient(directoryName).Create());
        EXPECT_NO_THROW(clientA.GetFileClient(fileName).Create(1024));
        directoryNameSetA.emplace_back(std::move(directoryName));
        fileNameSetA.emplace_back(std::move(fileName));
      }
      {
        auto directoryName = RandomString() + std::to_string(i);
        auto fileName = RandomString() + std::to_string(i);
        EXPECT_NO_THROW(clientB.GetSubdirectoryClient(directoryName).Create());
        EXPECT_NO_THROW(clientB.GetFileClient(fileName).Create(1024));
        directoryNameSetB.emplace_back(std::move(directoryName));
        fileNameSetB.emplace_back(std::move(fileName));
      }
    }
    {
      auto dirName = prefix + RandomString();
      auto filename = prefix + RandomString();
      clientB.GetSubdirectoryClient(dirName).Create();
      clientB.GetFileClient(filename).Create(1024);
      directoryNameSetB.emplace_back(dirName);
      fileNameSetB.emplace_back(filename);
    }
    {
      // Normal root share list.
      std::set<std::string> files;
      std::set<std::string> dirs;
      for (auto page = m_shareClient->GetRootDirectoryClient().ListFilesAndDirectories();
           page.HasPage();
           page.MoveToNextPage())
      {
        for (const auto& f : page.Files)
        {
          files.insert(f.Name);
        }
        for (const auto& d : page.Directories)
        {
          dirs.insert(d.Name);
        }
      }
      EXPECT_TRUE(files.empty());
      EXPECT_FALSE(dirs.empty());
      EXPECT_NE(dirs.find(directoryNameA), dirs.end());
      EXPECT_NE(dirs.find(directoryNameB), dirs.end());
    }
    {
      std::set<std::string> files;
      std::set<std::string> dirs;
      for (auto page = clientA.ListFilesAndDirectories(); page.HasPage(); page.MoveToNextPage())
      {
        for (const auto& f : page.Files)
        {
          files.insert(f.Name);
        }
        for (const auto& d : page.Directories)
        {
          dirs.insert(d.Name);
        }
      }
      for (const auto& d : directoryNameSetA)
      {
        EXPECT_NE(dirs.find(d), dirs.end());
      }
      for (const auto& f : fileNameSetA)
      {
        EXPECT_NE(files.find(f), files.end());
      }
    }
    {
      // List with prefix.
      std::set<std::string> files;
      std::set<std::string> dirs;
      Files::Shares::ListFilesAndDirectoriesOptions listOptions;
      listOptions.Prefix = prefix;
      for (auto page = clientB.ListFilesAndDirectories(listOptions); page.HasPage();
           page.MoveToNextPage())
      {
        for (const auto& f : page.Files)
        {
          files.insert(f.Name);
        }
        for (const auto& d : page.Directories)
        {
          dirs.insert(d.Name);
        }
      }
      for (const auto& f : fileNameSetB)
      {
        if (f.substr(0, prefix.length()) == prefix)
        {
          EXPECT_NE(files.find(f), files.end());
        }
        else
        {
          EXPECT_EQ(files.find(f), files.end());
        }
      }
      for (const auto& d : directoryNameSetB)
      {
        if (d.substr(0, prefix.length()) == prefix)
        {
          EXPECT_NE(dirs.find(d), dirs.end());
        }
        else
        {
          EXPECT_EQ(dirs.find(d), dirs.end());
        }
      }
    }
    {
      // List max result
      Files::Shares::ListFilesAndDirectoriesOptions options;
      options.PageSizeHint = 2;
      auto directoryNameAClient
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(directoryNameA);
      auto response = directoryNameAClient.ListFilesAndDirectories(options);
      EXPECT_LE(2U, response.Directories.size() + response.Files.size());
    }
    {
      // List with include option
      Files::Shares::ListFilesAndDirectoriesOptions options;
      options.Include = Files::Shares::Models::ListFilesIncludeFlags::Timestamps
          | Files::Shares::Models::ListFilesIncludeFlags::ETag
          | Files::Shares::Models::ListFilesIncludeFlags::Attributes
          | Files::Shares::Models::ListFilesIncludeFlags::PermissionKey;
      options.IncludeExtendedInfo = true;
      auto directoryNameAClient
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(directoryNameA);
      auto response = directoryNameAClient.ListFilesAndDirectories(options);
      auto directories = response.Directories;
      auto files = response.Files;
      for (const auto& name : directoryNameSetA)
      {
        auto iter = std::find_if(
            directories.begin(),
            directories.end(),
            [&name](const Files::Shares::Models::DirectoryItem& item) {
              return item.Name == name;
            });
        auto directoryProperties = directoryNameAClient.GetSubdirectoryClient(name).GetProperties();
        EXPECT_TRUE(iter->Details.Etag.HasValue());
        EXPECT_TRUE(iter->Details.LastAccessedOn.HasValue());
        EXPECT_EQ(iter->Details.LastModified, directoryProperties.Value.LastModified);
        EXPECT_EQ(
            iter->Details.SmbProperties.Attributes,
            directoryProperties.Value.SmbProperties.Attributes);
        EXPECT_EQ(
            iter->Details.SmbProperties.FileId, directoryProperties.Value.SmbProperties.FileId);
        EXPECT_EQ(
            iter->Details.SmbProperties.ChangedOn.Value(),
            directoryProperties.Value.SmbProperties.ChangedOn.Value());
        EXPECT_EQ(
            iter->Details.SmbProperties.CreatedOn.Value(),
            directoryProperties.Value.SmbProperties.CreatedOn.Value());
        EXPECT_EQ(
            iter->Details.SmbProperties.LastWrittenOn.Value(),
            directoryProperties.Value.SmbProperties.LastWrittenOn.Value());
        EXPECT_EQ(
            iter->Details.SmbProperties.PermissionKey.Value(),
            directoryProperties.Value.SmbProperties.PermissionKey.Value());
      }
      for (const auto& name : fileNameSetA)
      {
        auto iter = std::find_if(
            files.begin(), files.end(), [&name](const Files::Shares::Models::FileItem& item) {
              return item.Name == name;
            });
        auto fileProperties = directoryNameAClient.GetFileClient(name).GetProperties();
        EXPECT_TRUE(iter->Details.Etag.HasValue());
        EXPECT_TRUE(iter->Details.LastAccessedOn.HasValue());
        EXPECT_EQ(iter->Details.LastModified, fileProperties.Value.LastModified);
        EXPECT_EQ(
            iter->Details.SmbProperties.Attributes, fileProperties.Value.SmbProperties.Attributes);
        EXPECT_EQ(iter->Details.SmbProperties.FileId, fileProperties.Value.SmbProperties.FileId);
        EXPECT_EQ(
            iter->Details.SmbProperties.ChangedOn.Value(),
            fileProperties.Value.SmbProperties.ChangedOn.Value());
        EXPECT_EQ(
            iter->Details.SmbProperties.CreatedOn.Value(),
            fileProperties.Value.SmbProperties.CreatedOn.Value());
        EXPECT_EQ(
            iter->Details.SmbProperties.LastWrittenOn.Value(),
            fileProperties.Value.SmbProperties.LastWrittenOn.Value());
        EXPECT_EQ(
            iter->Details.SmbProperties.PermissionKey.Value(),
            fileProperties.Value.SmbProperties.PermissionKey.Value());
        EXPECT_EQ(1024, iter->Details.FileSize);
      }
      EXPECT_EQ(
          response.DirectoryId, directoryNameAClient.GetProperties().Value.SmbProperties.FileId);
    }
  }

  TEST_F(FileShareDirectoryClientTest, ListFilesAndDirectoriesEncoded)
  {
    const std::string prefix = "prefix\xEF\xBF\xBF";
    const std::string specialParentDirectoryName = prefix + "directory_parent";
    const std::string specialFileName = prefix + "file";
    const std::string specialDirectoryName = prefix + "directory";
    auto parentDirectoryClient
        = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(specialParentDirectoryName);
    auto fileClient = parentDirectoryClient.GetFileClient(specialFileName);
    auto directoryClient = parentDirectoryClient.GetSubdirectoryClient(specialDirectoryName);
    parentDirectoryClient.Create();
    fileClient.Create(1024);
    directoryClient.Create();
    auto fileUrl = fileClient.GetUrl();
    EXPECT_EQ(
        fileUrl, parentDirectoryClient.GetUrl() + "/" + _internal::UrlEncodePath(specialFileName));
    auto directoryUrl = directoryClient.GetUrl();
    EXPECT_EQ(
        directoryUrl,
        parentDirectoryClient.GetUrl() + "/" + _internal::UrlEncodePath(specialDirectoryName));
    Files::Shares::ListFilesAndDirectoriesOptions options;
    options.Prefix = prefix;
    auto response = parentDirectoryClient.ListFilesAndDirectories(options);
    EXPECT_EQ(response.DirectoryPath, specialParentDirectoryName);
    EXPECT_EQ(response.Prefix, prefix);
    EXPECT_EQ(response.Directories.size(), 1L);
    EXPECT_EQ(response.Directories[0].Name, specialDirectoryName);
    EXPECT_EQ(response.Files.size(), 1L);
    EXPECT_EQ(response.Files[0].Name, specialFileName);
  }

  // cspell:ignore myshare mydirectory
  TEST_F(FileShareDirectoryClientTest, HandlesFunctionalityWorks_PLAYBACKONLY_)
  {
    auto shareClient = Files::Shares::ShareClient::CreateFromConnectionString(
        StandardStorageConnectionString(),
        "myshare",
        InitStorageClientOptions<Files::Shares::ShareClientOptions>());
    auto directoryClient
        = shareClient.GetRootDirectoryClient().GetSubdirectoryClient("mydirectory");
    Files::Shares::ListDirectoryHandlesOptions options;
    options.PageSizeHint = 1;
    std::unordered_set<std::string> handles;
    for (auto pageResult = directoryClient.ListHandles(options); pageResult.HasPage();
         pageResult.MoveToNextPage())
    {
      ASSERT_EQ(1L, pageResult.DirectoryHandles.size());
      handles.insert(pageResult.DirectoryHandles[0].HandleId);
    }
    EXPECT_EQ(handles.size(), 2);

    EXPECT_NO_THROW(directoryClient.ForceCloseAllHandles());

    auto result = directoryClient.ListHandles();
    EXPECT_TRUE(result.DirectoryHandles.empty());
  }

  TEST_F(FileShareDirectoryClientTest, AllowTrailingDot)
  {
    const std::string directoryName = RandomString();
    const std::string directoryNameWithTrailingDot = directoryName + ".";
    const std::string connectionString = StandardStorageConnectionString();
    const std::string shareName = m_shareName;
    auto options = InitStorageClientOptions<Files::Shares::ShareClientOptions>();

    auto testTrailingDot = [&](Nullable<bool> allowTrailingDot) {
      options.AllowTrailingDot = allowTrailingDot;

      auto shareServiceClient = Files::Shares::ShareServiceClient::CreateFromConnectionString(
          connectionString, options);
      auto shareClient = shareServiceClient.GetShareClient(shareName);
      auto rootDirectoryClient = shareClient.GetRootDirectoryClient();
      auto directoryClient
          = rootDirectoryClient.GetSubdirectoryClient(directoryNameWithTrailingDot);

      // Create
      auto createResult = directoryClient.Create().Value;

      // ListFilesAndDirectories
      bool isFound = false;
      for (auto page = rootDirectoryClient.ListFilesAndDirectories(); page.HasPage();
           page.MoveToNextPage())
      {
        auto directories = page.Directories;
        auto iter = std::find_if(
            directories.begin(),
            directories.end(),
            [&allowTrailingDot, &directoryName, &directoryNameWithTrailingDot](
                const Files::Shares::Models::DirectoryItem& directory) {
              std::string name = allowTrailingDot.HasValue() && allowTrailingDot.Value()
                  ? directoryNameWithTrailingDot
                  : directoryName;
              return directory.Name == name;
            });
        if (iter != directories.end())
        {
          isFound = true;
          break;
        }
      }
      EXPECT_TRUE(isFound);

      // GetProperties
      auto properties = directoryClient.GetProperties().Value;
      EXPECT_EQ(createResult.LastModified, properties.LastModified);
      EXPECT_EQ(createResult.ETag, properties.ETag);

      // ListHandles
      auto handles = directoryClient.ListHandles().DirectoryHandles;
      EXPECT_EQ(handles.size(), 0L);

      // SetProperties
      EXPECT_NO_THROW(directoryClient.SetProperties(Files::Shares::Models::FileSmbProperties()));

      // SetMetadata
      EXPECT_NO_THROW(directoryClient.SetMetadata(RandomMetadata()));

      // ForceCloseHandles
      auto closeHandlesResult = directoryClient.ForceCloseAllHandles();
      EXPECT_EQ(closeHandlesResult.NumberOfHandlesClosed, 0);
      EXPECT_EQ(closeHandlesResult.NumberOfHandlesFailedToClose, 0);

      // Delete
      EXPECT_NO_THROW(directoryClient.Delete());
    };

    // allowTrailingDot not set
    testTrailingDot(Nullable<bool>());
    // allowTrailingDot = true
    testTrailingDot(true);
    // allowTrailingDot = false
    testTrailingDot(false);
  }

  TEST_F(FileShareDirectoryClientTest, RenameAllowTrailingDot)
  {
    const std::string directoryNameWithTrailingDot = RandomString() + ".";
    const std::string connectionString = StandardStorageConnectionString();
    const std::string shareName = m_shareName;
    auto options = InitStorageClientOptions<Files::Shares::ShareClientOptions>();

    auto testTrailingDot = [&](Nullable<bool> allowTrailingDot,
                               Nullable<bool> allowSourceTrailingDot) {
      options.AllowTrailingDot = allowTrailingDot;
      options.AllowSourceTrailingDot = allowSourceTrailingDot;

      auto shareServiceClient = Files::Shares::ShareServiceClient::CreateFromConnectionString(
          connectionString, options);
      auto shareClient = shareServiceClient.GetShareClient(shareName);
      auto rootDirectoryClient = shareClient.GetRootDirectoryClient();
      auto directoryClient
          = rootDirectoryClient.GetSubdirectoryClient(directoryNameWithTrailingDot);

      directoryClient.Create();

      bool allowTarget = allowTrailingDot.HasValue() && allowTrailingDot.Value();
      bool allowSource = allowSourceTrailingDot.HasValue() && allowSourceTrailingDot.Value();

      // Rename File
      const std::string fileName = RandomString() + "_file";
      const std::string fileNameWithTrailingDot = fileName + ".";
      const std::string destFileName = fileName + "_dest";
      const std::string destFileNameWithTrailingDot = destFileName + ".";
      auto fileClient = directoryClient.GetFileClient(fileNameWithTrailingDot);
      fileClient.Create(512);
      auto destFileClient = directoryClient.GetFileClient(destFileNameWithTrailingDot);
      if (allowTarget == allowSource)
      {
        EXPECT_NO_THROW(
            destFileClient
            = directoryClient
                  .RenameFile(
                      fileNameWithTrailingDot,
                      directoryNameWithTrailingDot + "/" + destFileNameWithTrailingDot)
                  .Value);
        EXPECT_NO_THROW(destFileClient.Delete());
      }
      else
      {
        EXPECT_THROW(
            directoryClient.RenameFile(
                fileNameWithTrailingDot,
                directoryNameWithTrailingDot + "/" + destFileNameWithTrailingDot),
            StorageException);
        EXPECT_NO_THROW(fileClient.Delete());
      }

      // Rename Directory
      const std::string subdirectoryName = RandomString() + "_sub";
      const std::string subdirectoryNameWithTrailingDot = subdirectoryName + ".";
      const std::string destSubdirectoryName = subdirectoryName + "_dest";
      const std::string destSubdirectoryNameWithTrailingDot = destSubdirectoryName + ".";
      auto subdirectoryClient
          = directoryClient.GetSubdirectoryClient(subdirectoryNameWithTrailingDot);
      subdirectoryClient.Create();
      auto destSubdirectoryClient
          = directoryClient.GetSubdirectoryClient(destSubdirectoryNameWithTrailingDot);
      if (allowTarget == allowSource)
      {
        EXPECT_NO_THROW(
            destSubdirectoryClient
            = directoryClient
                  .RenameSubdirectory(
                      subdirectoryNameWithTrailingDot,
                      directoryNameWithTrailingDot + "/" + destSubdirectoryNameWithTrailingDot)
                  .Value);
        EXPECT_NO_THROW(destSubdirectoryClient.Delete());
      }
      else
      {
        EXPECT_THROW(
            directoryClient.RenameSubdirectory(
                subdirectoryNameWithTrailingDot,
                directoryNameWithTrailingDot + "/" + destSubdirectoryNameWithTrailingDot),
            StorageException);
        EXPECT_NO_THROW(subdirectoryClient.Delete());
      }

      // Delete
      EXPECT_NO_THROW(directoryClient.Delete());
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

  TEST_F(FileShareDirectoryClientTest, OAuth_PLAYBACKONLY_)
  {
    const std::string directoryName = RandomString();

    // Create from client secret credential.
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential
        = std::make_shared<Azure::Identity::ClientSecretCredential>(
            AadTenantId(), AadClientId(), AadClientSecret(), GetTokenCredentialOptions());
    auto options = InitStorageClientOptions<Files::Shares::ShareClientOptions>();
    options.ShareTokenIntent = Files::Shares::Models::ShareTokenIntent::Backup;

    auto shareClient = Files::Shares::ShareClient(m_shareClient->GetUrl(), credential, options);
    auto rootDirectoryClient = shareClient.GetRootDirectoryClient();
    auto directoryClient = rootDirectoryClient.GetSubdirectoryClient(directoryName);

    // Create
    EXPECT_NO_THROW(directoryClient.Create());

    // ListFilesAndDirectories
    EXPECT_NO_THROW(directoryClient.ListFilesAndDirectories());

    // ListHandles
    EXPECT_NO_THROW(directoryClient.ListHandles());

    // GetProperties
    EXPECT_NO_THROW(directoryClient.GetProperties());

    // SetProperties
    EXPECT_NO_THROW(directoryClient.SetProperties(Files::Shares::Models::FileSmbProperties()));

    // SetMetadata
    EXPECT_NO_THROW(directoryClient.SetMetadata(RandomMetadata()));

    // ForceCloseHandles
    EXPECT_NO_THROW(directoryClient.ForceCloseAllHandles());

    // Rename File
    const std::string fileName = RandomString() + "_file";
    const std::string destFileName = fileName + "_dest";
    auto fileClient = directoryClient.GetFileClient(fileName);
    fileClient.Create(512);
    auto destFileClient = directoryClient.GetFileClient(destFileName);
    EXPECT_NO_THROW(
        destFileClient
        = directoryClient.RenameFile(fileName, directoryName + "/" + destFileName).Value);
    EXPECT_NO_THROW(destFileClient.Delete());

    // Rename Directory
    const std::string subdirectoryName = RandomString() + "_sub";
    const std::string destSubdirectoryName = subdirectoryName + "_dest";
    auto subdirectoryClient = directoryClient.GetSubdirectoryClient(subdirectoryName);
    subdirectoryClient.Create();
    auto destSubdirectoryClient = directoryClient.GetSubdirectoryClient(destSubdirectoryName);
    EXPECT_NO_THROW(
        destSubdirectoryClient
        = directoryClient
              .RenameSubdirectory(subdirectoryName, directoryName + "/" + destSubdirectoryName)
              .Value);
    EXPECT_NO_THROW(destSubdirectoryClient.Delete());

    // Delete
    EXPECT_NO_THROW(directoryClient.Delete());

    // OAuth Constructor
    auto directoryClient1 = Files::Shares::ShareDirectoryClient(
        m_fileShareDirectoryClient->GetUrl(),
        std::make_shared<Azure::Identity::ClientSecretCredential>(
            AadTenantId(), AadClientId(), AadClientSecret(), GetTokenCredentialOptions()),
        options);
    EXPECT_NO_THROW(directoryClient1.GetProperties());
  }

  // cspell:ignore myshare mydirectory
  TEST_F(FileShareDirectoryClientTest, ListHandlesAccessRights_PLAYBACKONLY_)
  {
    auto shareClient = Files::Shares::ShareClient::CreateFromConnectionString(
        StandardStorageConnectionString(),
        "myshare",
        InitStorageClientOptions<Files::Shares::ShareClientOptions>());
    auto directoryClient
        = shareClient.GetRootDirectoryClient().GetSubdirectoryClient("mydirectory");
    auto directoryHandles = directoryClient.ListHandles().DirectoryHandles;
    Files::Shares::Models::ShareFileHandleAccessRights allAccessRights
        = Files::Shares::Models::ShareFileHandleAccessRights::Read
        | Files::Shares::Models::ShareFileHandleAccessRights::Write
        | Files::Shares::Models::ShareFileHandleAccessRights::Delete;
    EXPECT_EQ(directoryHandles.size(), 1L);
    EXPECT_TRUE(directoryHandles[0].AccessRights.HasValue());
    EXPECT_EQ(allAccessRights, directoryHandles[0].AccessRights.Value());
  }

  TEST_F(FileShareDirectoryClientTest, WithShareSnapshot)
  {
    const std::string timestamp1 = "2001-01-01T01:01:01.1111000Z";
    const std::string timestamp2 = "2022-02-02T02:02:02.2222000Z";

    auto client1 = m_fileShareDirectoryClient->WithShareSnapshot(timestamp1);
    EXPECT_FALSE(client1.GetUrl().find("snapshot=" + timestamp1) == std::string::npos);
    EXPECT_TRUE(client1.GetUrl().find("snapshot=" + timestamp2) == std::string::npos);
    client1 = client1.WithShareSnapshot(timestamp2);
    EXPECT_TRUE(client1.GetUrl().find("snapshot=" + timestamp1) == std::string::npos);
    EXPECT_FALSE(client1.GetUrl().find("snapshot=" + timestamp2) == std::string::npos);
    client1 = client1.WithShareSnapshot("");
    EXPECT_TRUE(client1.GetUrl().find("snapshot=" + timestamp1) == std::string::npos);
    EXPECT_TRUE(client1.GetUrl().find("snapshot=" + timestamp2) == std::string::npos);
  }

  TEST_F(FileShareDirectoryClientTest, Audience_PLAYBACKONLY_)
  {
    auto credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
        AadTenantId(),
        AadClientId(),
        AadClientSecret(),
        InitStorageClientOptions<Azure::Identity::ClientSecretCredentialOptions>());
    auto clientOptions = InitStorageClientOptions<Files::Shares::ShareClientOptions>();
    clientOptions.ShareTokenIntent = Files::Shares::Models::ShareTokenIntent::Backup;

    // default audience
    auto directoryClient = Files::Shares::ShareDirectoryClient(
        m_fileShareDirectoryClient->GetUrl(), credential, clientOptions);
    EXPECT_NO_THROW(directoryClient.GetProperties());

    // custom audience
    auto directoryUrl = Azure::Core::Url(directoryClient.GetUrl());
    clientOptions.Audience = Files::Shares::Models::ShareAudience(
        directoryUrl.GetScheme() + "://" + directoryUrl.GetHost());
    directoryClient = Files::Shares::ShareDirectoryClient(
        m_fileShareDirectoryClient->GetUrl(), credential, clientOptions);
    EXPECT_NO_THROW(directoryClient.GetProperties());

    // error audience
    clientOptions.Audience = Files::Shares::Models::ShareAudience("https://disk.compute.azure.com");
    directoryClient = Files::Shares::ShareDirectoryClient(
        m_fileShareDirectoryClient->GetUrl(), credential, clientOptions);
    EXPECT_THROW(directoryClient.GetProperties(), StorageException);
  }
}}} // namespace Azure::Storage::Test
