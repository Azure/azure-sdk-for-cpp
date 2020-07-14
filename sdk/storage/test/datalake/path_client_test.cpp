// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "path_client_test.hpp"

#include <algorithm>

namespace Azure { namespace Storage { namespace Test {

  std::shared_ptr<Files::DataLake::PathClient> PathClientTest::m_pathClient;
  std::string PathClientTest::m_pathName;

  void PathClientTest::SetUpTestSuite()
  {
    FileSystemClientTest::SetUpTestSuite();
    m_pathName = LowercaseRandomString(10);
    m_pathClient = std::make_shared<Files::DataLake::PathClient>(
        m_fileSystemClient->GetPathClient(m_pathName));
    m_fileSystemClient->GetFileClient(m_pathName).Create();
  }

  void PathClientTest::TearDownTestSuite()
  {
    m_fileSystemClient->GetFileClient(m_pathName).Delete();
    FileSystemClientTest::TearDownTestSuite();
  }

  TEST_F(PathClientTest, PathMetadata)
  {
    auto metadata1 = RandomMetadata();
    auto metadata2 = RandomMetadata();
    {
      // Set/Get Metadata works
      EXPECT_NO_THROW(m_pathClient->SetMetadata(metadata1));
      auto result = m_pathClient->GetProperties().Metadata;
      EXPECT_EQ(metadata1, result);
      EXPECT_NO_THROW(m_pathClient->SetMetadata(metadata2));
      result = m_pathClient->GetProperties().Metadata;
      EXPECT_EQ(metadata2, result);
    }

    {
      // Create path with metadata works
      auto client1 = m_fileSystemClient->GetFileClient(LowercaseRandomString());
      auto client2 = m_fileSystemClient->GetFileClient(LowercaseRandomString());
      Files::DataLake::PathCreateOptions options1;
      Files::DataLake::PathCreateOptions options2;
      options1.Metadata = metadata1;
      options2.Metadata = metadata2;

      EXPECT_NO_THROW(client1.Create(options1));
      EXPECT_NO_THROW(client2.Create(options2));
      auto result = client1.GetProperties().Metadata;
      EXPECT_EQ(metadata1, result);
      result = client2.GetProperties().Metadata;
      EXPECT_EQ(metadata2, result);
    }
  }

  TEST_F(PathClientTest, PathProperties)
  {
    auto metadata1 = RandomMetadata();
    auto metadata2 = RandomMetadata();
    {
      // Get Metadata via properties works
      EXPECT_NO_THROW(m_pathClient->SetMetadata(metadata1));
      auto result = m_pathClient->GetProperties();
      EXPECT_EQ(metadata1, result.Metadata);
      EXPECT_NO_THROW(m_pathClient->SetMetadata(metadata2));
      result = m_pathClient->GetProperties();
      EXPECT_EQ(metadata2, result.Metadata);
    }

    {
      // Last modified Etag works.
      auto properties1 = m_pathClient->GetProperties();
      auto properties2 = m_pathClient->GetProperties();
      EXPECT_EQ(properties1.ETag, properties2.ETag);
      EXPECT_EQ(properties1.LastModified, properties2.LastModified);

      // This operation changes ETag/LastModified.
      EXPECT_NO_THROW(m_pathClient->SetMetadata(metadata1));

      auto properties3 = m_pathClient->GetProperties();
      EXPECT_NE(properties1.ETag, properties3.ETag);
      EXPECT_NE(properties1.LastModified, properties3.LastModified);
    }

    {
      // Http headers works.
      auto httpHeader = GetInterestingHttpHeaders();
      std::vector<Files::DataLake::PathClient> pathClient;
      for (int32_t i = 0; i < 2; ++i)
      {
        auto client = m_fileSystemClient->GetFileClient(LowercaseRandomString());
        Files::DataLake::PathCreateOptions options;
        options.HttpHeaders = httpHeader;
        EXPECT_NO_THROW(client.Create(options));
        pathClient.emplace_back(std::move(client));
      }
      for (const auto& client : pathClient)
      {
        auto result = client.GetProperties();
        EXPECT_EQ(httpHeader.CacheControl, result.HttpHeaders.CacheControl);
        EXPECT_EQ(httpHeader.ContentDisposition, result.HttpHeaders.ContentDisposition);
        EXPECT_EQ(httpHeader.ContentLanguage, result.HttpHeaders.ContentLanguage);
        EXPECT_EQ(httpHeader.ContentType, result.HttpHeaders.ContentType);
      }
    }
  }
}}} // namespace Azure::Storage::Test
