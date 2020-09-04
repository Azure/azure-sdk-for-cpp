// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "share_client_test.hpp"

#include "azure/storage/files/shares/share_sas_builder.hpp"

namespace Azure { namespace Storage { namespace Test {

  TEST_F(FileShareClientTest, FileSasTest)
  {
    std::string fileName = RandomString();
    Files::Shares::ShareSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = SasProtocol::HttpsAndHtttp;
    fileSasBuilder.StartsOn = ToIso8601(std::chrono::system_clock::now() - std::chrono::minutes(5));
    fileSasBuilder.ExpiresOn
        = ToIso8601(std::chrono::system_clock::now() + std::chrono::minutes(60));
    fileSasBuilder.ShareName = m_shareName;
    fileSasBuilder.FilePath = fileName;
    fileSasBuilder.Resource = Files::Shares::ShareSasResource::File;

    Files::Shares::ShareSasBuilder shareSasBuilder = fileSasBuilder;
    shareSasBuilder.FilePath.clear();
    shareSasBuilder.Resource = Files::Shares::ShareSasResource::Share;

    auto keyCredential
        = Details::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;
    auto accountName = keyCredential->AccountName;
    auto fileServiceClient0 = Files::Shares::ServiceClient::CreateFromConnectionString(
        StandardStorageConnectionString());
    auto shareClient0 = fileServiceClient0.GetShareClient(m_shareName);
    auto fileClient0 = shareClient0.GetFileClient(fileName);

    std::string shareUri = shareClient0.GetUri();
    std::string fileUri = fileClient0.GetUri();

    auto verifyFileRead = [&](const std::string& sas) {
      int64_t fileSize = 512;
      fileClient0.Create(fileSize);
      auto fileClient = Files::Shares::FileClient(fileUri + sas);
      auto downloadedContent = fileClient.Download();
      EXPECT_EQ(
          ReadBodyStream(downloadedContent->BodyStream).size(), static_cast<std::size_t>(fileSize));
    };

    auto verifyFileCreate = [&](const std::string& sas) {
      int64_t fileSize = 512;
      auto fileClient = Files::Shares::FileClient(fileUri + sas);
      EXPECT_NO_THROW(fileClient.Create(fileSize));
    };

    auto verifyFileWrite = [&](const std::string& sas) {
      int64_t fileSize = 512;
      fileClient0.Create(fileSize);
      auto fileClient = Files::Shares::FileClient(fileUri + sas);
      std::string fileContent = "a";
      EXPECT_NO_THROW(fileClient.UploadFrom(
          reinterpret_cast<const uint8_t*>(fileContent.data()), fileContent.size()));
    };

    auto verifyFileDelete = [&](const std::string& sas) {
      int64_t fileSize = 512;
      fileClient0.Create(fileSize);
      auto fileClient = Files::Shares::FileClient(fileUri + sas);
      EXPECT_NO_THROW(fileClient.Delete());
    };

    auto verifyFileList = [&](const std::string& sas) {
      auto shareClient = Files::Shares::ShareClient(shareUri + sas);
      EXPECT_NO_THROW(shareClient.ListFilesAndDirectoriesSegment());
    };

    for (auto permissions :
         {Files::Shares::ShareSasPermissions::Read,
          Files::Shares::ShareSasPermissions::Write,
          Files::Shares::ShareSasPermissions::Delete,
          Files::Shares::ShareSasPermissions::List,
          Files::Shares::ShareSasPermissions::Create,
          Files::Shares::ShareSasPermissions::All})
    {
      shareSasBuilder.SetPermissions(permissions);
      auto sasToken = shareSasBuilder.ToSasQueryParameters(*keyCredential);

      if ((permissions & Files::Shares::ShareSasPermissions::Read)
          == Files::Shares::ShareSasPermissions::Read)
      {
        verifyFileRead(sasToken);
      }
      if ((permissions & Files::Shares::ShareSasPermissions::Write)
          == Files::Shares::ShareSasPermissions::Write)
      {
        verifyFileWrite(sasToken);
      }
      if ((permissions & Files::Shares::ShareSasPermissions::Delete)
          == Files::Shares::ShareSasPermissions::Delete)
      {
        verifyFileDelete(sasToken);
      }
      if ((permissions & Files::Shares::ShareSasPermissions::List)
          == Files::Shares::ShareSasPermissions::List)
      {
        verifyFileList(sasToken);
      }
      if ((permissions & Files::Shares::ShareSasPermissions::Create)
          == Files::Shares::ShareSasPermissions::Create)
      {
        verifyFileCreate(sasToken);
      }
    }

    for (auto permissions :
         {Files::Shares::ShareFileSasPermissions::Read,
          Files::Shares::ShareFileSasPermissions::Write,
          Files::Shares::ShareFileSasPermissions::Delete,
          Files::Shares::ShareFileSasPermissions::Create})
    {
      fileSasBuilder.SetPermissions(permissions);
      auto sasToken = fileSasBuilder.ToSasQueryParameters(*keyCredential);

      if ((permissions & Files::Shares::ShareFileSasPermissions::Read)
          == Files::Shares::ShareFileSasPermissions::Read)
      {
        verifyFileRead(sasToken);
      }
      if ((permissions & Files::Shares::ShareFileSasPermissions::Write)
          == Files::Shares::ShareFileSasPermissions::Write)
      {
        verifyFileWrite(sasToken);
      }
      if ((permissions & Files::Shares::ShareFileSasPermissions::Delete)
          == Files::Shares::ShareFileSasPermissions::Delete)
      {
        verifyFileDelete(sasToken);
      }
      if ((permissions & Files::Shares::ShareFileSasPermissions::Create)
          == Files::Shares::ShareFileSasPermissions::Create)
      {
        verifyFileCreate(sasToken);
      }
    }

    fileSasBuilder.SetPermissions(Files::Shares::ShareFileSasPermissions::All);
    // Expires
    {
      Files::Shares::ShareSasBuilder builder2 = fileSasBuilder;
      builder2.StartsOn = ToIso8601(std::chrono::system_clock::now() - std::chrono::minutes(5));
      builder2.ExpiresOn = ToIso8601(std::chrono::system_clock::now() - std::chrono::minutes(1));
      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      EXPECT_THROW(verifyFileRead(sasToken), StorageError);
    }

    // Without start time
    {
      Files::Shares::ShareSasBuilder builder2 = fileSasBuilder;
      builder2.StartsOn.Reset();
      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      EXPECT_NO_THROW(verifyFileRead(sasToken));
    }

    // IP
    {
      Files::Shares::ShareSasBuilder builder2 = fileSasBuilder;
      builder2.IPRange = "0.0.0.0-0.0.0.1";
      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      EXPECT_THROW(verifyFileRead(sasToken), StorageError);

      builder2.IPRange = "0.0.0.0-255.255.255.255";
      sasToken = builder2.ToSasQueryParameters(*keyCredential);
      EXPECT_NO_THROW(verifyFileRead(sasToken));
    }

    // Identifier
    {
      Files::Shares::SignedIdentifier identifier;
      identifier.Id = RandomString(64);
      identifier.Policy.Start
          = ToIso8601(std::chrono::system_clock::now() - std::chrono::minutes(5));
      identifier.Policy.Expiry
          = ToIso8601(std::chrono::system_clock::now() + std::chrono::minutes(60));
      identifier.Policy.Permission
          = Files::Shares::ShareSasPermissionsToString(Files::Shares::ShareSasPermissions::Read);
      m_shareClient->SetAccessPolicy({identifier});

      Files::Shares::ShareSasBuilder builder2 = fileSasBuilder;
      builder2.StartsOn.Reset();
      builder2.ExpiresOn.clear();
      builder2.SetPermissions(static_cast<Files::Shares::ShareSasPermissions>(0));
      builder2.Identifier = identifier.Id;

      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      EXPECT_NO_THROW(verifyFileRead(sasToken));
    }

    // response headers override
    {
      Files::Shares::FileShareHttpHeaders headers;
      headers.ContentType = "application/x-binary";
      headers.ContentLanguage = "en-US";
      headers.ContentDisposition = "attachment";
      headers.CacheControl = "no-cache";
      headers.ContentEncoding = "identify";

      Files::Shares::ShareSasBuilder builder2 = fileSasBuilder;
      builder2.ContentType = "application/x-binary";
      builder2.ContentLanguage = "en-US";
      builder2.ContentDisposition = "attachment";
      builder2.CacheControl = "no-cache";
      builder2.ContentEncoding = "identify";
      auto sasToken = builder2.ToSasQueryParameters(*keyCredential);
      auto fileClient = Files::Shares::FileClient(fileUri + sasToken);
      fileClient0.Create(0);
      auto p = fileClient.GetProperties();
      EXPECT_EQ(p->HttpHeaders.ContentType, headers.ContentType);
      EXPECT_EQ(p->HttpHeaders.ContentLanguage, headers.ContentLanguage);
      EXPECT_EQ(p->HttpHeaders.ContentDisposition, headers.ContentDisposition);
      EXPECT_EQ(p->HttpHeaders.CacheControl, headers.CacheControl);
      EXPECT_EQ(p->HttpHeaders.ContentEncoding, headers.ContentEncoding);
    }
  }

}}} // namespace Azure::Storage::Test
