// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "share_client_test.hpp"

#include <algorithm>

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  bool operator==(
      const Azure::Storage::Files::Shares::SignedIdentifier& lhs,
      const Azure::Storage::Files::Shares::SignedIdentifier& rhs)
  {
    return lhs.Id == rhs.Id && lhs.Policy.Start == rhs.Policy.Start
        && lhs.Policy.Expiry == rhs.Policy.Expiry && lhs.Policy.Permission == rhs.Policy.Permission;
  }

}}}} // namespace Azure::Storage::Files::Shares

namespace Azure { namespace Storage { namespace Test {

  std::shared_ptr<Files::Shares::ShareClient> FileShareClientTest::m_shareClient;
  std::string FileShareClientTest::m_shareName;

  void FileShareClientTest::SetUpTestSuite()
  {
    m_shareName = LowercaseRandomString();
    m_shareClient = std::make_shared<Files::Shares::ShareClient>(
        Files::Shares::ShareClient::CreateFromConnectionString(
            StandardStorageConnectionString(), m_shareName));
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

  TEST_F(FileShareClientTest, CreateDeleteShares)
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

  TEST_F(FileShareClientTest, ShareMetadata)
  {
    auto metadata1 = RandomMetadata();
    auto metadata2 = RandomMetadata();
    {
      // Set/Get Metadata works
      EXPECT_NO_THROW(m_shareClient->SetMetadata(metadata1));
      auto result = m_shareClient->GetProperties()->Metadata;
      EXPECT_EQ(metadata1, result);
      EXPECT_NO_THROW(m_shareClient->SetMetadata(metadata2));
      result = m_shareClient->GetProperties()->Metadata;
      EXPECT_EQ(metadata2, result);
    }

    {
      // Create share with metadata works
      auto client1 = Files::Shares::ShareClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(), LowercaseRandomString());
      auto client2 = Files::Shares::ShareClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(), LowercaseRandomString());
      Files::Shares::CreateShareOptions options1;
      Files::Shares::CreateShareOptions options2;
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

  TEST_F(FileShareClientTest, ShareQuota)
  {
    const int32_t quota32GB = 32;
    const int32_t quota64GB = 64;
    const int32_t quota5120GB = 5120;

    {
      // Set quota /Get properties works
      EXPECT_NO_THROW(m_shareClient->SetQuota(quota32GB));
      auto result = m_shareClient->GetProperties();
      EXPECT_EQ(quota32GB, result->Quota);
      EXPECT_NO_THROW(m_shareClient->SetQuota(quota64GB));
      result = m_shareClient->GetProperties();
      EXPECT_EQ(quota64GB, result->Quota);
    }

    {
      // Create file system with quota works
      auto client1 = Files::Shares::ShareClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(), LowercaseRandomString());
      auto client2 = Files::Shares::ShareClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(), LowercaseRandomString());
      Files::Shares::CreateShareOptions options1;
      Files::Shares::CreateShareOptions options2;
      options1.ShareQuota = quota32GB;
      options2.ShareQuota = quota64GB;

      EXPECT_NO_THROW(client1.Create(options1));
      EXPECT_NO_THROW(client2.Create(options2));
      auto result = client1.GetProperties()->Quota;
      EXPECT_EQ(quota32GB, result);
      result = client2.GetProperties()->Quota;
      EXPECT_EQ(quota64GB, result);
    }

    {
      // Limit/negative cases:
      EXPECT_NO_THROW(m_shareClient->SetQuota(quota5120GB));
      auto result = m_shareClient->GetProperties()->Quota;
      EXPECT_EQ(quota5120GB, result);
    }
  }

  TEST_F(FileShareClientTest, ShareAccessPolicy)
  {
    std::vector<Files::Shares::SignedIdentifier> identifiers;
    for (unsigned i = 0; i < 3; ++i)
    {
      Files::Shares::SignedIdentifier identifier;
      identifier.Id = RandomString(64);
      identifier.Policy.Start
          = ToIso8601(std::chrono::system_clock::now() - std::chrono::minutes(10), 7);
      identifier.Policy.Expiry
          = ToIso8601(std::chrono::system_clock::now() - std::chrono::minutes(100), 7);
      identifier.Policy.Permission = "r";
      identifiers.emplace_back(identifier);
    }

    auto ret = m_shareClient->SetAccessPolicy(identifiers);
    EXPECT_FALSE(ret->ETag.empty());
    EXPECT_FALSE(ret->LastModified.empty());

    auto ret2 = m_shareClient->GetAccessPolicy();
    EXPECT_EQ(ret2->ETag, ret->ETag);
    EXPECT_EQ(ret2->LastModified, ret->LastModified);
    EXPECT_EQ(ret2->SignedIdentifiers, identifiers);
  }

  TEST_F(FileShareClientTest, SharePermissions)
  {
    std::string permission = "O:S-1-5-21-2127521184-1604012920-1887927527-21560751G:S-1-5-21-"
                             "2127521184-1604012920-1887927527-513D:AI(A;;FA;;;SY)(A;;FA;;;BA)(A;;"
                             "0x1200a9;;;S-1-5-21-397955417-626881126-188441444-3053964)";

    std::string expectedPermission = permission + "S:NO_ACCESS_CONTROL";

    auto ret = m_shareClient->CreatePermission(permission);
    EXPECT_FALSE(ret->FilePermissionKey.empty());

    auto ret2 = m_shareClient->GetPermission(ret->FilePermissionKey);
    EXPECT_EQ(expectedPermission, ret2->Permission);
  }
}}} // namespace Azure::Storage::Test
