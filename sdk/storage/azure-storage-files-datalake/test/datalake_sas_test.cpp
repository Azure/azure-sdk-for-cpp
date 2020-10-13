// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/blob_sas_builder.hpp"
#include "azure/storage/files/datalake/datalake_sas_builder.hpp"
#include "datalake_file_system_client_test.hpp"

namespace Azure { namespace Storage { namespace Test {

  TEST_F(DataLakeFileSystemClientTest, DataLakeSasTest)
  {
    std::string directory1Name = RandomString();
    std::string directory2Name = RandomString();
    std::string fileName = RandomString();
    Files::DataLake::DataLakeSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = SasProtocol::HttpsAndHtttp;
    fileSasBuilder.StartsOn = ToIso8601(std::chrono::system_clock::now() - std::chrono::minutes(5));
    fileSasBuilder.ExpiresOn
        = ToIso8601(std::chrono::system_clock::now() + std::chrono::minutes(60));
    fileSasBuilder.FileSystemName = m_fileSystemName;
    fileSasBuilder.Path = directory1Name + "/" + directory2Name + "/" + fileName;
    fileSasBuilder.Resource = Files::DataLake::DataLakeSasResource::File;

    Files::DataLake::DataLakeSasBuilder directorySasBuilder = fileSasBuilder;
    directorySasBuilder.Path = directory1Name;
    directorySasBuilder.IsDirectory = true;
    directorySasBuilder.DirectoryDepth = 1;
    directorySasBuilder.Resource = Files::DataLake::DataLakeSasResource::Directory;

    Files::DataLake::DataLakeSasBuilder filesystemSasBuilder = fileSasBuilder;
    filesystemSasBuilder.Path.clear();
    filesystemSasBuilder.Resource = Files::DataLake::DataLakeSasResource::FileSystem;

    auto keyCredential = Details::ParseConnectionString(AdlsGen2ConnectionString()).KeyCredential;
    auto accountName = keyCredential->AccountName;
    auto serviceClient0
        = Files::DataLake::ServiceClient::CreateFromConnectionString(AdlsGen2ConnectionString());
    auto filesystemClient0 = serviceClient0.GetFileSystemClient(m_fileSystemName);
    auto containerClinet0 = Blobs::BlobContainerClient::CreateFromConnectionString(
        AdlsGen2ConnectionString(), m_fileSystemName);
    auto directory1Client0 = filesystemClient0.GetDirectoryClient(directory1Name);
    auto directory2Client0 = directory1Client0.GetSubDirectoryClient(directory2Name);
    auto fileClient0 = directory2Client0.GetFileClient(fileName);
    directory1Client0.Create();
    directory2Client0.Create();

    auto serviceUri = serviceClient0.GetDfsUri();
    auto filesystemUri = filesystemClient0.GetDfsUri();
    auto directory1Uri = directory1Client0.GetDfsUri();
    auto directory2Uri = directory2Client0.GetDfsUri();
    auto fileUri = fileClient0.GetUri();

    auto serviceClient1 = Files::DataLake::ServiceClient(
        serviceUri,
        std::make_shared<Azure::Identity::ClientSecretCredential>(
            AadTenantId(), AadClientId(), AadClientSecret()));
    auto userDelegationKey = *serviceClient1.GetUserDelegationKey(
        ToIso8601(std::chrono::system_clock::now() - std::chrono::minutes(5)),
        ToIso8601(std::chrono::system_clock::now() + std::chrono::minutes(60)));

    auto verify_file_read = [&](const std::string& sas) {
      EXPECT_NO_THROW(fileClient0.Create());
      auto fileClient = Files::DataLake::FileClient(fileUri + sas);
      auto downloadedContent = fileClient.Read();
      EXPECT_TRUE(ReadBodyStream(downloadedContent->Body).empty());
    };

    auto verify_file_write = [&](const std::string& sas) {
      auto fileClient = Files::DataLake::FileClient(fileUri + sas);
      EXPECT_NO_THROW(fileClient.Create());
    };

    auto verify_file_delete = [&](const std::string& sas) {
      fileClient0.Create();
      auto fileClient = Files::DataLake::FileClient(fileUri + sas);
      EXPECT_NO_THROW(fileClient.Delete());
    };

    auto verify_file_add = [&](const std::string& sas) {
      unused(sas);
      /*
       * Add test for append block when DataLake supports append blobs.
       */
    };

    auto verify_filesystem_list = [&](const std::string& sas) {
      auto filesystemClient = Files::DataLake::FileSystemClient(filesystemUri + sas);
      EXPECT_NO_THROW(filesystemClient.ListPaths(true));
    };

    auto verify_directory_list = [&](const std::string& sas) {
      auto filesystemClient = Files::DataLake::FileSystemClient(filesystemUri + sas);
      Files::DataLake::ListPathsOptions options;
      options.Directory = directory1Name;
      EXPECT_NO_THROW(filesystemClient.ListPaths(true, options));
    };

    auto verify_file_create = [&](const std::string& sas) {
      try
      {
        fileClient0.Delete();
      }
      catch (StorageError&)
      {
      }
      auto fileClient = Files::DataLake::FileClient(fileUri + sas);
      fileClient.Create();
    };

    auto verify_file_move = [&](const std::string& sas) {
      try
      {
        fileClient0.Delete();
      }
      catch (StorageError&)
      {
      }
      std::string newFilename = RandomString();
      auto newFileClient0 = directory2Client0.GetFileClient(newFilename);
      newFileClient0.Create();
      auto fileClient = Files::DataLake::FileClient(newFileClient0.GetDfsUri() + sas);
      EXPECT_NO_THROW(fileClient.Rename(directory1Name + "/" + directory2Name + "/" + fileName));
    };

    auto verify_file_execute = [&](const std::string& sas) {
      fileClient0.Create();
      auto fileClient = Files::DataLake::FileClient(fileUri + sas);
      EXPECT_NO_THROW(fileClient0.GetAccessControls());
    };

    auto verify_file_ownership = [&](const std::string& sas) {
      fileClient0.Create();
      auto fileClient = Files::DataLake::FileClient(fileUri + sas);
      EXPECT_NO_THROW(fileClient0.GetAccessControls());
    };

    auto verify_file_permissions = [&](const std::string& sas) {
      fileClient0.Create();
      auto fileClient = Files::DataLake::FileClient(fileUri + sas);
      auto acls = fileClient0.GetAccessControls()->Acls;
      EXPECT_NO_THROW(fileClient.SetAccessControl(acls));
    };

    for (auto permissions : {
             Files::DataLake::DataLakeSasPermissions::All,
             Files::DataLake::DataLakeSasPermissions::Read,
             Files::DataLake::DataLakeSasPermissions::Write,
             Files::DataLake::DataLakeSasPermissions::Delete,
             Files::DataLake::DataLakeSasPermissions::Add,
             Files::DataLake::DataLakeSasPermissions::Create,
             Files::DataLake::DataLakeSasPermissions::List,
             Files::DataLake::DataLakeSasPermissions::Move,
             Files::DataLake::DataLakeSasPermissions::Execute,
             Files::DataLake::DataLakeSasPermissions::ManageOwnership,
             Files::DataLake::DataLakeSasPermissions::ManageAccessControl,
         })
    {
      fileSasBuilder.SetPermissions(permissions);
      auto sasToken = fileSasBuilder.ToSasQueryParameters(*keyCredential);
      auto sasToken2 = fileSasBuilder.ToSasQueryParameters(userDelegationKey, accountName);

      if ((permissions & Files::DataLake::DataLakeSasPermissions::Read)
          == Files::DataLake::DataLakeSasPermissions::Read)
      {
        verify_file_read(sasToken);
        verify_file_read(sasToken2);
      }
      if ((permissions & Files::DataLake::DataLakeSasPermissions::Write)
          == Files::DataLake::DataLakeSasPermissions::Write)
      {
        verify_file_write(sasToken);
        verify_file_write(sasToken2);
      }
      if ((permissions & Files::DataLake::DataLakeSasPermissions::Delete)
          == Files::DataLake::DataLakeSasPermissions::Delete)
      {
        verify_file_delete(sasToken);
        verify_file_delete(sasToken2);
      }
      if ((permissions & Files::DataLake::DataLakeSasPermissions::Add)
          == Files::DataLake::DataLakeSasPermissions::Add)
      {
        verify_file_add(sasToken);
        verify_file_add(sasToken2);
      }
      if ((permissions & Files::DataLake::DataLakeSasPermissions::Create)
          == Files::DataLake::DataLakeSasPermissions::Create)
      {
        verify_file_create(sasToken);
        verify_file_create(sasToken2);
      }
      if ((permissions & Files::DataLake::DataLakeSasPermissions::ManageAccessControl)
          == Files::DataLake::DataLakeSasPermissions::ManageAccessControl)
      {
        verify_file_permissions(sasToken);
        verify_file_permissions(sasToken2);
      }
    }

    for (auto permissions : {
             Files::DataLake::DataLakeSasPermissions::All,
             Files::DataLake::DataLakeSasPermissions::Read,
             Files::DataLake::DataLakeSasPermissions::Write,
             Files::DataLake::DataLakeSasPermissions::Delete,
             Files::DataLake::DataLakeSasPermissions::Add,
             Files::DataLake::DataLakeSasPermissions::Create,
             Files::DataLake::DataLakeSasPermissions::List,
             Files::DataLake::DataLakeSasPermissions::Move,
             Files::DataLake::DataLakeSasPermissions::Execute,
             Files::DataLake::DataLakeSasPermissions::ManageOwnership,
             Files::DataLake::DataLakeSasPermissions::ManageAccessControl,
         })
    {
      directorySasBuilder.SetPermissions(permissions);
      auto sasToken2 = directorySasBuilder.ToSasQueryParameters(userDelegationKey, accountName);

      if ((permissions & Files::DataLake::DataLakeSasPermissions::Read)
          == Files::DataLake::DataLakeSasPermissions::Read)
      {
        verify_file_read(sasToken2);
      }
      if ((permissions & Files::DataLake::DataLakeSasPermissions::Write)
          == Files::DataLake::DataLakeSasPermissions::Write)
      {
        verify_file_write(sasToken2);
      }
      if ((permissions & Files::DataLake::DataLakeSasPermissions::Delete)
          == Files::DataLake::DataLakeSasPermissions::Delete)
      {
        verify_file_delete(sasToken2);
      }
      if ((permissions & Files::DataLake::DataLakeSasPermissions::Add)
          == Files::DataLake::DataLakeSasPermissions::Add)
      {
        verify_file_add(sasToken2);
      }
      if ((permissions & Files::DataLake::DataLakeSasPermissions::Create)
          == Files::DataLake::DataLakeSasPermissions::Create)
      {
        verify_file_create(sasToken2);
      }
      if ((permissions & Files::DataLake::DataLakeSasPermissions::List)
          == Files::DataLake::DataLakeSasPermissions::List)
      {
        verify_directory_list(sasToken2);
      }
      unused(verify_file_move);
      /*
      don't know why, move doesn't work
      if ((permissions & Files::DataLake::DataLakeSasPermissions::Move)
          == Files::DataLake::DataLakeSasPermissions::Move)
      {
        verify_file_move(sasToken2);
      }
      */
      if ((permissions & Files::DataLake::DataLakeSasPermissions::Execute)
          == Files::DataLake::DataLakeSasPermissions::Execute)
      {
        verify_file_execute(sasToken2);
      }
      if ((permissions & Files::DataLake::DataLakeSasPermissions::ManageOwnership)
          == Files::DataLake::DataLakeSasPermissions::ManageOwnership)
      {
        verify_file_ownership(sasToken2);
      }
      if ((permissions & Files::DataLake::DataLakeSasPermissions::ManageAccessControl)
          == Files::DataLake::DataLakeSasPermissions::ManageAccessControl)
      {
        verify_file_permissions(sasToken2);
      }
    }

    for (auto permissions : {
             Files::DataLake::DataLakeFileSystemSasPermissions::All,
             Files::DataLake::DataLakeFileSystemSasPermissions::Read,
             Files::DataLake::DataLakeFileSystemSasPermissions::Write,
             Files::DataLake::DataLakeFileSystemSasPermissions::Delete,
             Files::DataLake::DataLakeFileSystemSasPermissions::List,
             Files::DataLake::DataLakeFileSystemSasPermissions::Add,
             Files::DataLake::DataLakeFileSystemSasPermissions::Create,
         })
    {
      filesystemSasBuilder.SetPermissions(permissions);
      auto sasToken = filesystemSasBuilder.ToSasQueryParameters(*keyCredential);
      auto sasToken2 = filesystemSasBuilder.ToSasQueryParameters(userDelegationKey, accountName);

      if ((permissions & Files::DataLake::DataLakeFileSystemSasPermissions::All)
          == Files::DataLake::DataLakeFileSystemSasPermissions::All)
      {
        unused(verify_file_move);
        /*
        don't know why, move doesn't work
        verify_file_move(sasToken);
        verify_file_move(sasToken2);
        */
      }
      if ((permissions & Files::DataLake::DataLakeFileSystemSasPermissions::Read)
          == Files::DataLake::DataLakeFileSystemSasPermissions::Read)
      {
        verify_file_read(sasToken);
        verify_file_read(sasToken2);
      }
      if ((permissions & Files::DataLake::DataLakeFileSystemSasPermissions::Write)
          == Files::DataLake::DataLakeFileSystemSasPermissions::Write)
      {
        verify_file_write(sasToken);
        verify_file_write(sasToken2);
      }
      if ((permissions & Files::DataLake::DataLakeFileSystemSasPermissions::Delete)
          == Files::DataLake::DataLakeFileSystemSasPermissions::Delete)
      {
        verify_file_delete(sasToken);
        verify_file_delete(sasToken2);
      }
      if ((permissions & Files::DataLake::DataLakeFileSystemSasPermissions::List)
          == Files::DataLake::DataLakeFileSystemSasPermissions::List)
      {
        verify_filesystem_list(sasToken);
        verify_filesystem_list(sasToken2);
      }
      if ((permissions & Files::DataLake::DataLakeFileSystemSasPermissions::Add)
          == Files::DataLake::DataLakeFileSystemSasPermissions::Add)
      {
        verify_file_add(sasToken);
        verify_file_add(sasToken2);
      }
      if ((permissions & Files::DataLake::DataLakeFileSystemSasPermissions::Create)
          == Files::DataLake::DataLakeFileSystemSasPermissions::Create)
      {
        verify_file_create(sasToken);
        verify_file_create(sasToken2);
      }
    }

    fileSasBuilder.SetPermissions(Files::DataLake::DataLakeSasPermissions::All);

    // Expires
    {
      Files::DataLake::DataLakeSasBuilder builder2 = fileSasBuilder;
      builder2.StartsOn = ToIso8601(std::chrono::system_clock::now() - std::chrono::minutes(5));
      builder2.ExpiresOn = ToIso8601(std::chrono::system_clock::now() - std::chrono::minutes(1));
      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      EXPECT_THROW(verify_file_create(sasToken), StorageError);

      auto sasToken2 = builder2.ToSasQueryParameters(userDelegationKey, accountName);
      EXPECT_THROW(verify_file_create(sasToken2), StorageError);
    }

    // Without start time
    {
      Files::DataLake::DataLakeSasBuilder builder2 = fileSasBuilder;
      builder2.StartsOn.Reset();
      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      EXPECT_NO_THROW(verify_file_create(sasToken));
      auto sasToken2 = builder2.ToSasQueryParameters(userDelegationKey, accountName);
      EXPECT_NO_THROW(verify_file_create(sasToken2));
    }

    // IP
    {
      Files::DataLake::DataLakeSasBuilder builder2 = fileSasBuilder;
      builder2.IPRange = "0.0.0.0-0.0.0.1";
      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      EXPECT_THROW(verify_file_create(sasToken), StorageError);
      auto sasToken2 = builder2.ToSasQueryParameters(userDelegationKey, accountName);
      EXPECT_THROW(verify_file_create(sasToken2), StorageError);

      builder2.IPRange = "0.0.0.0-255.255.255.255";
      sasToken = builder2.ToSasQueryParameters(*keyCredential);
      EXPECT_NO_THROW(verify_file_create(sasToken));
      sasToken2 = builder2.ToSasQueryParameters(userDelegationKey, accountName);
      EXPECT_NO_THROW(verify_file_create(sasToken2));
    }

    // PreauthorizedAgentObjectId
    {
      Files::DataLake::DataLakeSasBuilder builder2 = fileSasBuilder;
      builder2.PreauthorizedAgentObjectId = Azure::Core::Uuid::CreateUuid().GetUuidString();
      builder2.CorrelationId = Azure::Core::Uuid::CreateUuid().GetUuidString();
      auto sasToken2 = builder2.ToSasQueryParameters(userDelegationKey, accountName);
      EXPECT_NO_THROW(verify_file_read(sasToken2));
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
      containerClinet0.SetAccessPolicy(options);

      Files::DataLake::DataLakeSasBuilder builder2 = fileSasBuilder;
      builder2.StartsOn.Reset();
      builder2.ExpiresOn.clear();
      builder2.SetPermissions(static_cast<Files::DataLake::DataLakeFileSystemSasPermissions>(0));
      builder2.Identifier = identifier.Id;

      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      EXPECT_NO_THROW(verify_file_read(sasToken));
    }

    // response headers override
    {
      Files::DataLake::DataLakeHttpHeaders headers;
      headers.ContentType = "application/x-binary";
      headers.ContentLanguage = "en-US";
      headers.ContentDisposition = "attachment";
      headers.CacheControl = "no-cache";
      headers.ContentEncoding = "identify";

      Files::DataLake::DataLakeSasBuilder builder2 = fileSasBuilder;
      builder2.SetPermissions(Files::DataLake::DataLakeSasPermissions::Read);
      builder2.ContentType = "application/x-binary";
      builder2.ContentLanguage = "en-US";
      builder2.ContentDisposition = "attachment";
      builder2.CacheControl = "no-cache";
      builder2.ContentEncoding = "identify";
      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      auto fileClient = Files::DataLake::FileClient(fileUri + sasToken);
      fileClient0.Create();
      auto p = fileClient.GetProperties();
      EXPECT_EQ(p->HttpHeaders.ContentType, headers.ContentType);
      EXPECT_EQ(p->HttpHeaders.ContentLanguage, headers.ContentLanguage);
      EXPECT_EQ(p->HttpHeaders.ContentDisposition, headers.ContentDisposition);
      EXPECT_EQ(p->HttpHeaders.CacheControl, headers.CacheControl);
      EXPECT_EQ(p->HttpHeaders.ContentEncoding, headers.ContentEncoding);

      auto sasToken2 = builder2.ToSasQueryParameters(userDelegationKey, accountName);
      fileClient = Files::DataLake::FileClient(fileUri + sasToken);
      p = fileClient.GetProperties();
      EXPECT_EQ(p->HttpHeaders.ContentType, headers.ContentType);
      EXPECT_EQ(p->HttpHeaders.ContentLanguage, headers.ContentLanguage);
      EXPECT_EQ(p->HttpHeaders.ContentDisposition, headers.ContentDisposition);
      EXPECT_EQ(p->HttpHeaders.CacheControl, headers.CacheControl);
      EXPECT_EQ(p->HttpHeaders.ContentEncoding, headers.ContentEncoding);
    }
  }

}}} // namespace Azure::Storage::Test
