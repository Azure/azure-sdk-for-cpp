// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "share_client_test.hpp"

#include <azure/storage/files/shares/share_sas_builder.hpp>

#include <chrono>

namespace Azure { namespace Storage { namespace Test {

  class ShareSasTest : public FileShareClientTest {
  public:
    template <class T>
    T GetSasAuthenticatedClient(const T& shareClient, const std::string& sasToken)
    {
      T fileClient1(
          AppendQueryParameters(Azure::Core::Url(shareClient.GetUrl()), sasToken),
          InitStorageClientOptions<Files::Shares::ShareClientOptions>());
      return fileClient1;
    }
    void VerifyShareSasRead(
        const Files::Shares::ShareFileClient& fileClient,
        const std::string& sasToken)
    {
      auto fileClient1 = GetSasAuthenticatedClient(fileClient, sasToken);
      EXPECT_NO_THROW(fileClient1.GetProperties());
    }

    void VerifyShareSasNonRead(
        const Files::Shares::ShareFileClient& fileClient,
        const std::string& sasToken)
    {
      auto fileClient1 = GetSasAuthenticatedClient(fileClient, sasToken);
      EXPECT_THROW(fileClient1.GetProperties(), StorageException);
    }

    void VerifyShareSasWrite(
        const Files::Shares::ShareFileClient& fileClient,
        const std::string& sasToken)
    {
      auto fileClient1 = GetSasAuthenticatedClient(fileClient, sasToken);
      EXPECT_NO_THROW(fileClient1.UploadFrom(reinterpret_cast<const uint8_t*>("a"), 1));
    }

    void VerifyShareSasDelete(
        const Files::Shares::ShareFileClient& fileClient,
        const std::string& sasToken)
    {
      auto fileClient1 = GetSasAuthenticatedClient(fileClient, sasToken);
      EXPECT_NO_THROW(fileClient1.Delete());
      fileClient.UploadFrom(reinterpret_cast<const uint8_t*>("a"), 1);
    }

    void VerifyShareSasCreate(
        const Files::Shares::ShareFileClient& fileClient,
        const std::string& sasToken)
    {
      fileClient.DeleteIfExists();
      auto fileClient1 = GetSasAuthenticatedClient(fileClient, sasToken);
      EXPECT_NO_THROW(fileClient1.Create(1));
    }

    void VerifyShareSasList(
        const Files::Shares::ShareDirectoryClient& directoryClient,
        const std::string& sasToken)
    {
      auto directoryClient1 = GetSasAuthenticatedClient(directoryClient, sasToken);
      EXPECT_NO_THROW(directoryClient1.ListFilesAndDirectories());
    }
  };

  TEST_F(ShareSasTest, AccountSasPermissions)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.StartsOn = sasStartsOn;
    accountSasBuilder.ExpiresOn = sasExpiresOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Files;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    std::string directoryName = RandomString();
    std::string fileName = RandomString();

    auto shareClient = *m_shareClient;
    auto directoryClient
        = shareClient.GetRootDirectoryClient().GetSubdirectoryClient(directoryName);
    directoryClient.Create();
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);
    fileClient.Create(1);

    auto allPermissions = Sas::AccountSasPermissions::Read | Sas::AccountSasPermissions::Write
        | Sas::AccountSasPermissions::Delete | Sas::AccountSasPermissions::List
        | Sas::AccountSasPermissions::Add | Sas::AccountSasPermissions::Create;

    for (auto permissions : {
             allPermissions,
             Sas::AccountSasPermissions::Read,
             Sas::AccountSasPermissions::Write,
             Sas::AccountSasPermissions::Delete,
             Sas::AccountSasPermissions::List,
             Sas::AccountSasPermissions::Create,
         })
    {
      accountSasBuilder.SetPermissions(permissions);
      auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);

      if ((permissions & Sas::AccountSasPermissions::Read) == Sas::AccountSasPermissions::Read)
      {
        VerifyShareSasRead(fileClient, sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Write) == Sas::AccountSasPermissions::Write)
      {
        VerifyShareSasWrite(fileClient, sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Delete) == Sas::AccountSasPermissions::Delete)
      {
        VerifyShareSasDelete(fileClient, sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::List) == Sas::AccountSasPermissions::List)
      {
        VerifyShareSasList(directoryClient, sasToken);
      }
      if ((permissions & Sas::AccountSasPermissions::Create) == Sas::AccountSasPermissions::Create)
      {
        VerifyShareSasCreate(fileClient, sasToken);
      }
    }
  }

  TEST_F(ShareSasTest, ShareServiceSasPermissions)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    std::string fileName = RandomString();
    std::string directoryName = RandomString();

    Sas::ShareSasBuilder shareSasBuilder;
    shareSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    shareSasBuilder.StartsOn = sasStartsOn;
    shareSasBuilder.ExpiresOn = sasExpiresOn;
    shareSasBuilder.ShareName = m_shareName;
    shareSasBuilder.Resource = Sas::ShareSasResource::Share;

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    auto shareClient = *m_shareClient;
    auto directoryClient
        = shareClient.GetRootDirectoryClient().GetSubdirectoryClient(directoryName);
    directoryClient.Create();
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);
    fileClient.Create(1);

    for (auto permissions :
         {Sas::ShareSasPermissions::All,
          Sas::ShareSasPermissions::Read,
          Sas::ShareSasPermissions::Write,
          Sas::ShareSasPermissions::Delete,
          Sas::ShareSasPermissions::List,
          Sas::ShareSasPermissions::Create})
    {
      shareSasBuilder.SetPermissions(permissions);
      auto sasToken = shareSasBuilder.GenerateSasToken(*keyCredential);

      if ((permissions & Sas::ShareSasPermissions::Read) == Sas::ShareSasPermissions::Read)
      {
        VerifyShareSasRead(fileClient, sasToken);
      }
      if ((permissions & Sas::ShareSasPermissions::Write) == Sas::ShareSasPermissions::Write)
      {
        VerifyShareSasWrite(fileClient, sasToken);
      }
      if ((permissions & Sas::ShareSasPermissions::Delete) == Sas::ShareSasPermissions::Delete)
      {
        VerifyShareSasDelete(fileClient, sasToken);
      }
      if ((permissions & Sas::ShareSasPermissions::List) == Sas::ShareSasPermissions::List)
      {
        VerifyShareSasList(directoryClient, sasToken);
      }
      if ((permissions & Sas::ShareSasPermissions::Create) == Sas::ShareSasPermissions::Create)
      {
        VerifyShareSasCreate(fileClient, sasToken);
      }
    }
  }

  TEST_F(ShareSasTest, FileServiceSasPermissions)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    std::string fileName = RandomString();

    Sas::ShareSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSasBuilder.StartsOn = sasStartsOn;
    fileSasBuilder.ExpiresOn = sasExpiresOn;
    fileSasBuilder.ShareName = m_shareName;
    fileSasBuilder.FilePath = fileName;
    fileSasBuilder.Resource = Sas::ShareSasResource::File;

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    auto shareClient = *m_shareClient;
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);
    fileClient.Create(1);

    for (auto permissions :
         {Sas::ShareFileSasPermissions::All,
          Sas::ShareFileSasPermissions::Read,
          Sas::ShareFileSasPermissions::Write,
          Sas::ShareFileSasPermissions::Delete,
          Sas::ShareFileSasPermissions::Create})
    {
      fileSasBuilder.SetPermissions(permissions);
      auto sasToken = fileSasBuilder.GenerateSasToken(*keyCredential);

      if ((permissions & Sas::ShareFileSasPermissions::Read) == Sas::ShareFileSasPermissions::Read)
      {
        VerifyShareSasRead(fileClient, sasToken);
      }
      if ((permissions & Sas::ShareFileSasPermissions::Write)
          == Sas::ShareFileSasPermissions::Write)
      {
        VerifyShareSasWrite(fileClient, sasToken);
      }
      if ((permissions & Sas::ShareFileSasPermissions::Delete)
          == Sas::ShareFileSasPermissions::Delete)
      {
        VerifyShareSasDelete(fileClient, sasToken);
      }
      if ((permissions & Sas::ShareFileSasPermissions::Create)
          == Sas::ShareFileSasPermissions::Create)
      {
        VerifyShareSasCreate(fileClient, sasToken);
      }
    }
  }

  TEST_F(ShareSasTest, AccountSasExpired)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiredOn = std::chrono::system_clock::now() - std::chrono::minutes(1);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    std::string fileName = RandomString();

    auto shareClient = *m_shareClient;
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);
    fileClient.Create(1);

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.StartsOn = sasStartsOn;
    accountSasBuilder.ExpiresOn = sasExpiredOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Files;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;
    accountSasBuilder.SetPermissions(Sas::AccountSasPermissions::All);

    auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    VerifyShareSasNonRead(fileClient, sasToken);

    accountSasBuilder.ExpiresOn = sasExpiresOn;
    sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    VerifyShareSasRead(fileClient, sasToken);
  }

  TEST_F(ShareSasTest, ServiceSasExpired)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiredOn = std::chrono::system_clock::now() - std::chrono::minutes(1);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    std::string fileName = RandomString();

    auto shareClient = *m_shareClient;
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);
    fileClient.Create(1);

    Sas::ShareSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSasBuilder.StartsOn = sasStartsOn;
    fileSasBuilder.ExpiresOn = sasExpiredOn;
    fileSasBuilder.ShareName = m_shareName;
    fileSasBuilder.FilePath = fileName;
    fileSasBuilder.Resource = Sas::ShareSasResource::File;
    fileSasBuilder.SetPermissions(Sas::ShareFileSasPermissions::Read);

    auto sasToken = fileSasBuilder.GenerateSasToken(*keyCredential);
    VerifyShareSasNonRead(fileClient, sasToken);

    fileSasBuilder.ExpiresOn = sasExpiresOn;
    sasToken = fileSasBuilder.GenerateSasToken(*keyCredential);
    VerifyShareSasRead(fileClient, sasToken);
  }

  TEST_F(ShareSasTest, AccountSasWithoutStarttime)
  {

    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    std::string fileName = RandomString();

    auto shareClient = *m_shareClient;
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);
    fileClient.Create(1);

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.ExpiresOn = sasExpiresOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Files;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;
    accountSasBuilder.SetPermissions(Sas::AccountSasPermissions::All);

    auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    VerifyShareSasRead(fileClient, sasToken);
  }

  TEST_F(ShareSasTest, ServiceSasWithoutStarttime)
  {

    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    std::string fileName = RandomString();

    auto shareClient = *m_shareClient;
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);
    fileClient.Create(1);

    Sas::ShareSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSasBuilder.ExpiresOn = sasExpiresOn;
    fileSasBuilder.ShareName = m_shareName;
    fileSasBuilder.FilePath = fileName;
    fileSasBuilder.Resource = Sas::ShareSasResource::File;
    fileSasBuilder.SetPermissions(Sas::ShareFileSasPermissions::Read);

    auto sasToken = fileSasBuilder.GenerateSasToken(*keyCredential);
    VerifyShareSasRead(fileClient, sasToken);
  }

  TEST_F(ShareSasTest, AccountSasWithIP)
  {

    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    std::string fileName = RandomString();

    auto shareClient = *m_shareClient;
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);
    fileClient.Create(1);

    Sas::AccountSasBuilder accountSasBuilder;
    accountSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    accountSasBuilder.ExpiresOn = sasExpiresOn;
    accountSasBuilder.Services = Sas::AccountSasServices::Files;
    accountSasBuilder.ResourceTypes = Sas::AccountSasResource::All;
    accountSasBuilder.SetPermissions(Sas::AccountSasPermissions::All);

    auto sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    VerifyShareSasRead(fileClient, sasToken);

    accountSasBuilder.IPRange = "0.0.0.0-0.0.0.1";
    sasToken = accountSasBuilder.GenerateSasToken(*keyCredential);
    VerifyShareSasNonRead(fileClient, sasToken);
  }

  TEST_F(ShareSasTest, ServiceSasWithIP)
  {

    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    std::string fileName = RandomString();

    auto shareClient = *m_shareClient;
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);
    fileClient.Create(1);

    Sas::ShareSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSasBuilder.ExpiresOn = sasExpiresOn;
    fileSasBuilder.ShareName = m_shareName;
    fileSasBuilder.FilePath = fileName;
    fileSasBuilder.Resource = Sas::ShareSasResource::File;
    fileSasBuilder.SetPermissions(Sas::ShareFileSasPermissions::Read);

    auto sasToken = fileSasBuilder.GenerateSasToken(*keyCredential);
    VerifyShareSasRead(fileClient, sasToken);

    fileSasBuilder.IPRange = "0.0.0.0-0.0.0.1";
    sasToken = fileSasBuilder.GenerateSasToken(*keyCredential);
    VerifyShareSasNonRead(fileClient, sasToken);
  }

  TEST_F(ShareSasTest, SasWithIdentifier)
  {
    auto sasStartsOn = std::chrono::system_clock::now() - std::chrono::minutes(5);
    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    std::string fileName = RandomString();

    auto shareClient = *m_shareClient;
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);
    fileClient.Create(1);

    Files::Shares::Models::SignedIdentifier identifier;
    identifier.Id = RandomString(64);
    identifier.Policy.StartsOn = sasStartsOn;
    identifier.Policy.ExpiresOn = sasExpiresOn;
    identifier.Policy.Permission = "r";
    shareClient.SetAccessPolicy({identifier});

    Sas::ShareSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSasBuilder.ExpiresOn = sasExpiresOn;
    fileSasBuilder.ShareName = m_shareName;
    fileSasBuilder.FilePath = fileName;
    fileSasBuilder.Resource = Sas::ShareSasResource::File;
    fileSasBuilder.SetPermissions(static_cast<Sas::ShareSasPermissions>(0));
    fileSasBuilder.Identifier = identifier.Id;

    TestSleep(std::chrono::seconds(30));

    auto sasToken = fileSasBuilder.GenerateSasToken(*keyCredential);

    VerifyShareSasRead(fileClient, sasToken);
  }

  TEST_F(ShareSasTest, FileSasResponseHeadersOverride)
  {

    auto sasExpiresOn = std::chrono::system_clock::now() + std::chrono::minutes(60);

    auto keyCredential
        = _internal::ParseConnectionString(StandardStorageConnectionString()).KeyCredential;

    std::string fileName = RandomString();

    auto shareClient = *m_shareClient;
    auto fileClient = shareClient.GetRootDirectoryClient().GetFileClient(fileName);
    fileClient.Create(1);

    Sas::ShareSasBuilder fileSasBuilder;
    fileSasBuilder.Protocol = Sas::SasProtocol::HttpsAndHttp;
    fileSasBuilder.ExpiresOn = sasExpiresOn;
    fileSasBuilder.ShareName = m_shareName;
    fileSasBuilder.FilePath = fileName;
    fileSasBuilder.Resource = Sas::ShareSasResource::File;
    fileSasBuilder.SetPermissions(Sas::ShareFileSasPermissions::All);
    fileSasBuilder.ContentType = "application/x-binary";
    fileSasBuilder.ContentLanguage = "en-US";
    fileSasBuilder.ContentDisposition = "attachment";
    fileSasBuilder.CacheControl = "no-cache";
    fileSasBuilder.ContentEncoding = "identify";
    auto sasToken = fileSasBuilder.GenerateSasToken(*keyCredential);

    auto fileClient1 = GetSasAuthenticatedClient(fileClient, sasToken);
    auto properties = fileClient1.GetProperties();
    EXPECT_EQ(properties.Value.HttpHeaders.ContentType, fileSasBuilder.ContentType);
    EXPECT_EQ(properties.Value.HttpHeaders.ContentLanguage, fileSasBuilder.ContentLanguage);
    EXPECT_EQ(properties.Value.HttpHeaders.ContentDisposition, fileSasBuilder.ContentDisposition);
    EXPECT_EQ(properties.Value.HttpHeaders.CacheControl, fileSasBuilder.CacheControl);
    EXPECT_EQ(properties.Value.HttpHeaders.ContentEncoding, fileSasBuilder.ContentEncoding);
  }
}}} // namespace Azure::Storage::Test
