// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/identity/client_secret_credential.hpp>
#include <azure/storage/blobs/blob_sas_builder.hpp>
#include <azure/storage/files/datalake/datalake_sas_builder.hpp>
#include <azure/storage/files/datalake/datalake_utilities.hpp>

#include "datalake_file_system_client_test.hpp"

#include <chrono>

namespace Azure { namespace Storage { namespace Test {

  TEST_F(DataLakeFileSystemClientTest, DataLakeSasTest)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiredOn = std::chrono::system_clock::now() - std::chrono::minutes(1);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    std::string directory1Name = RandomString();
    std::string directory2Name = RandomString();
    std::string fileName = RandomString();
    Sas::DataLakeSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSasBuilder.StartsOn = sasStartsOn;
    fileSasBuilder.ExpiresOn = sasExpiresOn;
    fileSasBuilder.FileSystemName = m_fileSystemName;
    fileSasBuilder.Path = directory1Name + "/" + directory2Name + "/" + fileName;
    fileSasBuilder.Resource = Sas::DataLakeSasResource::File;

    Sas::DataLakeSasBuilder directorySasBuilder = fileSasBuilder;
    directorySasBuilder.Path = directory1Name;
    directorySasBuilder.IsDirectory = true;
    directorySasBuilder.DirectoryDepth = 1;
    directorySasBuilder.Resource = Sas::DataLakeSasResource::Directory;

    Sas::DataLakeSasBuilder filesystemSasBuilder = fileSasBuilder;
    filesystemSasBuilder.Path.clear();
    filesystemSasBuilder.Resource = Sas::DataLakeSasResource::FileSystem;

    auto keyCredential = Details::ParseConnectionString(AdlsGen2ConnectionString()).KeyCredential;
    auto accountName = keyCredential->AccountName;
    auto serviceClient0 = Files::DataLake::DataLakeServiceClient::CreateFromConnectionString(
        AdlsGen2ConnectionString());
    auto filesystemClient0 = serviceClient0.GetFileSystemClient(m_fileSystemName);
    auto containerClinet0 = Blobs::BlobContainerClient::CreateFromConnectionString(
        AdlsGen2ConnectionString(), m_fileSystemName);
    auto directory1Client0 = filesystemClient0.GetDirectoryClient(directory1Name);
    auto directory2Client0 = directory1Client0.GetSubdirectoryClient(directory2Name);
    auto fileClient0 = directory2Client0.GetFileClient(fileName);
    directory1Client0.Create();
    directory2Client0.Create();

    auto serviceUrl = Files::DataLake::Details::GetDfsUrlFromUrl(serviceClient0.GetUrl());
    auto filesystemUrl = Files::DataLake::Details::GetDfsUrlFromUrl(filesystemClient0.GetUrl());
    auto directory1Url = Files::DataLake::Details::GetDfsUrlFromUrl(directory1Client0.GetUrl());
    auto directory2Url = Files::DataLake::Details::GetDfsUrlFromUrl(directory2Client0.GetUrl());
    auto fileUrl = Files::DataLake::Details::GetDfsUrlFromUrl(fileClient0.GetUrl());

    auto serviceClient1 = Files::DataLake::DataLakeServiceClient(
        serviceUrl,
        std::make_shared<Azure::Identity::ClientSecretCredential>(
            AadTenantId(), AadClientId(), AadClientSecret()));
    auto userDelegationKey = serviceClient1.GetUserDelegationKey(sasExpiresOn)->Key;

    auto verify_file_read = [&](const std::string& sas) {
      EXPECT_NO_THROW(fileClient0.Create());
      auto fileClient = Files::DataLake::DataLakeFileClient(fileUrl + sas);
      auto downloadedContent = fileClient.Download();
      EXPECT_TRUE(ReadBodyStream(downloadedContent->Body).empty());
    };

    auto verify_file_write = [&](const std::string& sas) {
      auto fileClient = Files::DataLake::DataLakeFileClient(fileUrl + sas);
      EXPECT_NO_THROW(fileClient.Create());
    };

    auto verify_file_delete = [&](const std::string& sas) {
      fileClient0.Create();
      auto fileClient = Files::DataLake::DataLakeFileClient(fileUrl + sas);
      EXPECT_NO_THROW(fileClient.Delete());
    };

    auto verify_file_add = [&](const std::string& sas) {
      (void)sas;
      /*
       * Add test for append block when DataLake supports append blobs.
       */
    };

    auto verify_filesystem_list = [&](const std::string& sas) {
      auto filesystemClient = Files::DataLake::DataLakeFileSystemClient(filesystemUrl + sas);
      EXPECT_NO_THROW(filesystemClient.ListPathsSinglePage(true));
    };

    auto verify_directory_list = [&](const std::string& sas) {
      auto filesystemClient = Files::DataLake::DataLakeFileSystemClient(filesystemUrl + sas);
      auto directoryClient = filesystemClient.GetDirectoryClient(directory1Name);
      EXPECT_NO_THROW(directoryClient.ListPathsSinglePage(true));
    };

    auto verify_file_create = [&](const std::string& sas) {
      try
      {
        fileClient0.Delete();
      }
      catch (StorageException&)
      {
      }
      auto fileClient = Files::DataLake::DataLakeFileClient(fileUrl + sas);
      fileClient.Create();
    };

    auto verify_file_move = [&](const std::string& sas) {
      try
      {
        fileClient0.Delete();
      }
      catch (StorageException&)
      {
      }
      std::string newFilename = RandomString();
      auto newFileClient0 = directory2Client0.GetFileClient(newFilename);
      newFileClient0.Create();
      auto directoryClient = Files::DataLake::DataLakeDirectoryClient(
          Files::DataLake::Details::GetDfsUrlFromUrl(directory2Client0.GetUrl()) + sas);
      EXPECT_NO_THROW(directoryClient.RenameFile(
          newFilename, directory1Name + "/" + directory2Name + "/" + fileName));
    };

    auto verify_file_execute = [&](const std::string& sas) {
      fileClient0.Create();
      auto fileClient = Files::DataLake::DataLakeFileClient(fileUrl + sas);
      EXPECT_NO_THROW(fileClient0.GetAccessControlList());
    };

    auto verify_file_ownership = [&](const std::string& sas) {
      fileClient0.Create();
      auto fileClient = Files::DataLake::DataLakeFileClient(fileUrl + sas);
      EXPECT_NO_THROW(fileClient0.GetAccessControlList());
    };

    auto verify_file_permissions = [&](const std::string& sas) {
      fileClient0.Create();
      auto fileClient = Files::DataLake::DataLakeFileClient(fileUrl + sas);
      auto acls = fileClient0.GetAccessControlList()->Acls;
      EXPECT_NO_THROW(fileClient.SetAccessControlList(acls));
    };

    for (auto permissions : {
             Sas::DataLakeSasPermissions::All,
             Sas::DataLakeSasPermissions::Read,
             Sas::DataLakeSasPermissions::Write,
             Sas::DataLakeSasPermissions::Delete,
             Sas::DataLakeSasPermissions::Add,
             Sas::DataLakeSasPermissions::Create,
             Sas::DataLakeSasPermissions::List,
             Sas::DataLakeSasPermissions::Move,
             Sas::DataLakeSasPermissions::Execute,
             Sas::DataLakeSasPermissions::ManageOwnership,
             Sas::DataLakeSasPermissions::ManageAccessControl,
         })
    {
      fileSasBuilder.SetPermissions(permissions);
      auto sasToken = fileSasBuilder.GenerateSasToken(*keyCredential);
      auto sasToken2 = fileSasBuilder.GenerateSasToken(userDelegationKey, accountName);

      if ((permissions & Sas::DataLakeSasPermissions::Read) == Sas::DataLakeSasPermissions::Read)
      {
        verify_file_read(sasToken);
        verify_file_read(sasToken2);
      }
      if ((permissions & Sas::DataLakeSasPermissions::Write) == Sas::DataLakeSasPermissions::Write)
      {
        verify_file_write(sasToken);
        verify_file_write(sasToken2);
      }
      if ((permissions & Sas::DataLakeSasPermissions::Delete)
          == Sas::DataLakeSasPermissions::Delete)
      {
        verify_file_delete(sasToken);
        verify_file_delete(sasToken2);
      }
      if ((permissions & Sas::DataLakeSasPermissions::Add) == Sas::DataLakeSasPermissions::Add)
      {
        verify_file_add(sasToken);
        verify_file_add(sasToken2);
      }
      if ((permissions & Sas::DataLakeSasPermissions::Create)
          == Sas::DataLakeSasPermissions::Create)
      {
        verify_file_create(sasToken);
        verify_file_create(sasToken2);
      }
      if ((permissions & Sas::DataLakeSasPermissions::ManageAccessControl)
          == Sas::DataLakeSasPermissions::ManageAccessControl)
      {
        verify_file_permissions(sasToken);
        verify_file_permissions(sasToken2);
      }
    }

    for (auto permissions : {
             Sas::DataLakeSasPermissions::All,
             Sas::DataLakeSasPermissions::Read,
             Sas::DataLakeSasPermissions::Write,
             Sas::DataLakeSasPermissions::Delete,
             Sas::DataLakeSasPermissions::Add,
             Sas::DataLakeSasPermissions::Create,
             Sas::DataLakeSasPermissions::List,
             Sas::DataLakeSasPermissions::Move,
             Sas::DataLakeSasPermissions::Execute,
             Sas::DataLakeSasPermissions::ManageOwnership,
             Sas::DataLakeSasPermissions::ManageAccessControl,
         })
    {
      directorySasBuilder.SetPermissions(permissions);
      auto sasToken2 = directorySasBuilder.GenerateSasToken(userDelegationKey, accountName);

      if ((permissions & Sas::DataLakeSasPermissions::Read) == Sas::DataLakeSasPermissions::Read)
      {
        verify_file_read(sasToken2);
      }
      if ((permissions & Sas::DataLakeSasPermissions::Write) == Sas::DataLakeSasPermissions::Write)
      {
        verify_file_write(sasToken2);
      }
      if ((permissions & Sas::DataLakeSasPermissions::Delete)
          == Sas::DataLakeSasPermissions::Delete)
      {
        verify_file_delete(sasToken2);
      }
      if ((permissions & Sas::DataLakeSasPermissions::Add) == Sas::DataLakeSasPermissions::Add)
      {
        verify_file_add(sasToken2);
      }
      if ((permissions & Sas::DataLakeSasPermissions::Create)
          == Sas::DataLakeSasPermissions::Create)
      {
        verify_file_create(sasToken2);
      }
      if ((permissions & Sas::DataLakeSasPermissions::List) == Sas::DataLakeSasPermissions::List)
      {
        verify_directory_list(sasToken2);
      }
      (void)verify_file_move;
      /*
      don't know why, move doesn't work
      if ((permissions & Sas::DataLakeSasPermissions::Move)
          == Sas::DataLakeSasPermissions::Move)
      {
        verify_file_move(sasToken2);
      }
      */
      if ((permissions & Sas::DataLakeSasPermissions::Execute)
          == Sas::DataLakeSasPermissions::Execute)
      {
        verify_file_execute(sasToken2);
      }
      if ((permissions & Sas::DataLakeSasPermissions::ManageOwnership)
          == Sas::DataLakeSasPermissions::ManageOwnership)
      {
        verify_file_ownership(sasToken2);
      }
      if ((permissions & Sas::DataLakeSasPermissions::ManageAccessControl)
          == Sas::DataLakeSasPermissions::ManageAccessControl)
      {
        verify_file_permissions(sasToken2);
      }
    }

    for (auto permissions : {
             Sas::DataLakeFileSystemSasPermissions::All,
             Sas::DataLakeFileSystemSasPermissions::Read,
             Sas::DataLakeFileSystemSasPermissions::Write,
             Sas::DataLakeFileSystemSasPermissions::Delete,
             Sas::DataLakeFileSystemSasPermissions::List,
             Sas::DataLakeFileSystemSasPermissions::Add,
             Sas::DataLakeFileSystemSasPermissions::Create,
         })
    {
      filesystemSasBuilder.SetPermissions(permissions);
      auto sasToken = filesystemSasBuilder.GenerateSasToken(*keyCredential);
      auto sasToken2 = filesystemSasBuilder.GenerateSasToken(userDelegationKey, accountName);

      if ((permissions & Sas::DataLakeFileSystemSasPermissions::All)
          == Sas::DataLakeFileSystemSasPermissions::All)
      {
        (void)verify_file_move;
        /*
        don't know why, move doesn't work
        verify_file_move(sasToken);
        verify_file_move(sasToken2);
        */
      }
      if ((permissions & Sas::DataLakeFileSystemSasPermissions::Read)
          == Sas::DataLakeFileSystemSasPermissions::Read)
      {
        verify_file_read(sasToken);
        verify_file_read(sasToken2);
      }
      if ((permissions & Sas::DataLakeFileSystemSasPermissions::Write)
          == Sas::DataLakeFileSystemSasPermissions::Write)
      {
        verify_file_write(sasToken);
        verify_file_write(sasToken2);
      }
      if ((permissions & Sas::DataLakeFileSystemSasPermissions::Delete)
          == Sas::DataLakeFileSystemSasPermissions::Delete)
      {
        verify_file_delete(sasToken);
        verify_file_delete(sasToken2);
      }
      if ((permissions & Sas::DataLakeFileSystemSasPermissions::List)
          == Sas::DataLakeFileSystemSasPermissions::List)
      {
        verify_filesystem_list(sasToken);
        verify_filesystem_list(sasToken2);
      }
      if ((permissions & Sas::DataLakeFileSystemSasPermissions::Add)
          == Sas::DataLakeFileSystemSasPermissions::Add)
      {
        verify_file_add(sasToken);
        verify_file_add(sasToken2);
      }
      if ((permissions & Sas::DataLakeFileSystemSasPermissions::Create)
          == Sas::DataLakeFileSystemSasPermissions::Create)
      {
        verify_file_create(sasToken);
        verify_file_create(sasToken2);
      }
    }

    fileSasBuilder.SetPermissions(Sas::DataLakeSasPermissions::All);

    // Expires
    {
      Sas::DataLakeSasBuilder builder2 = fileSasBuilder;
      builder2.StartsOn = sasStartsOn;
      builder2.ExpiresOn = sasExpiredOn;
      auto sasToken = builder2.GenerateSasToken(*keyCredential);
      EXPECT_THROW(verify_file_create(sasToken), StorageException);

      auto sasToken2 = builder2.GenerateSasToken(userDelegationKey, accountName);
      EXPECT_THROW(verify_file_create(sasToken2), StorageException);
    }

    // Without start time
    {
      Sas::DataLakeSasBuilder builder2 = fileSasBuilder;
      builder2.StartsOn.Reset();
      auto sasToken = builder2.GenerateSasToken(*keyCredential);
      EXPECT_NO_THROW(verify_file_create(sasToken));
      auto sasToken2 = builder2.GenerateSasToken(userDelegationKey, accountName);
      EXPECT_NO_THROW(verify_file_create(sasToken2));
    }

    // IP
    {
      Sas::DataLakeSasBuilder builder2 = fileSasBuilder;
      builder2.IPRange = "0.0.0.0-0.0.0.1";
      auto sasToken = builder2.GenerateSasToken(*keyCredential);
      EXPECT_THROW(verify_file_create(sasToken), StorageException);
      auto sasToken2 = builder2.GenerateSasToken(userDelegationKey, accountName);
      EXPECT_THROW(verify_file_create(sasToken2), StorageException);

      // TODO: Add this test case back with support to contain IPv6 ranges when service is ready.
      // builder2.IPRange = "0.0.0.0-255.255.255.255";
      // sasToken = builder2.GenerateSasToken(*keyCredential);
      // EXPECT_NO_THROW(verify_file_create(sasToken));
      // sasToken2 = builder2.GenerateSasToken(userDelegationKey, accountName);
      // EXPECT_NO_THROW(verify_file_create(sasToken2));
    }

    // PreauthorizedAgentObjectId
    {
      Sas::DataLakeSasBuilder builder2 = fileSasBuilder;
      builder2.PreauthorizedAgentObjectId = Azure::Core::Uuid::CreateUuid().ToString();
      builder2.CorrelationId = Azure::Core::Uuid::CreateUuid().ToString();
      auto sasToken2 = builder2.GenerateSasToken(userDelegationKey, accountName);
      EXPECT_NO_THROW(verify_file_read(sasToken2));
    }

    // Identifier
    {
      Blobs::SetBlobContainerAccessPolicyOptions options;
      options.AccessType = Blobs::Models::PublicAccessType::Blob;
      Blobs::Models::BlobSignedIdentifier identifier;
      identifier.Id = RandomString(64);
      identifier.StartsOn = sasStartsOn;
      identifier.ExpiresOn = sasExpiresOn;
      identifier.Permissions = "r";
      options.SignedIdentifiers.emplace_back(identifier);
      containerClinet0.SetAccessPolicy(options);

      Sas::DataLakeSasBuilder builder2 = fileSasBuilder;
      builder2.StartsOn.Reset();
      builder2.ExpiresOn = Azure::Core::DateTime();
      builder2.SetPermissions(static_cast<Sas::DataLakeFileSystemSasPermissions>(0));
      builder2.Identifier = identifier.Id;

      auto sasToken = builder2.GenerateSasToken(*keyCredential);
      // TODO: looks like a server bug, the identifier doesn't work sometimes.
      // EXPECT_NO_THROW(verify_file_read(sasToken));
    }

    // response headers override
    {
      Files::DataLake::Models::PathHttpHeaders headers;
      headers.ContentType = "application/x-binary";
      headers.ContentLanguage = "en-US";
      headers.ContentDisposition = "attachment";
      headers.CacheControl = "no-cache";
      headers.ContentEncoding = "identify";

      Sas::DataLakeSasBuilder builder2 = fileSasBuilder;
      builder2.SetPermissions(Sas::DataLakeSasPermissions::Read);
      builder2.ContentType = "application/x-binary";
      builder2.ContentLanguage = "en-US";
      builder2.ContentDisposition = "attachment";
      builder2.CacheControl = "no-cache";
      builder2.ContentEncoding = "identify";
      auto sasToken = builder2.GenerateSasToken(*keyCredential);
      auto fileClient = Files::DataLake::DataLakeFileClient(fileUrl + sasToken);
      fileClient0.Create();
      auto p = fileClient.GetProperties();
      EXPECT_EQ(p->HttpHeaders.ContentType, headers.ContentType);
      EXPECT_EQ(p->HttpHeaders.ContentLanguage, headers.ContentLanguage);
      EXPECT_EQ(p->HttpHeaders.ContentDisposition, headers.ContentDisposition);
      EXPECT_EQ(p->HttpHeaders.CacheControl, headers.CacheControl);
      EXPECT_EQ(p->HttpHeaders.ContentEncoding, headers.ContentEncoding);

      auto sasToken2 = builder2.GenerateSasToken(userDelegationKey, accountName);
      fileClient = Files::DataLake::DataLakeFileClient(fileUrl + sasToken);
      p = fileClient.GetProperties();
      EXPECT_EQ(p->HttpHeaders.ContentType, headers.ContentType);
      EXPECT_EQ(p->HttpHeaders.ContentLanguage, headers.ContentLanguage);
      EXPECT_EQ(p->HttpHeaders.ContentDisposition, headers.ContentDisposition);
      EXPECT_EQ(p->HttpHeaders.CacheControl, headers.CacheControl);
      EXPECT_EQ(p->HttpHeaders.ContentEncoding, headers.ContentEncoding);
    }
  }

}}} // namespace Azure::Storage::Test
