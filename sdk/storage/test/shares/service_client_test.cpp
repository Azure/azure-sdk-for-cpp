// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "service_client_test.hpp"

#include <algorithm>

namespace Azure { namespace Storage { namespace Test {

  const size_t c_SHARE_TEST_SIZE = 5;

  std::shared_ptr<Files::Shares::ServiceClient>
      FileShareServiceClientTest::m_fileShareServiceClient;
  std::vector<std::string> FileShareServiceClientTest::m_shareNameSetA;
  std::vector<std::string> FileShareServiceClientTest::m_shareNameSetB;
  std::string FileShareServiceClientTest::m_sharePrefixA;
  std::string FileShareServiceClientTest::m_sharePrefixB;

  void FileShareServiceClientTest::SetUpTestSuite()
  {
    m_fileShareServiceClient = std::make_shared<Files::Shares::ServiceClient>(
        Files::Shares::ServiceClient::CreateFromConnectionString(
            StandardStorageConnectionString()));
    m_sharePrefixA = LowercaseRandomString(10);
    m_sharePrefixB = LowercaseRandomString(10);
    for (size_t i = 0; i < c_SHARE_TEST_SIZE; ++i)
    {
      {
        auto name = m_sharePrefixA + LowercaseRandomString(10);
        m_fileShareServiceClient->GetShareClient(name).Create();
        m_shareNameSetA.emplace_back(std::move(name));
      }
      {
        auto name = m_sharePrefixB + LowercaseRandomString(10);
        m_fileShareServiceClient->GetShareClient(name).Create();
        m_shareNameSetB.emplace_back(std::move(name));
      }
    }
  }

  void FileShareServiceClientTest::TearDownTestSuite()
  {
    for (const auto& name : m_shareNameSetA)
    {
      m_fileShareServiceClient->GetShareClient(name).Delete();
    }
    for (const auto& name : m_shareNameSetB)
    {
      m_fileShareServiceClient->GetShareClient(name).Delete();
    }
  }

  std::vector<Files::Shares::ShareItem> FileShareServiceClientTest::ListAllShares(
      const std::string& prefix)
  {
    std::vector<Files::Shares::ShareItem> result;
    std::string continuation;
    Files::Shares::ListSharesOptions options;
    if (!prefix.empty())
    {
      options.Prefix = prefix;
    }
    do
    {
      auto response = m_fileShareServiceClient->ListSharesSegment(options);
      result.insert(result.end(), response->ShareItems.begin(), response->ShareItems.end());
      continuation = response->NextMarker;
      options.Marker = continuation;
    } while (!continuation.empty());
    return result;
  }

  TEST_F(FileShareServiceClientTest, ListShares)
  {
    {
      // Normal list without prefix.
      auto result = ListAllShares();
      for (const auto& name : m_shareNameSetA)
      {
        auto iter = std::find_if(
            result.begin(), result.end(), [&name](const Files::Shares::ShareItem& share) {
              return share.Name == name;
            });
        EXPECT_EQ(iter->Name.substr(0U, m_sharePrefixA.size()), m_sharePrefixA);
        EXPECT_NE(result.end(), iter);
      }
      for (const auto& name : m_shareNameSetB)
      {
        auto iter = std::find_if(
            result.begin(), result.end(), [&name](const Files::Shares::ShareItem& share) {
              return share.Name == name;
            });
        EXPECT_EQ(iter->Name.substr(0U, m_sharePrefixB.size()), m_sharePrefixB);
        EXPECT_NE(result.end(), iter);
      }
    }
    {
      // List prefix.
      auto result = ListAllShares(m_sharePrefixA);
      for (const auto& name : m_shareNameSetA)
      {
        auto iter = std::find_if(
            result.begin(), result.end(), [&name](const Files::Shares::ShareItem& share) {
              return share.Name == name;
            });
        EXPECT_EQ(iter->Name.substr(0U, m_sharePrefixA.size()), m_sharePrefixA);
        EXPECT_NE(result.end(), iter);
      }
      for (const auto& name : m_shareNameSetB)
      {
        auto iter = std::find_if(
            result.begin(), result.end(), [&name](const Files::Shares::ShareItem& share) {
              return share.Name == name;
            });
        EXPECT_EQ(result.end(), iter);
      }
    }
    {
      // List max result
      Files::Shares::ListSharesOptions options;
      options.MaxResults = 2;
      auto response = m_fileShareServiceClient->ListSharesSegment(options);
      EXPECT_LE(2U, response->ShareItems.size());
    }
  }

}}} // namespace Azure::Storage::Test
