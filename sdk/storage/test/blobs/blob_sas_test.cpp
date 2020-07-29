// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blob_container_client_test.hpp"
#include "blobs/blob_sas_builder.hpp"

namespace Azure { namespace Storage { namespace Test {

  TEST_F(BlobContainerClientTest, BlobSasTest)
  {
    AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = SasProtocol::HttpsAndHtttp;
    accountSasBuilder.StartsOn
        = ToISO8601(std::chrono::system_clock::now() - std::chrono::minutes(5));
    accountSasBuilder.ExpiresOn
        = ToISO8601(std::chrono::system_clock::now() + std::chrono::minutes(60));
    accountSasBuilder.Services = AccountSasServices::Blobs;
    accountSasBuilder.ResourceTypes = AccountSasResource::Object | AccountSasResource::Container;

    std::string blobName = RandomString();
    Blobs::BlobSasBuilder blobSasBuilder;
    blobSasBuilder.Protocol = SasProtocol::HttpsAndHtttp;
    blobSasBuilder.StartsOn = ToISO8601(std::chrono::system_clock::now() - std::chrono::minutes(5));
    blobSasBuilder.ExpiresOn
        = ToISO8601(std::chrono::system_clock::now() + std::chrono::minutes(60));
    blobSasBuilder.ContainerName = m_containerName;
    blobSasBuilder.BlobName = blobName;
    blobSasBuilder.Resource = Blobs::BlobSasResource::Blob;

    Blobs::BlobSasBuilder containerSasBuilder = blobSasBuilder;
    containerSasBuilder.BlobName.clear();
    containerSasBuilder.Resource = Blobs::BlobSasResource::Container;

    auto keyCredential
        = Details::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;
    auto blobServiceClient0
        = Blobs::BlobServiceClient::CreateFromConnectionString(StandardStorageConnectionString());
    auto blobContainerClient0 = blobServiceClient0.GetBlobContainerClient(m_containerName);
    auto blobClient0 = blobContainerClient0.GetAppendBlobClient(blobName);

    auto serviceUri = blobServiceClient0.GetUri();
    auto containerUri = blobContainerClient0.GetUri();
    auto blobUri = blobClient0.GetUri();

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
      EXPECT_NO_THROW(blobContainerClient.ListBlobsFlat());
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
      // TODO: Add test for blob tags
    };

    auto verify_blob_filter = [&](const std::string& sas) {
      unused(sas);
      // TODO: Add test for blob tags
    };

    auto verify_blob_delete_version = [&](const std::string& sas) {
      unused(sas);
      // TODO: Add test for versions
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
      if ((permissions & AccountSasPermissions::DeleteVersion)
          == AccountSasPermissions::DeleteVersion)
      {
        verify_blob_delete_version(sasToken);
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

      if ((permissions & Blobs::BlobSasPermissions::Read) == Blobs::BlobSasPermissions::Read)
      {
        verify_blob_read(sasToken);
      }
      if ((permissions & Blobs::BlobSasPermissions::Write) == Blobs::BlobSasPermissions::Write)
      {
        verify_blob_write(sasToken);
      }
      if ((permissions & Blobs::BlobSasPermissions::Delete) == Blobs::BlobSasPermissions::Delete)
      {
        verify_blob_delete(sasToken);
      }
      if ((permissions & Blobs::BlobSasPermissions::Add) == Blobs::BlobSasPermissions::Add)
      {
        verify_blob_add(sasToken);
      }
      if ((permissions & Blobs::BlobSasPermissions::Create) == Blobs::BlobSasPermissions::Create)
      {
        verify_blob_create(sasToken);
      }
      if ((permissions & Blobs::BlobSasPermissions::Tags) == Blobs::BlobSasPermissions::Tags)
      {
        verify_blob_tags(sasToken);
      }
      if ((permissions & Blobs::BlobSasPermissions::DeleteVersion)
          == Blobs::BlobSasPermissions::DeleteVersion)
      {
        verify_blob_delete_version(sasToken);
      }
    }

    // Expires
    {
      AccountSasBuilder builder2 = accountSasBuilder;
      builder2.SetPermissions(AccountSasPermissions::All);
      builder2.StartsOn = ToISO8601(std::chrono::system_clock::now() - std::chrono::minutes(5));
      builder2.ExpiresOn = ToISO8601(std::chrono::system_clock::now() - std::chrono::minutes(1));
      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      EXPECT_THROW(verify_blob_create(sasToken), StorageError);
    }

    // Without start time
    {
      AccountSasBuilder builder2 = accountSasBuilder;
      builder2.SetPermissions(AccountSasPermissions::All);
      builder2.StartsOn.Reset();
      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      EXPECT_NO_THROW(verify_blob_create(sasToken));
    }

    // IP
    {
      AccountSasBuilder builder2 = accountSasBuilder;
      builder2.SetPermissions(AccountSasPermissions::All);
      builder2.IPRange = "1.1.1.1";
      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      auto blobClient = Blobs::AppendBlobClient(blobUri + sasToken);
      EXPECT_THROW(verify_blob_create(sasToken), StorageError);
      builder2.IPRange = "0.0.0.0-255.255.255.255";
      sasToken = builder2.ToSasQueryParameters(*keyCredential);
      blobClient = Blobs::AppendBlobClient(blobUri + sasToken);
      EXPECT_NO_THROW(verify_blob_create(sasToken));
    }

    // Account SAS Service
    {
      AccountSasBuilder builder2 = accountSasBuilder;
      builder2.SetPermissions(AccountSasPermissions::All);
      builder2.Services = AccountSasServices::Files;
      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      auto blobClient = Blobs::AppendBlobClient(blobUri + sasToken);
      EXPECT_THROW(verify_blob_create(sasToken), StorageError);

      builder2.Services = AccountSasServices::All;
      sasToken = builder2.ToSasQueryParameters(*keyCredential);
      blobClient = Blobs::AppendBlobClient(blobUri + sasToken);
      EXPECT_NO_THROW(verify_blob_create(sasToken));
    }

    // Account SAS Resource Types
    {
      AccountSasBuilder builder2 = accountSasBuilder;
      builder2.SetPermissions(AccountSasPermissions::All);
      builder2.ResourceTypes = AccountSasResource::Service;
      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      auto blobClient = Blobs::AppendBlobClient(blobUri + sasToken);
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
      if ((permissions & Blobs::BlobContainerSasPermissions::Read)
          == Blobs::BlobContainerSasPermissions::Read)
      {
        verify_blob_read(sasToken);
      }
      if ((permissions & Blobs::BlobContainerSasPermissions::Write)
          == Blobs::BlobContainerSasPermissions::Write)
      {
        verify_blob_write(sasToken);
      }
      if ((permissions & Blobs::BlobContainerSasPermissions::Delete)
          == Blobs::BlobContainerSasPermissions::Delete)
      {
        verify_blob_delete(sasToken);
      }
      if ((permissions & Blobs::BlobContainerSasPermissions::List)
          == Blobs::BlobContainerSasPermissions::List)
      {
        verify_blob_list(sasToken);
      }
      if ((permissions & Blobs::BlobContainerSasPermissions::Add)
          == Blobs::BlobContainerSasPermissions::Add)
      {
        verify_blob_add(sasToken);
      }
      if ((permissions & Blobs::BlobContainerSasPermissions::Create)
          == Blobs::BlobContainerSasPermissions::Create)
      {
        verify_blob_create(sasToken);
      }
      if ((permissions & Blobs::BlobContainerSasPermissions::Tags)
          == Blobs::BlobContainerSasPermissions::Tags)
      {
        verify_blob_tags(sasToken);
      }
    }

    blobSasBuilder.SetPermissions(Blobs::BlobSasPermissions::All);
    // Expires
    {
      Blobs::BlobSasBuilder builder2 = blobSasBuilder;
      builder2.StartsOn = ToISO8601(std::chrono::system_clock::now() - std::chrono::minutes(5));
      builder2.ExpiresOn = ToISO8601(std::chrono::system_clock::now() - std::chrono::minutes(1));
      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      EXPECT_THROW(verify_blob_create(sasToken), StorageError);
    }

    // IP
    {
      Blobs::BlobSasBuilder builder2 = blobSasBuilder;
      builder2.IPRange = "0.0.0.0-0.0.0.1";
      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      auto blobClient = Blobs::AppendBlobClient(blobUri + sasToken);
      EXPECT_THROW(verify_blob_create(sasToken), StorageError);
      builder2.IPRange = "0.0.0.0-255.255.255.255";
      sasToken = builder2.ToSasQueryParameters(*keyCredential);
      blobClient = Blobs::AppendBlobClient(blobUri + sasToken);
      EXPECT_NO_THROW(verify_blob_create(sasToken));
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
    }

    blobClient0.Create();
    Blobs::BlobSasBuilder BlobSnapshotSasBuilder = blobSasBuilder;
    BlobSnapshotSasBuilder.Resource = Blobs::BlobSasResource::BlobSnapshot;

    std::string blobSnapshotUri;

    auto verify_blob_snapshot_read = [&](const std::string sas) {
      auto blobSnapshotClient = Blobs::AppendBlobClient(blobSnapshotUri + "&" + sas.substr(1));
      auto downloadedContent = blobSnapshotClient.Download();
      EXPECT_TRUE(ReadBodyStream(downloadedContent->BodyStream).empty());
    };

    auto verify_blob_snapshot_delete = [&](const std::string sas) {
      auto blobSnapshotClient = Blobs::AppendBlobClient(blobSnapshotUri + "&" + sas.substr(1));
      EXPECT_NO_THROW(blobSnapshotClient.Delete());
    };

    for (auto permissions : {
             Blobs::BlobSasPermissions::Read | Blobs::BlobSasPermissions::Delete,
             Blobs::BlobSasPermissions::Read,
             Blobs::BlobSasPermissions::Delete,
         })
    {
      std::string snapshot = blobClient0.CreateSnapshot()->Snapshot;
      BlobSnapshotSasBuilder.Snapshot = snapshot;
      blobSnapshotUri = blobClient0.WithSnapshot(snapshot).GetUri();
      BlobSnapshotSasBuilder.SetPermissions(permissions);
      auto sasToken = BlobSnapshotSasBuilder.ToSasQueryParameters(*keyCredential);

      if ((permissions & Blobs::BlobSasPermissions::Read) == Blobs::BlobSasPermissions::Read)
      {
        verify_blob_snapshot_read(sasToken);
      }
      if ((permissions & Blobs::BlobSasPermissions::Delete) == Blobs::BlobSasPermissions::Delete)
      {
        verify_blob_snapshot_delete(sasToken);
      }
    }
  }

}}} // namespace Azure::Storage::Test
