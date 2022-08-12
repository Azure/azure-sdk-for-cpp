// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "share_directory_client_test.hpp"

#include <algorithm>
#include <chrono>

namespace Azure { namespace Storage { namespace Test {

  void FileShareDirectoryClientTest::SetUp()
  {
    FileShareClientTest::SetUp();
    CHECK_SKIP_TEST();

    m_directoryName = GetTestName() + "base";
    m_fileShareDirectoryClient = std::make_shared<Files::Shares::ShareDirectoryClient>(
        m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(m_directoryName));
    m_fileShareDirectoryClient->Create();
  }

  void FileShareDirectoryClientTest::TearDown()
  {
    CHECK_SKIP_TEST();
    m_shareClient->DeleteIfExists();
    FileShareClientTest::TearDown();
  }

  std::pair<
      std::vector<Files::Shares::Models::FileItem>,
      std::vector<Files::Shares::Models::DirectoryItem>>
  FileShareDirectoryClientTest::ListAllFilesAndDirectories(
      const std::string& directoryPath,
      const std::string& prefix)
  {
    std::vector<Files::Shares::Models::DirectoryItem> directoryResult;
    std::vector<Files::Shares::Models::FileItem> fileResult;
    Files::Shares::ListFilesAndDirectoriesOptions options;
    if (!prefix.empty())
    {
      options.Prefix = prefix;
    }
    auto directoryClient
        = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(directoryPath);
    for (auto pageResult = directoryClient.ListFilesAndDirectories(options); pageResult.HasPage();
         pageResult.MoveToNextPage())
    {
      directoryResult.insert(
          directoryResult.end(), pageResult.Directories.begin(), pageResult.Directories.end());
      fileResult.insert(fileResult.end(), pageResult.Files.begin(), pageResult.Files.end());
    }
    return std::make_pair<
        std::vector<Files::Shares::Models::FileItem>,
        std::vector<Files::Shares::Models::DirectoryItem>>(
        std::move(fileResult), std::move(directoryResult));
  }

  TEST_F(FileShareDirectoryClientTest, CreateDeleteDirectories)
  {
    {
      // Normal create/delete.
      std::vector<Files::Shares::ShareDirectoryClient> directoryClients;
      for (int32_t i = 0; i < 5; ++i)
      {
        Files::Shares::ShareDirectoryClient client
            = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(
                m_testName + std::to_string(i));
        EXPECT_NO_THROW(client.Create());
        directoryClients.emplace_back(std::move(client));
      }
      for (const auto& client : directoryClients)
      {
        EXPECT_NO_THROW(client.Delete());
      }
    }

    {
      // Create directory that already exist throws.
      for (int32_t i = 0; i < 5; ++i)
      {
        Files::Shares::ShareDirectoryClient client
            = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(
                m_testName + "e" + std::to_string(i));
        EXPECT_NO_THROW(client.Create());
        EXPECT_THROW(client.Create(), StorageException);
      }
    }
    {
      // CreateIfNotExists & DeleteIfExists.
      {
        auto client = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(
            m_testName + "CreateIfNotExists");
        EXPECT_NO_THROW(client.Create());
        EXPECT_NO_THROW(client.CreateIfNotExists());
        EXPECT_NO_THROW(client.Delete());
        EXPECT_NO_THROW(client.DeleteIfExists());
      }
      {
        auto client = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(
            m_testName + "StorageException");
        EXPECT_NO_THROW(client.CreateIfNotExists());
        EXPECT_THROW(client.Create(), StorageException);
        EXPECT_NO_THROW(client.DeleteIfExists());
      }
      {
        auto client
            = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(m_testName + "delete");
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
        auto client
            = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(m_testName + "deleted");
        auto deleteResult = client.DeleteIfExists();
        EXPECT_FALSE(deleteResult.Value.Deleted);
      }
      {
        auto shareClient = Files::Shares::ShareClient::CreateFromConnectionString(
            StandardStorageConnectionString(), m_testNameLowercase + "1", m_options);
        auto client
            = shareClient.GetRootDirectoryClient().GetSubdirectoryClient(m_testName + "new");
        auto deleteResult = client.DeleteIfExists();
        EXPECT_FALSE(deleteResult.Value.Deleted);
      }
      {
        auto client = m_shareClient->GetRootDirectoryClient()
                          .GetSubdirectoryClient(m_testName + "s")
                          .GetSubdirectoryClient(m_testName + "s1");
        auto deleteResult = client.DeleteIfExists();
        EXPECT_FALSE(deleteResult.Value.Deleted);
      }
    }
  }

  TEST_F(FileShareDirectoryClientTest, DirectoryMetadata)
  {
    auto metadata1 = GetMetadata();
    auto metadata2 = GetMetadata();
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
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(m_testName + "meta1");
      auto client2
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(m_testName + "meta2");
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
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(m_testName + "1");
      auto client2
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(m_testName + "2");
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
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(m_testName + "3");
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
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(m_testName + "4");
      auto client2
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(m_testName + "5");

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
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(m_testName + "6");
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
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(m_testName + "1");
      auto client2
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(m_testName + "2");
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
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(m_testName + "3");
      auto client2
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(m_testName + "4");

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
        = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(m_testName);
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

  TEST_F(FileShareDirectoryClientTest, ListFilesAndDirectoriesSinglePageTest)
  {
    // Setup
    auto directoryNameA = "dirA";
    auto directoryNameB = "dirB";
    std::vector<std::string> directoryNameSetA;
    std::vector<std::string> directoryNameSetB;
    std::vector<std::string> fileNameSetA;
    std::vector<std::string> fileNameSetB;
    auto clientA = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(directoryNameA);
    clientA.Create();
    auto clientB = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(directoryNameB);
    clientB.Create();
    for (size_t i = 0; i < 5; ++i)
    {
      {
        auto directoryName = m_testName + std::to_string(i);
        auto fileName = "file" + std::to_string(i);
        EXPECT_NO_THROW(clientA.GetSubdirectoryClient(directoryName).Create());
        EXPECT_NO_THROW(clientA.GetFileClient(fileName).Create(1024));
        directoryNameSetA.emplace_back(std::move(directoryName));
        fileNameSetA.emplace_back(std::move(fileName));
      }
      {
        auto directoryName = m_testName + "B" + std::to_string(i);
        auto fileName = "fileB" + std::to_string(i);
        EXPECT_NO_THROW(clientB.GetSubdirectoryClient(directoryName).Create());
        EXPECT_NO_THROW(clientB.GetFileClient(fileName).Create(1024));
        directoryNameSetB.emplace_back(std::move(directoryName));
        fileNameSetB.emplace_back(std::move(fileName));
      }
    }
    {
      // Normal root share list.
      auto result = ListAllFilesAndDirectories();
      EXPECT_TRUE(result.first.empty());
      EXPECT_TRUE(result.second.size() >= 2);
      auto iter = std::find_if(
          result.second.begin(),
          result.second.end(),
          [&directoryNameA](const Files::Shares::Models::DirectoryItem& item) {
            return item.Name == directoryNameA;
          });
      EXPECT_EQ(iter->Name, directoryNameA);
      EXPECT_NE(result.second.end(), iter);
      iter = std::find_if(
          result.second.begin(),
          result.second.end(),
          [&directoryNameB](const Files::Shares::Models::DirectoryItem& item) {
            return item.Name == directoryNameB;
          });
      EXPECT_EQ(iter->Name, directoryNameB);
      EXPECT_NE(result.second.end(), iter);
    }
    {
      // List with directory.
      auto result = ListAllFilesAndDirectories(directoryNameA, std::string());
      for (const auto& name : directoryNameSetA)
      {
        auto iter = std::find_if(
            result.second.begin(),
            result.second.end(),
            [&name](const Files::Shares::Models::DirectoryItem& item) {
              return item.Name == name;
            });
        EXPECT_EQ(iter->Name, name);
        EXPECT_NE(result.second.end(), iter);
      }
      for (const auto& name : fileNameSetA)
      {
        auto iter = std::find_if(
            result.first.begin(),
            result.first.end(),
            [&name](const Files::Shares::Models::FileItem& item) { return item.Name == name; });
        EXPECT_EQ(iter->Name, name);
        EXPECT_EQ(1024, iter->Details.FileSize);
        EXPECT_NE(result.first.end(), iter);
      }
      for (const auto& name : directoryNameSetB)
      {
        auto iter = std::find_if(
            result.second.begin(),
            result.second.end(),
            [&name](const Files::Shares::Models::DirectoryItem& item) {
              return item.Name == name;
            });
        EXPECT_EQ(result.second.end(), iter);
      }
      for (const auto& name : fileNameSetB)
      {
        auto iter = std::find_if(
            result.first.begin(),
            result.first.end(),
            [&name](const Files::Shares::Models::FileItem& item) { return item.Name == name; });
        EXPECT_EQ(result.first.end(), iter);
      }
    }
    {
      // List with prefix.
      auto result = ListAllFilesAndDirectories(std::string(), directoryNameA);
      EXPECT_TRUE(result.first.empty());
      EXPECT_EQ(result.second.size(), size_t(1));
      EXPECT_EQ(directoryNameA, result.second[0].Name);
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
  }

  TEST_F(FileShareDirectoryClientTest, HandlesFunctionalityWorks)
  {
    auto result = m_fileShareDirectoryClient->ListHandles();
    EXPECT_TRUE(result.DirectoryHandles.empty());
    EXPECT_FALSE(result.NextPageToken.HasValue());
    for (auto pageResult = m_fileShareDirectoryClient->ListHandles(); pageResult.HasPage();
         pageResult.MoveToNextPage())
    {
    }
    EXPECT_NO_THROW(m_fileShareDirectoryClient->ForceCloseAllHandles());
    for (auto pageResult = m_fileShareDirectoryClient->ForceCloseAllHandles(); pageResult.HasPage();
         pageResult.MoveToNextPage())
    {
    }
  }
}}} // namespace Azure::Storage::Test
