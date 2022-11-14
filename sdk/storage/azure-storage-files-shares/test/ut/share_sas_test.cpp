//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/storage/files/shares/share_sas_builder.hpp>

#include "share_client_test.hpp"

#include <chrono>

namespace Azure { namespace Storage { namespace Test {

  TEST_F(FileShareClientTest, FileSasTest_LIVEONLY_)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiredOn = std::chrono::system_clock::now() - std::chrono::minutes(1);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    std::string fileName = RandomString();
    Sas::ShareSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSasBuilder.StartsOn = sasStartsOn;
    fileSasBuilder.ExpiresOn = sasExpiresOn;
    fileSasBuilder.ShareName = m_shareName;
    fileSasBuilder.FilePath = fileName;
    fileSasBuilder.Resource = Sas::ShareSasResource::File;

    Sas::ShareSasBuilder shareSasBuilder = fileSasBuilder;
    shareSasBuilder.FilePath.clear();
    shareSasBuilder.Resource = Sas::ShareSasResource::Share;

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;
    auto accountName = keyCredential->AccountName;
    auto fileServiceClient0 = Files::Shares::ShareServiceClient::CreateFromConnectionString(
        StandardStorageConnectionString());
    auto shareClient0 = fileServiceClient0.GetShareClient(m_shareName);
    auto fileClient0 = shareClient0.GetRootDirectoryClient().GetFileClient(fileName);

    std::string shareUrl = shareClient0.GetUrl();
    std::string fileUrl = fileClient0.GetUrl();

    auto verifyFileRead = [&](const std::string& sas) {
      int64_t fileSize = 512;
      fileClient0.Create(fileSize);
      auto fileClient = Files::Shares::ShareFileClient(fileUrl + sas);
      auto downloadedContent = fileClient.Download();
      EXPECT_EQ(
          ReadBodyStream(downloadedContent.Value.BodyStream).size(), static_cast<size_t>(fileSize));
    };

    auto verifyFileCreate = [&](const std::string& sas) {
      int64_t fileSize = 512;
      auto fileClient = Files::Shares::ShareFileClient(fileUrl + sas);
      EXPECT_NO_THROW(fileClient.Create(fileSize));
    };

    auto verifyFileWrite = [&](const std::string& sas) {
      int64_t fileSize = 512;
      fileClient0.Create(fileSize);
      auto fileClient = Files::Shares::ShareFileClient(fileUrl + sas);
      std::string fileContent = "a";
      EXPECT_NO_THROW(fileClient.UploadFrom(
          reinterpret_cast<const uint8_t*>(fileContent.data()), fileContent.size()));
    };

    auto verifyFileDelete = [&](const std::string& sas) {
      int64_t fileSize = 512;
      fileClient0.Create(fileSize);
      auto fileClient = Files::Shares::ShareFileClient(fileUrl + sas);
      EXPECT_NO_THROW(fileClient.Delete());
    };

    auto verifyFileList = [&](const std::string& sas) {
      auto shareClient = Files::Shares::ShareClient(shareUrl + sas);
      EXPECT_NO_THROW(shareClient.GetRootDirectoryClient().ListFilesAndDirectories());
    };

    for (auto permissions :
         {Sas::ShareSasPermissions::Read,
          Sas::ShareSasPermissions::Write,
          Sas::ShareSasPermissions::Delete,
          Sas::ShareSasPermissions::List,
          Sas::ShareSasPermissions::Create,
          Sas::ShareSasPermissions::All})
    {
      shareSasBuilder.SetPermissions(permissions);
      auto sasToken = shareSasBuilder.GenerateSasToken(*keyCredential);

      if ((permissions & Sas::ShareSasPermissions::Read) == Sas::ShareSasPermissions::Read)
      {
        verifyFileRead(sasToken);
      }
      if ((permissions & Sas::ShareSasPermissions::Write) == Sas::ShareSasPermissions::Write)
      {
        verifyFileWrite(sasToken);
      }
      if ((permissions & Sas::ShareSasPermissions::Delete) == Sas::ShareSasPermissions::Delete)
      {
        verifyFileDelete(sasToken);
      }
      if ((permissions & Sas::ShareSasPermissions::List) == Sas::ShareSasPermissions::List)
      {
        verifyFileList(sasToken);
      }
      if ((permissions & Sas::ShareSasPermissions::Create) == Sas::ShareSasPermissions::Create)
      {
        verifyFileCreate(sasToken);
      }
    }

    for (auto permissions :
         {Sas::ShareFileSasPermissions::Read,
          Sas::ShareFileSasPermissions::Write,
          Sas::ShareFileSasPermissions::Delete,
          Sas::ShareFileSasPermissions::Create})
    {
      fileSasBuilder.SetPermissions(permissions);
      auto sasToken = fileSasBuilder.GenerateSasToken(*keyCredential);

      if ((permissions & Sas::ShareFileSasPermissions::Read) == Sas::ShareFileSasPermissions::Read)
      {
        verifyFileRead(sasToken);
      }
      if ((permissions & Sas::ShareFileSasPermissions::Write)
          == Sas::ShareFileSasPermissions::Write)
      {
        verifyFileWrite(sasToken);
      }
      if ((permissions & Sas::ShareFileSasPermissions::Delete)
          == Sas::ShareFileSasPermissions::Delete)
      {
        verifyFileDelete(sasToken);
      }
      if ((permissions & Sas::ShareFileSasPermissions::Create)
          == Sas::ShareFileSasPermissions::Create)
      {
        verifyFileCreate(sasToken);
      }
    }

    fileSasBuilder.SetPermissions(Sas::ShareFileSasPermissions::All);
    // Expires
    {
      Sas::ShareSasBuilder builder2 = fileSasBuilder;
      builder2.StartsOn = sasStartsOn;
      builder2.ExpiresOn = sasExpiredOn;
      auto sasToken = builder2.GenerateSasToken(*keyCredential);
      EXPECT_THROW(verifyFileRead(sasToken), StorageException);
    }

    // Without start time
    {
      Sas::ShareSasBuilder builder2 = fileSasBuilder;
      builder2.StartsOn.Reset();
      auto sasToken = builder2.GenerateSasToken(*keyCredential);
      EXPECT_NO_THROW(verifyFileRead(sasToken));
    }

    // IP
    {
      Sas::ShareSasBuilder builder2 = fileSasBuilder;
      builder2.IPRange = "0.0.0.0-0.0.0.1";
      auto sasToken = builder2.GenerateSasToken(*keyCredential);
      EXPECT_THROW(verifyFileRead(sasToken), StorageException);

      // TODO: Add this test case back with support to contain IPv6 ranges when service is ready.
      // builder2.IPRange = "0.0.0.0-255.255.255.255";
      // sasToken = builder2.GenerateSasToken(*keyCredential);
      // EXPECT_NO_THROW(verifyFileRead(sasToken));
    }

    // Identifier
    {
      Files::Shares::Models::SignedIdentifier identifier;
      identifier.Id = RandomString(64);
      identifier.Policy.StartsOn = sasStartsOn;
      identifier.Policy.ExpiresOn = sasExpiresOn;
      identifier.Policy.Permission = "r";
      m_shareClient->SetAccessPolicy({identifier});

      Sas::ShareSasBuilder builder2 = fileSasBuilder;
      builder2.StartsOn.Reset();
      builder2.ExpiresOn = Azure::DateTime();
      builder2.SetPermissions(static_cast<Sas::ShareSasPermissions>(0));
      builder2.Identifier = identifier.Id;

      auto sasToken = builder2.GenerateSasToken(*keyCredential);
      // TODO: looks like a server bug, the identifier doesn't work sometimes.
      // EXPECT_NO_THROW(verifyFileRead(sasToken));
    }

    // response headers override
    {
      Files::Shares::Models::FileHttpHeaders headers;
      headers.ContentType = "application/x-binary";
      headers.ContentLanguage = "en-US";
      headers.ContentDisposition = "attachment";
      headers.CacheControl = "no-cache";
      headers.ContentEncoding = "identify";

      Sas::ShareSasBuilder builder2 = fileSasBuilder;
      builder2.ContentType = "application/x-binary";
      builder2.ContentLanguage = "en-US";
      builder2.ContentDisposition = "attachment";
      builder2.CacheControl = "no-cache";
      builder2.ContentEncoding = "identify";
      auto sasToken = builder2.GenerateSasToken(*keyCredential);
      auto fileClient = Files::Shares::ShareFileClient(fileUrl + sasToken);
      fileClient0.Create(0);
      auto p = fileClient.GetProperties();
      EXPECT_EQ(p.Value.HttpHeaders.ContentType, headers.ContentType);
      EXPECT_EQ(p.Value.HttpHeaders.ContentLanguage, headers.ContentLanguage);
      EXPECT_EQ(p.Value.HttpHeaders.ContentDisposition, headers.ContentDisposition);
      EXPECT_EQ(p.Value.HttpHeaders.CacheControl, headers.CacheControl);
      EXPECT_EQ(p.Value.HttpHeaders.ContentEncoding, headers.ContentEncoding);
    }
  }

}}} // namespace Azure::Storage::Test
