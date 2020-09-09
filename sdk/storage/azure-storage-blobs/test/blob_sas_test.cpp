// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/blob_sas_builder.hpp"
#include "blob_container_client_test.hpp"

namespace Azure { namespace Storage { namespace Test {

  TEST_F(BlobContainerClientTest, BlobSasTest)
  {
    AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = SasProtocol::HttpsAndHtttp;
    accountSasBuilder.StartsOn
        = ToIso8601(std::chrono::system_clock::now() - std::chrono::minutes(5));
    accountSasBuilder.ExpiresOn
        = ToIso8601(std::chrono::system_clock::now() + std::chrono::minutes(60));
    accountSasBuilder.Services = AccountSasServices::Blobs;
    accountSasBuilder.ResourceTypes = AccountSasResource::All;

    std::string blobName = RandomString();
    Blobs::BlobSasBuilder blobSasBuilder;
    blobSasBuilder.Protocol = SasProtocol::HttpsAndHtttp;
    blobSasBuilder.StartsOn = ToIso8601(std::chrono::system_clock::now() - std::chrono::minutes(5));
    blobSasBuilder.ExpiresOn
        = ToIso8601(std::chrono::system_clock::now() + std::chrono::minutes(60));
    blobSasBuilder.ContainerName = m_containerName;
    blobSasBuilder.BlobName = blobName;
    blobSasBuilder.Resource = Blobs::BlobSasResource::Blob;

    Blobs::BlobSasBuilder containerSasBuilder = blobSasBuilder;
    containerSasBuilder.BlobName.clear();
    containerSasBuilder.Resource = Blobs::BlobSasResource::Container;

    auto keyCredential
        = Details::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;
    auto accountName = keyCredential->AccountName;
    auto blobServiceClient0
        = Blobs::BlobServiceClient::CreateFromConnectionString(StandardStorageConnectionString());
    auto blobContainerClient0 = blobServiceClient0.GetBlobContainerClient(m_containerName);
    auto blobClient0 = blobContainerClient0.GetAppendBlobClient(blobName);

    auto serviceUri = blobServiceClient0.GetUri();
    auto containerUri = blobContainerClient0.GetUri();
    auto blobUri = blobClient0.GetUri();

    auto blobServiceClient1 = Blobs::BlobServiceClient(
        serviceUri,
        std::make_shared<Azure::Core::Credentials::ClientSecretCredential>(
            AadTenantId(), AadClientId(), AadClientSecret()));
    auto userDelegationKey = *blobServiceClient1.GetUserDelegationKey(
        ToIso8601(std::chrono::system_clock::now() - std::chrono::minutes(5)),
        ToIso8601(std::chrono::system_clock::now() + std::chrono::minutes(60)));

    auto verify_blob_read = [&](const std::string& sas) {
      EXPECT_NO_THROW(blobClient0.Create());
      auto blobClient = Blobs::AppendBlobClient(blobUri + sas);
      auto downloadedContent = blobClient.Download();
      EXPECT_TRUE(ReadBodyStream(downloadedContent->BodyStream).empty());
    };

    auto verify_blob_write = [&](const std::string& sas) {
      auto blobClient = Blobs::AppendBlobClient(blobUri + sas);
      EXPECT_NO_THROW(blobClient.Create());
    };

    auto verify_blob_delete = [&](const std::string& sas) {
      blobClient0.Create();
      auto blobClient = Blobs::AppendBlobClient(blobUri + sas);
      EXPECT_NO_THROW(blobClient.Delete());
    };

    auto verify_blob_add = [&](const std::string& sas) {
      blobClient0.Create();
      std::string content = "Hello world";
      auto blockContent = Azure::Core::Http::MemoryBodyStream(
          reinterpret_cast<const uint8_t*>(content.data()), content.size());
      auto blobClient = Blobs::AppendBlobClient(blobUri + sas);
      EXPECT_NO_THROW(blobClient.AppendBlock(&blockContent));
    };

    auto verify_blob_list = [&](const std::string& sas) {
      auto blobContainerClient = Blobs::BlobContainerClient(containerUri + sas);
      EXPECT_NO_THROW(blobContainerClient.ListBlobsFlatSegment());
    };

    auto verify_blob_create = [&](const std::string& sas) {
      try
      {
        blobClient0.Delete();
      }
      catch (StorageError&)
      {
      }
      auto blobClient = Blobs::AppendBlobClient(blobUri + sas);
      blobClient.Create();
      blobClient.CreateSnapshot();
      Blobs::DeleteBlobOptions options;
      options.DeleteSnapshots = Blobs::DeleteSnapshotsOption::IncludeSnapshots;
      blobClient0.Delete(options);
    };

    auto verify_blob_tags = [&](const std::string& sas) {
      unused(sas);
      /*
      blobClient0.Create();
      std::map<std::string, std::string> tags = {{"tag_key1", "tag_value1"}};
      blobClient0.SetTags(tags);
      auto blobClient = Blobs::AppendBlobClient(blobUri + sas);
      EXPECT_NO_THROW(blobClient.GetTags());
      */
    };

    auto verify_blob_filter = [&](const std::string& sas) {
      unused(sas);
      /*
      auto serviceClient = Blobs::BlobServiceClient(serviceUri + sas);
      EXPECT_NO_THROW(serviceClient.FindBlobsByTags("\"tag_key1\" = 'tag_value1'"));
      */
    };

    for (auto permissions : {
             AccountSasPermissions::All,
             AccountSasPermissions::Read,
             AccountSasPermissions::Write,
             AccountSasPermissions::Delete,
             AccountSasPermissions::DeleteVersion,
             AccountSasPermissions::List,
             AccountSasPermissions::Add,
             AccountSasPermissions::Create,
             AccountSasPermissions::Tags,
             AccountSasPermissions::Filter,
         })
    {
      accountSasBuilder.SetPermissions(permissions);
      auto sasToken = accountSasBuilder.ToSasQueryParameters(*keyCredential);

      if ((permissions & AccountSasPermissions::Read) == AccountSasPermissions::Read)
      {
        verify_blob_read(sasToken);
      }
      if ((permissions & AccountSasPermissions::Write) == AccountSasPermissions::Write)
      {
        verify_blob_write(sasToken);
      }
      if ((permissions & AccountSasPermissions::Delete) == AccountSasPermissions::Delete)
      {
        verify_blob_delete(sasToken);
      }
      if ((permissions & AccountSasPermissions::List) == AccountSasPermissions::List)
      {
        verify_blob_list(sasToken);
      }
      if ((permissions & AccountSasPermissions::Add) == AccountSasPermissions::Add)
      {
        verify_blob_add(sasToken);
      }
      if ((permissions & AccountSasPermissions::Create) == AccountSasPermissions::Create)
      {
        verify_blob_create(sasToken);
      }
      if ((permissions & AccountSasPermissions::Tags) == AccountSasPermissions::Tags)
      {
        verify_blob_tags(sasToken);
      }
      if ((permissions & AccountSasPermissions::Filter) == AccountSasPermissions::Filter)
      {
        verify_blob_filter(sasToken);
      }
    }

    for (auto permissions :
         {Blobs::BlobSasPermissions::All,
          Blobs::BlobSasPermissions::Read,
          Blobs::BlobSasPermissions::Write,
          Blobs::BlobSasPermissions::Delete,
          Blobs::BlobSasPermissions::Add,
          Blobs::BlobSasPermissions::Create,
          Blobs::BlobSasPermissions::Tags,
          Blobs::BlobSasPermissions::DeleteVersion})
    {
      blobSasBuilder.SetPermissions(permissions);
      auto sasToken = blobSasBuilder.ToSasQueryParameters(*keyCredential);
      auto sasToken2 = blobSasBuilder.ToSasQueryParameters(userDelegationKey, accountName);

      if ((permissions & Blobs::BlobSasPermissions::Read) == Blobs::BlobSasPermissions::Read)
      {
        verify_blob_read(sasToken);
        verify_blob_read(sasToken2);
      }
      if ((permissions & Blobs::BlobSasPermissions::Write) == Blobs::BlobSasPermissions::Write)
      {
        verify_blob_write(sasToken);
        verify_blob_write(sasToken2);
      }
      if ((permissions & Blobs::BlobSasPermissions::Delete) == Blobs::BlobSasPermissions::Delete)
      {
        verify_blob_delete(sasToken);
        verify_blob_delete(sasToken2);
      }
      if ((permissions & Blobs::BlobSasPermissions::Add) == Blobs::BlobSasPermissions::Add)
      {
        verify_blob_add(sasToken);
        verify_blob_add(sasToken2);
      }
      if ((permissions & Blobs::BlobSasPermissions::Create) == Blobs::BlobSasPermissions::Create)
      {
        verify_blob_create(sasToken);
        verify_blob_create(sasToken2);
      }
      if ((permissions & Blobs::BlobSasPermissions::Tags) == Blobs::BlobSasPermissions::Tags)
      {
        verify_blob_tags(sasToken);
        verify_blob_tags(sasToken2);
      }
    }

    accountSasBuilder.SetPermissions(AccountSasPermissions::All);
    // Expires
    {
      AccountSasBuilder builder2 = accountSasBuilder;
      builder2.StartsOn = ToIso8601(std::chrono::system_clock::now() - std::chrono::minutes(5));
      builder2.ExpiresOn = ToIso8601(std::chrono::system_clock::now() - std::chrono::minutes(1));
      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      EXPECT_THROW(verify_blob_create(sasToken), StorageError);
    }

    // Without start time
    {
      AccountSasBuilder builder2 = accountSasBuilder;
      builder2.StartsOn.Reset();
      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      EXPECT_NO_THROW(verify_blob_create(sasToken));
    }

    // IP
    {
      AccountSasBuilder builder2 = accountSasBuilder;
      builder2.IPRange = "1.1.1.1";
      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      EXPECT_THROW(verify_blob_create(sasToken), StorageError);

      builder2.IPRange = "0.0.0.0-255.255.255.255";
      sasToken = builder2.ToSasQueryParameters(*keyCredential);
      EXPECT_NO_THROW(verify_blob_create(sasToken));
    }

    // Account SAS Service
    {
      AccountSasBuilder builder2 = accountSasBuilder;
      builder2.Services = AccountSasServices::Files;
      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      EXPECT_THROW(verify_blob_create(sasToken), StorageError);

      builder2.Services = AccountSasServices::All;
      sasToken = builder2.ToSasQueryParameters(*keyCredential);
      EXPECT_NO_THROW(verify_blob_create(sasToken));
    }

    // Account SAS Resource Types
    {
      AccountSasBuilder builder2 = accountSasBuilder;
      builder2.ResourceTypes = AccountSasResource::Service;
      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      EXPECT_THROW(verify_blob_create(sasToken), StorageError);

      auto serviceClient = Blobs::BlobServiceClient(serviceUri + sasToken);
      EXPECT_NO_THROW(serviceClient.ListBlobContainersSegment());
    }

    for (auto permissions :
         {Blobs::BlobContainerSasPermissions::All,
          Blobs::BlobContainerSasPermissions::Read,
          Blobs::BlobContainerSasPermissions::Write,
          Blobs::BlobContainerSasPermissions::Delete,
          Blobs::BlobContainerSasPermissions::List,
          Blobs::BlobContainerSasPermissions::Add,
          Blobs::BlobContainerSasPermissions::Create,
          Blobs::BlobContainerSasPermissions::Tags})
    {
      containerSasBuilder.SetPermissions(permissions);
      auto sasToken = containerSasBuilder.ToSasQueryParameters(*keyCredential);
      auto sasToken2 = containerSasBuilder.ToSasQueryParameters(userDelegationKey, accountName);

      if ((permissions & Blobs::BlobContainerSasPermissions::Read)
          == Blobs::BlobContainerSasPermissions::Read)
      {
        verify_blob_read(sasToken);
        verify_blob_read(sasToken2);
      }
      if ((permissions & Blobs::BlobContainerSasPermissions::Write)
          == Blobs::BlobContainerSasPermissions::Write)
      {
        verify_blob_write(sasToken);
        verify_blob_write(sasToken2);
      }
      if ((permissions & Blobs::BlobContainerSasPermissions::Delete)
          == Blobs::BlobContainerSasPermissions::Delete)
      {
        verify_blob_delete(sasToken);
        verify_blob_delete(sasToken2);
      }
      if ((permissions & Blobs::BlobContainerSasPermissions::List)
          == Blobs::BlobContainerSasPermissions::List)
      {
        verify_blob_list(sasToken);
        verify_blob_list(sasToken2);
      }
      if ((permissions & Blobs::BlobContainerSasPermissions::Add)
          == Blobs::BlobContainerSasPermissions::Add)
      {
        verify_blob_add(sasToken);
        verify_blob_add(sasToken2);
      }
      if ((permissions & Blobs::BlobContainerSasPermissions::Create)
          == Blobs::BlobContainerSasPermissions::Create)
      {
        verify_blob_create(sasToken);
        verify_blob_create(sasToken2);
      }
      if ((permissions & Blobs::BlobContainerSasPermissions::Tags)
          == Blobs::BlobContainerSasPermissions::Tags)
      {
        verify_blob_tags(sasToken);
        verify_blob_tags(sasToken2);
      }
    }

    blobSasBuilder.SetPermissions(Blobs::BlobSasPermissions::All);
    // Expires
    {
      Blobs::BlobSasBuilder builder2 = blobSasBuilder;
      builder2.StartsOn = ToIso8601(std::chrono::system_clock::now() - std::chrono::minutes(5));
      builder2.ExpiresOn = ToIso8601(std::chrono::system_clock::now() - std::chrono::minutes(1));
      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      EXPECT_THROW(verify_blob_create(sasToken), StorageError);

      auto sasToken2 = builder2.ToSasQueryParameters(userDelegationKey, accountName);
      EXPECT_THROW(verify_blob_create(sasToken2), StorageError);
    }

    // Without start time
    {
      Blobs::BlobSasBuilder builder2 = blobSasBuilder;
      builder2.StartsOn.Reset();
      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      EXPECT_NO_THROW(verify_blob_create(sasToken));
      auto sasToken2 = builder2.ToSasQueryParameters(userDelegationKey, accountName);
      EXPECT_NO_THROW(verify_blob_create(sasToken2));
    }

    // IP
    {
      Blobs::BlobSasBuilder builder2 = blobSasBuilder;
      builder2.IPRange = "0.0.0.0-0.0.0.1";
      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      EXPECT_THROW(verify_blob_create(sasToken), StorageError);
      auto sasToken2 = builder2.ToSasQueryParameters(userDelegationKey, accountName);
      EXPECT_THROW(verify_blob_create(sasToken2), StorageError);

      builder2.IPRange = "0.0.0.0-255.255.255.255";
      sasToken = builder2.ToSasQueryParameters(*keyCredential);
      EXPECT_NO_THROW(verify_blob_create(sasToken));
      sasToken2 = builder2.ToSasQueryParameters(userDelegationKey, accountName);
      EXPECT_NO_THROW(verify_blob_create(sasToken2));
    }

    // Identifier
    {
      Blobs::SetContainerAccessPolicyOptions options;
      options.AccessType = Blobs::PublicAccessType::Blob;
      Blobs::BlobSignedIdentifier identifier;
      identifier.Id = RandomString(64);
      identifier.StartsOn = ToIso8601(std::chrono::system_clock::now() - std::chrono::minutes(5));
      identifier.ExpiresOn = ToIso8601(std::chrono::system_clock::now() + std::chrono::minutes(60));
      identifier.Permissions
          = Blobs::BlobContainerSasPermissionsToString(Blobs::BlobContainerSasPermissions::Read);
      options.SignedIdentifiers.emplace_back(identifier);
      m_blobContainerClient->SetAccessPolicy(options);

      Blobs::BlobSasBuilder builder2 = blobSasBuilder;
      builder2.StartsOn.Reset();
      builder2.ExpiresOn.clear();
      builder2.SetPermissions(static_cast<Blobs::BlobContainerSasPermissions>(0));
      builder2.Identifier = identifier.Id;

      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      EXPECT_NO_THROW(verify_blob_read(sasToken));
    }

    // response headers override
    {
      Blobs::BlobHttpHeaders headers;
      headers.ContentType = "application/x-binary";
      headers.ContentLanguage = "en-US";
      headers.ContentDisposition = "attachment";
      headers.CacheControl = "no-cache";
      headers.ContentEncoding = "identify";

      Blobs::BlobSasBuilder builder2 = blobSasBuilder;
      builder2.ContentType = "application/x-binary";
      builder2.ContentLanguage = "en-US";
      builder2.ContentDisposition = "attachment";
      builder2.CacheControl = "no-cache";
      builder2.ContentEncoding = "identify";
      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      auto blobClient = Blobs::AppendBlobClient(blobUri + sasToken);
      blobClient0.Create();
      auto p = blobClient.GetProperties();
      EXPECT_EQ(p->HttpHeaders.ContentType, headers.ContentType);
      EXPECT_EQ(p->HttpHeaders.ContentLanguage, headers.ContentLanguage);
      EXPECT_EQ(p->HttpHeaders.ContentDisposition, headers.ContentDisposition);
      EXPECT_EQ(p->HttpHeaders.CacheControl, headers.CacheControl);
      EXPECT_EQ(p->HttpHeaders.ContentEncoding, headers.ContentEncoding);

      auto sasToken2 = builder2.ToSasQueryParameters(userDelegationKey, accountName);
      blobClient = Blobs::AppendBlobClient(blobUri + sasToken2);
      p = blobClient.GetProperties();
      EXPECT_EQ(p->HttpHeaders.ContentType, headers.ContentType);
      EXPECT_EQ(p->HttpHeaders.ContentLanguage, headers.ContentLanguage);
      EXPECT_EQ(p->HttpHeaders.ContentDisposition, headers.ContentDisposition);
      EXPECT_EQ(p->HttpHeaders.CacheControl, headers.CacheControl);
      EXPECT_EQ(p->HttpHeaders.ContentEncoding, headers.ContentEncoding);
    }

    blobClient0.Create();
    Blobs::BlobSasBuilder BlobSnapshotSasBuilder = blobSasBuilder;
    BlobSnapshotSasBuilder.Resource = Blobs::BlobSasResource::BlobSnapshot;

    std::string blobSnapshotUri;

    auto create_snapshot = [&]() {
      std::string snapshot = blobClient0.CreateSnapshot()->Snapshot;
      BlobSnapshotSasBuilder.Snapshot = snapshot;
      blobSnapshotUri = blobClient0.WithSnapshot(snapshot).GetUri();
    };

    auto verify_blob_snapshot_read = [&](const std::string sas) {
      Azure::Core::Http::Url blobSnapshotUriWithSas(blobSnapshotUri);
      blobSnapshotUriWithSas.AppendQueries(sas);
      auto blobSnapshotClient = Blobs::AppendBlobClient(blobSnapshotUriWithSas.GetAbsoluteUrl());
      auto downloadedContent = blobSnapshotClient.Download();
      EXPECT_TRUE(ReadBodyStream(downloadedContent->BodyStream).empty());
    };

    auto verify_blob_snapshot_delete = [&](const std::string sas) {
      Azure::Core::Http::Url blobSnapshotUriWithSas(blobSnapshotUri);
      blobSnapshotUriWithSas.AppendQueries(sas);
      auto blobSnapshotClient = Blobs::AppendBlobClient(blobSnapshotUriWithSas.GetAbsoluteUrl());
      EXPECT_NO_THROW(blobSnapshotClient.Delete());
    };

    for (auto permissions : {
             Blobs::BlobSasPermissions::Read | Blobs::BlobSasPermissions::Delete,
             Blobs::BlobSasPermissions::Read,
             Blobs::BlobSasPermissions::Delete,
         })
    {
      create_snapshot();
      BlobSnapshotSasBuilder.SetPermissions(permissions);
      auto sasToken = BlobSnapshotSasBuilder.ToSasQueryParameters(*keyCredential);
      auto sasToken2 = BlobSnapshotSasBuilder.ToSasQueryParameters(userDelegationKey, accountName);

      if ((permissions & Blobs::BlobSasPermissions::Read) == Blobs::BlobSasPermissions::Read)
      {
        verify_blob_snapshot_read(sasToken);
        verify_blob_snapshot_read(sasToken2);
      }
      if ((permissions & Blobs::BlobSasPermissions::Delete) == Blobs::BlobSasPermissions::Delete)
      {
        create_snapshot();
        sasToken = BlobSnapshotSasBuilder.ToSasQueryParameters(*keyCredential);
        verify_blob_snapshot_delete(sasToken);
        create_snapshot();
        sasToken2 = BlobSnapshotSasBuilder.ToSasQueryParameters(userDelegationKey, accountName);
        verify_blob_snapshot_delete(sasToken2);
      }
    }

    blobClient0.Create();
    Blobs::BlobSasBuilder BlobVersionSasBuilder = blobSasBuilder;
    BlobVersionSasBuilder.Resource = Blobs::BlobSasResource::BlobVersion;

    std::string blobVersionUri;

    auto create_version = [&]() {
      std::string versionId = blobClient0.CreateSnapshot()->VersionId.GetValue();
      BlobVersionSasBuilder.BlobVersionId = versionId;
      blobVersionUri = blobClient0.WithVersionId(versionId).GetUri();
      blobClient0.SetMetadata({});
    };

    auto verify_blob_version_read = [&](const std::string sas) {
      Azure::Core::Http::Url blobVersionUriWithSas(blobVersionUri);
      blobVersionUriWithSas.AppendQueries(sas);
      auto blobVersionClient = Blobs::AppendBlobClient(blobVersionUriWithSas.GetAbsoluteUrl());
      auto downloadedContent = blobVersionClient.Download();
      EXPECT_TRUE(ReadBodyStream(downloadedContent->BodyStream).empty());
    };

    auto verify_blob_delete_version = [&](const std::string& sas) {
      Azure::Core::Http::Url blobVersionUriWithSas(blobVersionUri);
      blobVersionUriWithSas.AppendQueries(sas);
      auto blobVersionClient = Blobs::AppendBlobClient(blobVersionUriWithSas.GetAbsoluteUrl());
      blobVersionClient.Delete();
    };

    for (auto permissions : {
             Blobs::BlobSasPermissions::Read | Blobs::BlobSasPermissions::DeleteVersion,
             Blobs::BlobSasPermissions::Read,
             Blobs::BlobSasPermissions::DeleteVersion,
         })
    {
      create_version();
      BlobVersionSasBuilder.SetPermissions(permissions);
      auto sasToken = BlobVersionSasBuilder.ToSasQueryParameters(*keyCredential);
      auto sasToken2 = BlobVersionSasBuilder.ToSasQueryParameters(userDelegationKey, accountName);

      if ((permissions & Blobs::BlobSasPermissions::Read) == Blobs::BlobSasPermissions::Read)
      {
        verify_blob_version_read(sasToken);
        verify_blob_version_read(sasToken2);
      }
      if ((permissions & Blobs::BlobSasPermissions::DeleteVersion)
          == Blobs::BlobSasPermissions::DeleteVersion)
      {
        create_version();
        sasToken = BlobVersionSasBuilder.ToSasQueryParameters(*keyCredential);
        verify_blob_delete_version(sasToken);
        create_version();
        sasToken2 = BlobVersionSasBuilder.ToSasQueryParameters(userDelegationKey, accountName);
        verify_blob_delete_version(sasToken2);
      }
    }
  }

}}} // namespace Azure::Storage::Test
