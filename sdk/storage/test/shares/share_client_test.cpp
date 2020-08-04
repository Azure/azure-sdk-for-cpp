// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "share_client_test.hpp"

#include <algorithm>

namespace Azure { namespace Storage { namespace Test {

  std::shared_ptr<Files::Shares::ShareClient> FileShareClientTest::m_shareClient;
  std::string FileShareClientTest::m_fileSystemName;

  void FileShareClientTest::SetUpTestSuite()
  {
    m_fileSystemName = LowercaseRandomString();
    m_shareClient = std::make_shared<Files::Shares::ShareClient>(
        Files::Shares::ShareClient::CreateFromConnectionString(
            StandardStorageConnectionString(), m_fileSystemName));
    m_shareClient->Create();
  }

  void FileShareClientTest::TearDownTestSuite() { m_shareClient->Delete(); }

  Files::Shares::FileShareHttpHeaders FileShareClientTest::GetInterestingHttpHeaders()
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

  TEST_F(FileShareClientTest, CreateDeleteFileSystems)
  {
    {
      // Normal create/delete.
      std::vector<Files::Shares::ShareClient> fileSystemClient;
      for (int32_t i = 0; i < 5; ++i)
      {
        auto client = Files::Shares::ShareClient::CreateFromConnectionString(
            StandardStorageConnectionString(), LowercaseRandomString());
        EXPECT_NO_THROW(client.Create());
        fileSystemClient.emplace_back(std::move(client));
      }
      for (const auto& client : fileSystemClient)
      {
        EXPECT_NO_THROW(client.Delete());
      }
    }
  }

}}} // namespace Azure::Storage::Test
