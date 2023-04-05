// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "share_client_test.hpp"

#include <algorithm>
#include <chrono>
#include <unordered_map>

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

  void FileShareClientTest::SetUp()
  {
    FileShareServiceClientTest::SetUp();
    if (shouldSkipTest())
    {
      return;
    }
    m_shareName = GetLowercaseIdentifier();
    m_shareClient = std::make_shared<Files::Shares::ShareClient>(
        m_shareServiceClient->GetShareClient(m_shareName));
    while (true)
    {
      try
      {
        m_shareClient->CreateIfNotExists();
        break;
      }
      catch (StorageException& e)
      {
        if (e.ErrorCode != "ShareBeingDeleted")
        {
          throw;
        }
        SUCCEED() << "Share is being deleted. Will try again after 3 seconds.";
        std::this_thread::sleep_for(std::chrono::seconds(3));
      }
    }

    m_resourceCleanupFunctions.push_back([shareClient = *m_shareClient]() {
      Files::Shares::DeleteShareOptions options;
      options.DeleteSnapshots = true;
      shareClient.DeleteIfExists(options);
    });
  }

  Files::Shares::ShareClient FileShareClientTest::GetShareClientForTest(
      const std::string& shareName,
      Files::Shares::ShareClientOptions clientOptions)
  {
    InitStorageClientOptions(clientOptions);
    auto shareClient = Files::Shares::ShareClient::CreateFromConnectionString(
        StandardStorageConnectionString(), shareName, clientOptions);
    m_resourceCleanupFunctions.push_back([shareClient]() {
      Files::Shares::DeleteShareOptions options;
      options.DeleteSnapshots = true;
      shareClient.DeleteIfExists(options);
    });

    return shareClient;
  }

  Files::Shares::ShareClient FileShareClientTest::GetPremiumShareClientForTest(
      const std::string& shareName,
      Files::Shares::ShareClientOptions clientOptions)
  {
    InitStorageClientOptions(clientOptions);
    auto shareClient = Files::Shares::ShareClient::CreateFromConnectionString(
        PremiumFileConnectionString(), shareName, clientOptions);
    m_resourceCleanupFunctions.push_back([shareClient]() { shareClient.DeleteIfExists(); });

    return shareClient;
  }

  TEST_F(FileShareClientTest, CreateDeleteShares)
  {
    {
      // Normal create/delete.
      auto shareClient = GetShareClientForTest(LowercaseRandomString());
      EXPECT_NO_THROW(shareClient.Create());
      EXPECT_NO_THROW(shareClient.Delete());
    }
    {
      // CreateIfNotExists & DeleteIfExists.
      {
        auto shareClient = GetShareClientForTest(LowercaseRandomString());
        EXPECT_NO_THROW(shareClient.Create());
        EXPECT_NO_THROW(shareClient.CreateIfNotExists());
        EXPECT_NO_THROW(shareClient.Delete());
        EXPECT_NO_THROW(shareClient.DeleteIfExists());
      }
      {
        auto shareClient = GetShareClientForTest(LowercaseRandomString());
        EXPECT_NO_THROW(shareClient.CreateIfNotExists());
        EXPECT_THROW(shareClient.Create(), StorageException);
        EXPECT_NO_THROW(shareClient.DeleteIfExists());
      }
      {
        auto shareClient = GetShareClientForTest(LowercaseRandomString());
        auto created = shareClient.Create().Value.Created;
        EXPECT_TRUE(created);
        auto createResult = shareClient.CreateIfNotExists();
        EXPECT_FALSE(createResult.Value.Created);
        EXPECT_FALSE(createResult.Value.ETag.HasValue());
        EXPECT_EQ(DateTime(), createResult.Value.LastModified);
        auto deleted = shareClient.Delete().Value.Deleted;
        EXPECT_TRUE(deleted);
      }
      {
        auto shareClient = GetShareClientForTest(LowercaseRandomString());
        auto deleteResult = shareClient.DeleteIfExists();
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
      auto client1 = GetShareClientForTest(LowercaseRandomString());
      auto client2 = GetShareClientForTest(LowercaseRandomString());
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
      client1.DeleteIfExists();
      client2.DeleteIfExists();
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

      auto client1 = GetShareClientForTest(LowercaseRandomString());
      auto client2 = GetShareClientForTest(LowercaseRandomString());
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
      client1.DeleteIfExists();
      client2.DeleteIfExists();
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
    if (m_testContext.IsLiveMode())
    {
      EXPECT_EQ(ret2.Value.SignedIdentifiers, identifiers);
    }
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
    if (m_testContext.IsLiveMode())
    {
      EXPECT_EQ(ret2.Value.SignedIdentifiers, identifiers);
    }
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

  TEST_F(FileShareClientTest, Lease)
  {
    EXPECT_NE(
        Files::Shares::ShareLeaseClient::CreateUniqueLeaseId(),
        Files::Shares::ShareLeaseClient::CreateUniqueLeaseId());
    {
      std::string leaseId1 = RandomUUID();
      auto lastModified = m_shareClient->GetProperties().Value.LastModified;
      std::chrono::seconds leaseDuration(20);
      Files::Shares::ShareLeaseClient leaseClient(*m_shareClient, leaseId1);
      auto aLease = leaseClient.Acquire(leaseDuration).Value;
      EXPECT_TRUE(aLease.ETag.HasValue());
      EXPECT_GE(aLease.LastModified, lastModified);
      EXPECT_EQ(aLease.LeaseId, leaseId1);
      lastModified = m_shareClient->GetProperties().Value.LastModified;
      aLease = leaseClient.Acquire(Files::Shares::ShareLeaseClient::InfiniteLeaseDuration).Value;
      EXPECT_TRUE(aLease.ETag.HasValue());
      EXPECT_GE(aLease.LastModified, lastModified);
      EXPECT_EQ(aLease.LeaseId, leaseId1);

      auto properties = m_shareClient->GetProperties().Value;
      EXPECT_EQ(properties.LeaseState.Value(), Files::Shares::Models::LeaseState::Leased);
      EXPECT_EQ(properties.LeaseStatus.Value(), Files::Shares::Models::LeaseStatus::Locked);

      lastModified = m_shareClient->GetProperties().Value.LastModified;
      auto rLease = leaseClient.Renew().Value;
      EXPECT_TRUE(aLease.ETag.HasValue());
      EXPECT_GE(aLease.LastModified, lastModified);
      EXPECT_EQ(rLease.LeaseId, leaseId1);

      lastModified = m_shareClient->GetProperties().Value.LastModified;
      std::string leaseId2 = RandomUUID();
      EXPECT_NE(leaseId1, leaseId2);
      auto cLease = leaseClient.Change(leaseId2).Value;
      EXPECT_TRUE(cLease.ETag.HasValue());
      EXPECT_GE(cLease.LastModified, lastModified);
      EXPECT_EQ(cLease.LeaseId, leaseId2);
      EXPECT_EQ(leaseClient.GetLeaseId(), leaseId2);

      lastModified = m_shareClient->GetProperties().Value.LastModified;
      auto relLease = leaseClient.Release().Value;
      EXPECT_TRUE(relLease.ETag.HasValue());
      EXPECT_GE(relLease.LastModified, lastModified);
    }

    {
      Files::Shares::ShareLeaseClient leaseClient(*m_shareClient, RandomUUID());
      auto aLease
          = leaseClient.Acquire(Files::Shares::ShareLeaseClient::InfiniteLeaseDuration).Value;
      auto properties = m_shareClient->GetProperties().Value;
      EXPECT_EQ(
          Files::Shares::Models::LeaseDurationType::Infinite, properties.LeaseDuration.Value());
      auto brokenLease = leaseClient.Break().Value;
      EXPECT_TRUE(brokenLease.ETag.HasValue());
      EXPECT_GE(brokenLease.LastModified, properties.LastModified);
    }
  }

  TEST_F(FileShareClientTest, SnapshotLease)
  {
    auto snapshotResult = m_shareClient->CreateSnapshot();
    auto shareSnapshot = m_shareClient->WithSnapshot(snapshotResult.Value.Snapshot);
    {
      std::string leaseId1 = RandomUUID();
      auto lastModified = m_shareClient->GetProperties().Value.LastModified;
      std::chrono::seconds leaseDuration(20);
      Files::Shares::ShareLeaseClient shareSnapshotLeaseClient(shareSnapshot, leaseId1);
      auto aLease = shareSnapshotLeaseClient.Acquire(leaseDuration).Value;
      EXPECT_TRUE(aLease.ETag.HasValue());
      EXPECT_GE(aLease.LastModified, lastModified);
      EXPECT_EQ(aLease.LeaseId, leaseId1);
      lastModified = shareSnapshot.GetProperties().Value.LastModified;
      aLease
          = shareSnapshotLeaseClient.Acquire(Files::Shares::ShareLeaseClient::InfiniteLeaseDuration)
                .Value;
      EXPECT_TRUE(aLease.ETag.HasValue());
      EXPECT_GE(aLease.LastModified, lastModified);
      EXPECT_EQ(aLease.LeaseId, leaseId1);

      auto properties = shareSnapshot.GetProperties().Value;
      EXPECT_EQ(properties.LeaseState.Value(), Files::Shares::Models::LeaseState::Leased);
      EXPECT_EQ(properties.LeaseStatus.Value(), Files::Shares::Models::LeaseStatus::Locked);

      lastModified = shareSnapshot.GetProperties().Value.LastModified;
      auto rLease = shareSnapshotLeaseClient.Renew().Value;
      EXPECT_TRUE(aLease.ETag.HasValue());
      EXPECT_GE(aLease.LastModified, lastModified);
      EXPECT_EQ(rLease.LeaseId, leaseId1);

      lastModified = shareSnapshot.GetProperties().Value.LastModified;
      std::string leaseId2 = RandomUUID();
      EXPECT_NE(leaseId1, leaseId2);
      auto cLease = shareSnapshotLeaseClient.Change(leaseId2).Value;
      EXPECT_TRUE(cLease.ETag.HasValue());
      EXPECT_GE(cLease.LastModified, lastModified);
      EXPECT_EQ(cLease.LeaseId, leaseId2);
      EXPECT_EQ(shareSnapshotLeaseClient.GetLeaseId(), leaseId2);

      lastModified = shareSnapshot.GetProperties().Value.LastModified;
      auto relLease = shareSnapshotLeaseClient.Release().Value;
      EXPECT_TRUE(relLease.ETag.HasValue());
      EXPECT_GE(relLease.LastModified, lastModified);
    }

    {
      Files::Shares::ShareLeaseClient shareSnapshotLeaseClient(shareSnapshot, RandomUUID());
      auto aLease
          = shareSnapshotLeaseClient.Acquire(Files::Shares::ShareLeaseClient::InfiniteLeaseDuration)
                .Value;
      auto properties = shareSnapshot.GetProperties().Value;
      EXPECT_EQ(
          Files::Shares::Models::LeaseDurationType::Infinite, properties.LeaseDuration.Value());
      auto brokenLease = shareSnapshotLeaseClient.Break().Value;
      EXPECT_TRUE(brokenLease.ETag.HasValue());
      EXPECT_GE(brokenLease.LastModified, properties.LastModified);
      shareSnapshotLeaseClient.Release();
    }

    EXPECT_THROW(m_shareClient->Delete(), StorageException);
  }

  TEST_F(FileShareClientTest, UnencodedDirectoryFileNameWorks)
  {
    const std::string non_ascii_word = "\xE6\xB5\x8B\xE8\xAF\x95";
    const std::string encoded_non_ascii_word = "%E6%B5%8B%E8%AF%95";
    std::string baseName = "a b c !@#$%^&(,.;'[]{}`~) def" + non_ascii_word;

    {
      std::string directoryName = baseName + LowercaseRandomString() + "1";
      auto directoryClient
          = m_shareClient->GetRootDirectoryClient().GetSubdirectoryClient(directoryName);
      EXPECT_NO_THROW(directoryClient.Create());
      auto directoryUrl = directoryClient.GetUrl();
      EXPECT_EQ(
          directoryUrl, m_shareClient->GetUrl() + "/" + _internal::UrlEncodePath(directoryName));
    }
    {
      std::string fileName = baseName + LowercaseRandomString() + "2";
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
    std::string prefix = "prefix";
    Files::Shares::Models::ShareProperties properties;
    {
      auto shareName = prefix + LowercaseRandomString() + "1";
      auto shareClient = GetShareClientForTest(shareName);
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
      auto shareName = prefix + LowercaseRandomString() + "2";
      auto shareClient = GetShareClientForTest(shareName);
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
      auto shareName = prefix + LowercaseRandomString() + "3";
      auto shareClient = GetShareClientForTest(shareName);
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
      auto shareClient = GetShareClientForTest(LowercaseRandomString());
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
    for (auto pageResult = m_shareServiceClient->ListShares(listOptions); pageResult.HasPage();
         pageResult.MoveToNextPage())
    {
      shareItems.insert(shareItems.end(), pageResult.Shares.begin(), pageResult.Shares.end());
    }
    EXPECT_EQ(3U, shareItems.size());
    for (const auto& shareItem : shareItems)
    {
      EXPECT_NE(shareClients.find(shareItem.Name), shareClients.end());
      properties = shareClients.at(shareItem.Name).GetProperties().Value;
      ASSERT_TRUE(shareItem.Details.AccessTier.HasValue());
      ASSERT_TRUE(properties.AccessTier.HasValue());
      EXPECT_EQ(shareItem.Details.AccessTier.Value(), properties.AccessTier.Value());
      ASSERT_TRUE(shareItem.Details.AccessTierChangedOn.HasValue());
      ASSERT_TRUE(properties.AccessTierChangedOn.HasValue());
      EXPECT_EQ(
          shareItem.Details.AccessTierChangedOn.Value(), properties.AccessTierChangedOn.Value());
      EXPECT_FALSE(shareItem.Details.AccessTierTransitionState.HasValue());
      EXPECT_FALSE(properties.AccessTierTransitionState.HasValue());
    }
  }

  TEST_F(FileShareClientTest, GetStatistics) { EXPECT_NO_THROW(m_shareClient->GetStatistics()); }

  TEST_F(FileShareClientTest, PremiumShare)
  {
    auto shareClientOptions = InitStorageClientOptions<Files::Shares::ShareClientOptions>();
    auto shareServiceClient = Files::Shares::ShareServiceClient::CreateFromConnectionString(
        PremiumFileConnectionString(), shareClientOptions);
    {
      auto shareName = LowercaseRandomString();
      auto shareClient = GetPremiumShareClientForTest(shareName);
      // create works
      EXPECT_NO_THROW(shareClient.Create());
      Files::Shares::Models::ShareProperties properties;
      EXPECT_NO_THROW(properties = shareClient.GetProperties().Value);
      EXPECT_EQ(Files::Shares::Models::AccessTier::Premium, properties.AccessTier.Value());
      EXPECT_FALSE(properties.AccessTierTransitionState.HasValue());
      EXPECT_FALSE(properties.AccessTierChangedOn.HasValue());
      EXPECT_TRUE(properties.ProvisionedBandwidthMBps.HasValue());

      // list shares works
      Files::Shares::ListSharesOptions listOptions;
      listOptions.Prefix = shareName;
      std::vector<Files::Shares::Models::ShareItem> shareItems;
      for (auto pageResult = shareServiceClient.ListShares(listOptions); pageResult.HasPage();
           pageResult.MoveToNextPage())
      {
        shareItems.insert(shareItems.end(), pageResult.Shares.begin(), pageResult.Shares.end());
      }
      EXPECT_EQ(1U, shareItems.size());
      EXPECT_EQ(
          Files::Shares::Models::AccessTier::Premium, shareItems[0].Details.AccessTier.Value());
      EXPECT_FALSE(shareItems[0].Details.AccessTierTransitionState.HasValue());
      EXPECT_FALSE(shareItems[0].Details.AccessTierChangedOn.HasValue());
      EXPECT_TRUE(shareItems[0].Details.ProvisionedBandwidthMBps.HasValue());

      // set&get properties works
      auto setPropertiesOptions = Files::Shares::SetSharePropertiesOptions();
      setPropertiesOptions.AccessTier = Files::Shares::Models::AccessTier::Hot;
      EXPECT_THROW(shareClient.SetProperties(setPropertiesOptions), StorageException);
      setPropertiesOptions.AccessTier = Files::Shares::Models::AccessTier::Cool;
      EXPECT_THROW(shareClient.SetProperties(setPropertiesOptions), StorageException);
      setPropertiesOptions.AccessTier = Files::Shares::Models::AccessTier::TransactionOptimized;
      EXPECT_THROW(shareClient.SetProperties(setPropertiesOptions), StorageException);
      setPropertiesOptions.AccessTier = Files::Shares::Models::AccessTier::Premium;
      EXPECT_NO_THROW(shareClient.SetProperties(setPropertiesOptions));
      EXPECT_NO_THROW(properties = shareClient.GetProperties().Value);
      EXPECT_EQ(Files::Shares::Models::AccessTier::Premium, properties.AccessTier.Value());
      EXPECT_FALSE(properties.AccessTierTransitionState.HasValue());
      EXPECT_FALSE(properties.AccessTierChangedOn.HasValue());
      shareClient.DeleteIfExists();
    }
    // nfs protocol works
    {
      auto shareName = LowercaseRandomString();
      auto shareClient = GetPremiumShareClientForTest(shareName);
      // create works
      Files::Shares::CreateShareOptions options;
      options.EnabledProtocols = Files::Shares::Models::ShareProtocols::Nfs;
      options.RootSquash = Files::Shares::Models::ShareRootSquash::AllSquash;
      EXPECT_NO_THROW(shareClient.Create(options));
      Files::Shares::Models::ShareProperties properties;
      EXPECT_NO_THROW(properties = shareClient.GetProperties().Value);
      EXPECT_EQ(options.EnabledProtocols.Value(), properties.EnabledProtocols.Value());
      EXPECT_EQ(options.RootSquash.Value(), properties.RootSquash.Value());

      // list shares works
      Files::Shares::ListSharesOptions listOptions;
      listOptions.Prefix = shareName;
      std::vector<Files::Shares::Models::ShareItem> shareItems;
      for (auto pageResult = shareServiceClient.ListShares(listOptions); pageResult.HasPage();
           pageResult.MoveToNextPage())
      {
        shareItems.insert(shareItems.end(), pageResult.Shares.begin(), pageResult.Shares.end());
      }
      EXPECT_EQ(1U, shareItems.size());
      EXPECT_EQ(options.EnabledProtocols.Value(), shareItems[0].Details.EnabledProtocols.Value());
      EXPECT_EQ(options.RootSquash.Value(), shareItems[0].Details.RootSquash.Value());

      // set&get properties works
      auto setPropertiesOptions = Files::Shares::SetSharePropertiesOptions();
      setPropertiesOptions.RootSquash = Files::Shares::Models::ShareRootSquash::NoRootSquash;
      EXPECT_NO_THROW(shareClient.SetProperties(setPropertiesOptions));
      EXPECT_NO_THROW(properties = shareClient.GetProperties().Value);
      EXPECT_EQ(setPropertiesOptions.RootSquash.Value(), properties.RootSquash.Value());
      shareClient.DeleteIfExists();
    }
    // smb protocol works
    {
      auto shareName = LowercaseRandomString();
      auto shareClient = GetPremiumShareClientForTest(shareName);
      // create works
      Files::Shares::CreateShareOptions options;
      options.EnabledProtocols = Files::Shares::Models::ShareProtocols::Smb;
      EXPECT_NO_THROW(shareClient.Create(options));
      Files::Shares::Models::ShareProperties properties;
      EXPECT_NO_THROW(properties = shareClient.GetProperties().Value);
      EXPECT_EQ(options.EnabledProtocols.Value(), properties.EnabledProtocols.Value());

      // list shares works
      Files::Shares::ListSharesOptions listOptions;
      listOptions.Prefix = shareName;
      std::vector<Files::Shares::Models::ShareItem> shareItems;
      for (auto pageResult = shareServiceClient.ListShares(listOptions); pageResult.HasPage();
           pageResult.MoveToNextPage())
      {
        shareItems.insert(shareItems.end(), pageResult.Shares.begin(), pageResult.Shares.end());
      }
      EXPECT_EQ(1U, shareItems.size());
      EXPECT_EQ(options.EnabledProtocols.Value(), shareItems[0].Details.EnabledProtocols.Value());
      shareClient.DeleteIfExists();
    }
  }

}}} // namespace Azure::Storage::Test
