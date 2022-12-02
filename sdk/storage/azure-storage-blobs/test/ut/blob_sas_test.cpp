// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/identity/client_secret_credential.hpp>
#include <azure/storage/blobs/blob_sas_builder.hpp>

#include <chrono>

#include "blob_container_client_test.hpp"

namespace Azure { namespace Storage { namespace Test {

  TEST_F(BlobContainerClientTest, BlobSasTest_LIVEONLY_)
  {
    CHECK_SKIP_TEST();

    auto blobContainerClient = GetBlobContainerTestClient();
    blobContainerClient.CreateIfNotExists();

    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiredOn = std::chrono::system_clock::now() - std::chrono::minutes(1);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.StartsOn = sasStartsOn;
    accountSasBuilder.ExpiresOn = sasExpiresOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Blobs;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;

    std::string blobName = RandomString();
    Sas::BlobSasBuilder blobSasBuilder;
    blobSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    blobSasBuilder.StartsOn = sasStartsOn;
    blobSasBuilder.ExpiresOn = sasExpiresOn;
    blobSasBuilder.BlobContainerName = m_containerName;
    blobSasBuilder.BlobName = blobName;
    blobSasBuilder.Resource = Sas::BlobSasResource::Blob;

    Sas::BlobSasBuilder containerSasBuilder = blobSasBuilder;
    containerSasBuilder.BlobName.clear();
    containerSasBuilder.Resource = Sas::BlobSasResource::BlobContainer;

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;
    auto accountName = keyCredential->AccountName;
    auto blobServiceClient0
        = Blobs::BlobServiceClient::CreateFromConnectionString(StandardStorageConnectionString());
    auto blobContainerClient0 = blobServiceClient0.GetBlobContainerClient(m_containerName);
    auto blobClient0 = blobContainerClient0.GetAppendBlobClient(blobName);

    auto serviceUrl = blobServiceClient0.GetUrl();
    auto containerUrl = blobContainerClient0.GetUrl();
    auto blobUrl = blobClient0.GetUrl();

    auto blobServiceClient1 = Blobs::BlobServiceClient(
        serviceUrl,
        std::make_shared<Azure::Identity::ClientSecretCredential>(
            AadTenantId(), AadClientId(), AadClientSecret()));
    auto userDelegationKey = blobServiceClient1.GetUserDelegationKey(sasExpiresOn).Value;

    auto verify_blob_read = [&](const std::string& sas) {
      EXPECT_NO_THROW(blobClient0.Create());
      auto blobClient = Blobs::AppendBlobClient(blobUrl + sas);
      auto downloadedContent = blobClient.Download();
      EXPECT_TRUE(ReadBodyStream(downloadedContent.Value.BodyStream).empty());
      blobClient0.Delete();
    };

    auto verify_blob_write = [&](const std::string& sas) {
      auto blobClient = Blobs::AppendBlobClient(blobUrl + sas);
      EXPECT_NO_THROW(blobClient.Create());
      blobClient0.Delete();
    };

    auto verify_blob_delete = [&](const std::string& sas) {
      blobClient0.Create();
      auto blobClient = Blobs::AppendBlobClient(blobUrl + sas);
      EXPECT_NO_THROW(blobClient.Delete());
    };

    auto verify_blob_add = [&](const std::string& sas) {
      blobClient0.Create();
      std::string content = "Hello world";
      auto blockContent = Azure::Core::IO::MemoryBodyStream(
          reinterpret_cast<const uint8_t*>(content.data()), content.size());
      auto blobClient = Blobs::AppendBlobClient(blobUrl + sas);
      EXPECT_NO_THROW(blobClient.AppendBlock(blockContent));
      blobClient0.Delete();
    };

    auto verify_blob_list = [&](const std::string& sas) {
      auto blobContainerClient = Blobs::BlobContainerClient(containerUrl + sas);
      EXPECT_NO_THROW(blobContainerClient.ListBlobs());
    };

    auto verify_blob_create = [&](const std::string& sas) {
      try
      {
        blobClient0.Delete();
      }
      catch (const StorageException&)
      {
      }
      auto blobClient = Blobs::AppendBlobClient(blobUrl + sas);
      blobClient.Create();
      blobClient.CreateSnapshot();
      Blobs::DeleteBlobOptions options;
      options.DeleteSnapshots = Blobs::Models::DeleteSnapshotsOption::IncludeSnapshots;
      blobClient0.Delete(options);
    };

    auto verify_blob_tags = [&](const std::string& sas) {
      blobClient0.Create();
      std::map<std::string, std::string> tags = {{"tag_key1", "tag_value1"}};
      blobClient0.SetTags(tags);
      auto blobClient = Blobs::AppendBlobClient(blobUrl + sas);
      EXPECT_NO_THROW(blobClient.GetTags());
      blobClient0.Delete();
    };

    auto verify_blob_filter = [&](const std::string& sas) {
      auto serviceClient = Blobs::BlobServiceClient(serviceUrl + sas);
      EXPECT_NO_THROW(serviceClient.FindBlobsByTags("\"tag_key1\" = 'tag_value1'"));
    };

    auto verify_blob_immutability = [&](const std::string& sas) {
      (void)sas;
      // Disabled because there's no way to enable immutability on a container with dataplane API
      // blobClient0.Create();
      // auto blobClient = Blobs::AppendBlobClient(blobUrl + sas);
      // Blobs::Models::BlobImmutabilityPolicy policy;
      // policy.ExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(10);
      // policy.PolicyMode = Blobs::Models::BlobImmutabilityPolicyMode::Unlocked;
      // EXPECT_NO_THROW(blobClient.SetImmutabilityPolicy(policy));
      // blobClient0.DeleteImmutabilityPolicy();
      // blobClient0.Delete();
    };

    for (auto permissions : {
             Sas::AccountSasPermissions::All,
             Sas::AccountSasPermissions::Read,
             Sas::AccountSasPermissions::Write,
             Sas::AccountSasPermissions::Delete,
             Sas::AccountSasPermissions::DeleteVersion,
             Sas::AccountSasPermissions::List,
             Sas::AccountSasPermissions::Add,
             Sas::AccountSasPermissions::Create,
             Sas::AccountSasPermissions::Tags,
             Sas::AccountSasPermissions::Filter,
             Sas::AccountSasPermissions::SetImmutabilityPolicy,
         })
    {
      accountSasBuilder.SetPermissions(permissions);
      auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);

      if ((permissions & Sas::AccountSasPermissions::Read) == Sas::AccountSasPermissions::Read)
      {
        verify_blob_read(sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Write) == Sas::AccountSasPermissions::Write)
      {
        verify_blob_write(sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Delete) == Sas::AccountSasPermissions::Delete)
      {
        verify_blob_delete(sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::List) == Sas::AccountSasPermissions::List)
      {
        verify_blob_list(sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Add) == Sas::AccountSasPermissions::Add)
      {
        verify_blob_add(sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Create) == Sas::AccountSasPermissions::Create)
      {
        verify_blob_create(sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Tags) == Sas::AccountSasPermissions::Tags)
      {
        verify_blob_tags(sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Filter) == Sas::AccountSasPermissions::Filter)
      {
        verify_blob_filter(sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::SetImmutabilityPolicy)
          == Sas::AccountSasPermissions::SetImmutabilityPolicy)
      {
        verify_blob_immutability(sasToken);
      }
    }

    for (auto permissions : {
             Sas::BlobSasPermissions::All,
             Sas::BlobSasPermissions::Read,
             Sas::BlobSasPermissions::Write,
             Sas::BlobSasPermissions::Delete,
             Sas::BlobSasPermissions::Add,
             Sas::BlobSasPermissions::Create,
             Sas::BlobSasPermissions::Tags,
             Sas::BlobSasPermissions::DeleteVersion,
             Sas::BlobSasPermissions::SetImmutabilityPolicy,
         })
    {
      blobSasBuilder.SetPermissions(permissions);
      auto sasToken = blobSasBuilder.GenerateSasToken(*keyCredential);
      auto sasToken2 = blobSasBuilder.GenerateSasToken(userDelegationKey, accountName);

      if ((permissions & Sas::BlobSasPermissions::Read) == Sas::BlobSasPermissions::Read)
      {
        verify_blob_read(sasToken);
        verify_blob_read(sasToken2);
      }
      if ((permissions & Sas::BlobSasPermissions::Write) == Sas::BlobSasPermissions::Write)
      {
        verify_blob_write(sasToken);
        verify_blob_write(sasToken2);
      }
      if ((permissions & Sas::BlobSasPermissions::Delete) == Sas::BlobSasPermissions::Delete)
      {
        verify_blob_delete(sasToken);
        verify_blob_delete(sasToken2);
      }
      if ((permissions & Sas::BlobSasPermissions::Add) == Sas::BlobSasPermissions::Add)
      {
        verify_blob_add(sasToken);
        verify_blob_add(sasToken2);
      }
      if ((permissions & Sas::BlobSasPermissions::Create) == Sas::BlobSasPermissions::Create)
      {
        verify_blob_create(sasToken);
        verify_blob_create(sasToken2);
      }
      if ((permissions & Sas::BlobSasPermissions::Tags) == Sas::BlobSasPermissions::Tags)
      {
        verify_blob_tags(sasToken);
        verify_blob_tags(sasToken2);
      }
      if ((permissions & Sas::BlobSasPermissions::SetImmutabilityPolicy)
          == Sas::BlobSasPermissions::SetImmutabilityPolicy)
      {
        verify_blob_immutability(sasToken);
      }
    }

    accountSasBuilder.SetPermissions(Sas::AccountSasPermissions::All);
    // Expires
    {
      Sas::AccountSasBuilder builder2 = accountSasBuilder;
      builder2.StartsOn = sasStartsOn;
      builder2.ExpiresOn = sasExpiredOn;
      auto sasToken = builder2.GenerateSasToken(*keyCredential);
      EXPECT_THROW(verify_blob_create(sasToken), StorageException);
    }

    // Without start time
    {
      Sas::AccountSasBuilder builder2 = accountSasBuilder;
      builder2.StartsOn.Reset();
      auto sasToken = builder2.GenerateSasToken(*keyCredential);
      EXPECT_NO_THROW(verify_blob_create(sasToken));
    }

    // IP
    {
      Sas::AccountSasBuilder builder2 = accountSasBuilder;
      builder2.IPRange = "1.1.1.1";
      auto sasToken = builder2.GenerateSasToken(*keyCredential);
      EXPECT_THROW(verify_blob_create(sasToken), StorageException);

      // TODO: Add this test case back with support to contain IPv6 ranges when service is ready.
      // builder2.IPRange = "0.0.0.0-255.255.255.255";
      // sasToken = builder2.GenerateSasToken(*keyCredential);
      // EXPECT_NO_THROW(verify_blob_create(sasToken));
    }

    // Account SAS Service
    {
      Sas::AccountSasBuilder builder2 = accountSasBuilder;
      builder2.Services = Sas::AccountSasServices::Files;
      auto sasToken = builder2.GenerateSasToken(*keyCredential);
      EXPECT_THROW(verify_blob_create(sasToken), StorageException);

      builder2.Services = Sas::AccountSasServices::All;
      sasToken = builder2.GenerateSasToken(*keyCredential);
      EXPECT_NO_THROW(verify_blob_create(sasToken));
    }

    // Account SAS Resource Types
    {
      Sas::AccountSasBuilder builder2 = accountSasBuilder;
      builder2.ResourceTypes = Sas::AccountSasResource::Service;
      auto sasToken = builder2.GenerateSasToken(*keyCredential);
      EXPECT_THROW(verify_blob_create(sasToken), StorageException);

      auto serviceClient = Blobs::BlobServiceClient(serviceUrl + sasToken);
      EXPECT_NO_THROW(serviceClient.ListBlobContainers());
    }

    for (auto permissions : {
             Sas::BlobContainerSasPermissions::All,
             Sas::BlobContainerSasPermissions::Read,
             Sas::BlobContainerSasPermissions::Write,
             Sas::BlobContainerSasPermissions::Delete,
             Sas::BlobContainerSasPermissions::List,
             Sas::BlobContainerSasPermissions::Add,
             Sas::BlobContainerSasPermissions::Create,
             Sas::BlobContainerSasPermissions::Tags,
             Sas::BlobContainerSasPermissions::SetImmutabilityPolicy,
         })
    {
      containerSasBuilder.SetPermissions(permissions);
      auto sasToken = containerSasBuilder.GenerateSasToken(*keyCredential);
      auto sasToken2 = containerSasBuilder.GenerateSasToken(userDelegationKey, accountName);

      if ((permissions & Sas::BlobContainerSasPermissions::Read)
          == Sas::BlobContainerSasPermissions::Read)
      {
        verify_blob_read(sasToken);
        verify_blob_read(sasToken2);
      }
      if ((permissions & Sas::BlobContainerSasPermissions::Write)
          == Sas::BlobContainerSasPermissions::Write)
      {
        verify_blob_write(sasToken);
        verify_blob_write(sasToken2);
      }
      if ((permissions & Sas::BlobContainerSasPermissions::Delete)
          == Sas::BlobContainerSasPermissions::Delete)
      {
        verify_blob_delete(sasToken);
        verify_blob_delete(sasToken2);
      }
      if ((permissions & Sas::BlobContainerSasPermissions::List)
          == Sas::BlobContainerSasPermissions::List)
      {
        verify_blob_list(sasToken);
        verify_blob_list(sasToken2);
      }
      if ((permissions & Sas::BlobContainerSasPermissions::Add)
          == Sas::BlobContainerSasPermissions::Add)
      {
        verify_blob_add(sasToken);
        verify_blob_add(sasToken2);
      }
      if ((permissions & Sas::BlobContainerSasPermissions::Create)
          == Sas::BlobContainerSasPermissions::Create)
      {
        verify_blob_create(sasToken);
        verify_blob_create(sasToken2);
      }
      if ((permissions & Sas::BlobContainerSasPermissions::Tags)
          == Sas::BlobContainerSasPermissions::Tags)
      {
        verify_blob_tags(sasToken);
        verify_blob_tags(sasToken2);
      }
      if ((permissions & Sas::BlobContainerSasPermissions::SetImmutabilityPolicy)
          == Sas::BlobContainerSasPermissions::SetImmutabilityPolicy)
      {
        verify_blob_immutability(sasToken);
      }
    }

    blobSasBuilder.SetPermissions(Sas::BlobSasPermissions::All);
    // Expires
    {
      Sas::BlobSasBuilder builder2 = blobSasBuilder;
      builder2.StartsOn = sasStartsOn;
      builder2.ExpiresOn = sasExpiredOn;
      auto sasToken = builder2.GenerateSasToken(*keyCredential);
      EXPECT_THROW(verify_blob_create(sasToken), StorageException);

      auto sasToken2 = builder2.GenerateSasToken(userDelegationKey, accountName);
      EXPECT_THROW(verify_blob_create(sasToken2), StorageException);
    }

    // Without start time
    {
      Sas::BlobSasBuilder builder2 = blobSasBuilder;
      builder2.StartsOn.Reset();
      auto sasToken = builder2.GenerateSasToken(*keyCredential);
      EXPECT_NO_THROW(verify_blob_create(sasToken));
      auto sasToken2 = builder2.GenerateSasToken(userDelegationKey, accountName);
      EXPECT_NO_THROW(verify_blob_create(sasToken2));
    }

    // IP
    {
      Sas::BlobSasBuilder builder2 = blobSasBuilder;
      builder2.IPRange = "0.0.0.0-0.0.0.1";
      auto sasToken = builder2.GenerateSasToken(*keyCredential);
      EXPECT_THROW(verify_blob_create(sasToken), StorageException);
      auto sasToken2 = builder2.GenerateSasToken(userDelegationKey, accountName);
      EXPECT_THROW(verify_blob_create(sasToken2), StorageException);

      // TODO: Add this test case back with support to contain IPv6 ranges when service is ready.
      // builder2.IPRange = "0.0.0.0-255.255.255.255";
      // sasToken = builder2.GenerateSasToken(*keyCredential);
      // EXPECT_NO_THROW(verify_blob_create(sasToken));
      // sasToken2 = builder2.GenerateSasToken(userDelegationKey, accountName);
      // EXPECT_NO_THROW(verify_blob_create(sasToken2));
    }

    // Identifier
    {
      Blobs::SetBlobContainerAccessPolicyOptions options;
      options.AccessType = Blobs::Models::PublicAccessType::Blob;
      Blobs::Models::SignedIdentifier identifier;
      identifier.Id = RandomString(64);
      identifier.StartsOn = sasStartsOn;
      identifier.ExpiresOn = sasExpiresOn;
      identifier.Permissions = "r";
      options.SignedIdentifiers.emplace_back(identifier);
      blobContainerClient.SetAccessPolicy(options);

      Sas::BlobSasBuilder builder2 = blobSasBuilder;
      builder2.StartsOn.Reset();
      builder2.ExpiresOn = Azure::DateTime();
      builder2.SetPermissions(static_cast<Sas::BlobContainerSasPermissions>(0));
      builder2.Identifier = identifier.Id;

      auto sasToken = builder2.GenerateSasToken(*keyCredential);
      // TODO: looks like a server bug, the identifier doesn't work sometimes.
      // EXPECT_NO_THROW(verify_blob_read(sasToken));
    }

    // response headers override
    {
      Blobs::Models::BlobHttpHeaders headers;
      headers.ContentType = "application/x-binary";
      headers.ContentLanguage = "en-US";
      headers.ContentDisposition = "attachment";
      headers.CacheControl = "no-cache";
      headers.ContentEncoding = "identify";

      Sas::BlobSasBuilder builder2 = blobSasBuilder;
      builder2.ContentType = "application/x-binary";
      builder2.ContentLanguage = "en-US";
      builder2.ContentDisposition = "attachment";
      builder2.CacheControl = "no-cache";
      builder2.ContentEncoding = "identify";
      auto sasToken = builder2.GenerateSasToken(*keyCredential);
      auto blobClient = Blobs::AppendBlobClient(blobUrl + sasToken);
      blobClient0.Create();
      auto p = blobClient.GetProperties();
      EXPECT_EQ(p.Value.HttpHeaders.ContentType, headers.ContentType);
      EXPECT_EQ(p.Value.HttpHeaders.ContentLanguage, headers.ContentLanguage);
      EXPECT_EQ(p.Value.HttpHeaders.ContentDisposition, headers.ContentDisposition);
      EXPECT_EQ(p.Value.HttpHeaders.CacheControl, headers.CacheControl);
      EXPECT_EQ(p.Value.HttpHeaders.ContentEncoding, headers.ContentEncoding);

      auto sasToken2 = builder2.GenerateSasToken(userDelegationKey, accountName);
      blobClient = Blobs::AppendBlobClient(blobUrl + sasToken2);
      p = blobClient.GetProperties();
      EXPECT_EQ(p.Value.HttpHeaders.ContentType, headers.ContentType);
      EXPECT_EQ(p.Value.HttpHeaders.ContentLanguage, headers.ContentLanguage);
      EXPECT_EQ(p.Value.HttpHeaders.ContentDisposition, headers.ContentDisposition);
      EXPECT_EQ(p.Value.HttpHeaders.CacheControl, headers.CacheControl);
      EXPECT_EQ(p.Value.HttpHeaders.ContentEncoding, headers.ContentEncoding);
      blobClient0.Delete();
    }

    blobClient0.Create();
    Sas::BlobSasBuilder BlobSnapshotSasBuilder = blobSasBuilder;
    BlobSnapshotSasBuilder.Resource = Sas::BlobSasResource::BlobSnapshot;

    std::string blobSnapshotUrl;

    auto create_snapshot = [&]() {
      std::string snapshot = blobClient0.CreateSnapshot().Value.Snapshot;
      BlobSnapshotSasBuilder.Snapshot = snapshot;
      blobSnapshotUrl = blobClient0.WithSnapshot(snapshot).GetUrl();
    };

    auto verify_blob_snapshot_read = [&](const std::string sas) {
      Azure::Core::Url blobSnapshotUrlWithSas(blobSnapshotUrl);
      auto blobSnapshotClient
          = Blobs::AppendBlobClient(AppendQueryParameters(blobSnapshotUrlWithSas, sas));
      auto downloadedContent = blobSnapshotClient.Download();
      EXPECT_TRUE(ReadBodyStream(downloadedContent.Value.BodyStream).empty());
    };

    auto verify_blob_snapshot_delete = [&](const std::string sas) {
      Azure::Core::Url blobSnapshotUrlWithSas(blobSnapshotUrl);
      auto blobSnapshotClient
          = Blobs::AppendBlobClient(AppendQueryParameters(blobSnapshotUrlWithSas, sas));
      EXPECT_NO_THROW(blobSnapshotClient.Delete());
    };

    for (auto permissions : {
             Sas::BlobSasPermissions::Read | Sas::BlobSasPermissions::Delete,
             Sas::BlobSasPermissions::Read,
             Sas::BlobSasPermissions::Delete,
         })
    {
      create_snapshot();
      BlobSnapshotSasBuilder.SetPermissions(permissions);
      auto sasToken = BlobSnapshotSasBuilder.GenerateSasToken(*keyCredential);
      auto sasToken2 = BlobSnapshotSasBuilder.GenerateSasToken(userDelegationKey, accountName);

      if ((permissions & Sas::BlobSasPermissions::Read) == Sas::BlobSasPermissions::Read)
      {
        verify_blob_snapshot_read(sasToken);
        verify_blob_snapshot_read(sasToken2);
      }
      if ((permissions & Sas::BlobSasPermissions::Delete) == Sas::BlobSasPermissions::Delete)
      {
        create_snapshot();
        sasToken = BlobSnapshotSasBuilder.GenerateSasToken(*keyCredential);
        verify_blob_snapshot_delete(sasToken);
        create_snapshot();
        sasToken2 = BlobSnapshotSasBuilder.GenerateSasToken(userDelegationKey, accountName);
        verify_blob_snapshot_delete(sasToken2);
      }
    }

    {
      Blobs::DeleteBlobOptions options;
      options.DeleteSnapshots = Blobs::Models::DeleteSnapshotsOption::IncludeSnapshots;
      blobClient0.Delete(options);
    }

    blobClient0.Create();
    Sas::BlobSasBuilder BlobVersionSasBuilder = blobSasBuilder;
    BlobVersionSasBuilder.Resource = Sas::BlobSasResource::BlobVersion;

    std::string blobVersionUrl;

    auto create_version = [&]() {
      std::string versionId = blobClient0.CreateSnapshot().Value.VersionId.Value();
      BlobVersionSasBuilder.BlobVersionId = versionId;
      blobVersionUrl = blobClient0.WithVersionId(versionId).GetUrl();
      blobClient0.SetMetadata({});
    };

    auto verify_blob_version_read = [&](const std::string sas) {
      Azure::Core::Url blobVersionUrlWithSas(blobVersionUrl);
      auto blobVersionClient
          = Blobs::AppendBlobClient(AppendQueryParameters(blobVersionUrlWithSas, sas));
      auto downloadedContent = blobVersionClient.Download();
      EXPECT_TRUE(ReadBodyStream(downloadedContent.Value.BodyStream).empty());
    };

    auto verify_blob_delete_version = [&](const std::string& sas) {
      Azure::Core::Url blobVersionUrlWithSas(blobVersionUrl);
      auto blobVersionClient
          = Blobs::AppendBlobClient(AppendQueryParameters(blobVersionUrlWithSas, sas));
      blobVersionClient.Delete();
    };

    for (auto permissions : {
             Sas::BlobSasPermissions::Read | Sas::BlobSasPermissions::DeleteVersion,
             Sas::BlobSasPermissions::Read,
             Sas::BlobSasPermissions::DeleteVersion,
         })
    {
      create_version();
      BlobVersionSasBuilder.SetPermissions(permissions);
      auto sasToken = BlobVersionSasBuilder.GenerateSasToken(*keyCredential);
      auto sasToken2 = BlobVersionSasBuilder.GenerateSasToken(userDelegationKey, accountName);

      if ((permissions & Sas::BlobSasPermissions::Read) == Sas::BlobSasPermissions::Read)
      {
        verify_blob_version_read(sasToken);
        verify_blob_version_read(sasToken2);
      }
      if ((permissions & Sas::BlobSasPermissions::DeleteVersion)
          == Sas::BlobSasPermissions::DeleteVersion)
      {
        create_version();
        sasToken = BlobVersionSasBuilder.GenerateSasToken(*keyCredential);
        verify_blob_delete_version(sasToken);
        create_version();
        sasToken2 = BlobVersionSasBuilder.GenerateSasToken(userDelegationKey, accountName);
        verify_blob_delete_version(sasToken2);
      }
    }
    {
      Blobs::DeleteBlobOptions options;
      options.DeleteSnapshots = Blobs::Models::DeleteSnapshotsOption::IncludeSnapshots;
      blobClient0.Delete(options);
    }

    // Encryption scope
    const auto encryptionScope = GetTestEncryptionScope();
    {
      auto sasBuilderWithEncryptionScope = blobSasBuilder;
      sasBuilderWithEncryptionScope.EncryptionScope = encryptionScope;
      auto blobClientEncryptionScopeSas = Blobs::AppendBlobClient(
          blobUrl + sasBuilderWithEncryptionScope.GenerateSasToken(*keyCredential));
      blobClientEncryptionScopeSas.Create();
      auto blobProperties = blobClientEncryptionScopeSas.GetProperties().Value;
      ASSERT_TRUE(blobProperties.EncryptionScope.HasValue());
      EXPECT_EQ(blobProperties.EncryptionScope.Value(), encryptionScope);

      blobClientEncryptionScopeSas = Blobs::AppendBlobClient(
          blobUrl + sasBuilderWithEncryptionScope.GenerateSasToken(userDelegationKey, accountName));
      blobClientEncryptionScopeSas.Create();
      blobProperties = blobClientEncryptionScopeSas.GetProperties().Value;
      ASSERT_TRUE(blobProperties.EncryptionScope.HasValue());
      EXPECT_EQ(blobProperties.EncryptionScope.Value(), encryptionScope);
    }
    {
      auto sasBuilderWithEncryptionScope = accountSasBuilder;
      sasBuilderWithEncryptionScope.EncryptionScope = encryptionScope;
      auto blobClientEncryptionScopeSas = Blobs::AppendBlobClient(
          blobUrl + sasBuilderWithEncryptionScope.GenerateSasToken(*keyCredential));
      blobClientEncryptionScopeSas.Create();
      auto blobProperties = blobClientEncryptionScopeSas.GetProperties().Value;
      ASSERT_TRUE(blobProperties.EncryptionScope.HasValue());
      EXPECT_EQ(blobProperties.EncryptionScope.Value(), encryptionScope);
    }
  }

}}} // namespace Azure::Storage::Test