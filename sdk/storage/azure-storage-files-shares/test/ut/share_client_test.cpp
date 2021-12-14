// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "share_client_test.hpp"

#include <algorithm>
#include <chrono>

#include <azure/storage/common/crypt.hpp>

namespace Azure { namespace Storage { namespace Files { namespace Shares { namespace Models {

  bool operator==(
      const Azure::Storage::Files::Shares::Models::SignedIdentifier& lhs,
      const Azure::Storage::Files::Shares::Models::SignedIdentifier& rhs)
  {
    return lhs.Id == rhs.Id && lhs.Policy.StartsOn.HasValue() == rhs.Policy.StartsOn.HasValue()
        && (!lhs.Policy.StartsOn.HasValue()
            || lhs.Policy.StartsOn.Value() == rhs.Policy.StartsOn.Value())
        && lhs.Policy.ExpiresOn.HasValue() == rhs.Policy.ExpiresOn.HasValue()
        && (!lhs.Policy.ExpiresOn.HasValue()
            || lhs.Policy.ExpiresOn.Value() == rhs.Policy.ExpiresOn.Value())
        && lhs.Policy.Permission == rhs.Policy.Permission;
  }

}}}}} // namespace Azure::Storage::Files::Shares::Models

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

  void FileShareClientTest::TearDownTestSuite()
  {
    auto deleteOptions = Files::Shares::DeleteShareOptions();
    deleteOptions.DeleteSnapshots = true;
    m_shareClient->Delete(deleteOptions);
  }

  Files::Shares::Models::FileHttpHeaders FileShareClientTest::GetInterestingHttpHeaders()
  {
    static Files::Shares::Models::FileHttpHeaders result = []() {
      Files::Shares::Models::FileHttpHeaders ret;
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
      std::vector<Files::Shares::ShareClient> shareClients;
      for (int32_t i = 0; i < 5; ++i)
      {
        auto client = Files::Shares::ShareClient::CreateFromConnectionString(
            StandardStorageConnectionString(), LowercaseRandomString());
        EXPECT_NO_THROW(client.Create());
        shareClients.emplace_back(std::move(client));
      }
      for (const auto& client : shareClients)
      {
        EXPECT_NO_THROW(client.Delete());
      }
    }
    {
      // CreateIfNotExists & DeleteIfExists.
      {
        auto client = Files::Shares::ShareClient::CreateFromConnectionString(
            StandardStorageConnectionString(), LowercaseRandomString());
        EXPECT_NO_THROW(client.Create());
        EXPECT_NO_THROW(client.CreateIfNotExists());
        EXPECT_NO_THROW(client.Delete());
        EXPECT_NO_THROW(client.DeleteIfExists());
      }
      {
        auto client = Files::Shares::ShareClient::CreateFromConnectionString(
            StandardStorageConnectionString(), LowercaseRandomString());
        EXPECT_NO_THROW(client.CreateIfNotExists());
        EXPECT_THROW(client.Create(), StorageException);
        EXPECT_NO_THROW(client.DeleteIfExists());
      }
      {
        auto client = Files::Shares::ShareClient::CreateFromConnectionString(
            StandardStorageConnectionString(), LowercaseRandomString());
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
        auto client = Files::Shares::ShareClient::CreateFromConnectionString(
            StandardStorageConnectionString(), LowercaseRandomString());
        auto deleteResult = client.DeleteIfExists();
        EXPECT_FALSE(deleteResult.Value.Deleted);
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
      auto result = m_shareClient->GetProperties().Value.Metadata;
      EXPECT_EQ(metadata1, result);
      EXPECT_NO_THROW(m_shareClient->SetMetadata(metadata2));
      result = m_shareClient->GetProperties().Value.Metadata;
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
      auto result = client1.GetProperties().Value.Metadata;
      EXPECT_EQ(metadata1, result);
      result = client2.GetProperties().Value.Metadata;
      EXPECT_EQ(metadata2, result);
    }
  }

  TEST_F(FileShareClientTest, ShareProperties)
  {
    const int32_t quota32GB = 32;
    const int32_t quota64GB = 64;
    const int32_t quota5120GB = 5120;

    {
      // Set quota /Get properties works
      Files::Shares::SetSharePropertiesOptions options;
      options.ShareQuotaInGiB = quota32GB;
      EXPECT_NO_THROW(m_shareClient->SetProperties(options));
      auto result = m_shareClient->GetProperties();
      EXPECT_EQ(quota32GB, result.Value.Quota);
      options.ShareQuotaInGiB = quota64GB;
      EXPECT_NO_THROW(m_shareClient->SetProperties(options));
      result = m_shareClient->GetProperties();
      EXPECT_EQ(quota64GB, result.Value.Quota);
    }

    {
      // Create share with quota works
      auto client1 = Files::Shares::ShareClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(), LowercaseRandomString());
      auto client2 = Files::Shares::ShareClient::CreateFromConnectionString(
          AdlsGen2ConnectionString(), LowercaseRandomString());
      Files::Shares::CreateShareOptions options1;
      Files::Shares::CreateShareOptions options2;
      options1.ShareQuotaInGiB = quota32GB;
      options2.ShareQuotaInGiB = quota64GB;

      EXPECT_NO_THROW(client1.Create(options1));
      EXPECT_NO_THROW(client2.Create(options2));
      auto result = client1.GetProperties().Value.Quota;
      EXPECT_EQ(quota32GB, result);
      result = client2.GetProperties().Value.Quota;
      EXPECT_EQ(quota64GB, result);
    }

    {
      // Limit/negative cases:
      Files::Shares::SetSharePropertiesOptions options;
      options.ShareQuotaInGiB = quota5120GB;
      EXPECT_NO_THROW(m_shareClient->SetProperties(options));
      auto result = m_shareClient->GetProperties().Value.Quota;
      EXPECT_EQ(quota5120GB, result);
    }
  }

  TEST_F(FileShareClientTest, ShareAccessPolicy)
  {
    std::vector<Files::Shares::Models::SignedIdentifier> identifiers;
    for (unsigned i = 0; i < 3; ++i)
    {
      Files::Shares::Models::SignedIdentifier identifier;
      identifier.Id = RandomString(64);
      identifier.Policy.StartsOn = std::chrono::system_clock::now() - std::chrono::minutes(10);
      identifier.Policy.ExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(100);
      identifier.Policy.Permission = "r";
      identifiers.emplace_back(identifier);
    }

    auto ret = m_shareClient->SetAccessPolicy(identifiers);
    EXPECT_TRUE(IsValidTime(ret.Value.LastModified));

    auto ret2 = m_shareClient->GetAccessPolicy();
    EXPECT_EQ(ret2.Value.SignedIdentifiers, identifiers);
  }

  TEST_F(FileShareClientTest, ShareAccessPolicyNullable)
  {
    std::vector<Files::Shares::Models::SignedIdentifier> identifiers;
    {
      Files::Shares::Models::SignedIdentifier identifier;
      identifier.Id = RandomString(64);
      identifier.Policy.Permission = "r";
      identifiers.emplace_back(identifier);
    }
    {
      Files::Shares::Models::SignedIdentifier identifier;
      identifier.Id = RandomString(64);
      identifier.Policy.StartsOn = std::chrono::system_clock::now() - std::chrono::minutes(10);
      identifier.Policy.Permission = "r";
      identifiers.emplace_back(identifier);
    }
    {
      Files::Shares::Models::SignedIdentifier identifier;
      identifier.Id = RandomString(64);
      identifier.Policy.ExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(100);
      identifier.Policy.Permission = "r";
      identifiers.emplace_back(identifier);
    }

    auto ret = m_shareClient->SetAccessPolicy(identifiers);
    EXPECT_TRUE(IsValidTime(ret.Value.LastModified));

    auto ret2 = m_shareClient->GetAccessPolicy();
    EXPECT_EQ(ret2.Value.SignedIdentifiers, identifiers);
  }

  TEST_F(FileShareClientTest, SharePermissions)
  {
    std::string permission = "O:S-1-5-21-2127521184-1604012920-1887927527-21560751G:S-1-5-21-"
                             "2127521184-1604012920-1887927527-513D:AI(A;;FA;;;SY)(A;;FA;;;BA)(A;;"
                             "0x1200a9;;;S-1-5-21-397955417-626881126-188441444-3053964)";

    std::string expectedPermission = permission + "S:NO_ACCESS_CONTROL";

    auto ret = m_shareClient->CreatePermission(permission);
    EXPECT_FALSE(ret.Value.FilePermissionKey.empty());

    auto ret2 = m_shareClient->GetPermission(ret.Value.FilePermissionKey);
    EXPECT_EQ(expectedPermission, ret2.Value);
  }

  // TEST_F(FileShareClientTest, Lease)
  //{
  //  std::string leaseId1 = CreateUniqueLeaseId();
  //  std::chrono::seconds leaseDuration(20);
  //  auto leaseClient = Files::Shares::ShareLeaseClient(*m_shareClient, leaseId1);

  //  auto aLease = *leaseClient.Acquire(leaseDuration);
  //  EXPECT_FALSE(aLease.ETag.empty());
  //  EXPECT_NE(Azure::DateTime(), aLease.LastModified);
  //  EXPECT_EQ(aLease.LeaseId, leaseId1);
  //  aLease = *leaseClient.Acquire(leaseDuration);
  //  EXPECT_FALSE(aLease.ETag.empty());
  //  EXPECT_NE(Azure::DateTime(), aLease.LastModified);
  //  EXPECT_EQ(aLease.LeaseId, leaseId1);

  //  auto properties = *m_shareClient->GetProperties();
  //  EXPECT_EQ(properties.LeaseState.Value(), Files::Shares::Models::LeaseStateType::Leased);
  //  EXPECT_EQ(properties.LeaseStatus.Value(), Files::Shares::Models::LeaseStatusType::Locked);
  //  EXPECT_EQ(Files::Shares::Models::LeaseDurationType::Fixed,
  //  properties.LeaseDuration.Value());

  //  auto rLease = *leaseClient.Renew();
  //  EXPECT_FALSE(rLease.ETag.empty());
  //  EXPECT_NE(Azure::DateTime(), rLease.LastModified);
  //  EXPECT_EQ(rLease.LeaseId, leaseId1);

  //  std::string leaseId2 = CreateUniqueLeaseId();
  //  EXPECT_NE(leaseId1, leaseId2);
  //  auto cLease = *leaseClient.Change(leaseId2);
  //  EXPECT_FALSE(cLease.ETag.empty());
  //  EXPECT_NE(Azure::DateTime(), cLease.LastModified);
  //  EXPECT_EQ(cLease.LeaseId, leaseId2);

  //  auto relLease = *leaseClient.Release();
  //  EXPECT_FALSE(relLease.ETag.empty());
  //  EXPECT_NE(Azure::DateTime(), relLease.LastModified);

  //  leaseClient = Files::Shares::ShareLeaseClient(*m_shareClient, CreateUniqueLeaseId());
  //  aLease = *leaseClient.Acquire(Files::Shares::ShareLeaseClient::InfiniteLeaseDuration);
  //  properties = *m_shareClient->GetProperties();
  //  EXPECT_EQ(
  //      Files::Shares::Models::LeaseDurationType::Infinite, properties.LeaseDuration.Value());
  //  auto brokenLease = *leaseClient.Break();
  //  EXPECT_FALSE(brokenLease.ETag.empty());
  //  EXPECT_NE(Azure::DateTime(), brokenLease.LastModified);
  //  EXPECT_EQ(brokenLease.LeaseTime, 0);

  //  Files::Shares::BreakLeaseOptions options;
  //  options.BreakPeriod = 0;
  //  leaseClient.Break(options);
  //}

  // TEST_F(FileShareClientTest, SnapshotLease)
  //{
  //  std::string leaseId1 = CreateUniqueLeaseId();
  //  std::chrono::seconds leaseDuration(20);
  //  auto snapshotResult = m_shareClient->CreateSnapshot();
  //  auto shareSnapshot = m_shareClient->WithSnapshot(snapshotResult.Value.Snapshot);
  //  auto shareSnapshotLeaseClient = Files::Shares::ShareLeaseClient(shareSnapshot, leaseId1);
  //  auto aLease = *shareSnapshotLeaseClient.Acquire(leaseDuration);
  //  EXPECT_FALSE(aLease.ETag.empty());
  //  EXPECT_NE(Azure::DateTime(), aLease.LastModified);
  //  EXPECT_EQ(aLease.LeaseId, leaseId1);
  //  aLease = *shareSnapshotLeaseClient.Acquire(leaseDuration);
  //  EXPECT_FALSE(aLease.ETag.empty());
  //  EXPECT_NE(Azure::DateTime(), aLease.LastModified);
  //  EXPECT_EQ(aLease.LeaseId, leaseId1);

  //  auto properties = *shareSnapshot.GetProperties();
  //  EXPECT_EQ(properties.LeaseState.Value(), Files::Shares::Models::LeaseStateType::Leased);
  //  EXPECT_EQ(properties.LeaseStatus.Value(), Files::Shares::Models::LeaseStatusType::Locked);
  //  EXPECT_EQ(Files::Shares::Models::LeaseDurationType::Fixed,
  //  properties.LeaseDuration.Value());

  //  auto rLease = *shareSnapshotLeaseClient.Renew();
  //  EXPECT_FALSE(rLease.ETag.empty());
  //  EXPECT_NE(Azure::DateTime(), rLease.LastModified);
  //  EXPECT_EQ(rLease.LeaseId, leaseId1);

  //  std::string leaseId2 = CreateUniqueLeaseId();
  //  EXPECT_NE(leaseId1, leaseId2);
  //  auto cLease = *shareSnapshotLeaseClient.Change(leaseId2);
  //  EXPECT_FALSE(cLease.ETag.empty());
  //  EXPECT_NE(Azure::DateTime(), cLease.LastModified);
  //  EXPECT_EQ(cLease.LeaseId, leaseId2);

  //  auto relLease = *shareSnapshotLeaseClient.Release();
  //  EXPECT_FALSE(relLease.ETag.empty());
  //  EXPECT_NE(Azure::DateTime(), relLease.LastModified);

  //  shareSnapshotLeaseClient
  //      = Files::Shares::ShareLeaseClient(shareSnapshot, CreateUniqueLeaseId());
  //  aLease
  //      =
  //      *shareSnapshotLeaseClient.Acquire(Files::Shares::ShareLeaseClient::InfiniteLeaseDuration);
  //  properties = *shareSnapshot.GetProperties();
  //  EXPECT_EQ(
  //      Files::Shares::Models::LeaseDurationType::Infinite, properties.LeaseDuration.Value());
  //  auto brokenLease = *shareSnapshotLeaseClient.Break();
  //  EXPECT_FALSE(brokenLease.ETag.empty());
  //  EXPECT_NE(Azure::DateTime(), brokenLease.LastModified);
  //  EXPECT_EQ(brokenLease.LeaseTime, 0);

  //  Files::Shares::BreakLeaseOptions options;
  //  options.BreakPeriod = 0;
  //  shareSnapshotLeaseClient.Break(options);

  //  EXPECT_THROW(m_shareClient->Delete(), StorageException);
  //}

  TEST_F(FileShareClientTest, UnencodedDirectoryFileNameWorks)
  {
    const std::string non_ascii_word = "\xE6\xB5\x8B\xE8\xAF\x95";
    const std::string encoded_non_ascii_word = "%E6%B5%8B%E8%AF%95";
    std::string baseName = "a b c !@#$%^&(,.;'[]{}`~) def" + non_ascii_word;

    {
      std::string directoryName = baseName + RandomString();
      auto directoryClient
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(directoryName);
      EXPECT_NO_THROW(directoryClient.Create());
      auto directoryUrl = directoryClient.GetUrl();
      EXPECT_EQ(
          directoryUrl, m_shareClient->GetUrl() + "/" + _internal::UrlEncodePath(directoryName));
    }
    {
      std::string fileName = baseName + RandomString();
      auto fileClient = m_shareClient->GetRootDirectoryClient().GetFileClient(fileName);
      EXPECT_NO_THROW(fileClient.Create(1024));
      auto fileUrl = fileClient.GetUrl();
      EXPECT_EQ(fileUrl, m_shareClient->GetUrl() + "/" + _internal::UrlEncodePath(fileName));
    }
  }

  TEST_F(FileShareClientTest, ShareTierRelated)
  {
    // Create/Get properties works
    std::unordered_map<std::string, Files::Shares::ShareClient> shareClients;
    std::string prefix = LowercaseRandomString(5);
    Files::Shares::Models::ShareProperties properties;
    {
      auto shareName = prefix + LowercaseRandomString(5);
      auto shareClient = Files::Shares::ShareClient::CreateFromConnectionString(
          StandardStorageConnectionString(), shareName);
      auto options = Files::Shares::CreateShareOptions();
      options.AccessTier = Files::Shares::Models::AccessTier::TransactionOptimized;
      EXPECT_NO_THROW(shareClient.Create(options));
      EXPECT_NO_THROW(properties = shareClient.GetProperties().Value);
      EXPECT_EQ(
          Files::Shares::Models::AccessTier::TransactionOptimized, properties.AccessTier.Value());
      EXPECT_FALSE(properties.AccessTierTransitionState.HasValue());
      EXPECT_TRUE(IsValidTime(properties.AccessTierChangedOn.Value()));
      shareClients.emplace(std::move(shareName), std::move(shareClient));
    }
    {
      auto shareName = prefix + LowercaseRandomString(5);
      auto shareClient = Files::Shares::ShareClient::CreateFromConnectionString(
          StandardStorageConnectionString(), shareName);
      auto options = Files::Shares::CreateShareOptions();
      options.AccessTier = Files::Shares::Models::AccessTier::Hot;
      EXPECT_NO_THROW(shareClient.Create(options));
      EXPECT_NO_THROW(properties = shareClient.GetProperties().Value);
      EXPECT_EQ(Files::Shares::Models::AccessTier::Hot, properties.AccessTier.Value());
      EXPECT_FALSE(properties.AccessTierTransitionState.HasValue());
      EXPECT_EQ(properties.LastModified, properties.AccessTierChangedOn.Value());
      shareClients.emplace(std::move(shareName), std::move(shareClient));
    }
    {
      auto shareName = prefix + LowercaseRandomString(5);
      auto shareClient = Files::Shares::ShareClient::CreateFromConnectionString(
          StandardStorageConnectionString(), shareName);
      auto options = Files::Shares::CreateShareOptions();
      options.AccessTier = Files::Shares::Models::AccessTier::Cool;
      EXPECT_NO_THROW(shareClient.Create(options));
      EXPECT_NO_THROW(properties = shareClient.GetProperties().Value);
      EXPECT_EQ(Files::Shares::Models::AccessTier::Cool, properties.AccessTier.Value());
      EXPECT_FALSE(properties.AccessTierTransitionState.HasValue());
      EXPECT_EQ(properties.LastModified, properties.AccessTierChangedOn.Value());
      shareClients.emplace(std::move(shareName), std::move(shareClient));
    }

    // Set properties works
    {
      auto shareClient = Files::Shares::ShareClient::CreateFromConnectionString(
          StandardStorageConnectionString(), LowercaseRandomString());
      auto options = Files::Shares::CreateShareOptions();
      options.AccessTier = Files::Shares::Models::AccessTier::Cool;
      EXPECT_NO_THROW(shareClient.Create(options));
      EXPECT_EQ(
          Files::Shares::Models::AccessTier::Cool,
          shareClient.GetProperties().Value.AccessTier.Value());

      auto setPropertiesOptions = Files::Shares::SetSharePropertiesOptions();
      setPropertiesOptions.AccessTier = Files::Shares::Models::AccessTier::Hot;
      EXPECT_NO_THROW(shareClient.SetProperties(setPropertiesOptions));
      properties = shareClient.GetProperties().Value;
      if (properties.AccessTierTransitionState.HasValue())
      {
        EXPECT_EQ(Files::Shares::Models::AccessTier::Cool, properties.AccessTier.Value());
      }
      else
      {
        EXPECT_EQ(Files::Shares::Models::AccessTier::Hot, properties.AccessTier.Value());
      }
      EXPECT_EQ(properties.LastModified, properties.AccessTierChangedOn.Value());
    }

    // List shares works.
    Files::Shares::ListSharesOptions listOptions;
    listOptions.Prefix = prefix;
    std::vector<Files::Shares::Models::ShareItem> shareItems;
    for (auto pageResult = Files::Shares::ShareServiceClient::CreateFromConnectionString(
                               StandardStorageConnectionString())
                               .ListShares(listOptions);
         pageResult.HasPage();
         pageResult.MoveToNextPage())
    {
      shareItems.insert(shareItems.end(), pageResult.Shares.begin(), pageResult.Shares.end());
    }
    EXPECT_EQ(3U, shareItems.size());
    for (const auto& shareItem : shareItems)
    {
      EXPECT_NE(shareClients.find(shareItem.Name), shareClients.end());
      properties = shareClients.at(shareItem.Name).GetProperties().Value;
      EXPECT_EQ(true, shareItem.Details.AccessTier.HasValue() && properties.AccessTier.HasValue());
      EXPECT_EQ(shareItem.Details.AccessTier.Value(), properties.AccessTier.Value());
      EXPECT_EQ(
          true,
          shareItem.Details.AccessTierChangedOn.HasValue()
              && properties.AccessTierChangedOn.HasValue());
      EXPECT_EQ(
          shareItem.Details.AccessTierChangedOn.Value(), properties.AccessTierChangedOn.Value());
      EXPECT_EQ(
          false,
          shareItem.Details.AccessTierTransitionState.HasValue()
              || properties.AccessTierTransitionState.HasValue());
    }
  }

  TEST_F(FileShareClientTest, PremiumShare)
  {
    auto shareName = LowercaseRandomString();
    auto shareClient = Files::Shares::ShareClient::CreateFromConnectionString(
        PremiumFileConnectionString(), shareName);
    EXPECT_NO_THROW(shareClient.Create());
    Files::Shares::Models::ShareProperties properties;
    EXPECT_NO_THROW(properties = shareClient.GetProperties().Value);
    EXPECT_EQ(Files::Shares::Models::AccessTier::Premium, properties.AccessTier.Value());
    EXPECT_FALSE(properties.AccessTierTransitionState.HasValue());
    EXPECT_FALSE(properties.AccessTierChangedOn.HasValue());

    Files::Shares::ListSharesOptions listOptions;
    listOptions.Prefix = shareName;
    std::vector<Files::Shares::Models::ShareItem> shareItems;
    for (auto pageResult = Files::Shares::ShareServiceClient::CreateFromConnectionString(
                               PremiumFileConnectionString())
                               .ListShares(listOptions);
         pageResult.HasPage();
         pageResult.MoveToNextPage())
    {
      shareItems.insert(shareItems.end(), pageResult.Shares.begin(), pageResult.Shares.end());
    }
    EXPECT_EQ(1U, shareItems.size());
    EXPECT_EQ(Files::Shares::Models::AccessTier::Premium, shareItems[0].Details.AccessTier.Value());
    EXPECT_FALSE(shareItems[0].Details.AccessTierTransitionState.HasValue());
    EXPECT_FALSE(shareItems[0].Details.AccessTierChangedOn.HasValue());

    auto setPropertiesOptions = Files::Shares::SetSharePropertiesOptions();
    setPropertiesOptions.AccessTier = Files::Shares::Models::AccessTier::Hot;
    EXPECT_THROW(shareClient.SetProperties(setPropertiesOptions), StorageException);
    setPropertiesOptions.AccessTier = Files::Shares::Models::AccessTier::Cool;
    EXPECT_THROW(shareClient.SetProperties(setPropertiesOptions), StorageException);
    setPropertiesOptions.AccessTier = Files::Shares::Models::AccessTier::TransactionOptimized;
    EXPECT_THROW(shareClient.SetProperties(setPropertiesOptions), StorageException);
    setPropertiesOptions.AccessTier = Files::Shares::Models::AccessTier::Premium;
    EXPECT_NO_THROW(shareClient.SetProperties(setPropertiesOptions));
    EXPECT_EQ(Files::Shares::Models::AccessTier::Premium, properties.AccessTier.Value());
    EXPECT_FALSE(properties.AccessTierTransitionState.HasValue());
    EXPECT_FALSE(properties.AccessTierChangedOn.HasValue());
  }
}}} // namespace Azure::Storage::Test
