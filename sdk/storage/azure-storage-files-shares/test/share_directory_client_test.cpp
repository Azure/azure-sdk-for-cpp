// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "share_directory_client_test.hpp"

#include <algorithm>

namespace Azure { namespace Storage { namespace Test {

  std::shared_ptr<Files::Shares::DirectoryClient>
      FileShareDirectoryClientTest::m_fileShareDirectoryClient;
  std::string FileShareDirectoryClientTest::m_directoryName;

  void FileShareDirectoryClientTest::SetUpTestSuite()
  {
    m_directoryName = LowercaseRandomString();
    m_shareName = LowercaseRandomString();
    m_shareClient = std::make_shared<Files::Shares::ShareClient>(
        Files::Shares::ShareClient::CreateFromConnectionString(
            StandardStorageConnectionString(), m_shareName));
    m_shareClient->Create();
    m_fileShareDirectoryClient = std::make_shared<Files::Shares::DirectoryClient>(
        m_shareClient->GetDirectoryClient(m_directoryName));
    m_fileShareDirectoryClient->Create();
  }

  void FileShareDirectoryClientTest::TearDownTestSuite() { m_shareClient->Delete(); }

  Files::Shares::FileShareHttpHeaders FileShareDirectoryClientTest::GetInterestingHttpHeaders()
  {
    static Files::Shares::FileShareHttpHeaders result = []() {
      Files::Shares::FileShareHttpHeaders ret;
      ret.CacheControl = std::string("no-cache");
      ret.ContentDisposition = std::string("attachment");
      ret.ContentEncoding = std::string("deflate");
      ret.ContentLanguage = std::string("en-US");
      ret.ContentType = std::string("application/octet-stream");
      return ret;
    }();
    return result;
  }

  std::pair<std::vector<Files::Shares::FileItem>, std::vector<Files::Shares::DirectoryItem>>
  FileShareDirectoryClientTest::ListAllFilesAndDirectories(
      const std::string& directoryPath,
      const std::string& prefix)
  {
    std::vector<Files::Shares::DirectoryItem> directoryResult;
    std::vector<Files::Shares::FileItem> fileResult;
    std::string continuation;
    Files::Shares::ListFilesAndDirectoriesSegmentOptions options;
    if (!prefix.empty())
    {
      options.Prefix = prefix;
    }
    auto directoryClient = m_shareClient->GetDirectoryClient(directoryPath);
    do
    {
      auto response = directoryClient.ListFilesAndDirectoriesSegment(options);
      directoryResult.insert(
          directoryResult.end(), response->DirectoryItems.begin(), response->DirectoryItems.end());
      fileResult.insert(fileResult.end(), response->FileItems.begin(), response->FileItems.end());
      continuation = response->ContinuationToken;
      options.ContinuationToken = continuation;
    } while (!continuation.empty());
    return std::
        make_pair<std::vector<Files::Shares::FileItem>, std::vector<Files::Shares::DirectoryItem>>(
            std::move(fileResult), std::move(directoryResult));
  }

  TEST_F(FileShareDirectoryClientTest, CreateDeleteDirectories)
  {
    {
      // Normal create/delete.
      std::vector<Files::Shares::DirectoryClient> directoryClients;
      for (int32_t i = 0; i < 5; ++i)
      {
        auto name = RandomString(10);
        Files::Shares::DirectoryClient client = m_shareClient->GetDirectoryClient(name);
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
        auto name = RandomString(10);
        Files::Shares::DirectoryClient client = m_shareClient->GetDirectoryClient(name);
        EXPECT_NO_THROW(client.Create());
        EXPECT_THROW(client.Create(), StorageError);
      }
    }
  }

  TEST_F(FileShareDirectoryClientTest, DirectoryMetadata)
  {
    auto metadata1 = RandomMetadata();
    auto metadata2 = RandomMetadata();
    {
      // Set/Get Metadata works
      EXPECT_NO_THROW(m_fileShareDirectoryClient->SetMetadata(metadata1));
      auto result = m_fileShareDirectoryClient->GetProperties()->Metadata;
      EXPECT_EQ(metadata1, result);
      EXPECT_NO_THROW(m_fileShareDirectoryClient->SetMetadata(metadata2));
      result = m_fileShareDirectoryClient->GetProperties()->Metadata;
      EXPECT_EQ(metadata2, result);
    }

    {
      // Create directory with metadata works
      auto client1 = m_shareClient->GetDirectoryClient(LowercaseRandomString());
      auto client2 = m_shareClient->GetDirectoryClient(LowercaseRandomString());
      Files::Shares::CreateDirectoryOptions options1;
      Files::Shares::CreateDirectoryOptions options2;
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

  TEST_F(FileShareDirectoryClientTest, DirectoryPermission)
  {
    std::string permission = "O:S-1-5-21-2127521184-1604012920-1887927527-21560751G:S-1-5-21-"
                             "2127521184-1604012920-1887927527-513D:AI(A;;FA;;;SY)(A;;FA;;;BA)(A;;"
                             "0x1200a9;;;S-1-5-21-397955417-626881126-188441444-3053964)";

    {
      // Create directory with permission/permission key works
      auto client1 = m_shareClient->GetDirectoryClient(LowercaseRandomString());
      auto client2 = m_shareClient->GetDirectoryClient(LowercaseRandomString());
      Files::Shares::CreateDirectoryOptions options1;
      Files::Shares::CreateDirectoryOptions options2;
      options1.DirectoryPermission = permission;
      options2.DirectoryPermission = permission;

      EXPECT_NO_THROW(client1.Create(options1));
      EXPECT_NO_THROW(client2.Create(options2));
      auto result1 = client1.GetProperties()->FilePermissionKey;
      auto result2 = client2.GetProperties()->FilePermissionKey;
      EXPECT_EQ(result1, result2);

      auto client3 = m_shareClient->GetDirectoryClient(LowercaseRandomString());
      Files::Shares::CreateDirectoryOptions options3;
      options3.SmbProperties.PermissionKey = result1;
      EXPECT_NO_THROW(client3.Create(options3));
      auto result3 = client3.GetProperties()->FilePermissionKey;
      EXPECT_EQ(result1, result3);
    }

    {
      // Set permission with SetProperties works
      Files::Shares::FileShareSmbProperties properties;
      properties.Attributes = Files::Shares::FileAttributes::Directory
          | Files::Shares::FileAttributes::NotContentIndexed;
      properties.CreationTime = ToIso8601(std::chrono::system_clock::now(), 7);
      properties.LastWriteTime = ToIso8601(std::chrono::system_clock::now(), 7);
      properties.PermissionKey = "";
      auto client1 = m_shareClient->GetDirectoryClient(LowercaseRandomString());
      auto client2 = m_shareClient->GetDirectoryClient(LowercaseRandomString());

      EXPECT_NO_THROW(client1.Create());
      EXPECT_NO_THROW(client2.Create());
      Files::Shares::SetDirectoryPropertiesOptions options1;
      Files::Shares::SetDirectoryPropertiesOptions options2;
      options1.FilePermission = permission;
      options2.FilePermission = permission;
      EXPECT_NO_THROW(client1.SetProperties(properties, options1));
      EXPECT_NO_THROW(client2.SetProperties(properties, options2));
      auto result1 = client1.GetProperties()->FilePermissionKey;
      auto result2 = client2.GetProperties()->FilePermissionKey;
      EXPECT_EQ(result1, result2);

      auto client3 = m_shareClient->GetDirectoryClient(LowercaseRandomString());
      Files::Shares::CreateDirectoryOptions options3;
      options3.SmbProperties.PermissionKey = result1;
      std::string permissionKey;
      EXPECT_NO_THROW(permissionKey = client3.Create(options3)->FilePermissionKey);
      auto result3 = client3.GetProperties()->FilePermissionKey;
      EXPECT_EQ(permissionKey, result3);
    }
  }

  TEST_F(FileShareDirectoryClientTest, DirectorySmbProperties)
  {
    Files::Shares::FileShareSmbProperties properties;
    properties.Attributes = Files::Shares::FileAttributes::Directory
        | Files::Shares::FileAttributes::NotContentIndexed;
    properties.CreationTime = ToIso8601(std::chrono::system_clock::now(), 7);
    properties.LastWriteTime = ToIso8601(std::chrono::system_clock::now(), 7);
    properties.PermissionKey = m_fileShareDirectoryClient->GetProperties()->FilePermissionKey;
    {
      // Create directory with SmbProperties works
      auto client1 = m_shareClient->GetDirectoryClient(LowercaseRandomString());
      auto client2 = m_shareClient->GetDirectoryClient(LowercaseRandomString());
      Files::Shares::CreateDirectoryOptions options1;
      Files::Shares::CreateDirectoryOptions options2;
      options1.SmbProperties = properties;
      options2.SmbProperties = properties;

      EXPECT_NO_THROW(client1.Create(options1));
      EXPECT_NO_THROW(client2.Create(options2));
      auto directoryProperties1 = client1.GetProperties();
      auto directoryProperties2 = client2.GetProperties();
      EXPECT_EQ(directoryProperties2->FileCreationTime, directoryProperties1->FileCreationTime);
      EXPECT_EQ(directoryProperties2->FileLastWriteTime, directoryProperties1->FileLastWriteTime);
      EXPECT_EQ(directoryProperties2->FileAttributes, directoryProperties1->FileAttributes);
    }

    {
      // SetProperties works
      auto client1 = m_shareClient->GetDirectoryClient(LowercaseRandomString());
      auto client2 = m_shareClient->GetDirectoryClient(LowercaseRandomString());

      EXPECT_NO_THROW(client1.Create());
      EXPECT_NO_THROW(client2.Create());
      EXPECT_NO_THROW(client1.SetProperties(properties));
      EXPECT_NO_THROW(client2.SetProperties(properties));
      auto directoryProperties1 = client1.GetProperties();
      auto directoryProperties2 = client2.GetProperties();
      EXPECT_EQ(directoryProperties2->FileCreationTime, directoryProperties1->FileCreationTime);
      EXPECT_EQ(directoryProperties2->FileLastWriteTime, directoryProperties1->FileLastWriteTime);
      EXPECT_EQ(directoryProperties2->FileAttributes, directoryProperties1->FileAttributes);
    }
  }

  TEST_F(FileShareDirectoryClientTest, ListFilesAndDirectoriesSegmentTest)
  {
    // Setup
    auto directoryNameA = LowercaseRandomString();
    auto directoryNameB = LowercaseRandomString();
    std::vector<std::string> directoryNameSetA;
    std::vector<std::string> directoryNameSetB;
    std::vector<std::string> fileNameSetA;
    std::vector<std::string> fileNameSetB;
    auto clientA = m_shareClient->GetDirectoryClient(directoryNameA);
    clientA.Create();
    auto clientB = m_shareClient->GetDirectoryClient(directoryNameB);
    clientB.Create();
    for (size_t i = 0; i < 5; ++i)
    {
      {
        auto directoryName = LowercaseRandomString();
        auto fileName = LowercaseRandomString();
        EXPECT_NO_THROW(clientA.GetSubDirectoryClient(directoryName).Create());
        EXPECT_NO_THROW(clientA.GetFileClient(fileName).Create(1024));
        directoryNameSetA.emplace_back(std::move(directoryName));
        fileNameSetA.emplace_back(std::move(fileName));
      }
      {
        auto directoryName = LowercaseRandomString();
        auto fileName = LowercaseRandomString();
        EXPECT_NO_THROW(clientB.GetSubDirectoryClient(directoryName).Create());
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
          [&directoryNameA](const Files::Shares::DirectoryItem& item) {
            return item.Name == directoryNameA;
          });
      EXPECT_EQ(iter->Name, directoryNameA);
      EXPECT_NE(result.second.end(), iter);
      iter = std::find_if(
          result.second.begin(),
          result.second.end(),
          [&directoryNameB](const Files::Shares::DirectoryItem& item) {
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
            [&name](const Files::Shares::DirectoryItem& item) { return item.Name == name; });
        EXPECT_EQ(iter->Name, name);
        EXPECT_NE(result.second.end(), iter);
      }
      for (const auto& name : fileNameSetA)
      {
        auto iter = std::find_if(
            result.first.begin(), result.first.end(), [&name](const Files::Shares::FileItem& item) {
              return item.Name == name;
            });
        EXPECT_EQ(iter->Name, name);
        EXPECT_EQ(1024, iter->Properties.ContentLength);
        EXPECT_NE(result.first.end(), iter);
      }
      for (const auto& name : directoryNameSetB)
      {
        auto iter = std::find_if(
            result.second.begin(),
            result.second.end(),
            [&name](const Files::Shares::DirectoryItem& item) { return item.Name == name; });
        EXPECT_EQ(result.second.end(), iter);
      }
      for (const auto& name : fileNameSetB)
      {
        auto iter = std::find_if(
            result.first.begin(), result.first.end(), [&name](const Files::Shares::FileItem& item) {
              return item.Name == name;
            });
        EXPECT_EQ(result.first.end(), iter);
      }
    }
    {
      // List with prefix.
      auto result = ListAllFilesAndDirectories(std::string(), directoryNameA);
      EXPECT_TRUE(result.first.empty());
      EXPECT_TRUE(result.second.size() == 1U);
      EXPECT_EQ(directoryNameA, result.second[0].Name);
    }
    {
      // List max result
      Files::Shares::ListFilesAndDirectoriesSegmentOptions options;
      options.MaxResults = 2;
      auto directoryNameAClient = m_shareClient->GetDirectoryClient(directoryNameA);
      auto response = directoryNameAClient.ListFilesAndDirectoriesSegment(options);
      EXPECT_LE(2U, response->DirectoryItems.size() + response->FileItems.size());
    }
  }

  TEST_F(FileShareDirectoryClientTest, HandlesFunctionalityWorks)
  {
    auto result = m_fileShareDirectoryClient->ListHandlesSegment();
    EXPECT_TRUE(result->HandleList.empty());
    EXPECT_TRUE(result->ContinuationToken.empty());
    EXPECT_NO_THROW(m_fileShareDirectoryClient->ForceCloseAllHandles());
  }
}}} // namespace Azure::Storage::Test
